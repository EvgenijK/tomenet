import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,rm,symlink} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {resolve} from 'node:path';
import {createHash} from 'node:crypto';
import {workerReadonlyHostDelivery} from '../.sandcastle/host-readonly-delivery.mjs';
import {classifyFailure} from '../.sandcastle/failure.mjs';
const sha=bytes=>createHash('sha256').update(bytes).digest('hex');
async function fixture(t){
 const root=await mkdtemp(resolve(tmpdir(),'host-readonly-'));t.after(()=>rm(root,{recursive:true,force:true}));
 const directory=resolve(root,'.sandcastle/runs/example/host-delivery/capture');await mkdir(directory,{recursive:true});
 const targetScope={runId:'example',wave:8,ticket:'08.md',branch:'worker08'},scope={...targetScope,baseCommit:'base'};
 const data=Buffer.from('actual case/no-send evidence'),path=resolve(directory,'case.log');await writeFile(path,data);
 const index={version:1,purpose:'worker-delivery-readback',targetScope,tested_head:'original-head',contract_digest:'digest',finalAcceptance:false,sources:{'producer.py':{sha256:'producer-hash',size:12}},artifacts:{'case.log':{size:data.length,sha256:sha(data)}}};
 const descriptorPath=resolve(directory,'index.json');await writeFile(descriptorPath,JSON.stringify(index));
 const descriptorSha256=sha(Buffer.from(JSON.stringify(index))),calls=[];
 const policy={contractDigest:'digest',runtimeFiles:{[descriptorPath]:descriptorSha256},readonlyDeliveries:[{version:1,targetScope,directory,descriptorPath,descriptorSha256,sourceAction:'delivery-equivalence',review:{reviewer:'controller-independent-review',reference:'independent-review.json'}}]};
 const channel={authorize:async request=>{calls.push(request);return {approval:{inputs:{'producer.py':'producer-hash'}},sourceApproval:{reviewedHead:'original-head',executionTree:'bookkeeping-tree',method:'reviewed-carry-forward'}};}};
 return {root,policy,scope,head:'bookkeeping-head',channel,directory,path,descriptorPath,index,calls};
}
test('mount verifies original evidence and current source equivalence without host execution',async t=>{
 const f=await fixture(t),result=await workerReadonlyHostDelivery(f);
 assert.equal(result.head,'original-head');assert.equal(result.directory,f.directory);assert.equal(result.artifactCount,1);
 assert.equal(f.calls.length,1);assert.equal(f.calls[0].head,'bookkeeping-head');assert.equal(f.calls[0].action,'delivery-equivalence');
 assert.equal(await workerReadonlyHostDelivery({...f,scope:{...f.scope,wave:9}}),undefined);
});
test('corrupt artifact or changed descriptor cannot be mounted',async t=>{
 const f=await fixture(t);await writeFile(f.path,'altered');await assert.rejects(workerReadonlyHostDelivery(f),/changed capture artifact/);
 await writeFile(f.descriptorPath,'{}');await assert.rejects(workerReadonlyHostDelivery(f),/descriptor changed/);
});
test('source approval is required even when artifact bytes remain intact',async t=>{
 const f=await fixture(t);await assert.rejects(workerReadonlyHostDelivery({...f,channel:{authorize:async()=>{throw Object.assign(new Error('unreviewed producer'),{code:'HOST_APPROVAL_REQUIRED'});}}}),{code:'HOST_APPROVAL_REQUIRED'});
 await assert.rejects(workerReadonlyHostDelivery({...f,channel:{authorize:async()=>({approval:{inputs:{'producer.py':'changed'}},sourceApproval:{reviewedHead:'original-head'}})}}),/capture source differs/);
});
test('private bundle symlinks and unpinned descriptors are rejected',async t=>{
 const f=await fixture(t);await rm(f.path);await writeFile(resolve(f.directory,'elsewhere'),'actual case/no-send evidence');await symlink('elsewhere',f.path);
 await assert.rejects(workerReadonlyHostDelivery(f),/symlink path/);
 await assert.rejects(workerReadonlyHostDelivery({...f,policy:{...f.policy,runtimeFiles:{}}}),/pinned independently/);
});
test('readonly mount never exposes unreviewed extra files',async t=>{
 const f=await fixture(t);await writeFile(resolve(f.directory,'unreviewed.json'),'{}');
 await assert.rejects(workerReadonlyHostDelivery(f),/unlisted artifact/);
});
test('missing private evidence pauses before worker AI repair',async t=>{
 const f=await fixture(t);await rm(f.path);
 await assert.rejects(workerReadonlyHostDelivery(f),error=>error.code==='HOST_APPROVAL_REQUIRED'&&classifyFailure(error).automaticRetry===false);
});
