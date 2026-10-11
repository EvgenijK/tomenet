import test from 'node:test';import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,readFile} from 'node:fs/promises';import {tmpdir} from 'node:os';import {resolve} from 'node:path';import {execFileSync} from 'node:child_process';import {createHash} from 'node:crypto';
import {validateHostApprovalResume,applyHostApprovalResume,hostCheckpointFingerprint} from '../.sandcastle/host-approval-resume.mjs';
const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
async function fixture(){
 const root=await mkdtemp(resolve(tmpdir(),'host-approval-'));execFileSync('git',['init','-q',root]);await writeFile(resolve(root,'source'),'approved');execFileSync('git',['-C',root,'add','.']);execFileSync('git',['-C',root,'-c','user.name=Fixture','-c','user.email=fixture@example.invalid','commit','-qm','source']);
 const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim(),run=resolve(root,'.sandcastle/runs/3fd6b311');await mkdir(run,{recursive:true});
 const digest='b'.repeat(64),binding='c'.repeat(64),controller='d'.repeat(64),ticket='ticket07';
 const state={id:'3fd6b311',phase:'awaiting',resumePhase:'parallel-work',pauseReason:'Automatic recovery made no progress after three decisions at this checkpoint',currentFailure:{code:'attempts'},acceptance:{contractDigest:digest},executionPlan:{digest:binding},completedTickets:['01','02','03','04','05','06','10'],reviewRound:0,buildAttempts:1,testAttempts:0,budget:{consumedPercent:13,tokens:99},buildLimit:10,testLimit:10,reviewLimit:10,recoveryAttempts:{'parallel-work:7:7:0:1:0':3},wave:{id:7,baseCommit:head,members:[{head,branch:'master',ticket:{path:ticket},status:'blocked',result:{summary:'Requires controller-owned independent source/tool approval'}}]}};
 const stateBytes=JSON.stringify(state);await writeFile(resolve(run,'state.json'),stateBytes);
 const policyPath=resolve(run,'policy.json'),policy={runId:state.id,contractDigest:digest,actions:{interaction:{sourceWorktree:true,inventory:{roots:[],files:['source']}}},approvals:[{tree:execFileSync('git',['-C',root,'rev-parse',head+'^{tree}']).toString().trim(),base:head,inputs:{source:hash('approved')},actions:['interaction']}],review:{reviewer:'independent',reference:'review.json'}};const policyBytes=JSON.stringify(policy);await writeFile(policyPath,policyBytes,{mode:0o600});
 const reviewFile='review.json',reviewBytes=JSON.stringify({approved:true,reviewer:'independent',head,policyDigest:hash(policyBytes)});await writeFile(resolve(run,reviewFile),reviewBytes);
 const proof={version:1,driver:'x11',rawStateSha256:hash(stateBytes),authorization:'user-requested-host-approval-correction',runId:state.id,stateFingerprint:hostCheckpointFingerprint(state),controllerDigest:controller,contractDigest:digest,bindingDigest:binding,head,previousHead:head,policyDigest:hash(policyBytes),reviewFile,reviewDigest:hash(reviewBytes),receipts:[]};
 for(const [index,renderer]of ['software','opengl'].entries()){
  const nonce=String(index+1).repeat(32),job=resolve(run,'host-jobs',nonce),output=resolve(job,'output');await mkdir(output,{recursive:true});
  const report={kind:'linux-synthetic-interaction-producer',head,contract_digest:digest,nonce,status:'passed',physical_acceptance:false,milestone_acceptance:false,inputs_before:{source:hash('approved')},inputs_after:{source:hash('approved')},runtime_before:{tools:{[resolve(root,'source')]:hash('approved')},libraries:{[resolve(root,'source')]:hash('approved')}},runtime_after:{tools:{[resolve(root,'source')]:hash('approved')},libraries:{[resolve(root,'source')]:hash('approved')}},reports:['existing-character','existing-account-create','new-account-create'].flatMap(route=>['keyboard-text','composition','mouse','resize','minimize','restore'].map(category=>({route,category,executions:[{renderer,status:'passed',sessions:[{order:1,status:'passed',video:'x11',renderer,route,category},{order:0,status:'passed',video:'x11',renderer,route,category}]}]})))};
  const bytes=JSON.stringify(report);await writeFile(resolve(output,'interaction-report.json'),bytes);
  const receipt=JSON.stringify({head,contractDigest:digest,nonce,action:'interaction',exitCode:0,policyDigest:hash(policyBytes),scope:{runId:state.id,wave:7,branch:'master',ticket,baseCommit:head},output,artifacts:{'interaction-report.json':{size:Buffer.byteLength(bytes),sha256:hash(bytes)}}});await writeFile(resolve(job,'receipt.json'),receipt);proof.receipts.push({nonce,sha256:hash(receipt)});
 }return {root,run,state,proof,options:{controllerDigest:controller,policyPath}};
}
test('concrete corrected approval resumes exact checkpoint without granting or resetting allowances',async()=>{
 const {root,state,proof,options}=await fixture(),before=structuredClone(state);
 const record=await validateHostApprovalResume(root,state,proof,options);assert.deepEqual(state,before);assert.throws(()=>record.head='forged',TypeError);
 await applyHostApprovalResume(root,state,record);assert.equal(state.phase,'parallel-work');
 for(const key of ['budget','completedTickets','recoveryAttempts','buildAttempts','testAttempts','reviewRound','buildLimit','testLimit','reviewLimit','wave'])assert.deepEqual(state[key],before[key]);
 assert.deepEqual(JSON.parse(await readFile(state.hostApprovalResumes[0].archive)),before);assert.equal(state.hostApprovalResumes[0].allowanceGranted,false);
});
test('fingerprint, review and changed bytes fail closed before archive or mutation',async()=>{
 const {root,run,state,proof,options}=await fixture(),before=structuredClone(state);
 await assert.rejects(validateHostApprovalResume(root,state,{...proof,stateFingerprint:'0'.repeat(64)},options),/fingerprint/);
 await assert.rejects(validateHostApprovalResume(root,state,{...proof,reviewDigest:'0'.repeat(64)},options),/review required/);
 await assert.rejects(applyHostApprovalResume(root,state,{stateFingerprint:proof.stateFingerprint}),/validated/);
 await writeFile(resolve(run,'host-jobs',proof.receipts[0].nonce,'output/interaction-report.json'),'changed');await assert.rejects(validateHostApprovalResume(root,state,proof,options),/artifact changed/);assert.deepEqual(state,before);
});

