// A controller schema repair changes controller identity, not native authority.
import {open,readFile,realpath} from 'node:fs/promises';
import {resolve} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {digest} from './verification.mjs';
import {controllerExecutionIdentity} from './execution.mjs';
import {assertStrictOutputSchema} from './strict-output-schema.mjs';
import {resumeExecutionCheckpoint} from './dispatch.mjs';
import {readPrivateHostProfile,readPrivateHostJSON} from './controller-host-review-workflow.mjs';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_PROFILE_UPGRADE_REFUSED]: '+message),{code:'HOST_PROFILE_UPGRADE_REFUSED',deterministic:true});};
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const controllerPath=p=>typeof p==='string'&&(/^[.]sandcastle\/[a-zA-Z0-9_.-]+\.(?:mjs|json|sh|md)$/.test(p)||/^tests\/sandcastle[a-zA-Z0-9_.-]*\.(?:mjs|json|sh)$/.test(p));
export async function prepareHostProfileUpgrade({root,state,newPin,controllerDigest}){
 need(state.phase==='awaiting'&&state.resumePhase==='parallel-work'&&state.currentFailure?.fatalPreflight===true&&(state.currentFailure.code==='schema'||state.currentFailure.diagnosticCode==='SCHEMA_INVALID')&&!state.hostReviewPending&&!state.verificationTarget,'saved fatal schema checkpoint required');
 resumeExecutionCheckpoint(structuredClone(state));
 need((await controllerExecutionIdentity(root)).digest===controllerDigest,'loaded controller changed');
 assertStrictOutputSchema(JSON.parse(await readFile(resolve(root,'.sandcastle/ticket-output.schema.json'),'utf8')));
 const oldPin=state.hostReviewProfile,old=await readPrivateHostProfile(root,state,oldPin),next=await readPrivateHostProfile(root,state,newPin);
 need(state.activeHostPolicy?.path===old.policyPath&&state.activeHostPolicy.sha256===old.policySha256&&next.controllerDigest===controllerDigest,'active profile/policy or loaded controller differs');
 const policy=await readPrivateHostJSON(root,state,{path:old.policyPath,sha256:old.policySha256}),updated=await readPrivateHostJSON(root,state,{path:next.policyPath,sha256:next.policySha256});
 const seal=await readPrivateHostJSON(root,state,{path:old.sealPath,sha256:old.sealSha256});
 need(seal.complete_consumed_input_seal===true&&seal.external_files&&Array.isArray(seal.compiler_dependencies)&&seal.independent_review&&policy.maxJobs>0&&policy.maxJobs<=8&&Object.keys(seal.sources??{}).length===390&&seal.contract_digest===state.acceptance.contractDigest&&policy.runId===state.id&&policy.contractDigest===state.acceptance.contractDigest&&old.bindingDigest===state.executionPlan.digest,'complete unchanged native source/resource seal required');
 const profileBody=p=>{const q=structuredClone(p);for(const k of ['controllerDigest','policyPath','policySha256'])delete q[k];return q;};
 need(same(profileBody(old),profileBody(next))&&next.sealPath===old.sealPath&&next.sealSha256===old.sealSha256&&policy.runtimeFiles?.[old.sealPath]===old.sealSha256,'profile changed native seal/binding/scope');
 const name=old.equivalenceAction,action=policy.actions?.[name],newAction=updated.actions?.[name];
 need(action?.inspectOnly===true&&newAction?.inspectOnly===true&&action.sourceWorktree===true&&same(action.inventory.files.slice().sort(),Object.keys(seal.sources).sort())&&action.inventory.roots.length===0,'fixed inspect-only390 equivalence action required');
 const oldFiles=action.carryForward?.reviewedFiles,nextFiles=newAction.carryForward?.reviewedFiles;
 need(oldFiles&&nextFiles&&Object.keys(oldFiles).every(p=>Object.hasOwn(nextFiles,p)),'reviewed controller versions cannot be removed');
 const policyBody=p=>{const q=structuredClone(p);delete q.actions[name].carryForward.reviewedFiles;return q;};
 need(same(policyBody(policy),policyBody(updated)),'capture recipes/approvals/runtime pins/delivery/limits changed');
 const changes={};
 const validVersion=v=>v&&Object.keys(v).sort().join(',')==='mode,sha256'&&['100644','100755'].includes(v.mode)&&/^[a-f0-9]{64}$/.test(v.sha256??'');
 for(const [path,versions]of Object.entries(nextFiles)){
  const previous=oldFiles[path]??[];
  need(Array.isArray(previous)&&Array.isArray(versions)&&previous.every(validVersion)&&versions.every(validVersion),'reviewed file versions require exact mode/hash arrays');
  if(same(versions,previous))continue;
  need(versions.length===previous.length+1&&same(versions.slice(0,previous.length),previous),'reviewed versions must preserve the exact old prefix and append only one current version');
  const version=versions.at(-1);need(!previous.some(v=>same(v,version))&&controllerPath(path)&&!Object.hasOwn(seal.sources,path),'changed file is not new nonconsumed controller/test version');
  const entry=(await exec('git',['-C',root,'ls-tree','HEAD','--',path])).stdout,mode=/^(100644|100755) blob /.exec(entry)?.[1];need(mode===version.mode,'regular committed controller mode required');
  const blob=(await exec('git',['-C',root,'show',`HEAD:${path}`],{encoding:'buffer',maxBuffer:4*1024*1024})).stdout;
  const full=resolve(root,path);need(await realpath(full)===full&&digest(blob)===version.sha256&&digest(await readFile(full))===version.sha256,'reviewed version differs from committed loaded code');changes[path]=version;
 }
 const record={version:1,reason:'independently reviewed controller strict-schema repair',oldPin:structuredClone(oldPin),newPin:structuredClone(newPin),controllerDigest,oldPolicySha256:old.policySha256,newPolicySha256:next.policySha256,sealSha256:old.sealSha256,bindingDigest:old.bindingDigest,reviewedControllerFiles:changes};
 record.fingerprint=digest(canonical(record));
 return {record,profile:next,checkpointDigest:digest(JSON.stringify(state))};
}
export async function applyHostProfileUpgrade({root,state,prepared,controllerDigest}){
 const lock=resolve(root,'.sandcastle/runs',state.id,'run.lock');need((await readFile(lock,'utf8')).trim()===String(process.pid),'held controller run lock required');
 need(digest(JSON.stringify(state))===prepared.checkpointDigest&&same(JSON.parse(await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'),'utf8')),state),'checkpoint changed before profile upgrade');
 const fresh=await prepareHostProfileUpgrade({root,state,newPin:prepared.record.newPin,controllerDigest});need(same(fresh.record,prepared.record),'profile/policy/controller bytes changed after preview');
 need((await readFile(lock,'utf8')).trim()===String(process.pid)&&same(JSON.parse(await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'),'utf8')),state),'held lock/checkpoint changed during upgrade validation');
 const path=resolve(root,'.sandcastle/runs',state.id,`profile-before-upgrade-${prepared.record.fingerprint}.json`),raw=JSON.stringify(state)+'\n';
 try{const fd=await open(path,'wx',0o600);try{await fd.writeFile(raw);await fd.sync();}finally{await fd.close();}}catch(error){if(error.code!=='EEXIST'||await readFile(path,'utf8')!==raw)throw error;}
 const dir=await open(resolve(root,'.sandcastle/runs',state.id),'r');try{await dir.sync();}finally{await dir.close();}
 (state.hostProfileUpgrades??=[]).push({...prepared.record,checkpointPath:path,checkpointSha256:digest(raw),at:new Date().toISOString()});
 state.hostReviewProfile=structuredClone(prepared.record.newPin);state.activeHostPolicy={...state.activeHostPolicy,path:prepared.profile.policyPath,sha256:prepared.profile.policySha256};
 return prepared.profile;
}
