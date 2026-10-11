// A controller schema repair changes controller identity, not native authority.
import {open,readFile,realpath} from 'node:fs/promises';
import {resolve} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {digest} from './verification.mjs';
import {prepareControllerHostReview} from './controller-host-review.mjs';
import {normalizeControllerHostRequest} from './host-capture-failure.mjs';
import {controllerExecutionIdentity} from './execution.mjs';
import {assertStrictOutputSchema} from './strict-output-schema.mjs';
import {resumeExecutionCheckpoint} from './dispatch.mjs';
import {readPrivateHostProfile,readPrivateHostJSON} from './controller-host-review-workflow.mjs';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_PROFILE_UPGRADE_REFUSED]: '+message),{code:'HOST_PROFILE_UPGRADE_REFUSED',deterministic:true});};
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const controllerPath=p=>typeof p==='string'&&(/^[.]sandcastle\/[a-zA-Z0-9_.-]+\.(?:mjs|json|sh|md)$/.test(p)||/^tests\/sandcastle[a-zA-Z0-9_.-]*\.(?:mjs|json|sh)$/.test(p));
const evidencePaths=['.scratch/sandcastle-3fd6b311/evidence/08-collector-readback/README.md','.scratch/sandcastle-3fd6b311/evidence/08-collector-readback/verification.json'];
export async function prepareHostProfileUpgrade(args){return prepareProfileChange(args,false);}
export async function prepareHostRequestReconcile(args){return prepareProfileChange(args,true);}
async function prepareProfileChange({root,state,newPin,controllerDigest},reconcile){
 if(!reconcile)need(state.phase==='awaiting'&&state.resumePhase==='parallel-work'&&state.currentFailure?.fatalPreflight===true&&(state.currentFailure.code==='schema'||state.currentFailure.diagnosticCode==='SCHEMA_INVALID')&&!state.hostReviewPending&&!state.verificationTarget,'saved fatal schema checkpoint required');
 if(!reconcile)resumeExecutionCheckpoint(structuredClone(state));
 const pending=state.hostReviewPending,request=pending?.request;
 if(reconcile){
  need(state.id==='3fd6b311'&&state.phase==='awaiting'&&state.resumePhase==='controller-host-review'&&state.currentFailure?.fatalPreflight===true&&state.currentFailure.diagnosticCode==='HOST_REVIEW_REFUSED'&&!state.verificationTarget&&!state.quotaError&&state.budget?.consumedPercent<state.budget?.limitPercent,'saved unfrozen unknown-request checkpoint with available quota required');
  need(pending?.originalPhase==='parallel-work'&&request?.code==='HOST_APPROVAL_REQUIRED'&&request.deterministic===true&&request.action==='entry-runtime-discovery'&&request.runId===state.id&&request.wave===8&&request.contractDigest===state.acceptance.contractDigest&&request.ticket==='.scratch/sandcastle-3fd6b311/issues/08-integration-collector.md'&&request.branch==='codex/sandcastle-3fd6b311-wave-8-1'&&state.wave?.id===request.wave&&state.wave.baseCommit===request.baseCommit,'exact historical controller-guidance request required');
  need(state.wave.members.some(m=>m.branch===request.branch&&m.ticket.path===request.ticket&&m.status==='blocked'&&same(m.failure?.request,request))&&same(pending.profilePin,state.hostReviewProfile),'saved blocked member/profile differs');
 }
 need((await controllerExecutionIdentity(root)).digest===controllerDigest,'loaded controller changed');
 assertStrictOutputSchema(JSON.parse(await readFile(resolve(root,'.sandcastle/ticket-output.schema.json'),'utf8')));
 const oldPin=state.hostReviewProfile,old=await readPrivateHostProfile(root,state,oldPin),next=await readPrivateHostProfile(root,state,newPin);
 need(state.activeHostPolicy?.path===old.policyPath&&state.activeHostPolicy.sha256===old.policySha256&&next.controllerDigest===controllerDigest,'active profile/policy or loaded controller differs');
 const policy=await readPrivateHostJSON(root,state,{path:old.policyPath,sha256:old.policySha256}),updated=await readPrivateHostJSON(root,state,{path:next.policyPath,sha256:next.policySha256});
 const seal=await readPrivateHostJSON(root,state,{path:old.sealPath,sha256:old.sealSha256});
 need(seal.complete_consumed_input_seal===true&&seal.external_files&&Array.isArray(seal.compiler_dependencies)&&seal.independent_review&&policy.maxJobs>0&&policy.maxJobs<=8&&Object.keys(seal.sources??{}).length===390&&seal.contract_digest===state.acceptance.contractDigest&&policy.runId===state.id&&policy.contractDigest===state.acceptance.contractDigest&&old.bindingDigest===state.executionPlan.digest,'complete unchanged native source/resource seal required');
 const profileBody=p=>{const q=structuredClone(p);for(const k of ['controllerDigest','policyPath','policySha256',...(reconcile?['requestReconciliationReview']:[])])delete q[k];return q;};
 need(same(profileBody(old),profileBody(next))&&next.sealPath===old.sealPath&&next.sealSha256===old.sealSha256&&policy.runtimeFiles?.[old.sealPath]===old.sealSha256,'profile changed native seal/binding/scope');
 const name=old.equivalenceAction,action=policy.actions?.[name],newAction=updated.actions?.[name];
 need(action?.inspectOnly===true&&newAction?.inspectOnly===true&&action.sourceWorktree===true&&same(action.inventory.files.slice().sort(),Object.keys(seal.sources).sort())&&action.inventory.roots.length===0,'fixed inspect-only390 equivalence action required');
 const oldFiles=action.carryForward?.reviewedFiles,nextFiles=newAction.carryForward?.reviewedFiles;
 need(oldFiles&&nextFiles&&Object.keys(oldFiles).every(p=>Object.hasOwn(nextFiles,p)),'reviewed controller versions cannot be removed');
 const policyBody=p=>{const q=structuredClone(p);delete q.actions[name].carryForward.reviewedFiles;return q;};
 need(same(policyBody(policy),policyBody(updated)),'capture recipes/approvals/runtime pins/delivery/limits changed');
 if(reconcile)need(Object.keys(seal.external_files).length===1051&&!policy.actions?.[request.action]&&!updated.actions?.[request.action],'unchanged1051 seal and unknown original action required');
 const review=next.requestReconciliationReview;
 if(reconcile)need(review&&Object.keys(review).sort().join(',')==='evidenceFiles,head,oldRequestDigest,reference,reviewer'&&typeof review.reviewer==='string'&&review.reviewer.trim()&&typeof review.reference==='string'&&review.reference.trim()&&review.head===request.head&&review.oldRequestDigest===digest(request)&&same(Object.keys(review.evidenceFiles??{}).sort(),evidencePaths.slice().sort()),'exact independently reviewed evidence record required');
 const changes={},evidenceChanges={};
 const validVersion=v=>v&&Object.keys(v).sort().join(',')==='mode,sha256'&&['100644','100755'].includes(v.mode)&&/^[a-f0-9]{64}$/.test(v.sha256??'');
 for(const [path,versions]of Object.entries(nextFiles)){
  const previous=oldFiles[path]??[];
  need(Array.isArray(previous)&&Array.isArray(versions)&&previous.every(validVersion)&&versions.every(validVersion),'reviewed file versions require exact mode/hash arrays');
  if(same(versions,previous))continue;
  need(versions.length===previous.length+1&&same(versions.slice(0,previous.length),previous),'reviewed versions must preserve the exact old prefix and append only one current version');
  const version=versions.at(-1),evidence=reconcile&&evidencePaths.includes(path);
  need(!previous.some(v=>same(v,version))&&(controllerPath(path)||evidence)&&!Object.hasOwn(seal.sources,path),'changed file is not new nonconsumed controller/test version or exact reviewed evidence');
  const revision=evidence?request.head:'HEAD';
  const entry=(await exec('git',['-C',root,'ls-tree',revision,'--',path])).stdout,mode=/^(100644|100755) blob /.exec(entry)?.[1];need(mode===version.mode,'regular committed reviewed mode required');
  const blob=(await exec('git',['-C',root,'show',`${revision}:${path}`],{encoding:'buffer',maxBuffer:4*1024*1024})).stdout;
  need(digest(blob)===version.sha256,'reviewed version differs from committed code/evidence');
  if(evidence){need(previous.length===0&&mode==='100644'&&same(review.evidenceFiles[path],version),'exact new evidence blob review required');evidenceChanges[path]=version;}
  else {const full=resolve(root,path);need(await realpath(full)===full&&digest(await readFile(full))===version.sha256,'reviewed version differs from committed loaded code');changes[path]=version;}
 }
 const record={version:1,reason:reconcile?'independently reviewed correction of historical controller-generated unknown request':'independently reviewed controller strict-schema repair',oldPin:structuredClone(oldPin),newPin:structuredClone(newPin),controllerDigest,oldPolicySha256:old.policySha256,newPolicySha256:next.policySha256,sealSha256:old.sealSha256,bindingDigest:old.bindingDigest,reviewedControllerFiles:changes};
 if(reconcile){
  need(same(evidenceChanges,review.evidenceFiles),'both exact evidence additions required');
  const freshRequest=normalizeControllerHostRequest({...request,...next.defaultRequest});
  need(updated.actions?.[freshRequest.action]?.controllerOnly===true&&freshRequest.action==='capture-runtime-baseline'&&freshRequest.driver==='x11'&&freshRequest.renderer==='software','reviewed controller default must remain exact native baseline request');
  const check=structuredClone(state);check.phase='controller-host-review';delete check.resumePhase;check.hostReviewPending.request=freshRequest;
  const validated=await prepareControllerHostReview({root,state:check,request:freshRequest,profile:next,ops:{currentControllerDigest:async()=>controllerDigest}});
  need(Object.keys(validated.proposal.resourceAdditions).length===0,'request reconciliation cannot add runtime resources');
  Object.assign(record,{oldRequest:structuredClone(request),freshRequest,evidenceReview:structuredClone(review),sourceProof:validated.proposal.sourceProof});
 }
 record.fingerprint=digest(canonical(record));
 return {record,profile:next,checkpointDigest:digest(JSON.stringify(state))};
}
export async function applyHostProfileUpgrade(args){return applyProfileChange(args,false);}
export async function applyHostRequestReconcile(args){return applyProfileChange(args,true);}
async function applyProfileChange({root,state,prepared,controllerDigest},reconcile){
 const lock=resolve(root,'.sandcastle/runs',state.id,'run.lock');need((await readFile(lock,'utf8')).trim()===String(process.pid),'held controller run lock required');
 need(digest(JSON.stringify(state))===prepared.checkpointDigest&&same(JSON.parse(await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'),'utf8')),state),'checkpoint changed before profile upgrade');
 const fresh=await prepareProfileChange({root,state,newPin:prepared.record.newPin,controllerDigest},reconcile);need(same(fresh.record,prepared.record),'profile/policy/controller bytes changed after preview');
 need((await readFile(lock,'utf8')).trim()===String(process.pid)&&same(JSON.parse(await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'),'utf8')),state),'held lock/checkpoint changed during upgrade validation');
 const path=resolve(root,'.sandcastle/runs',state.id,`${reconcile?'request-before-reconcile':'profile-before-upgrade'}-${prepared.record.fingerprint}.json`),raw=JSON.stringify(state)+'\n';
 try{const fd=await open(path,'wx',0o600);try{await fd.writeFile(raw);await fd.sync();}finally{await fd.close();}}catch(error){if(error.code!=='EEXIST'||await readFile(path,'utf8')!==raw)throw error;}
 const dir=await open(resolve(root,'.sandcastle/runs',state.id),'r');try{await dir.sync();}finally{await dir.close();}
 (reconcile?(state.hostRequestReconciliations??=[]):(state.hostProfileUpgrades??=[])).push({...fresh.record,checkpointPath:path,checkpointSha256:digest(raw),at:new Date().toISOString()});
 state.hostReviewProfile=structuredClone(fresh.record.newPin);state.activeHostPolicy={...state.activeHostPolicy,path:fresh.profile.policyPath,sha256:fresh.profile.policySha256};
 if(reconcile){
  state.hostReviewPending={...state.hostReviewPending,request:structuredClone(fresh.record.freshRequest),profilePin:structuredClone(fresh.record.newPin)};
  state.hostReviewObserved={request:structuredClone(fresh.record.freshRequest),origin:'independently pinned controller default; historical request archived',reconciliationFingerprint:fresh.record.fingerprint};
  state.phase='controller-host-review';delete state.resumePhase;delete state.pauseReason;
 }
 return fresh.profile;
}