async function refreshState(f){const bytes=JSON.stringify(f.state);await writeFile(resolve(f.run,'state.json'),bytes);f.proof.rawStateSha256=hash(bytes);f.proof.stateFingerprint=hostCheckpointFingerprint(f.state);}
async function changeReport(f,index,change){const job=resolve(f.run,'host-jobs',f.proof.receipts[index].nonce),path=resolve(job,'output/interaction-report.json'),report=JSON.parse(await readFile(path));change(report);const bytes=JSON.stringify(report);await writeFile(path,bytes);const receiptPath=resolve(job,'receipt.json'),receipt=JSON.parse(await readFile(receiptPath));receipt.artifacts['interaction-report.json']={size:Buffer.byteLength(bytes),sha256:hash(bytes)};const receiptBytes=JSON.stringify(receipt);await writeFile(receiptPath,receiptBytes);f.proof.receipts[index].sha256=hash(receiptBytes);}
test('quota exhaustion and unrelated checkpoint/ledger reject a correctly fingerprinted proof',async()=>{
 for(const change of [s=>s.budget.consumedPercent=25,s=>s.resumePhase='test-gate',s=>s.recoveryAttempts['parallel-work:7:7:0:1:0']=2]){const f=await fixture();change(f.state);await refreshState(f);await assert.rejects(validateHostApprovalResume(f.root,f.state,f.proof,f.options));}
});
test('raw saved state, duplicate renderer, missing case and missing build session fail closed',async()=>{
 const raw=await fixture();await writeFile(resolve(raw.run,'state.json'),JSON.stringify({...raw.state,updatedAt:'changed'}));await assert.rejects(validateHostApprovalResume(raw.root,raw.state,raw.proof,raw.options),/raw saved/);
 for(const change of [r=>{for(const row of r.reports){row.executions[0].renderer='software';for(const session of row.executions[0].sessions)session.renderer='software';}},r=>r.reports.pop(),r=>r.reports[0].executions[0].sessions.pop(),r=>{r.inputs_before.source=r.inputs_after.source=hash('forged');}]){const f=await fixture();await changeReport(f,1,change);await assert.rejects(validateHostApprovalResume(f.root,f.state,f.proof,f.options));}
});
test('unrelated approved tree cannot validate source even with a matching review witness',async()=>{
 const f=await fixture(),policy=JSON.parse(await readFile(f.options.policyPath));policy.approvals[0].tree='0'.repeat(40);const bytes=JSON.stringify(policy);await writeFile(f.options.policyPath,bytes);f.proof.policyDigest=hash(bytes);const review=JSON.stringify({approved:true,reviewer:'independent',head:f.proof.head,policyDigest:f.proof.policyDigest});await writeFile(resolve(f.run,f.proof.reviewFile),review);f.proof.reviewDigest=hash(review);await assert.rejects(validateHostApprovalResume(f.root,f.state,f.proof,f.options),/exact reviewed whole tree/);
});
