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
 const channel={stats:{},authorize:async request=>{calls.push(request);throw Object.assign(new Error('Execution preflight [HOST_APPROVAL_REQUIRED]: unapproved action'),{code:'HOST_APPROVAL_REQUIRED'});}};
 const base={version:1,runId:'test',head:'committed-collector',nonce:'fresh'};
 const request={action:'entry-runtime-discovery',driver:'x11',renderer:'software'};
 const result={status:'blocked',summary:'Collectors delivered; controller review needed.',hostApprovalRequest:request};
 const failure=await workerHostFailure(channel,result,base);
 assert.deepEqual(failure,{code:'HOST_APPROVAL_REQUIRED',deterministic:true});
 assert.deepEqual(calls,[{...base,...request}]);
 assert.equal(classifyFailure(failure).automaticRetry,false);
 assert.equal(await workerHostFailure({...channel,authorize:async()=>({})},result,base),undefined);
 assert.equal(await workerHostFailure(channel,{status:'blocked',summary:'HOST_APPROVAL_REQUIRED'},base),undefined);
 assert.equal(await workerHostFailure(channel,{...result,status:'completed'},base),undefined);
 await assert.rejects(workerHostFailure(channel,{...result,hostApprovalRequest:{...request,argv:['shell']}},base),/Invalid structured/);
});
