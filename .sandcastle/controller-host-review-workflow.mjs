// Saved-phase adapter: only the controller can select a reviewed fixed profile.
import {open,lstat,realpath,readFile} from 'node:fs/promises';
import {constants} from 'node:fs';
import {resolve,relative,isAbsolute} from 'node:path';
import {digest} from './verification.mjs';
import {normalizeControllerHostRequest,extractTypedHostResourceFailure} from './host-capture-failure.mjs';
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_REVIEW_REFUSED]: '+message),{code:'HOST_REVIEW_REFUSED',deterministic:true});};
const safe=p=>p&&!isAbsolute(p)&&p.split('/').every(x=>x&&x!=='.'&&x!=='..');
export async function readPrivateHostJSON(root,state,pin){
 need(pin&&typeof pin.path==='string'&&pin.path&&/^[a-f0-9]{64}$/.test(pin.sha256??''),'pinned private profile required');
 const path=resolve(pin.path),run=resolve(root,'.sandcastle/runs',state.id);need(safe(relative(run,path))&&await realpath(path)===path,'profile outside private run or symlink');
 const stat=await lstat(path);need(stat.isFile()&&stat.nlink===1&&stat.size<=4*1024*1024&&(stat.mode&0o077)===0,'regular private bounded profile required');
 const fd=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);let bytes;try{const before=await fd.stat();bytes=await fd.readFile();const after=await fd.stat();need(before.dev===stat.dev&&before.ino===stat.ino&&before.size===bytes.length&&before.size===after.size&&before.mtimeMs===after.mtimeMs&&before.ctimeMs===after.ctimeMs,'profile changed during read');}finally{await fd.close();}
 need(digest(bytes)===pin.sha256,'profile bytes differ from independent pin');return JSON.parse(bytes);
}
export async function readPrivateHostProfile(root,state,pin){
 const profile=await readPrivateHostJSON(root,state,pin);need(profile.id==='sv-entry-collector-v3','unknown fixed profile');return profile;
}
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
// Resource provenance can belong to a different immutable policy version than
// the independently reviewed activation profile. Both raw pins are required;
// this reads the original failed capture and grants no execution permission.
export async function attestControllerHostResourceOrigin({root,state,resource,profile,binding}){
 const original=resource.originRequest,pin={path:resource.originPolicyPath??profile.policyPath,sha256:resource.originPolicySha256??profile.policySha256};
 need(original&&original.runId===state.id&&original.nonce===resource.nonce&&original.head===resource.head&&original.action===resource.action&&original.contractDigest===state.acceptance.contractDigest&&resource.policyDigest===pin.sha256,'original private resource attempt scope/policy not pinned');
 const policy=await readPrivateHostJSON(root,state,pin),active=await readPrivateHostJSON(root,state,{path:profile.policyPath,sha256:profile.policySha256});
 const action=policy.actions?.[original.action],current=active.actions?.[original.action];
 const recipe=a=>{if(!Array.isArray(a?.argv))return undefined;const argv=a.argv.slice(),index=argv.indexOf('--reviewed-seal');if(index<0||argv.filter(x=>x==='--reviewed-seal').length!==1)return undefined;argv[index+1]='$PINNED_SEAL';return {argv,inventory:a.inventory,checkId:a.checkId,bindingCommand:a.bindingCommand,controllerOnly:a.controllerOnly,sourceWorktree:a.sourceWorktree,timeoutMs:a.timeoutMs};};
 need(action&&current&&recipe(action)&&same(recipe(action),recipe(current))&&binding?.id===action.checkId&&same(binding.command,action.bindingCommand),'original capture template/inventory/binding differs from fixed activation profile');
 const failure=await extractTypedHostResourceFailure({root,policy,policyDigest:pin.sha256,action,binding,request:original,job:resolve(root,'.sandcastle/runs',state.id,'host-jobs',resource.nonce),sourceApproval:resource.approvedSourceProof});
 const actual=failure?.resourceProposal;
 need(actual&&['head','nonce','graphPath','graphSha256','failedManifestPath','failedManifestSha256','policyDigest'].every(k=>actual[k]===resource[k])&&same(actual.missing,resource.missing),'original failed capture resource evidence changed');
 return {verified:true,head:resource.head,nonce:resource.nonce,graphSha256:resource.graphSha256,failedManifestSha256:resource.failedManifestSha256,policyDigest:resource.policyDigest,originPolicyPath:pin.path};
}
export function hasSavedHostReviewFailure(state){
 return state.currentFailure?.diagnosticCode==='HOST_APPROVAL_REQUIRED'||state.wave?.members.some(m=>m.failure?.code==='HOST_APPROVAL_REQUIRED'&&m.failure.deterministic===true)||state.hostReviewPending?.request?.code==='HOST_APPROVAL_REQUIRED';
}
// An old checkpoint lacks request fields. Only a reviewed default supplies the
// action; scope derives from the saved failed member, never provider prose.
export async function savedControllerHostRequest(state,profile,git,observed){
 const old=observed??state.hostReviewPending?.request??state.hostReviewObserved?.request;
 const phase=state.phase==='controller-host-review'?state.hostReviewPending?.originalPhase:state.phase==='awaiting'?state.resumePhase:state.phase;
 need(['parallel-work','assemble','test-gate'].includes(phase)&&!state.verificationTarget&&!state.quotaError,'unfrozen host prerequisite required');
 need(hasSavedHostReviewFailure(state)||observed?.code==='HOST_APPROVAL_REQUIRED','saved typed host failure required');
 const member=state.wave?.members.find(m=>m.failure?.code==='HOST_APPROVAL_REQUIRED'&&m.failure.deterministic===true&&(!old||m.branch===old.branch));
 const saved=old??member?.failure?.request;
 const defaults=profile.defaultRequest;const shape=saved??defaults;
 need(shape&&typeof shape.action==='string'&&['x11','wayland'].includes(shape.driver)&&['software','opengl'].includes(shape.renderer),'reviewed default or observed request required');
 const scope=member?{runId:state.id,wave:state.wave.id,ticket:member.ticket.path,branch:member.branch,baseCommit:state.wave.baseCommit}:saved;
 need(scope&&scope.runId===state.id&&scope.branch&&scope.ticket,'saved scoped owner required');
 const head=await git('rev-parse',`refs/heads/${scope.branch}`);
 if(saved)need(saved.head===head,'observed source branch changed; fresh controller observation required');
 return normalizeControllerHostRequest({...scope,head,action:shape.action,driver:shape.driver,renderer:shape.renderer,contractDigest:state.acceptance.contractDigest});
}
export function enterControllerHostReview(state,request,profilePin){
 const originalPhase=state.phase==='controller-host-review'?state.hostReviewPending?.originalPhase:state.phase==='awaiting'?state.resumePhase:state.phase;
 need(['parallel-work','assemble','test-gate'].includes(originalPhase)&&request.code==='HOST_APPROVAL_REQUIRED'&&request.deterministic===true&&!state.verificationTarget&&!state.quotaError,'typed unfrozen checkpoint required');
 state.hostReviewPending={originalPhase,request:structuredClone(request),profilePin:structuredClone(profilePin)};state.phase='controller-host-review';delete state.resumePhase;delete state.pauseReason;return state;
}
export function finishControllerHostReview(state,result,profilePin){
 need(state.phase==='controller-host-review'&&state.hostReviewPending&&result.originalPhase===state.hostReviewPending.originalPhase,'host review checkpoint changed');
 (state.hostReviewHistory??=[]).push({...structuredClone(result),at:new Date().toISOString()});
 if(result.status!=='approved'){state.resumePhase='controller-host-review';state.phase='awaiting';state.pauseReason='Execution preflight [HOST_REVIEW_REFUSED]: independent prerequisite review rejected';return;}
 need(profilePin&&result.finalAcceptance===false&&result.policyPath&&result.policySha256,'verified installed prerequisite required');
 state.hostReviewProfile=structuredClone(profilePin);state.activeHostPolicy={path:result.policyPath,sha256:result.policySha256,proposalFingerprint:result.proposalFingerprint};state.phase=result.originalPhase;
 // Keep historical member failure/attempt ledger; the next ordinary worker
 // dispatch clears its own failure after the controller prerequisite is ready.
 delete state.hostReviewPending;delete state.hostReviewObserved;delete state.currentFailure;delete state.pendingFailure;delete state.resumePhase;delete state.pauseReason;
}
export async function verifyHostReviewEnvelope({root,state,envelope,proposal,controllerDigest,schemaPath}){
 const e=envelope?.execution;if(e?.role!=='controller-host-source-review'||e.readOnly!==true||e.proposalFingerprint!==proposal.fingerprint)return false;
 const history=state.executionHistory?.find(r=>r.id===e.id&&r.role===e.role);const result=state.hostReviewResults?.find(r=>r.executionId===e.id&&r.proposalFingerprint===proposal.fingerprint);
 if(!history||!result||history.path!==resolve(root,'.sandcastle/runs',state.id,`execution-${e.id}.json`))return false;
 try{
  const record=JSON.parse(await readFile(history.path,'utf8'));const {id,role,at,digest:recordDigest,hostReview,...identity}=record;
  if(id!==e.id||role!==e.role||recordDigest!==history.digest||digest(JSON.stringify(identity))!==recordDigest||record.controller.digest!==controllerDigest||record.runtime.kind!=='host'||record.contractDigest!==proposal.contractDigest||record.schema?.digest!==digest(await readFile(schemaPath))||hostReview?.readOnly!==true||hostReview.proposalFingerprint!==proposal.fingerprint)return false;
  const path=resolve(root,'.sandcastle/runs',state.id,`host-review-result-${e.id}.json`);if(result.path!==path||await realpath(path)!==path)return false;
  const raw=await readFile(path);return digest(raw)===result.sha256&&JSON.stringify(JSON.parse(raw))===JSON.stringify(envelope);
 }catch{return false;}
}
