// Sequential fixed host groups keep each channel's existing job guard. A fold
// belongs to the controller; original host receipts are never relabelled.
import {constants} from 'node:fs';
import {mkdir,open,realpath,lstat} from 'node:fs/promises';
import {resolve} from 'node:path';
import {randomUUID} from 'node:crypto';
import {digest} from './verification.mjs';
import {startLinuxHostChannel} from './linux-host.mjs';
import {hostCollectorParts,mergeHostCollectorParts} from './host-collector-parts.mjs';
import {hostCollectorConstituents,mergeHostCollectorConstituents} from './host-collector-constituents.mjs';
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Execution preflight [HOST_APPROVAL_REQUIRED]: '+message),{code:'HOST_APPROVAL_REQUIRED',deterministic:true});};
async function sealIdentity(policy,path){
 try{return await readSealIdentity(policy,path);}catch(error){if(error.code==='HOST_APPROVAL_REQUIRED')throw error;need(false,'reviewed constituent seal unavailable');}
}
async function readSealIdentity(policy,path){
 need(typeof path==='string'&&await realpath(path)===path,'constituent seal path differs');
 const before=await lstat(path);need(before.isFile()&&!before.isSymbolicLink()&&before.nlink===1&&before.size<=24*1024*1024,'bounded regular constituent seal required');
 const file=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);
 try{
  const opened=await file.stat(),bytes=await file.readFile(),after=await file.stat();
  need(opened.ino===before.ino&&opened.dev===before.dev&&after.size===before.size&&after.size===bytes.length&&after.mtimeMs===before.mtimeMs&&after.ctimeMs===before.ctimeMs,'constituent seal changed while reading');
  const identity={size:bytes.length,sha256:digest(bytes)};
  need(policy.runtimeFiles?.[path]===identity.sha256,'constituent seal is not independently pinned');return {identity,seal:JSON.parse(bytes)};
 }finally{await file.close();}
}
export async function captureHostCollectorParts({root,policyPath,policy,binding,target,inbox,scope,runDirectory,inputHashes},
 {startChannel=startLinuxHostChannel,fold=mergeHostCollectorConstituents,merge=mergeHostCollectorParts}={}){
 const parts=hostCollectorParts(policy,binding),captured=[];
 // Validate every group before executing any host job.
 const planned=[];
 for(const spec of parts){
  const children=hostCollectorConstituents(policy,binding,spec);
  need((children?.length??1)<=policy.maxJobs,'fixed constituent group exceeds channel guard');
  const reviewed=children?await sealIdentity(policy,spec.constituents.reviewedSealPath):undefined;
  if(reviewed)need(reviewed.seal.tested_head===target.head&&reviewed.seal.contract_digest===target.contractDigest&&reviewed.seal.complete_consumed_input_seal===true&&reviewed.seal.independent_review,'constituent seal target/review differs');
  planned.push({spec,children,reviewedSealIdentity:reviewed?.identity,reviewedSeal:reviewed?.seal});
 }
 for(const {spec,children,reviewedSealIdentity,reviewedSeal}of planned){
  const channel=await startChannel({root,policyPath,scope,directory:resolve(runDirectory,'host-channels',randomUUID())});
  try{
   const request=child=>({version:1,...scope,head:target.head,contractDigest:target.contractDigest,nonce:randomUUID().replaceAll('-',''),action:child.action,driver:child.driver,renderer:child.renderer});
   if(!children){
    const receipt=await channel.request(request(spec),{binding,...(parts.length===1?{output:inbox}:{})});
    if(parts.length===1)return receipt;captured.push({spec,receipt});
   }else{
    const returned=[];for(const child of children)returned.push({spec:child,receipt:await channel.request(request(child),{binding})});
    await mkdir(resolve(runDirectory,'host-folds'),{recursive:true,mode:0o700});const output=resolve(runDirectory,'host-folds',randomUUID());
    const receipt=await fold({inbox:output,binding,target:{...target,inputHashes,reviewedSealIdentity,reviewedSeal},part:spec,children:returned});captured.push({spec,receipt});
   }
  }finally{await channel.close();}
 }
 return await merge({inbox,binding,target,parts:captured});
}
