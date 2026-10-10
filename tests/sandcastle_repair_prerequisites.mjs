import test from 'node:test';import assert from 'node:assert/strict';
import {readFile}from'node:fs/promises';import{resolve}from'node:path';
import {registerRepairBatch,countIntegratedRepairs,digest,assertContract}from'../.sandcastle/acceptance.mjs';
import {beginRecovery,previewRecovery}from'../.sandcastle/recovery.mjs';
const root=resolve(import.meta.dirname,'..');
const contract=JSON.parse(await readFile(resolve(root,'tests/fixtures/sandcastle-0241e0da-contract.json'),'utf8'));
function fixture(){
 const a=contract.contract??contract;const hash=digest(a),criterion=a.criteria.find(c=>c.id==='P1-ac8');
 const direct={path:'.scratch/sandcastle-test/issues/10.md',blockedBy:[],findingIds:['D-test'],delivery:{owner:'worker',phase:'parallel-work',prerequisites:[]}};
 const producer={path:'.scratch/sandcastle-test/issues/13.md',blockedBy:[],findingIds:[],delivery:{owner:'worker',phase:'parallel-work',prerequisites:[]}};
 const plan={version:1,contractDigest:hash,criteria:a.criteria.map(c=>({id:c.id,evidence:c.applicability==='deferred'?'deferred':c.id===criterion.id?'artifact':'inspection',artifactCheckIds:c.id===criterion.id?['runtime']:[]})),artifactChecks:[{id:'runtime',owner:'controller',phase:'test-gate',target:'integrated-head',command:['python3','tests/collector.py'],outputArgument:true,manifestPath:'manifest.json',syntheticOnly:true,inputPaths:['tests/collector.py'],expectedSuites:{cases:1},producedBy:producer.path}]};
 // Fixture mirrors all mandatory runtime requirements, not only the repaired criterion.
 for(const c of a.criteria){const b=plan.criteria.find(b=>b.id===c.id);if(c.applicability!=='deferred'&&/(?:runtime|controlled[- ]peer|production[- ]path) (?:evidence|capture|trace)|(?:runtime|trace).*fresh final|fresh.*runtime/i.test(c.requirement)){b.evidence='artifact';b.artifactCheckIds=['runtime'];}}
 const review={approved:true,summary:'Independently inspected collector prerequisite and repair scope',missingRequirements:[]};
 const state={id:'test',phase:'plan-validate',verificationVersion:1,completedTickets:[],tickets:[],reviewFindings:[{id:'D-test'}],reviewRound:2,reviewLimit:10,buildAttempts:1,testAttempts:1,budget:{consumedPercent:9,limitPercent:25},acceptance:{contract:a,contractDigest:hash,ledger:[{id:'D-test',status:'open',repairs:0,criterionIds:[criterion.id]}],repairBatches:[]},executionPlan:{plan,digest:digest(plan),review}};
 return{state,tickets:[direct,producer],direct,producer};
}
test('independently approved collector prerequisite registers without fake finding IDs and gates repair accounting',()=>{
 const{state,tickets,direct,producer}=fixture();const before=structuredClone(state.acceptance);
 registerRepairBatch(state,tickets,'batch');assert.deepEqual(state.acceptance.repairBatches[0].prerequisites,[{ticketPath:producer.path,artifactCheckIds:['runtime'],criterionIds:['P1-ac8'],findingIds:['D-test']}]);
 state.completedTickets=[direct.path];countIntegratedRepairs(state);assert.equal(state.acceptance.ledger[0].repairs,0);
 state.deliveredTickets=[producer.path];countIntegratedRepairs(state);countIntegratedRepairs(state);assert.equal(state.acceptance.ledger[0].repairs,1);assertContract(state);assert.deepEqual(state.acceptance.contract,before.contract);
});
test('unlinked producer, unknown finding, unapproved scope, cycles and reused delivery remain rejected',()=>{
 for(const change of [f=>f.state.executionPlan.plan.artifactChecks[0].producedBy='unknown',f=>f.direct.findingIds=['unknown'],f=>f.state.executionPlan.review.approved=false,f=>f.state.acceptance.ledger[0].criterionIds=['P2-8'],f=>f.producer.blockedBy=[f.producer.path],f=>f.state.deliveredTickets=[f.producer.path]]){
  const f=fixture();change(f);f.state.executionPlan.digest=digest(f.state.executionPlan.plan);assert.throws(()=>registerRepairBatch(f.state,f.tickets,'batch'));
 }
});
test('linked prerequisite cannot be deferred through generic worker recovery',()=>{
 const{state,tickets,producer}=fixture();registerRepairBatch(state,tickets,'batch');state.tickets=tickets;state.phase='parallel-work';state.wave={id:1,after:'tickets',members:[{ticket:producer,status:'blocked'}]};beginRecovery(state,'Missing collector dependency');assert.throws(()=>previewRecovery(state,{action:'defer',summary:'Defer',ticketPaths:[producer.path],dependency:'docs/tasks/owner.md'}),/cannot be deferred/);
});
test('typed registration errors repair the planner without weakening contract or defect limits',async()=>{
 const{classifyFailure}=await import('../.sandcastle/failure.mjs');const{guidanceForAgent}=await import('../.sandcastle/recovery-guidance.mjs');
 const f=fixture();f.direct.findingIds=['unknown'];let error;try{registerRepairBatch(f.state,f.tickets,'batch')}catch(e){error=e;}
 assert.equal(error.code,'REPAIR_PLAN_INVALID');assert.equal(classifyFailure(error,{phase:'plan-validate'}).code,'repair_plan');beginRecovery(f.state,error);
 assert.throws(()=>previewRecovery(f.state,{action:'retry',summary:'unchanged',ticketPaths:[],dependency:''}),/requires repair/);
 const next=previewRecovery(f.state,{action:'repair',summary:'Correct exact mapping',ticketPaths:[],dependency:''}).next;assert.equal(next.phase,'plan');assert.match(guidanceForAgent(next,'ticket planning'),/open ledger IDs/);
 const exhausted=fixture();exhausted.state.acceptance.ledger[0].repairs=3;assert.throws(()=>registerRepairBatch(exhausted.state,exhausted.tickets,'batch'),/human decision required/);
});
test('supported historical continuation repeats validation and preserves contract, limits and history',async()=>{
 const{resumeRepairPlanCheckpoint}=await import('../.sandcastle/repair-plan.mjs');const f=fixture();const s=f.state;s.phase='awaiting';s.resumePhase='plan-validate';s.pauseReason='Acceptance: repair ticket must name open ledger IDs';s.baseCommit='a'.repeat(40);s.planReviewHistory=[{head:'b'.repeat(40),bindingDigest:s.executionPlan.digest,contractDigest:s.acceptance.contractDigest,review:s.executionPlan.review}];const before=structuredClone(s);
 resumeRepairPlanCheckpoint(s);assert.equal(s.phase,'plan-validate');for(const k of ['acceptance','budget','buildAttempts','testAttempts','reviewRound','baseCommit','planReviewHistory'])assert.deepEqual(s[k],before[k]);assert.equal(s.acceptance.repairBatches.length,0);
 for(const change of [x=>x.pauseReason='Acceptance: defect repair limit reached',x=>x.wave={},x=>x.verificationTarget={},x=>x.budget.consumedPercent=100,x=>x.planReviewHistory[0].review.approved=false,x=>x.executionPlan.digest='0'.repeat(64)]){const bad=structuredClone(before);change(bad);const untouched=structuredClone(bad);assert.throws(()=>resumeRepairPlanCheckpoint(bad));assert.deepEqual(bad,untouched);}
});
