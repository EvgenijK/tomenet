// Readonly status, not executable policy, delivered to an isolated worker.
import {readPrivateHostProfile,readPrivateHostJSON} from './controller-host-review-workflow.mjs';
import {deliverySourceApproval} from './host-source-approval.mjs';
import {digest} from './verification.mjs';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_REVIEW_REFUSED]: '+message),{code:'HOST_REVIEW_REFUSED',deterministic:true});};
export async function workerHostApprovalSnapshot({root,state,scope,head,controllerDigest}){
 const pin=state.hostReviewProfile;if(!pin)return undefined;
 const profile=await readPrivateHostProfile(root,state,pin);
 need(profile.controllerDigest===controllerDigest&&profile.bindingDigest===state.executionPlan.digest&&state.activeHostPolicy?.path===profile.policyPath&&state.activeHostPolicy.sha256===profile.policySha256,'current independently pinned host profile required');
 const policy=await readPrivateHostJSON(root,state,{path:profile.policyPath,sha256:profile.policySha256}),seal=await readPrivateHostJSON(root,state,{path:profile.sealPath,sha256:profile.sealSha256});
 need(policy.runId===state.id&&policy.contractDigest===state.acceptance.contractDigest&&seal.contract_digest===state.acceptance.contractDigest&&seal.complete_consumed_input_seal===true&&seal.independent_review&&Object.keys(seal.sources??{}).length===390,'current complete approved source/resource scope required');
 const request=profile.defaultRequest,action=policy.actions?.[request?.action],equivalence=policy.actions?.[profile.equivalenceAction];
 need(action?.sourceWorktree===true&&equivalence?.inspectOnly===true&&digest(action.inventory?.files?.slice().sort())===digest(Object.keys(seal.sources).sort())&&digest(equivalence.inventory)===digest(action.inventory)&&action.inventory.roots?.length===0&&['x11','wayland'].includes(request.driver)&&['software','opengl'].includes(request.renderer),'reviewed default request must name an existing action');
 const git=async(...args)=>(await exec('git',['-C',root,...args],{maxBuffer:8*1024*1024})).stdout.trim();
 const blob=async(h,p)=>(await exec('git',['-C',root,'show',`${h}:${p}`],{encoding:'buffer',maxBuffer:8*1024*1024})).stdout;
 need(scope.runId===state.id&&await git('rev-parse',`refs/heads/${scope.branch}`)===head,'worker snapshot scope changed');
 const hashes={};let inputsMatch=true;
 for(const [path,id]of Object.entries(seal.sources)){const entry=await git('ls-tree',head,'--',path);if(!/^(100644|100755) blob /.test(entry)){inputsMatch=false;continue;}const raw=await blob(head,path);hashes[path]=digest(raw);if(raw.length!==id.size||hashes[path]!==id.sha256)inputsMatch=false;}
 const approval=policy.approvals?.find(a=>a.base===seal.tested_head&&a.actions?.includes(request.action)&&Object.keys(a.inputs??{}).length===390&&Object.entries(seal.sources).every(([p,id])=>a.inputs[p]===id.sha256));
 need(approval,'default request requires an independently approved sealed inventory');
 const proof=inputsMatch?await deliverySourceApproval({git,readBlob:blob,action:equivalence,approval,head}):undefined;
 const headers=['/usr/include/bits/siginfo-consts-arch.h','/usr/include/bits/timex.h','/usr/include/linux/sched/types.h'];
 return {version:1,...scope,head,contractDigest:state.acceptance.contractDigest,profileSha256:pin.sha256,policySha256:profile.policySha256,sealSha256:profile.sealSha256,sourceCount:390,resourceCount:Object.keys(seal.external_files??{}).length,sourceEquivalence:!!proof,defaultRequest:structuredClone(request),knownActions:Object.keys(policy.actions).sort(),approvedRecoveryHeaders:headers.filter(p=>Object.hasOwn(seal.external_files??{},p)),historicalEvidenceOnly:true,finalAcceptance:false};
}
export function hostApprovalGuidance(snapshot){
 if(!snapshot)return 'No independently pinned host-review profile is available. Do not invent an action name or claim approval; report the missing fixed profile with hostApprovalRequest:null.';
 return `Read /home/agent/.sv-host-approval-status.json for controller-owned current prerequisite status at HEAD ${snapshot.head}. The approved source/resource seal has ${snapshot.sourceCount} source identities and ${snapshot.resourceCount} resources; current source equivalence is ${snapshot.sourceEquivalence}. Already approved recovery headers: ${JSON.stringify(snapshot.approvedRecoveryHeaders)}. Historical failed capture header gaps and older producer identity rejections remain historical; they do not override this current approval status and they are not passing captures. Use historical dependency verification at its original producer HEAD; never relabel old receipts as current acceptance. If a fresh controller-owned source approval is actually needed, return status=blocked with hostApprovalRequest:${JSON.stringify(snapshot.defaultRequest)} from this independently reviewed profile. Do not invent or alias action names. When no current approval is missing return hostApprovalRequest:null; local implementation/test failures require their own repair. The controller observes the request without executing a job. Implement collectors, focused checks and one bounded transport proof; the complete acceptance matrix runs after assembly through the controller. Preserve original receipt HEAD/scope/nonce and keep final acceptance pending.`;
}
