// Extract a controller-owned missing-resource prerequisite before scanning raw
// failed artifacts. Failed captures remain failed; no bytes become acceptance.
import {open,lstat,realpath} from 'node:fs/promises';
import {constants} from 'node:fs';
import {resolve,relative,isAbsolute} from 'node:path';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {gunzipSync} from 'node:zlib';
import {digest} from './verification.mjs';
const exec=promisify(execFile);
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Host capture failure: '+message),{code:'HOST_CAPTURE_INVALID',deterministic:true});};
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const safe=p=>typeof p==='string'&&p&&!isAbsolute(p)&&p.split('/').every(x=>x&&x!=='.'&&x!=='..');
const sha=x=>typeof x==='string'&&/^[a-f0-9]{64}$/.test(x);
async function bytes(path){
 need(await realpath(path)===path,'symlink input');const original=await lstat(path);
 need(original.isFile()&&original.nlink===1&&original.size<=24*1024*1024,'regular bounded diagnostic required');
 const f=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);try{const before=await f.stat(),data=await f.readFile(),after=await f.stat();need(original.dev===before.dev&&original.ino===before.ino&&before.size===data.length&&before.size===after.size&&before.mtimeMs===after.mtimeMs&&before.ctimeMs===after.ctimeMs,'diagnostic changed while reading');return data;}finally{await f.close();}
}
const identity=data=>({size:data.length,sha256:digest(data)});
const collectorPaths=new Set(['$SOURCE/tests/sv_entry_runtime_evidence.py','$SOURCE/tests/sv_entry_automated_evidence.py']);
export async function extractTypedHostResourceFailure({root,policy,policyDigest,action,binding,request,job,sourceApproval}){
 if(!binding||!['entry-runtime','entry-automated'].includes(binding.id))return undefined;
 if(action?.controllerOnly!==true||action.sourceWorktree!==true||action.checkId!==binding.id||!Array.isArray(action.argv)||!collectorPaths.has(action.argv[2])||action.argv[0]!=='/usr/bin/python3'||action.argv[1]!=='-B')return undefined;
 need(same(action,policy.actions?.[request.action])&&same(action.bindingCommand,binding.command)&&sha(policyDigest),'fixed action/private policy identity required');
 need(policy.runId===request.runId&&policy.contractDigest===request.contractDigest&&/^[a-f0-9]{40}$/.test(request.head??'')&&/^[a-f0-9]{32}$/.test(request.nonce??'')&&sourceApproval,'approved source request required');
 root=await realpath(root);job=resolve(job);const expected=resolve(root,'.sandcastle/runs',request.runId,'host-jobs',request.nonce);need(job===expected&&await realpath(job)===job,'original private attempt required');
 const manifestPath=resolve(job,'output',binding.manifestPath);need(safe(binding.manifestPath),'safe failed manifest path required');
 let raw;try{raw=await bytes(manifestPath);}catch(error){if(error.code==='ENOENT')return undefined;throw error;}
 const manifest=JSON.parse(raw);if(manifest.status!=='failed')return undefined;
 need(manifest.tested_head===request.head&&manifest.contract_digest===request.contractDigest&&manifest.nonce===request.nonce&&manifest.capture_scope==='reviewed-capture'&&manifest.physical_acceptance===false&&manifest.milestone_acceptance===false,'failed capture identity differs');
 const sealIndex=action.argv.indexOf('--reviewed-seal');need(sealIndex>=0&&action.argv.filter(x=>x==='--reviewed-seal').length===1,'independently pinned capture seal required');
 const sealPath=action.argv[sealIndex+1];need(typeof sealPath==='string'&&safe(relative(resolve(root,'.sandcastle/runs',request.runId),resolve(sealPath)))&&sha(policy.runtimeFiles?.[sealPath]),'private independently pinned seal path required');
 const sealRaw=await bytes(sealPath),seal=JSON.parse(sealRaw);need(digest(sealRaw)===policy.runtimeFiles[sealPath]&&same(manifest.reviewed_seal_identity,identity(sealRaw)),'capture seal changed');
 need(seal.tested_head===request.head&&seal.contract_digest===request.contractDigest&&seal.complete_consumed_input_seal===true&&seal.independent_review&&Object.keys(seal.sources??{}).length===390&&same(manifest.sources,seal.sources)&&same(manifest.independent_review,seal.independent_review),'source/reviewed seal differs');
 const paths=Object.keys(seal.sources).sort();need(same(paths,binding.inputPaths.slice().sort())&&same(paths,action.inventory?.files?.slice().sort())&&action.inventory?.roots?.length===0,'complete declared source closure required');
 const approved=policy.approvals?.find(p=>p.base===sourceApproval.reviewedHead&&p.tree===sourceApproval.reviewedTree&&p.actions?.includes(request.action)&&same(p.inputs,Object.fromEntries(Object.entries(seal.sources).map(([name,id])=>[name,id.sha256]))));need(approved,'manifest source closure has no independent approval');
 const tree=(await exec('git',['-C',root,'rev-parse',`${request.head}^{tree}`])).stdout.trim();need(tree===sourceApproval.executionTree,'execution tree differs from approved attempt');
 need(Array.isArray(manifest.consumption_graphs)&&manifest.consumption_graphs.length>0&&manifest.artifacts,'original consumption graph required');
 // Preserve ordinary game failures. A resource review never marks their suite
 // passed, and there is no error-string classification or provider prose parse.
 if(manifest.suites?.some(s=>s.status==='failed'||(Number.isInteger(s.returncode)&&s.returncode!==0)))return undefined;
 const missing={},graphs=[],seen=new Set();
 for(const record of manifest.consumption_graphs){
  need(/^[a-z][a-z0-9-]{0,63}$/.test(record.label??'')&&record.artifact===record.label+'.consumption.json.gz'&&!seen.has(record.artifact),'safe distinct graph artifact required');seen.add(record.artifact);
  const graphPath=resolve(job,'output',record.artifact),data=await bytes(graphPath);need(same(identity(data),manifest.artifacts[record.artifact]),'original graph SHA differs from failed manifest');
  const graph=JSON.parse(gunzipSync(data,{maxOutputLength:64*1024*1024}));need(Array.isArray(graph.unresolved_inputs)&&graph.unresolved_inputs.length===0&&graph.repository&&graph.external_files,'closed graph required before resource review');
  for(const[name,id]of Object.entries(graph.repository))need(Object.hasOwn(seal.sources,name)&&same(id,seal.sources[name]),'unsealed/changed repository consumption');
  const additions={};for(const[name,id]of Object.entries(graph.external_files)){
   if(Object.hasOwn(seal.external_files,name)){need(same(id,seal.external_files[name]),'previous consumed resource changed');continue;}
   need(name.startsWith('/usr/include/')&&name.endsWith('.h')&&resolve(name)===name&&Number.isSafeInteger(id.size)&&id.size>=0&&sha(id.sha256),'unknown resource class; no automatic host grant');
   const current=await bytes(name);need(same(identity(current),id),'observed public header changed');
   need(!Object.hasOwn(missing,name)||same(missing[name],id),'resource observations conflict');missing[name]=id;additions[name]=id;
  }
  if(Object.keys(additions).length)graphs.push({graphPath,graphSha256:digest(data),missing:additions,label:record.label});
 }
 if(!Object.keys(missing).length)return undefined;
 need(Object.keys(missing).length<=32&&graphs.length===1,'bounded single original missing-header graph required');
 const resourceProposal={...graphs[0],head:request.head,contractDigest:request.contractDigest,nonce:request.nonce,action:request.action,policyDigest,failedManifestPath:manifestPath,failedManifestSha256:digest(raw),approvedSourceProof:structuredClone(sourceApproval)};
 return {code:'HOST_APPROVAL_REQUIRED',deterministic:true,reason:'Independently review newly observed public compiler headers; failed capture remains pending.',hostApprovalRequest:{action:request.action,driver:request.driver,renderer:request.renderer},request:{code:'HOST_APPROVAL_REQUIRED',deterministic:true,runId:request.runId,wave:request.wave,ticket:request.ticket,branch:request.branch,baseCommit:request.baseCommit,head:request.head,action:request.action,driver:request.driver,renderer:request.renderer,contractDigest:request.contractDigest},resourceProposal,finalAcceptance:false};
}

export function normalizeControllerHostRequest(request){
 return {code:'HOST_APPROVAL_REQUIRED',deterministic:true,...Object.fromEntries(['runId','wave','ticket','branch','baseCommit','head','action','driver','renderer','contractDigest'].map(key=>[key,request[key]]))};
}
