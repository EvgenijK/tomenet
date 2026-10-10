import { digest, verificationError } from './verification.mjs';
const need=(condition,message)=>{if(!condition)throw verificationError('DELIVERY_INVALID',message);};
export function acceptDelivery(state,{revision,ticketPath,ticketDigest,changeDigest,review}) {
  need(/^[a-f0-9]{40,64}$/.test(revision)&&[ticketDigest,changeDigest].every(v=>/^[a-f0-9]{64}$/.test(v)),'committed delivery identities required');
  need(state.executionPlan&&state.acceptance&&state.tickets.some(t=>t.path===ticketPath),'delivery requires accepted binding and active ticket');
  need(review?.approved===true&&review.localPredicatesPassed===true&&review.head===revision&&review.ticketDigest===ticketDigest&&Array.isArray(review.pendingCriterionIds)&&review.pendingCriterionIds.length>0&&review.pendingCriterionIds.every(id=>state.acceptance.contract.criteria.some(c=>c.id===id&&c.applicability==='current')),'independent exact-revision delivery review required; controller acceptance remains pending');
  const body={version:1,revision,ticketPath,ticketDigest,changeDigest,contractDigest:state.acceptance.contractDigest,bindingDigest:state.executionPlan.digest,review:structuredClone(review),controllerAcceptance:'pending'};
  const receipt={...body,digest:digest(body)};(state.deliveryReceipts??=[]).push(receipt);return receipt;
}
export function isDeliveryReady(state,member) {
  const receipt=member.deliveryReceipt;if(!receipt)return false;
  const {digest:hash,...body}=receipt;
  need(receipt.version===1&&digest(body)===hash&&receipt.revision===member.head&&receipt.ticketPath===member.ticket.path&&receipt.contractDigest===state.acceptance?.contractDigest&&receipt.bindingDigest===state.executionPlan?.digest&&receipt.controllerAcceptance==='pending','receipt identity differs');
  need((state.deliveryReceipts??[]).some(saved=>saved.digest===hash&&digest(saved)===digest(receipt)),'controller receipt is not recorded');
  return true;
}
export function dependencyReadyTickets(state) { return new Set([...(state.completedTickets??[]),...(state.deliveredTickets??[])]); }
