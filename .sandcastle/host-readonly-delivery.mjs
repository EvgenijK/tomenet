// A private controller review may expose delivery bytes to a worker. This is
// inspection only: original capture identities survive, and final gates stay
// bound to the separately assembled and frozen source.
import {lstat,open,realpath,readdir} from 'node:fs/promises';
import {resolve,relative,isAbsolute} from 'node:path';
import {constants} from 'node:fs';
import {createHash} from 'node:crypto';
import {execFile} from 'node:child_process';
import {promisify} from 'node:util';
import {normalizeControllerHostRequest} from './host-capture-failure.mjs';
const exec=promisify(execFile);
const sha=bytes=>createHash('sha256').update(bytes).digest('hex');
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_APPROVAL_REQUIRED]: Readonly host delivery: '+message),{code:'HOST_APPROVAL_REQUIRED',deterministic:true});};
const safe=path=>typeof path==='string'&&path&&!isAbsolute(path)&&path.split('/').every(part=>part&&part!=='.'&&part!=='..');
async function bytes(path,maximum){
 need(await realpath(path)===path,'symlink path');
 const metadata=await lstat(path);need(metadata.isFile()&&!metadata.isSymbolicLink()&&metadata.nlink===1&&metadata.size<=maximum,'regular bounded artifact required');
 const file=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);
 try{const before=await file.stat(),data=await file.readFile(),after=await file.stat();need(before.ino===metadata.ino&&before.dev===metadata.dev&&before.size===data.length&&after.size===before.size&&after.mtimeMs===before.mtimeMs&&after.ctimeMs===before.ctimeMs,'artifact changed while reading');return data;}finally{await file.close();}
}
export async function workerReadonlyHostDelivery(args){
 try{return await reviewedDelivery(args);}catch(error){if(error.code==='HOST_APPROVAL_REQUIRED')throw error;need(false,'reviewed private capture bytes unavailable');}
}
async function reviewedDelivery({root,policy,scope,head,channel}){
 const records=(policy.readonlyDeliveries??[]).filter(record=>['runId','wave','ticket','branch'].every(key=>record.targetScope?.[key]===scope[key]));
 need(records.length<=1,'ambiguous reviewed target');if(!records.length)return undefined;
 const record=records[0];need(record.version===1&&record.review?.reviewer==='controller-independent-review'&&record.review.reference,'independent review required');
 const privateRoot=resolve(root,'.sandcastle/runs',scope.runId,'host-delivery'),directory=resolve(record.directory),descriptor=resolve(record.descriptorPath);
 need(safe(relative(privateRoot,directory))&&safe(relative(directory,descriptor))&&await realpath(directory)===directory,'private delivery directory required');
 need(/^[a-f0-9]{64}$/.test(record.descriptorSha256??'')&&policy.runtimeFiles?.[descriptor]===record.descriptorSha256,'descriptor must be pinned independently');
 const data=await bytes(descriptor,4*1024*1024);need(sha(data)===record.descriptorSha256,'descriptor changed');
 const index=JSON.parse(data);need(index.version===1&&index.purpose==='worker-delivery-readback'&&index.finalAcceptance===false&&index.contract_digest===policy.contractDigest&&['runId','wave','ticket','branch'].every(key=>index.targetScope?.[key]===scope[key]),'wrong delivery identity or scope');
 let proof;
 if(record.mode==='historical-only'){
  need(/^[a-f0-9]{40}$/.test(index.tested_head)&&index.sources&&Object.keys(index.sources).length===390,'original390 historical source identities required');
  const prior=policy.approvals?.find(a=>a.base===index.tested_head&&a.actions?.includes(record.sourceAction)&&Object.keys(a.inputs??{}).length===390&&Object.entries(a.inputs).every(([p,id])=>index.sources[p]?.sha256===id));
  need(prior&&/^[a-f0-9]{40}$/.test(prior.tree),'original independent source approval required');
  const git=async(...args)=>(await exec('git',['-C',root,...args],{maxBuffer:8*1024*1024})).stdout.trim();
  need(await git('rev-parse',`${index.tested_head}^{tree}`)===prior.tree,'original source tree differs');
  for(const[path,id]of Object.entries(index.sources)){
   need(safe(path)&&!/[\r\n\0]/.test(path)&&Number.isSafeInteger(id.size)&&/^[a-f0-9]{64}$/.test(id.sha256),'bounded historical source path/identity');
   need(/^(100644|100755) blob /.test(await git('ls-tree',index.tested_head,'--',path)),'regular original source blob required');
   const raw=(await exec('git',['-C',root,'show',`${index.tested_head}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;
   need(raw.length===id.size&&sha(raw)===id.sha256,'original historical source differs');
  }
  proof={sourceApproval:{method:'historical-original-source',reviewedHead:prior.base,reviewedTree:prior.tree},approval:prior};
 }else{
  need(record.mode===undefined,'unknown readonly inspection mode');
  const observation={version:1,...scope,head,contractDigest:policy.contractDigest,nonce:'0'.repeat(32),action:record.sourceAction,driver:'x11',renderer:'software'};
  try{proof=await channel.authorize(observation);}catch(error){if(error.code==='HOST_APPROVAL_REQUIRED')error.request=normalizeControllerHostRequest(observation);throw error;}
  need(proof.sourceApproval?.reviewedHead===index.tested_head&&index.sources&&Object.keys(index.sources).length===Object.keys(proof.approval.inputs).length&&Object.entries(proof.approval.inputs).every(([path,hash])=>index.sources[path]?.sha256===hash),'reviewed capture source differs');
 }
 const files=Object.entries(index.artifacts??{});need(files.length>0&&files.length<=50000,'bounded artifact inventory required');let total=0;
 const declared=new Set([descriptor,...files.map(([path])=>{need(safe(path),'artifact path');return resolve(directory,path);})]),observed=new Set();
 async function inspect(path){for(const entry of await readdir(path,{withFileTypes:true})){const full=resolve(path,entry.name);need(!entry.isSymbolicLink(),'symlink path');if(entry.isDirectory())await inspect(full);else{need(entry.isFile()&&declared.has(full),'unlisted artifact');observed.add(full);need(observed.size<=50001,'bounded artifact inventory required');}}}
 await inspect(directory);need(observed.size===declared.size,'missing artifact');
 for(const [path,identity]of files){need(safe(path)&&resolve(directory,path)!==descriptor&&Number.isSafeInteger(identity.size)&&identity.size>=0,'artifact path or size');total+=identity.size;need(total<=192*1024*1024,'delivery bundle size');const content=await bytes(resolve(directory,path),24*1024*1024);need(content.length===identity.size&&sha(content)===identity.sha256,'changed capture artifact');}
 return {directory,descriptor:relative(directory,descriptor),head:index.tested_head,sourceApproval:proof.sourceApproval,artifactCount:files.length,...(record.mode==='historical-only'?{historicalOnly:true,currentSourceEquivalence:false,finalAcceptance:false}:{})};
}
