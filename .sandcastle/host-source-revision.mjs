// Fixed one-test delegation. The controller derives the diff; the reviewer may
// approve it, but never supplies executable recipes or a replacement policy.
import {digest} from './verification.mjs';
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Controller host review: '+message),{code:'HOST_REVIEW_REFUSED',deterministic:true});};
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>JSON.stringify(canonical(a))===JSON.stringify(canonical(b));
const sourcePath='tests/sv_entry_collector_checks.py';
export const sourceRevisionEvidencePaths=['README.md','approval-status.json','collector-checks.log','consumption-checks.log','input-identities.json.gz','verification.json'].map(p=>'.scratch/sandcastle-3fd6b311/evidence/08-approved-inventory/'+p);
export function assertSourceRevisionDelegation(d){
 need(d&&Object.keys(d).sort().join(',')==='baseHead,candidateHead,evidenceFiles,newSource,oldSource,reference,reviewer,runId,sourcePath,ticket,version,wave'&&d.version===1&&d.runId==='3fd6b311'&&d.wave===8&&d.ticket==='.scratch/sandcastle-3fd6b311/issues/08-integration-collector.md'&&d.sourcePath===sourcePath&&/^[a-f0-9]{40}$/.test(d.baseHead)&&/^[a-f0-9]{40}$/.test(d.candidateHead)&&typeof d.reviewer==='string'&&d.reviewer.trim()&&typeof d.reference==='string'&&d.reference.trim(),'fixed independently reviewed one-test delegation required');
 const identity=v=>v&&Object.keys(v).sort().join(',')==='sha256,size'&&Number.isSafeInteger(v.size)&&v.size>0&&v.size<=1024*1024&&/^[a-f0-9]{64}$/.test(v.sha256);
 need(identity(d.oldSource)&&identity(d.newSource)&&!same(d.oldSource,d.newSource),'exact old/new one-test identities required');
 need(JSON.stringify(Object.keys(d.evidenceFiles??{}).sort())===JSON.stringify(sourceRevisionEvidencePaths.slice().sort())&&Object.values(d.evidenceFiles).every(v=>v&&Object.keys(v).sort().join(',')==='mode,sha256,size'&&v.mode==='100644'&&identity({size:v.size,sha256:v.sha256})),'exact six bounded evidence blob identities required');
 return d;
}
export async function deriveHostSourceRevision({state,request,profile,seal,identities,prior,git,blob}){
 const d=assertSourceRevisionDelegation(profile.sourceRevisionDelegation),approvedHead=seal.approved_source_head??seal.tested_head;
 need(request.runId===d.runId&&request.wave===d.wave&&request.ticket===d.ticket&&request.head===d.candidateHead&&approvedHead===d.baseHead&&!profile.resourceProposal,'delegation differs from current saved source scope');
 const changed=Object.keys(identities).filter(p=>!same(identities[p],seal.sources[p]));
 need(changed.length===1&&changed[0]===sourcePath&&same(seal.sources[sourcePath],d.oldSource)&&same(identities[sourcePath],d.newSource),'only the exact reviewed consumed test may change');
 need(prior.base===d.baseHead&&await git('rev-parse',`${prior.base}^{tree}`)===prior.tree,'previous independently approved source tree differs');
 const diff=(await git('diff','--name-status','--no-renames',d.baseHead,d.candidateHead)).split('\n').filter(Boolean).map(line=>line.split('\t'));
 const expected=new Map([[sourcePath,'M'],[d.ticket,'M'],...sourceRevisionEvidencePaths.map(p=>[p,'A'])]);
 need(diff.length===expected.size&&diff.every(([status,path,...rest])=>!rest.length&&expected.get(path)===status),'source revision has unrelated code, metadata or path changes');
 const evidence={};
 for(const path of sourceRevisionEvidencePaths){
  const mode=/^(100644) blob /.exec(await git('ls-tree',request.head,'--',path))?.[1],raw=await blob(request.head,path),actual={mode,size:raw.length,sha256:digest(raw)};
  need(same(actual,d.evidenceFiles[path]),'source revision evidence bytes/mode differ from independent review');evidence[path]=actual;
 }
 need(/^100644 blob /.test(await git('ls-tree',request.head,'--',sourcePath))&&/^100644 blob /.test(await git('ls-tree',request.head,'--',d.ticket)),'source revision requires regular nonexecutable test and ticket');
 return {kind:'one-test-source-revision',previousApprovedHead:d.baseHead,approvedSourceHead:request.head,changedSources:{[sourcePath]:{before:d.oldSource,after:d.newSource}},unchangedSourceCount:389,evidenceFiles:evidence,delegationDigest:digest(d),reviewReference:d.reference,finalAcceptance:false};
}
