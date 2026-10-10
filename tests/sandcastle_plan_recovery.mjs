import test from 'node:test';
import assert from 'node:assert/strict';
import { acceptExecutionPlan } from '../.sandcastle/verification-plan.mjs';
import { classifyFailure } from '../.sandcastle/failure.mjs';
import { beginRecovery, previewRecovery } from '../.sandcastle/recovery.mjs';
import { guidanceForAgent } from '../.sandcastle/recovery-guidance.mjs';
import { releaseVerificationTarget } from '../.sandcastle/verification-gates.mjs';
import { verificationError } from '../.sandcastle/verification.mjs';
import { acceptDelivery } from '../.sandcastle/delivery.mjs';
const fixture=()=>({id:'fixture',phase:'plan-validate',completedTickets:[],tickets:[],reviewRound:2,reviewLimit:10,buildAttempts:3,testAttempts:2,budget:{consumedPercent:8,limitPercent:25,tokens:99},acceptance:{contractDigest:'b'.repeat(64),contract:{criteria:[{id:'AC',requirement:'inspect production',mandatory:true,applicability:'current'}]}},baseCommit:'a'.repeat(40)});
const plan=state=>({version:1,contractDigest:state.acceptance.contractDigest,criteria:[{id:'AC',evidence:'inspection',artifactCheckIds:[]}],artifactChecks:[]});
test('independent plan rejection reaches bounded recovery with exact remarks, then planner and fresh review',()=>{
 const state=fixture(), before=structuredClone(state), review={approved:false,summary:'Input inventory incomplete',missingRequirements:['src/common/pack.h','src/common/md5.h','lib/xtra/font resources']};
 let failure;try{acceptExecutionPlan(state,plan(state),review)}catch(error){failure=error;}
 assert.equal(classifyFailure(failure,{phase:state.phase}).fatalPreflight,false);
 assert.deepEqual(failure.planReview.review,review);
 beginRecovery(state,failure);assert.deepEqual(state.recovery.planReview.review,review);
 assert.throws(()=>previewRecovery(state,{action:'retry',summary:'same plan',ticketPaths:[],dependency:''}),/requires repair/);
 const repaired=previewRecovery(state,{action:'repair',summary:'Correct collector inputs through a worker prerequisite',ticketPaths:[],dependency:''}).next;
 assert.equal(repaired.phase,'plan');assert.match(guidanceForAgent(repaired,'ticket planning'),/src\/common\/pack.h/);
 assert.deepEqual(repaired.acceptance,before.acceptance);assert.deepEqual(repaired.budget,before.budget);assert.equal(repaired.baseCommit,before.baseCommit);
 assert.throws(()=>acceptExecutionPlan(repaired,plan(repaired),review),/PLAN_REVIEW/);
 acceptExecutionPlan(repaired,plan(repaired),{approved:true,summary:'Independently reinspected full sources',missingRequirements:[]});
 assert.equal(repaired.executionPlan.review.approved,true);
});
test('integrity and uncontextualized PLAN_REVIEW failures remain fatal',()=>{
 for(const code of ['EVIDENCE_INTEGRITY','PLAN_IDENTITY','PLAN_REVIEW','VERIFICATION_RUNTIME'])assert.equal(classifyFailure(verificationError(code,'invalid'),{phase:'plan-validate'}).fatalPreflight,true);
});
test('replanning archives frozen target and retains old evidence history while requiring a fresh target',()=>{
 const state=fixture();state.verificationTarget={head:'c'.repeat(40)};state.verifiedArtifactReferences=[{bundlePath:'/sealed/old'}];state.artifactEvidence=[{bundlePath:'/sealed/old'}];
 const old=structuredClone(state);releaseVerificationTarget(state);
 assert.deepEqual(state.verificationTargets,[old.verificationTarget]);assert.deepEqual(state.artifactEvidence,old.artifactEvidence);assert.equal(state.verificationTarget,undefined);assert.equal(state.verifiedArtifactReferences,undefined);
});
test('assembly lists only exact recorded delivery receipts and completed members',async()=>{
 const {assemblyContributions}=await import('../.sandcastle/delivery.mjs');
 const state=fixture();state.executionPlan={digest:'c'.repeat(64)};const ticket={path:'.scratch/test/issues/09.md'};state.tickets=[ticket];
 const receipt=acceptDelivery(state,{revision:'d'.repeat(40),ticketPath:ticket.path,ticketDigest:'e'.repeat(64),changeDigest:'f'.repeat(64),review:{approved:true,localPredicatesPassed:true,head:'d'.repeat(40),ticketDigest:'e'.repeat(64),pendingCriterionIds:['AC']}});
 const member={ticket,branch:'worker',head:receipt.revision,status:'blocked',deliveryReceipt:receipt};
 assert.deepEqual(assemblyContributions(state,{members:[member,{...member,deliveryReceipt:undefined}]}),[{ticket:ticket.path,branch:'worker',head:receipt.revision,deliveryReceipt:receipt}]);
 assert.throws(()=>assemblyContributions(state,{members:[{...member,head:'0'.repeat(40)}]}),/DELIVERY_INVALID/);
});
