// Controller-owned prerequisite recovery. Providers submit identities, never
// commands or policies. The workflow owns the saved phase and quota accounting.
import {open,lstat,realpath,mkdir,rename,rm,readdir} from 'node:fs/promises';
import {constants} from 'node:fs';
import {resolve,relative,isAbsolute} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {gunzipSync} from 'node:zlib';
import {digest} from './verification.mjs';
import {deliverySourceApproval} from './host-source-approval.mjs';
import {deriveHostSourceRevision} from './host-source-revision.mjs';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Controller host review: '+message),{code:'HOST_REVIEW_REFUSED',deterministic:true});};
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const hash=x=>typeof x==='string'&&/^[a-f0-9]{64}$/.test(x);
const head=x=>typeof x==='string'&&/^[a-f0-9]{40}$/.test(x);
const safe=p=>typeof p==='string'&&p&&!isAbsolute(p)&&p.split('/').every(x=>x&&x!=='.'&&x!=='..');
async function bytes(path,maximum=24*1024*1024){
 need(await realpath(path)===path,'symlinked private input');const original=await lstat(path);
 need(original.isFile()&&original.nlink===1&&original.size<=maximum,'regular bounded input required');
 const f=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);try{const before=await f.stat(),data=await f.readFile(),after=await f.stat();need(before.dev===original.dev&&before.ino===original.ino&&data.length===before.size&&before.size===after.size&&before.mtimeMs===after.mtimeMs&&before.ctimeMs===after.ctimeMs,'input changed during read');return data;}finally{await f.close();}
}
const identity=data=>({size:data.length,sha256:digest(data)});
function protectedState(state){
 return {id:state.id,hostReviewProfile:state.hostReviewProfile,hostReviewPending:state.hostReviewPending,branch:state.branch,baseCommit:state.baseCommit,acceptance:state.acceptance,executionPlan:state.executionPlan,tickets:state.tickets,completedTickets:state.completedTickets,deliveredTickets:state.deliveredTickets,deferredTickets:state.deferredTickets,wave:state.wave,reviewRound:state.reviewRound,buildAttempts:state.buildAttempts,testAttempts:state.testAttempts,testBuildAttempts:state.testBuildAttempts,recoveryAttempts:state.recoveryAttempts,recoveryAttemptLimits:state.recoveryAttemptLimits,recoveryAllowanceHistory:state.recoveryAllowanceHistory,recoverySequence:state.recoverySequence,buildLimit:state.buildLimit,testLimit:state.testLimit,quotaLimit:state.quotaLimit,allowanceGrants:state.allowanceGrants,budgetLimits:Object.fromEntries(Object.entries(state.budget??{}).filter(([k])=>!['tokens','consumedPercent','lastUsedPercent','resetsAt','windowId','windowResetAt','windowHighWaterPercent','usageHistory'].includes(k)))};
}
// Known observation fields can advance during review; allowance fields remain
// in protectedState. Consumption cannot reset and prior observations are immutable.
function assertQuotaTelemetry(before={},after={}){
 for(const key of ['tokens','consumedPercent'])if(before[key]!==undefined)need(Number.isFinite(before[key])&&Number.isFinite(after[key])&&after[key]>=before[key],'task consumption decreased during prerequisite review');
 const old=before.usageHistory??[],current=after.usageHistory??[];
 need(Array.isArray(old)&&Array.isArray(current)&&current.length>=old.length&&same(current.slice(0,old.length),old),'quota usage history was truncated or rewritten');
}
async function gitAccess(root){
 const git=async(...args)=>(await exec('git',['-C',root,...args],{maxBuffer:24*1024*1024})).stdout.trim();
 const blob=async(revision,path)=>(await exec('git',['-C',root,'show',`${revision}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;
 return {git,blob};
}
function checkpoint(state){return state.phase==='controller-host-review'?state.hostReviewPending?.originalPhase:state.phase==='awaiting'?state.resumePhase:state.phase;}
/** Prepare a read-only, exact-source proposal. profile pins an ALREADY reviewed
 * private template and seal; a reviewer cannot grant a new executable recipe.
 * Resource proposals additionally need ops.attestResourceOrigin to validate an
 * actual private controller attempt, not a producer's summary or fabricated JSON.
 */
export async function prepareControllerHostReview({root,state,request,profile,ops={}}){
 root=await realpath(root);const originalPhase=checkpoint(state);
 need(['parallel-work','assemble','test-gate'].includes(originalPhase)&&!state.verificationTarget&&!state.quotaError,'unfrozen saved host prerequisite checkpoint required');
 need(request&&Object.keys(request).every(k=>['code','deterministic','runId','wave','ticket','branch','baseCommit','head','action','driver','renderer','contractDigest'].includes(k)),'unknown request field/privilege');
 need(request?.code==='HOST_APPROVAL_REQUIRED'&&request.deterministic===true&&request.runId===state.id&&request.contractDigest===state.acceptance?.contractDigest,'typed controller-observed request required');
 need(Number.isSafeInteger(request.wave)&&request.wave>=0&&head(request.head)&&head(request.baseCommit)&&safe(request.ticket)&&/^codex\/[a-zA-Z0-9_./-]+$/.test(request.branch)&&!request.branch.includes('..')&&['x11','wayland'].includes(request.driver)&&['software','opengl'].includes(request.renderer),'valid saved request identities required');
 if(request.wave){need(request.wave===state.wave?.id&&state.wave.baseCommit===request.baseCommit&&state.wave.members.some(m=>m.branch===request.branch&&m.ticket.path===request.ticket&&m.failure?.code==='HOST_APPROVAL_REQUIRED'&&m.failure.deterministic),'request must match saved channel failure');}
 else need(request.branch===state.branch&&state.tickets.some(t=>t.path===request.ticket),'final controller request must match current owner');
 need(profile?.id==='sv-entry-collector-v3'&&hash(profile.policySha256)&&hash(profile.sealSha256)&&hash(profile.bindingDigest)&&hash(profile.controllerDigest),'fixed independently reviewed profile required');
 need(profile.bindingDigest===state.executionPlan?.digest&&typeof ops.currentControllerDigest==='function'&&profile.controllerDigest===await ops.currentControllerDigest(),'binding/controller profile differs from saved loaded execution');
 const runDir=resolve(root,'.sandcastle/runs',state.id);
 for(const path of [profile.policyPath,profile.sealPath])need(safe(relative(runDir,resolve(path))),'profile must belong to this private run');
 need(((await lstat(profile.policyPath)).mode&0o077)===0&&((await lstat(profile.sealPath)).mode&0o077)===0,'profile policy/seal must be private');
 const policyBytes=await bytes(profile.policyPath),sealBytes=await bytes(profile.sealPath);need(digest(policyBytes)===profile.policySha256&&digest(sealBytes)===profile.sealSha256,'reviewed profile bytes changed');
 const policy=JSON.parse(policyBytes),seal=JSON.parse(sealBytes);
 need(policy.version===1&&policy.runId===state.id&&policy.contractDigest===request.contractDigest&&policy.review?.reviewer&&policy.review.reference&&policy.maxJobs>0&&policy.maxJobs<=8,'independently reviewed bounded policy required');
 need(seal.complete_consumed_input_seal===true&&seal.contract_digest===request.contractDigest&&head(seal.tested_head)&&seal.independent_review&&Object.keys(seal.sources??{}).length===390&&seal.external_files&&Array.isArray(seal.compiler_dependencies),'complete reviewed seal required');
 const selected=policy.actions?.[request.action];need(selected,'unknown action requires a new independently reviewed fixed profile');
 const paths=Object.keys(seal.sources).sort();need(selected.sourceWorktree===true&&same(selected.inventory?.files?.slice().sort(),paths)&&selected.inventory.roots?.length===0,'requested action must use exact sealed390 inventory');
 const {git,blob}=await gitAccess(root);need(await git('rev-parse',`refs/heads/${request.branch}`)===request.head,'branch changed');await git('merge-base','--is-ancestor',request.baseCommit,request.head);
 const tree=await git('rev-parse',`${request.head}^{tree}`),inputHashes={},sourceIdentities={};
 for(const path of paths){need(safe(path),'source path escape');const entry=await git('ls-tree',request.head,'--',path);need(/^(100644|100755) blob /.test(entry),'regular committed source required');const actual=await blob(request.head,path);sourceIdentities[path]=identity(actual);inputHashes[path]=digest(actual);}
 const oldHashes=Object.fromEntries(Object.entries(seal.sources).map(([p,id])=>[p,id.sha256])),approvedHead=seal.approved_source_head??seal.tested_head;
 const prior=policy.approvals?.find(a=>a.actions?.includes(request.action)&&same(a.inputs,oldHashes)&&a.base===approvedHead);need(prior&&head(prior.base)&&head(prior.tree),'matching prior source approval required');
 const equivalence=policy.actions[profile.equivalenceAction??request.action];need(equivalence?.sourceWorktree===true&&same(equivalence.inventory,selected.inventory),'equivalence action must use same reviewed inventory');
 let sourceRevision=null,sourceProof;
 if(!same(sourceIdentities,seal.sources)){
  need(profile.sourceRevisionDelegation,'changed producer requires an independently reviewed fixed source profile');
  sourceRevision=await deriveHostSourceRevision({state,request,profile,seal,identities:sourceIdentities,prior,git,blob});
  sourceProof={method:'delegated-source-revision-proposal',reviewedHead:prior.base,reviewedTree:prior.tree,executionTree:tree,revision:sourceRevision};
 }else{sourceProof=await deliverySourceApproval({git,readBlob:blob,action:equivalence,approval:prior,head:request.head});need(sourceProof,'unlisted tree/code changes require a new independently reviewed fixed profile');}
 const additions={};let resourceOrigin=null;
 if(profile.resourceProposal){
  const resource=profile.resourceProposal;need(hash(resource.graphSha256)&&[request.head,seal.tested_head].includes(resource.head)&&resource.contractDigest===request.contractDigest&&/^[a-f0-9]{32}$/.test(resource.nonce??'')&&typeof ops.attestResourceOrigin==='function','actual resource attempt attestation required');
  need(safe(relative(resolve(runDir,'host-jobs',resource.nonce),resolve(resource.graphPath))),'resource graph outside original private attempt');
  const data=await bytes(resource.graphPath);need(digest(data)===resource.graphSha256,'resource graph changed');
  resourceOrigin=await ops.attestResourceOrigin({request,resource,graphIdentity:identity(data)});need(resourceOrigin?.verified===true&&resourceOrigin.head===resource.head&&resourceOrigin.nonce===resource.nonce&&resourceOrigin.graphSha256===resource.graphSha256,'resource origin not independently controller-attested');
  const graph=JSON.parse(gunzipSync(data,{maxOutputLength:64*1024*1024}));need(Array.isArray(graph.unresolved_inputs)&&graph.unresolved_inputs.length===0&&graph.repository&&graph.external_files,'closed resource graph required');
  for(const[path,id]of Object.entries(graph.repository))need(Object.hasOwn(seal.sources,path)&&same(id,seal.sources[path]),'unsealed repository input in resource graph');
  for(const[path,id]of Object.entries(graph.external_files)){
   if(Object.hasOwn(seal.external_files,path)){need(same(id,seal.external_files[path]),'changed previous resource cannot be silently renewed');continue;}
   need(path.startsWith('/usr/include/')&&path.endsWith('.h')&&!path.split('/').some(x=>x==='..')&&Number.isSafeInteger(id.size)&&hash(id.sha256),'new resource requires an independently reviewed fixed resource class');
   const current=await bytes(path);need(same(identity(current),id),'new header bytes differ from observed attempt');additions[path]=id;
  }
  need(Object.keys(additions).length>0&&Object.keys(additions).length<=32&&same(additions,resource.missing),'exact bounded observed header additions required');
 }
 const proposal={version:1,profile:'sv-entry-collector-v3',originalPhase,request:structuredClone(request),head:request.head,tree,contractDigest:request.contractDigest,bindingDigest:profile.bindingDigest,controllerDigest:profile.controllerDigest,profileDigest:digest({policy:profile.policySha256,seal:profile.sealSha256}),inputDigest:digest(canonical(inputHashes)),resourceDigest:digest(canonical(additions)),sourceProof,sourceRevision,resourceOrigin,resourceAdditions:additions,protectedStateDigest:digest(canonical(protectedState(state))),finalAcceptance:false};
 proposal.fingerprint=digest(canonical(proposal));return {proposal,policy,seal,policyBytes,sealBytes,inputHashes,sourceIdentities,profile:structuredClone(profile),priorApproval:structuredClone(prior)};
}
export function validateControllerHostReview(prepared,envelope,{writerExecutionIds=[]}={}){
 const p=prepared.proposal,r=envelope?.response,e=envelope?.execution;
 need(e?.readOnly===true&&e.role==='controller-host-source-review'&&typeof e.id==='string'&&e.id.length>0&&!writerExecutionIds.includes(e.id)&&e.proposalFingerprint===p.fingerprint,'independent verified reviewer execution required');
 const allowed=['approved','summary','head','tree','contractDigest','bindingDigest','controllerDigest','profileDigest','inputDigest','resourceDigest','findings'];
 need(r&&Object.keys(r).length===allowed.length&&Object.keys(r).every(k=>allowed.includes(k))&&typeof r.approved==='boolean'&&typeof r.summary==='string'&&r.summary.trim()&&r.summary.length<=4000&&Array.isArray(r.findings)&&r.findings.every(x=>typeof x==='string'&&x.length<=4000)&&r.findings.length<=100,'structured bounded review required');
 for(const key of ['head','tree','contractDigest','bindingDigest','controllerDigest','profileDigest','inputDigest','resourceDigest'])need(r[key]===p[key],'review identity differs');
 return {approved:r.approved&&r.findings.length===0,summary:r.summary,findings:r.findings,executionId:e.id,role:e.role,proposalFingerprint:p.fingerprint};
}
async function durable(path,data){const f=await open(path,'wx',0o600);try{await f.writeFile(data);await f.sync();}finally{await f.close();}}
async function syncDir(path){const f=await open(path,'r');try{await f.sync();}finally{await f.close();}}
// Typed handoff only; prose, unrelated startup failures and exhausted allowance
// requests do not acquire controller-host-review semantics.
export function isControllerHostReviewRequest(state,request){
 return request?.code==='HOST_APPROVAL_REQUIRED'&&request.deterministic===true&&request.runId===state.id&&request.contractDigest===state.acceptance?.contractDigest&&!state.verificationTarget&&!state.quotaError&&['parallel-work','assemble','test-gate'].includes(checkpoint(state));
}
function renewal(prepared,review,folder){
 const p=prepared.proposal,profile=prepared.profile,renewedSeal=structuredClone(prepared.seal),policy=structuredClone(prepared.policy),sealPath=resolve(folder,'seal.json'),policyPath=resolve(folder,'policy.json');
 renewedSeal.tested_head=p.head;renewedSeal.approved_source_head=p.head;renewedSeal.identityKind='approved-inputs';renewedSeal.finalAcceptance=false;renewedSeal.milestone_acceptance=false;renewedSeal.sources=structuredClone(prepared.sourceIdentities);renewedSeal.external_files={...renewedSeal.external_files,...p.resourceAdditions};renewedSeal.independent_review={reviewer:review.role,executionId:review.executionId,proposalFingerprint:p.fingerprint,summary:review.summary};renewedSeal.renewal={previousHead:prepared.seal.tested_head,previousSealSha256:profile.sealSha256,sourceProof:p.sourceProof,resourceOrigin:p.resourceOrigin,...(p.sourceRevision?{sourceRevision:p.sourceRevision}:{} )};
 if(p.sourceRevision){renewedSeal.source_revision={...p.sourceRevision,independentReviewExecutionId:review.executionId,previousSealSha256:profile.sealSha256};
  const rules=policy.actions[profile.equivalenceAction].carryForward;rules.reviewedFiles??={};for(const[path,id]of Object.entries(p.sourceRevision.evidenceFiles))(rules.reviewedFiles[path]??=[]).push({mode:id.mode,sha256:id.sha256});
 }
 const sealBytes=Buffer.from(JSON.stringify(renewedSeal)+'\n');
 for(const action of Object.values(policy.actions??{}))if(Array.isArray(action.argv))action.argv=action.argv.map(x=>x===profile.sealPath?sealPath:x);
 for(const parts of Object.values(policy.hostParts??{}))for(const part of parts)if(part.constituents?.reviewedSealPath===profile.sealPath)part.constituents.reviewedSealPath=sealPath;
 policy.runtimeFiles={...policy.runtimeFiles,[sealPath]:digest(sealBytes)};
 const actionNames=Object.entries(policy.actions).filter(([name,a])=>prepared.priorApproval.actions.includes(name)&&a.inventory?.files&&same(a.inventory.files.slice().sort(),Object.keys(prepared.inputHashes).sort())&&a.inventory.roots?.length===0).map(([name])=>name);
 policy.approvals.push({base:p.head,tree:p.tree,inputs:prepared.inputHashes,actions:actionNames});
 policy.review={reviewer:review.role,reference:resolve(folder,'review.json')};policy.controllerHostReview={proposalFingerprint:p.fingerprint,previousPolicySha256:profile.policySha256,originalPhase:p.originalPhase,finalAcceptance:false};
 return {policyPath,sealPath,policyBytes:Buffer.from(JSON.stringify(policy)+'\n'),sealBytes};
}
/** Hooks are trusted controller adapters. withLock asserts/reuses the existing
 * workflow run lock; it must not recursively acquire that lock. review returns
 * a verified independent dispatch envelope; verifyReviewExecution attests its
 * durable execution record on both initial use and crash recovery. authorize is
 * inspect-only. This module never starts native jobs or executes full matrices.
 * record saves the disposition/phase using the normal controller save path.
 */
export async function runControllerHostReview({root,state,request,profile,ops,writerExecutionIds=[]}){
 need(['quota','review','withLock','assertCurrent','authorize','record','currentControllerDigest','verifyReviewExecution'].every(k=>typeof ops?.[k]==='function'),'verified controller adapters required');
 let telemetry=structuredClone(state.budget??{});
 const quota=async()=>{assertQuotaTelemetry(telemetry,state.budget);await ops.quota();assertQuotaTelemetry(telemetry,state.budget);telemetry=structuredClone(state.budget??{});};
 await quota();const prepared=await prepareControllerHostReview({root,state,request,profile,ops}),p=prepared.proposal;
 const previous=(state.hostReviewHistory??[]).find(x=>x.proposalFingerprint===p.fingerprint);
 need(!previous||previous.status==='approved','same rejected prerequisite fingerprint was already reviewed; no retry or allowance grant');
 const parent=resolve(root,'.sandcastle/runs',state.id,'controller-host-review'),folder=resolve(parent,p.fingerprint);
 const installed=await lstat(folder).catch(error=>{if(error.code==='ENOENT')return null;throw error;});
 let envelope,review;
 if(installed){
  need(installed.isDirectory()&&!installed.isSymbolicLink(),'regular installed review version required');
  const index=JSON.parse(await bytes(resolve(folder,'installation.json')));
  need(index.version===1&&index.proposalFingerprint===p.fingerprint&&index.files,'installed proposal identity differs');
  const expectedFiles=['previous-policy.json','previous-seal.json','checkpoint-before.json','proposal.json','review.json','review-envelope.json','seal.json','policy.json'];
  need(same(Object.keys(index.files).sort(),expectedFiles.sort())&&same((await readdir(folder)).sort(),[...expectedFiles,'installation.json'].sort()),'missing/extra installed review file');
  for(const [name,id]of Object.entries(index.files)){need(safe(name)&&!name.includes('/'),'installed private file path');need(same(identity(await bytes(resolve(folder,name))),id),'installed review bytes changed');}
  assertQuotaTelemetry(JSON.parse(await bytes(resolve(folder,'checkpoint-before.json'))).budget,state.budget);
  need(same(JSON.parse(await bytes(resolve(folder,'proposal.json'))),p)&&digest(await bytes(resolve(folder,'previous-policy.json')))===profile.policySha256&&digest(await bytes(resolve(folder,'previous-seal.json')))===profile.sealSha256,'installed profile/proposal changed');
  envelope=JSON.parse(await bytes(resolve(folder,'review-envelope.json')));
 }else envelope=await ops.review(structuredClone(p));
 need(await ops.verifyReviewExecution(envelope,p)===true,'independent reviewer durable execution is unverifiable');
 review=validateControllerHostReview(prepared,envelope,{writerExecutionIds});await quota();
 if(!review.approved){const result={status:'awaiting',proposalFingerprint:p.fingerprint,review,originalPhase:p.originalPhase,reason:'independent host prerequisite review rejected',finalAcceptance:false};await ops.record(result);return result;}
 return await ops.withLock(async()=>{
  await ops.assertCurrent(p);need(digest(canonical(protectedState(state)))===p.protectedStateDigest,'saved checkpoint/limits changed during review');
  const fresh=await prepareControllerHostReview({root,state,request,profile,ops});need(fresh.proposal.fingerprint===p.fingerprint,'source/profile/resource changed after review');await quota();
  const version=renewal(prepared,review,folder),{policyPath,sealPath,policyBytes,sealBytes}=version;
  if(installed){
   need((await bytes(policyPath)).equals(policyBytes)&&(await bytes(sealPath)).equals(sealBytes)&&same(JSON.parse(await bytes(resolve(folder,'review.json'))),review),'installed recipe/seal differs from fixed reviewed renewal');
  }else{
   await mkdir(parent,{recursive:true,mode:0o700});const temporary=folder+'.tmp';await mkdir(temporary,{mode:0o700});
   try{
    const files={'previous-policy.json':prepared.policyBytes,'previous-seal.json':prepared.sealBytes,'checkpoint-before.json':Buffer.from(JSON.stringify(state)+'\n'),'proposal.json':Buffer.from(JSON.stringify(p)+'\n'),'review.json':Buffer.from(JSON.stringify(review)+'\n'),'review-envelope.json':Buffer.from(JSON.stringify(envelope)+'\n'),'seal.json':sealBytes,'policy.json':policyBytes};
    for(const[name,data]of Object.entries(files))await durable(resolve(temporary,name),data);
    await durable(resolve(temporary,'installation.json'),JSON.stringify({version:1,proposalFingerprint:p.fingerprint,files:Object.fromEntries(Object.entries(files).map(([name,data])=>[name,identity(data)]))})+'\n');await syncDir(temporary);await rename(temporary,folder);await syncDir(parent);
   }catch(error){await rm(temporary,{recursive:true,force:true});throw error;}
  }
  await ops.authorize({policyPath,request:{version:1,runId:request.runId,wave:request.wave,ticket:request.ticket,branch:request.branch,baseCommit:request.baseCommit,head:request.head,contractDigest:request.contractDigest,nonce:'0'.repeat(32),action:request.action,driver:request.driver,renderer:request.renderer}});await ops.assertCurrent(p);
  const result={status:'approved',proposalFingerprint:p.fingerprint,review,policyPath,policySha256:digest(policyBytes),sealPath,sealSha256:digest(sealBytes),head:p.head,tree:p.tree,originalPhase:p.originalPhase,sourceProof:p.sourceProof,resourceAdditions:p.resourceAdditions,resumedInstallation:Boolean(installed),finalAcceptance:false};await ops.record(result);return result;
 });
}
