// A private controller review may expose delivery bytes to a worker. This is
// inspection only: original capture identities survive, and final gates stay
// bound to the separately assembled and frozen source.
import {lstat,open,realpath} from 'node:fs/promises';
import {resolve,relative,isAbsolute} from 'node:path';
import {constants} from 'node:fs';
import {createHash} from 'node:crypto';
const sha=bytes=>createHash('sha256').update(bytes).digest('hex');
const need=(ok,message)=>{if(!ok)throw new Error('Readonly host delivery: '+message);};
const safe=path=>typeof path==='string'&&path&&!isAbsolute(path)&&path.split('/').every(part=>part&&part!=='.'&&part!=='..');
async function bytes(path,maximum){
 need(await realpath(path)===path,'symlink path');
 const metadata=await lstat(path);need(metadata.isFile()&&!metadata.isSymbolicLink()&&metadata.nlink===1&&metadata.size<=maximum,'regular bounded artifact required');
 const file=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);
 try{const before=await file.stat(),data=await file.readFile(),after=await file.stat();need(before.ino===metadata.ino&&before.dev===metadata.dev&&before.size===data.length&&after.size===before.size&&after.mtimeMs===before.mtimeMs&&after.ctimeMs===before.ctimeMs,'artifact changed while reading');return data;}finally{await file.close();}
}
export async function workerReadonlyHostDelivery({root,policy,scope,head,channel}){
 const records=(policy.readonlyDeliveries??[]).filter(record=>['runId','wave','ticket','branch'].every(key=>record.targetScope?.[key]===scope[key]));
 need(records.length<=1,'ambiguous reviewed target');if(!records.length)return undefined;
 const record=records[0];need(record.version===1&&record.review?.reviewer==='controller-independent-review'&&record.review.reference,'independent review required');
 const privateRoot=resolve(root,'.sandcastle/runs',scope.runId,'host-delivery'),directory=resolve(record.directory),descriptor=resolve(record.descriptorPath);
 need(safe(relative(privateRoot,directory))&&safe(relative(directory,descriptor))&&await realpath(directory)===directory,'private delivery directory required');
 need(/^[a-f0-9]{64}$/.test(record.descriptorSha256??'')&&policy.runtimeFiles?.[descriptor]===record.descriptorSha256,'descriptor must be pinned independently');
 const data=await bytes(descriptor,4*1024*1024);need(sha(data)===record.descriptorSha256,'descriptor changed');
 const index=JSON.parse(data);need(index.version===1&&index.purpose==='worker-delivery-readback'&&index.finalAcceptance===false&&index.contract_digest===policy.contractDigest&&['runId','wave','ticket','branch'].every(key=>index.targetScope?.[key]===scope[key]),'wrong delivery identity or scope');
 const proof=await channel.authorize({version:1,...scope,head,contractDigest:policy.contractDigest,nonce:'0'.repeat(32),action:record.sourceAction,driver:'x11',renderer:'software'});
 need(proof.sourceApproval?.reviewedHead===index.tested_head&&index.sources&&Object.keys(index.sources).length===Object.keys(proof.approval.inputs).length&&Object.entries(proof.approval.inputs).every(([path,hash])=>index.sources[path]?.sha256===hash),'reviewed capture source differs');
 const files=Object.entries(index.artifacts??{});need(files.length>0&&files.length<=50000,'bounded artifact inventory required');let total=0;
 for(const [path,identity]of files){need(safe(path)&&resolve(directory,path)!==descriptor&&Number.isSafeInteger(identity.size)&&identity.size>=0,'artifact path or size');total+=identity.size;need(total<=192*1024*1024,'delivery bundle size');const content=await bytes(resolve(directory,path),24*1024*1024);need(content.length===identity.size&&sha(content)===identity.sha256,'changed capture artifact');}
 return {directory,descriptor:relative(directory,descriptor),head:index.tested_head,sourceApproval:proof.sourceApproval,artifactCount:files.length};
}
