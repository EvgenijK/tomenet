// One explicit scope-v2 activation. Old frozen identities are archived, never rewritten.
import {readFile,open} from 'node:fs/promises';
import {resolve} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {digest} from './verification.mjs';
import {controllerExecutionIdentity} from './execution.mjs';
import {readPrivateHostProfile,readPrivateHostJSON} from './controller-host-review-workflow.mjs';
import {prepareControllerHostReview} from './controller-host-review.mjs';
import {releaseVerificationTarget} from './verification-gates.mjs';
import {assertHostInputScope} from './host-collector-constituents.mjs';
const exec=promisify(execFile),canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x,same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const need=(v,m)=>{if(!v)throw Error('Linker profile activation: '+m);};
const reviewedBaselineSha256='f66621236096736fba32cf9958aa7ce105b14b90a722611a7f9bdcc8979e236a';
export function assertReviewedLinkerBaseline(pin){need(pin?.sha256===reviewedBaselineSha256,'exact independently reviewed linker baseline pin required');}
const failedId='d616a0a7-6a39-4e37-af32-9353b90fc5f1',oldHead='97506a83bf2fb8be18b70db25f05cf7b1c2ba662';
const changed=['tests/sv_entry_capture.py','tests/sv_entry_collector_checks.py','tests/sv_entry_consumption.py','tests/sv_entry_discover.py'];
const controllerPath=p=>/^\.sandcastle\/[^/]+\.(mjs|md)$/.test(p)||/^tests\/sandcastle[^/]*\.mjs$/.test(p);
function profileBody(p){const q=structuredClone(p);for(const k of ['controllerDigest','policyPath','policySha256'])delete q[k];return q;}
export async function prepareLinkerProfileActivation({root,state,proofPin,controllerDigest}){
 need(state.id==='3fd6b311'&&state.phase==='awaiting'&&state.resumePhase==='test-gate'&&state.verificationVersion===1&&state.verificationTarget?.head===oldHead&&state.artifactAttempt?.attemptId===failedId&&state.artifactAttempt.checkId==='entry-runtime'&&state.artifactAttempt.status==='published'&&state.artifactAttempt.receipt?.passed===false&&!state.wave&&!state.recovery&&!state.hostReviewPending&&!state.quotaError&&state.currentFailure?.fatalPreflight===true&&state.completedTickets?.length===11&&!state.linkerProfileActivations?.length,'exact frozen failed runtime checkpoint required');
 need(state.budget?.limitPercent===60&&state.budget.consumedPercent<60&&state.testAttempts===1&&state.testLimit===10,'saved allowance/attempts and remaining quota required');
 need((await controllerExecutionIdentity(root)).digest===controllerDigest,'loaded controller changed');
 const proof=await readPrivateHostJSON(root,state,proofPin);
 need(proof.version===1&&proof.runId===state.id&&proof.authorization==='user-requested-linker-scope-v2-activation'&&proof.failedAttemptId===failedId&&proof.controllerDigest===controllerDigest&&same(proof.oldProfilePin,state.hostReviewProfile)&&proof.review?.reviewer&&proof.review.reference,'independently pinned activation proof required');
 assertReviewedLinkerBaseline(proof.scopeProfilePin);
 const raw=await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'));
 need(digest(raw)===proof.rawStateSha256&&digest(JSON.stringify(state))===proof.stateFingerprint&&digest(state.verificationTarget)===proof.oldTargetDigest,'raw checkpoint/frozen target differs');
 const failed=state.artifactAttempt.receipt;
 need(proof.failedReceipt.path===failed.receiptPath&&proof.failedReceipt.sha256===failed.receiptDigest,'failed receipt pin differs');
 const receipt=await readPrivateHostJSON(root,state,{path:proof.failedReceipt.path,sha256:proof.failedReceipt.sha256});need(receipt.result?.exitCode!==0&&receipt.result?.validation==='failed'&&receipt.attemptId===failedId&&same(receipt.target,state.verificationTarget),'original published failure required');
 const old=await readPrivateHostProfile(root,state,proof.oldProfilePin),scope=await readPrivateHostProfile(root,state,proof.scopeProfilePin),next=await readPrivateHostProfile(root,state,proof.newProfilePin);
 need(state.activeHostPolicy?.path===old.policyPath&&state.activeHostPolicy.sha256===old.policySha256&&scope.id===next.id&&same(profileBody(scope),profileBody(next))&&next.controllerDigest===controllerDigest&&next.bindingDigest===state.executionPlan.digest,'active or successor fixed profile differs');
 const original=await readPrivateHostJSON(root,state,{path:old.sealPath,sha256:old.sealSha256}),seal=await readPrivateHostJSON(root,state,{path:next.sealPath,sha256:next.sealSha256});
 assertHostInputScope(seal.consumed_input_scope);
 need(original.tested_head===oldHead&&original.complete_consumed_input_seal===true&&original.independent_review&&original.contract_digest===state.acceptance.contractDigest,'old frozen seal identity differs');
 need(seal.consumed_input_scope?.version===2&&seal.identityKind==='approved-inputs'&&seal.complete_consumed_input_seal===true&&seal.independent_review&&seal.contract_digest===state.acceptance.contractDigest&&Object.keys(seal.sources??{}).length===390&&Object.keys(original.sources??{}).length===390&&Object.keys(original.external_files??{}).length===1051&&Object.keys(seal.external_files??{}).length===1014,'independently reviewed scoped390/1014 seal required');
 need(same(seal.compiler_dependencies,original.compiler_dependencies)&&same(seal.resource_discovery_origins,original.resource_discovery_origins)&&Object.entries(seal.external_files).every(([p,id])=>Object.hasOwn(original.external_files,p)&&same(id,original.external_files[p])),'old retained resource/compiler provenance changed');
 need(same(Object.keys(seal.sources).sort(),Object.keys(original.sources).sort())&&same(Object.keys(seal.sources).filter(p=>!same(seal.sources[p],original.sources[p])).sort(),changed),'exact four reviewed source changes required');
 const scopeReview=await readPrivateHostJSON(root,state,seal.independent_review.reference);
 need(scopeReview.reviewer&&scopeReview.sourceCount===390&&scopeReview.unchangedSources===386&&scopeReview.excludedLinkerOnlyResources===37&&scopeReview.retainedResources===1014&&scopeReview.finalAcceptance===false&&scopeReview.nativeJobsExecuted===0,'scoped independent source/resource review required');
 const base=await readPrivateHostJSON(root,state,{path:scope.policyPath,sha256:scope.policySha256}),policy=await readPrivateHostJSON(root,state,{path:next.policyPath,sha256:next.policySha256});
 const name=scope.equivalenceAction;need(name===next.equivalenceAction&&base.actions[name]?.inspectOnly===true&&policy.actions[name]?.inspectOnly===true,'fixed readonly equivalence action required');
 const body=p=>{const q=structuredClone(p);delete q.review;delete q.actions[name].carryForward.reviewedFiles;return q;};need(same(body(base),body(policy))&&policy.review?.reviewer&&policy.review.reference,'successor changed native recipes/approvals/runtime/delivery/caps');
 const git=async(...args)=>(await exec('git',['-C',root,...args],{maxBuffer:32*1024*1024})).stdout.trim(),head=await git('rev-parse',`refs/heads/${state.branch}`);
 need(head===proof.integrationHead,'assembled HEAD changed');
 const main=await git('rev-parse','refs/heads/modern_interface');await git('merge-base','--is-ancestor',main,head);
 const oldFiles=base.actions[name].carryForward.reviewedFiles,newFiles=policy.actions[name].carryForward.reviewedFiles;need(oldFiles&&newFiles&&Object.keys(oldFiles).every(p=>Object.hasOwn(newFiles,p)),'reviewed versions removed');
 for(const [path,versions]of Object.entries(newFiles)){
  const previous=oldFiles[path]??[];need(Array.isArray(versions)&&same(versions.slice(0,previous.length),previous)&&versions.length<=previous.length+1,'reviewed versions replaced/dropped');
  if(versions.length===previous.length)continue;
  need(controllerPath(path)&&!Object.hasOwn(seal.sources,path),'only nonconsumed committed controller versions may be appended');const entry=versions.at(-1);need(Object.keys(entry).sort().join(',')==='mode,sha256','exact controller identity required');
  const mode=await git('ls-tree',head,'--',path),bytes=(await exec('git',['-C',root,'show',`${head}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;need(mode.startsWith(entry.mode+' blob ')&&['100644','100755'].includes(entry.mode)&&digest(bytes)===entry.sha256,'reviewed controller version differs from current committed bytes');
  need(digest(await readFile(resolve(root,path)))===entry.sha256,'loaded controller working bytes differ');
 }
 const request={code:'HOST_APPROVAL_REQUIRED',deterministic:true,runId:state.id,wave:0,ticket:'.scratch/sandcastle-3fd6b311/issues/08-integration-collector.md',branch:state.branch,baseCommit:state.baseCommit,head,contractDigest:state.acceptance.contractDigest,...next.defaultRequest};
 need(request.action==='capture-runtime-baseline'&&request.driver==='x11'&&request.renderer==='software','fixed controller-selected baseline required');
 const future=structuredClone(state);delete future.verificationTarget;delete future.verifiedArtifactReferences;const preview=await prepareControllerHostReview({root,state:future,request,profile:next,ops:{currentControllerDigest:async()=>controllerDigest}});
 need(!preview.proposal.sourceRevision&&Object.keys(preview.proposal.resourceAdditions).length===0,'current390 source identity differs or new resource grant requested');
 return {proofPin:structuredClone(proofPin),proof,profile:next,request,previewFingerprint:preview.proposal.fingerprint,checkpointDigest:digest(JSON.stringify(state)),fingerprint:digest(canonical({proof,proofPin,request,preview:preview.proposal.fingerprint}))};
}
export async function applyLinkerProfileActivation({root,state,prepared,controllerDigest}){
 const run=resolve(root,'.sandcastle/runs',state.id),lock=resolve(run,'run.lock');need((await readFile(lock,'utf8')).trim()===String(process.pid),'held controller lock required');need(digest(JSON.stringify(state))===prepared.checkpointDigest,'checkpoint changed');
 const fresh=await prepareLinkerProfileActivation({root,state,proofPin:prepared.proofPin,controllerDigest});need(fresh.fingerprint===prepared.fingerprint,'reviewed profile/source/proof drift');
 const raw=await readFile(resolve(run,'state.json'));need(digest(raw)===fresh.proof.rawStateSha256&&(await readFile(lock,'utf8')).trim()===String(process.pid),'raw CAS/lock changed');
 const archive=resolve(run,'linker-scope-before-'+fresh.fingerprint+'.json');try{const fd=await open(archive,'wx',0o600);try{await fd.writeFile(raw);await fd.sync();}finally{await fd.close();}}catch(e){if(e.code!=='EEXIST'||!(await readFile(archive)).equals(raw))throw e;}const dir=await open(run,'r');try{await dir.sync();}finally{await dir.close();}
 const next=structuredClone(state);releaseVerificationTarget(next);next.archivedArtifactAttempts??=[];next.archivedArtifactAttempts.push(structuredClone(next.artifactAttempt));delete next.artifactAttempt;
 next.linkerProfileActivations=[{fingerprint:fresh.fingerprint,proofPin:fresh.proofPin,archive,archiveSha256:digest(raw),oldTargetDigest:fresh.proof.oldTargetDigest,oldProfilePin:fresh.proof.oldProfilePin,newProfilePin:fresh.proof.newProfilePin,allowanceGranted:false,at:new Date().toISOString()}];
 next.hostReviewProfile=structuredClone(fresh.proof.newProfilePin);next.activeHostPolicy={path:fresh.profile.policyPath,sha256:fresh.profile.policySha256};next.hostReviewPending={originalPhase:'test-gate',request:fresh.request,profilePin:structuredClone(fresh.proof.newProfilePin)};next.phase='controller-host-review';for(const k of ['resumePhase','pauseReason','currentFailure'])delete next[k];next.updatedAt=new Date().toISOString();for(const k of ['verificationTarget','verifiedArtifactReferences','artifactAttempt','resumePhase','pauseReason','currentFailure'])delete state[k];Object.assign(state,next);return fresh.profile;
}
