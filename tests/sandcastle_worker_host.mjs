import test from 'node:test';
import assert from 'node:assert/strict';
import {workerHostDelivery,workerHostFailure} from '../.sandcastle/worker-host.mjs';
import {classifyFailure} from '../.sandcastle/failure.mjs';

test('completed prerequisite delivery remains readable when new collector source needs review',()=>{
 const record={ticket:'07.md',wave:7,branch:'worker07',head:'original-tested-head',deliveryDirectory:'/private/delivery'};
 const state={hostApprovalResumes:[record],completedTickets:['07.md']};
 const member={branch:'worker08',ticket:{path:'08.md',blockedBy:['07.md']}};
 assert.equal(workerHostDelivery(state,member,{id:8}),record);
 assert.equal(workerHostDelivery({...state,completedTickets:[]},member,{id:8}),undefined);
 assert.equal(workerHostDelivery(state,{...member,ticket:{...member.ticket,blockedBy:[]}},{id:8}),undefined);
 assert.equal(record.head,'original-tested-head');
});

test('structured request routes observed missing host approval directly to pause',async()=>{
 const calls=[];
 const channel={stats:{},observeApprovalRequest:async request=>{calls.push(request);throw Object.assign(new Error('Execution preflight [HOST_APPROVAL_REQUIRED]: unapproved action'),{code:'HOST_APPROVAL_REQUIRED'});}};
 const base={version:1,runId:'3fd6b311',wave:8,ticket:'08.md',branch:'codex/worker08',baseCommit:'a'.repeat(40),head:'b'.repeat(40),nonce:'c'.repeat(32),contractDigest:'d'.repeat(64)};
 const request={action:'entry-runtime-discovery',driver:'x11',renderer:'software'};
 const result={status:'blocked',summary:'Collectors delivered; controller review needed.',hostApprovalRequest:request};
 const failure=await workerHostFailure(channel,result,base);
 assert.deepEqual(failure,{code:'HOST_APPROVAL_REQUIRED',deterministic:true,request:{code:'HOST_APPROVAL_REQUIRED',deterministic:true,...Object.fromEntries(['runId','wave','ticket','branch','baseCommit','head','contractDigest'].map(k=>[k,base[k]])),...request}});
 assert.deepEqual(calls,[{...base,...request}]);
 assert.equal(classifyFailure(failure).automaticRetry,false);
 assert.equal(await workerHostFailure({...channel,observeApprovalRequest:async()=>({})},result,base),undefined);
 assert.equal(await workerHostFailure(channel,{status:'blocked',summary:'HOST_APPROVAL_REQUIRED'},base),undefined);
 assert.equal(await workerHostFailure(channel,{...result,status:'completed'},base),undefined);
 await assert.rejects(workerHostFailure(channel,{...result,hostApprovalRequest:{...request,argv:['shell']}},base),/Invalid structured/);
});

test('nullable host request preserves completed/blocked results without model-side review or channel authorization',async()=>{let calls=0;const channel={stats:{},observeApprovalRequest:async()=>{calls++;}};for(const status of ['completed','blocked'])assert.equal(await workerHostFailure(channel,{status,summary:'Focused result',hostApprovalRequest:null},{}),undefined);assert.equal(calls,0);});
