import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,writeFile,mkdir,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';import {resolve} from 'node:path';
import {digest} from '../.sandcastle/verification.mjs';
import {captureHostCollectorParts} from '../.sandcastle/host-collector-dispatch.mjs';
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
async function fixture(t){
 const root=await mkdtemp(resolve(tmpdir(),'host-dispatch-'));t.after(()=>rm(root,{recursive:true,force:true}));
 const seal=resolve(root,'reviewed-seal.json'),sealData=JSON.stringify({tested_head:'a'.repeat(40),contract_digest:'b'.repeat(64),complete_consumed_input_seal:true,independent_review:{reviewer:'fixture'}});await writeFile(seal,sealData);
 const paths=Array.from({length:390},(_,i)=>'source/'+i+'.c'),binding={id:'entry-runtime',command:['python3','collector.py'],expectedSuites:{ordinary:1,'recovery-surfaces':16},inputPaths:paths};
 const shards=Array.from({length:8},(_,i)=>'recovery-'+i),tasks=Array.from({length:6690},(_,i)=>({id:'recovery/task-'+i,kind:'recovery-surfaces',shard:shards[Math.floor(i*8/6690)]}));
 const plan={kind:'recovery',required_shards:shards,full_task_inventory:tasks,full_task_digest:digest(JSON.stringify(canonical(tasks))),selected_indices_by_shard:Object.fromEntries(shards.map(s=>[s,tasks.flatMap((task,i)=>task.shard===s?[i]:[])])),reviewedSealPath:seal,children:shards.map(label=>({label,action:'capture-'+label,driver:'x11',renderer:'software'}))};
 const specs=[{label:'baseline',action:'capture-baseline',driver:'x11',renderer:'software',suites:['ordinary']},{label:'recovery',action:'capture-recovery',driver:'x11',renderer:'software',suites:['recovery-surfaces'],constituents:plan}];
 const names=specs.map(s=>s.action).concat(plan.children.map(s=>s.action));
 const actions=Object.fromEntries(names.map(name=>[name,{controllerOnly:true,checkId:binding.id,bindingCommand:binding.command,inventory:{roots:[],files:paths},argv:['python3','collector.py','--runtime-part','recovery','--constituent',name.replace(/^capture-/,''),'--reviewed-seal',seal]}]));
 const policy={maxJobs:8,runtimeFiles:{[seal]:digest(sealData)},hostParts:{[binding.id]:specs},actions};
 const args={sealData,root,runDirectory:root,policyPath:resolve(root,'policy.json'),policy,binding,target:{head:'a'.repeat(40),contractDigest:'b'.repeat(64)},inbox:resolve(root,'inbox'),scope:{runId:'3fd6b311'},inputHashes:Object.fromEntries(paths.map(p=>[p,'c'.repeat(64)]))};await mkdir(args.inbox);
 return args;
}
test('fixed nine-job matrix keeps each channel at eight and folds with frozen identities',async t=>{
 const args=await fixture(t),channels=[],folds=[],merged=[];
 const startChannel=async()=>{const calls=[];const channel={calls,closed:false,request:async r=>{assert.ok(calls.length<8);calls.push(r);return {head:r.head,nonce:r.nonce};},close:async()=>{channel.closed=true;}};channels.push(channel);return channel;};
 const result=await captureHostCollectorParts(args,{startChannel,fold:async f=>{folds.push(f);return {aggregation:true,head:f.target.head};},merge:async f=>{merged.push(f);return {exitCode:0};}});
 assert.equal(result.exitCode,0);assert.deepEqual(channels.map(c=>c.calls.length),[1,8]);assert.ok(channels.every(c=>c.closed));assert.equal(new Set(channels.flatMap(c=>c.calls.map(r=>r.nonce))).size,9);
 assert.equal(folds.length,1);assert.deepEqual(folds[0].target.inputHashes,args.inputHashes);assert.deepEqual(folds[0].target.reviewedSealIdentity,{size:Buffer.byteLength(args.sealData),sha256:digest(args.sealData)});assert.deepEqual(folds[0].target.reviewedSeal,JSON.parse(args.sealData));assert.equal(merged[0].parts.length,2);assert.equal(merged[0].parts[1].receipt.aggregation,true);
});
test('failed child closes channel and cannot fold or merge partial matrix',async t=>{
 const args=await fixture(t);let requested=0,closed=0,folded=0,merged=0;
 await assert.rejects(captureHostCollectorParts(args,{startChannel:async()=>({request:async()=>{if(++requested===4)throw new Error('native job failure');return {};},close:async()=>{closed++;}}),fold:async()=>{folded++;},merge:async()=>{merged++;}}),/native job failure/);
 assert.equal(closed,2);assert.equal(folded,0);assert.equal(merged,0);
});
test('unpinned seal or a smaller channel guard rejects before any host execution',async t=>{
 for(const mutate of [a=>a.policy.runtimeFiles={},a=>a.policy.maxJobs=7]){const args=await fixture(t);mutate(args);let opened=0;await assert.rejects(captureHostCollectorParts(args,{startChannel:async()=>{opened++;}}),e=>e.code==='HOST_APPROVAL_REQUIRED');assert.equal(opened,0);}
});
