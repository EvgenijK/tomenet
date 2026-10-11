// Reconcile only delivered collector input paths at a stopped host checkpoint.
// This is not a contract amendment, execution grant or acceptance receipt.
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {digest} from './verification.mjs';
import {assertExecutionPlan,acceptExecutionPlan} from './verification-plan.mjs';
import {taskQuotaLimit} from './limits.mjs';
import {controllerExecutionIdentity} from './execution.mjs';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw new Error('Host binding review: '+message);};
const git=async(root,...args)=>(await exec('git',['-C',root,...args],{maxBuffer:32*1024*1024})).stdout.trim();
export async function previewHostBinding(root,state){
 need(state.phase==='awaiting'&&state.currentFailure?.diagnosticCode==='HOST_APPROVAL_REQUIRED'&&!state.verificationTarget,'requires a stopped unfrozen host approval checkpoint');
 need(!state.quotaError&&(state.budget?.consumedPercent??0)<taskQuotaLimit(state),'cannot grant quota');
 assertExecutionPlan(state);
 need(digest(state.acceptance.contract)===state.acceptance.contractDigest,'immutable contract identity differs');
 const heads={};for(const member of state.wave?.members??[]){
  if(!member.integrated)heads[member.branch]=await git(root,'rev-parse',`refs/heads/${member.branch}`);
 }
 return {version:1,runId:state.id,stateDigest:digest(state),contractDigest:state.acceptance.contractDigest,
  bindingDigest:state.executionPlan.digest,controllerDigest:(await controllerExecutionIdentity(root)).digest,heads};
}
export async function applyReviewedHostBinding(root,state,{authorizationId,review}){
 need(/^[a-zA-Z0-9_-]{8,100}$/.test(authorizationId??''),'unique scoped authorization required');
 const preview=await previewHostBinding(root,state);
 need(review?.version===1&&review.approved===true&&review.reviewer==='controller-independent-review','independent review required');
 for(const key of ['runId','stateDigest','contractDigest','bindingDigest','controllerDigest'])need(review[key]===preview[key],key+' changed since review');
 need(preview.heads[review.branch]===review.head&&/^[a-f0-9]{40}$/.test(review.head??''),'reviewed retained worker changed');
 need(!(state.hostBindingReviews??[]).some(record=>record.authorizationId===authorizationId),'authorization already applied');
 const original=structuredClone(state.executionPlan.plan),proposed=structuredClone(review.plan);
 need(proposed&&Array.isArray(proposed.artifactChecks)&&proposed.artifactChecks.length===original.artifactChecks.length,'same artifact checks required');
 // Every structural field besides inputPaths must remain byte-identical.
 const stripped=plan=>({...plan,artifactChecks:plan.artifactChecks.map(({inputPaths,...rest})=>rest)});
 need(digest(stripped(proposed))===digest(stripped(original)),'only inputPaths may change; criteria/commands/suite counts cannot change');
 need(proposed.artifactChecks.every(check=>Array.isArray(check.inputPaths)&&check.inputPaths.length===new Set(check.inputPaths).size),'duplicate collector input paths');
 const paths=[...new Set(proposed.artifactChecks.flatMap(check=>check.inputPaths??[]))].sort();
 need(paths.length&&JSON.stringify(paths)===JSON.stringify(Object.keys(review.inputs??{}).sort()),'complete reviewed path/hash union required');
 for(const path of paths){
  need(typeof path==='string'&&/^[a-zA-Z0-9_.\/-]+$/.test(path)&&!path.startsWith('/')&&path.split('/').every(part=>part&&part!=='.'&&part!=='..'),'unsafe source path');
  need(/^(100644|100755) blob /.test(await git(root,'ls-tree',review.head,'--',path)),'reviewed input is not a committed regular blob');
  const bytes=(await exec('git',['-C',root,'show',`${review.head}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;
  need(digest(bytes)===review.inputs[path],'reviewed input changed: '+path);
 }
 const next=structuredClone(state);
 acceptExecutionPlan(next,proposed,{approved:true,summary:review.summary,missingRequirements:[]},next.tickets);
 (next.hostBindingReviews??=[]).push({authorizationId,at:new Date().toISOString(),head:review.head,branch:review.branch,
  reviewer:review.reviewer,reviewDigest:digest(review),previousBinding:state.executionPlan,
  bindingDigest:next.executionPlan.digest,contractDigest:preview.contractDigest,stateDigest:preview.stateDigest,
  inputCount:paths.length,allowanceGranted:false,finalAcceptance:false});
 // Remain awaiting: actual host policy/source approval and separate preflight
 // continuation are still required. Histories and all attempt guards survive.
 return next;
}
