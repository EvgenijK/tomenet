// Only the private, independently reviewed policy can delegate tree transitions.
// This proves equivalence for delivery; it never approves changed producers or
// substitutes old delivery evidence for capture at the final assembled HEAD.
import { createHash } from 'node:crypto';
const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
const safe=path=>typeof path==='string'&&path.length>0&&!path.startsWith('/')&&path.split('/').every(part=>part&&part!=='.'&&part!=='..');
function entries(output){
 const result=new Map();
 for(const record of output.split('\0').filter(Boolean)){
  const match=/^(\d{6}) (blob|tree|commit) ([a-f0-9]+)\t([\s\S]+)$/.exec(record);
  if(!match||!safe(match[4]))throw new Error('Linux host: invalid committed tree entry');
  result.set(match[4],{mode:match[1],type:match[2],blob:match[3]});
 }return result;
}
export async function deliverySourceApproval({git,readBlob,action,approval,head,binding}){
 const tree=await git('rev-parse',`${head}^{tree}`);
 if(approval.tree===tree)return {method:'exact-tree',reviewedHead:approval.base,reviewedTree:tree,executionTree:tree};
 const rules=action.carryForward;
 // Final binding is strict even when a delivery action delegates transitions.
 if(binding||!action.sourceWorktree||rules?.version!==1||!approval.tree)return undefined;
 if(await git('rev-parse',`${approval.base}^{tree}`)!==approval.tree)return undefined;
 const paths=rules.metadataPaths??[],roots=rules.metadataRoots??[],reviewed=rules.reviewedFiles??{};
 if(!Array.isArray(paths)||!Array.isArray(roots)||!paths.every(safe)||!roots.every(safe)||!Object.keys(reviewed).every(safe))return undefined;
 const before=entries(await git('ls-tree','-rz',approval.tree));
 const after=entries(await git('ls-tree','-rz',tree)),changes=[];
 for(const path of [...new Set([...before.keys(),...after.keys()])].sort()){
  const old=before.get(path),next=after.get(path);
  if(JSON.stringify(old)===JSON.stringify(next))continue;
  // Input hashes/inventory were checked by the host channel. Still reject any
  // overlapping rule instead of granting metadata status to consumed inputs.
  if(Object.hasOwn(approval.inputs??{},path))return undefined;
  const metadata=paths.includes(path)||roots.some(root=>path.startsWith(root+'/'));
  let reason;
  if(metadata&&/\.(md|json)$/.test(path)&&[old,next].filter(Boolean).every(entry=>entry.type==='blob'&&entry.mode==='100644'))reason='declared-metadata';
  else {
   const versions=reviewed[path];
   if(!next||next.type!=='blob'||!['100644','100755'].includes(next.mode)||!Array.isArray(versions))return undefined;
   const digest=hash(await readBlob(head,path));
   if(!versions.some(version=>version.mode===next.mode&&version.sha256===digest))return undefined;
   reason='independently-reviewed-file';
  }
  changes.push({path,before:old??null,after:next??null,reason});
 }
 if(!changes.length)return undefined;
 return {method:'reviewed-carry-forward',reviewedHead:approval.base,reviewedTree:approval.tree,executionTree:tree,changes};
}
