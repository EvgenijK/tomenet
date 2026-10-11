import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,readFile,rm,symlink} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {resolve} from 'node:path';
import {hostCollectorParts,mergeHostCollectorParts} from '../.sandcastle/host-collector-parts.mjs';
import {digest,publishEvidence,verifyEvidence} from '../.sandcastle/verification.mjs';
const binding={id:'fixture-automated',command:['python3','collector.py'],manifestPath:'manifest.json',inputPaths:['collector.py'],expectedSuites:{software:2,opengl:2},caseArtifactSuffix:'.log'};
const target={head:'a'.repeat(40),contractDigest:'b'.repeat(64),bindingDigest:'c'.repeat(64),controllerDigest:'d'.repeat(64),runtime:{kind:'fixture'}};
const specs=['software','opengl'].map(label=>({label,action:'capture-'+label,driver:'x11',renderer:label,suites:[label]}));
async function fixture(t){
 const root=await mkdtemp(resolve(tmpdir(),'sv-parts-'));t.after(()=>rm(root,{recursive:true,force:true}));
 const inbox=resolve(root,'inbox');await mkdir(inbox);
 const parts=[];
 for(const [index,spec] of specs.entries()){
  const output=resolve(root,spec.label);await mkdir(output);
  const nonce=String(index+1).repeat(32),manifest={tested_head:target.head,contract_digest:target.contractDigest,nonce,status:'passed',inputs_unchanged:true,complete_consumed_input_seal:true,physical_acceptance:false,sources:{'collector.py':{sha256:'e'.repeat(64),size:12}},suites:[{name:spec.label,status:'passed',returncode:0,caseCount:2,caseLogs:['send.log','no-send.log']}]};
  const docs={'manifest.json':JSON.stringify(manifest),'send.log':'real fixture trace\n','no-send.log':'no send fixture trace\n'};
  const receipt={head:target.head,contractDigest:target.contractDigest,nonce,action:spec.action,exitCode:0,policyDigest:'f'.repeat(64),output,artifacts:{}};
  for(const [name,data] of Object.entries(docs)){await writeFile(resolve(output,name),data);receipt.artifacts[name]={sha256:digest(data),size:Buffer.byteLength(data)};}
  parts.push({spec:structuredClone(spec),receipt});
 }
 return {root,inbox,binding,target,parts};
}
async function editManifest(part,edit){const path=resolve(part.receipt.output,'manifest.json'),m=JSON.parse(await readFile(path));edit(m);const data=JSON.stringify(m);await writeFile(path,data);part.receipt.artifacts['manifest.json']={size:Buffer.byteLength(data),sha256:digest(data)};}
test('fixed policy requires complete suite coverage before dispatch',()=>{
 const policy={hostParts:{[binding.id]:specs},actions:Object.fromEntries(specs.map(s=>[s.action,{controllerOnly:true,checkId:binding.id,bindingCommand:binding.command}]))};
 assert.deepEqual(hostCollectorParts(policy,binding),specs);
 for(const broken of [[],[specs[0]],[specs[0],specs[0]],[{...specs[0],suites:['unknown']},specs[1]]])assert.throws(()=>hostCollectorParts({...policy,hostParts:{[binding.id]:broken}},binding));
});
test('both immutable renderer parts pass through standard publisher and retain identities',async t=>{
 const f=await fixture(t),result=await mergeHostCollectorParts(f);
 const manifest=JSON.parse(await readFile(resolve(f.inbox,binding.manifestPath)));
 assert.deepEqual(manifest.suites.map(s=>s.caseLogs),[['software/send.log','software/no-send.log'],['opengl/send.log','opengl/no-send.log']]);
 assert.equal(manifest.parts.length,2);assert.equal(manifest.physical_acceptance,false);assert.equal(manifest.milestone_acceptance,false);
 const receipt=await publishEvidence({store:resolve(f.root,'store'),staging:f.inbox,target,checkId:binding.id,result,artifactPolicy:{syntheticOnly:true}});
 assert.equal((await verifyEvidence(receipt,target)).passed,true);
 assert.equal(Object.keys(JSON.parse(await readFile(receipt.receiptPath)).artifacts).length,7);
});
test('partial captures never produce final manifest',async t=>{
 const f=await fixture(t);f.parts.pop();await assert.rejects(mergeHostCollectorParts(f),/missing accepted suite/);await assert.rejects(readFile(resolve(f.inbox,'manifest.json')),{code:'ENOENT'});
});
test('stale head, failed exit and replayed part nonce cannot merge',async t=>{
 for(const mutate of [p=>p[1].receipt.head='0'.repeat(40),p=>p[1].receipt.exitCode=1,p=>p[1].receipt.nonce=p[0].receipt.nonce]){const f=await fixture(t);mutate(f.parts);await assert.rejects(mergeHostCollectorParts(f),/receipt identity/);}
});
test('lost no-send case, unsealed source and changed source inventory reject',async t=>{
 for(const edit of [m=>m.suites[0].caseLogs.pop(),m=>m.complete_consumed_input_seal=false,m=>m.sources.extra={sha256:'1'.repeat(64),size:1},m=>m.sources['collector.py'].sha256='0'.repeat(64)]){const f=await fixture(t);await editManifest(f.parts[1],edit);await assert.rejects(mergeHostCollectorParts(f));}
});
test('changed bytes and symlink artifacts reject even with successful producer claims',async t=>{
 const f=await fixture(t);await writeFile(resolve(f.parts[0].receipt.output,'send.log'),'forged');await assert.rejects(mergeHostCollectorParts(f),/changed private/);
 const g=await fixture(t),path=resolve(g.parts[0].receipt.output,'send.log');await rm(path);await symlink(resolve(g.parts[1].receipt.output,'send.log'),path);await assert.rejects(mergeHostCollectorParts(g),/regular private/);
});
test('entry matrix requires paired logical scenarios and renderer/session provenance',async t=>{
 for(const broken of [false,'renderer','session','category']){
  const f=await fixture(t);f.binding={...binding,id:'entry-automated'};
  for(const part of f.parts)await editManifest(part,m=>{
   m.renderer_part={renderer:part.spec.renderer,logical_scenarios:18,executions:18,sessions:36,full_execution_records:36,required_parts:['software','opengl'],final_acceptance:false};
   m.logical_reports=Array.from({length:18},(_,i)=>({route:'route',category:'case-'+i,executions:[{renderer:part.spec.renderer,sessions:[{},{}]}]}));
  });
  if(broken)await editManifest(f.parts[1],m=>{if(broken==='renderer')m.renderer_part.renderer='software';if(broken==='session')m.logical_reports[0].executions[0].sessions.pop();if(broken==='category')m.logical_reports[0].category='different';});
  if(broken)await assert.rejects(mergeHostCollectorParts(f));else {await mergeHostCollectorParts(f);const m=JSON.parse(await readFile(resolve(f.inbox,'manifest.json')));assert.equal(m.logical_reports.length,18);assert.ok(m.logical_reports.every(row=>row.executions.length===2));assert.equal(m.parts[1].renderer,'opengl');}
 }
});
