import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, readFile, symlink } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { resolve } from 'node:path';
import { execFileSync } from 'node:child_process';
import { createHash } from 'node:crypto';
import { startLinuxHostChannel } from './linux-host.mjs';
import { requestLinuxHost } from './linux-host-client.mjs';
const hash=b=>createHash('sha256').update(b).digest('hex');
async function fixture(options={}){
  const root=await mkdtemp(resolve(tmpdir(),'sv-host-test-'));
  execFileSync('git',['init','-q',root]);
  await writeFile(resolve(root,'input.txt'),'approved source');
  await writeFile(resolve(root,'runner.mjs'),options.runner??`import {mkdir,writeFile} from 'node:fs/promises';const out=process.argv[2];await mkdir(out,{recursive:true});await writeFile(out+'/report.json',JSON.stringify({status:'ready',tested_head:process.argv[3],contract_digest:process.argv[4],nonce:process.argv[5]}));`);
  execFileSync('git',['-C',root,'add','.']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','fixture']);
  const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim(),digest='d'.repeat(64);
  const policy={version:1,runId:'run-test',contractDigest:digest,launcherCheckout:root,launcherHead:head,launcherFiles:{'runner.mjs':hash(await readFile(resolve(root,'runner.mjs')))},approvals:[{base:head,inputs:{'input.txt':hash(Buffer.from('approved source'))},actions:['readiness']}],actions:{readiness:{argv:[process.execPath,resolve(root,'runner.mjs'),'$OUTPUT','$HEAD','$DIGEST','$NONCE'],timeoutMs:options.timeoutMs??5000,requiredArtifacts:['report.json'],inventory:{roots:['src'],files:['input.txt']}}},runtimeFiles:{[process.execPath]:hash(await readFile(process.execPath))},review:{reviewer:'independent-test',reference:'fixture'},maxJobs:8};
  const policyPath=resolve(root,'policy.json');await writeFile(policyPath,JSON.stringify(policy),{mode:0o600});
  const pinned=root+'-launcher';execFileSync('git',['clone','-q',root,pinned]);policy.launcherCheckout=pinned;policy.actions.readiness.argv[1]=resolve(pinned,'runner.mjs');await writeFile(policyPath,JSON.stringify(policy),{mode:0o600});
  if(options.capture){
    const inputs={'input.txt':hash(await readFile(resolve(root,'input.txt'))),'runner.mjs':hash(await readFile(resolve(root,'runner.mjs')))};
    policy.actions.capture={controllerOnly:true,checkId:'capture-test',bindingCommand:['node','runner.mjs'],argv:[process.execPath,'$SOURCE/runner.mjs','$OUTPUT','$HEAD','$DIGEST','$NONCE'],timeoutMs:5000,requiredArtifacts:['report.json'],inventory:{roots:['.'],files:[]}};
    if(options.delivery){policy.actions.delivery={...policy.actions.capture,controllerOnly:false,checkId:undefined,bindingCommand:undefined,sourceWorktree:true,inventory:{roots:['src'],files:['input.txt','runner.mjs']},carryForward:options.carryForward};}
    policy.approvals.push({base:head,tree:options.withoutTree?undefined:execFileSync('git',['-C',root,'rev-parse','HEAD^{tree}']).toString().trim(),inputs,actions:options.delivery?['capture','delivery']:['capture']});await writeFile(policyPath,JSON.stringify(policy),{mode:0o600});
  }
  const scope={runId:'run-test',wave:7,ticket:'ticket07',branch:'master',baseCommit:head};
  const channel=await startLinuxHostChannel({root,policyPath,scope,directory:resolve(root,'channel')});
  const request={version:1,...scope,head,contractDigest:digest,nonce:'1'.repeat(32),action:'readiness',driver:'wayland',renderer:'software'};
  return {root,channel,request,policy};
}
test('bounded job returns artifacts and rejects replay and unapproved source',async()=>{
  const {root,channel,request}=await fixture();try{
    const result=await channel.request(request);assert.equal(result.exitCode,0);assert.equal(JSON.parse(await readFile(resolve(result.output,'report.json'))).tested_head,request.head);
    await assert.rejects(channel.request(request),/replay/);
    await assert.rejects(channel.request({...request,nonce:'2'.repeat(32),action:'shell',command:'touch pwned'}),/fields|action/);
    await assert.rejects(channel.request({...request,nonce:'3'.repeat(32),contractDigest:'0'.repeat(64)}),/digest/);
    await writeFile(resolve(root,'input.txt'),'unreviewed');execFileSync('git',['-C',root,'add','input.txt']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','change']);
    const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();await assert.rejects(channel.request({...request,nonce:'4'.repeat(32),head}),/approval/);
  }finally{await channel.close();}
});

test('controller carries reviewed delivery approval across declared bookkeeping and executes current HEAD',async()=>{
 const {root,channel,request}=await fixture({capture:true,delivery:true,carryForward:{version:1,metadataPaths:['docs/progress.md']}});
 try{
  await mkdir(resolve(root,'docs'));await writeFile(resolve(root,'docs/progress.md'),'Ticket07 completed; ticket08 claimed.');
  execFileSync('git',['-C',root,'add','docs/progress.md']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','bookkeeping']);
  const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();
  const {version,head:oldHead,nonce,action,driver,renderer,...scope}=request;
  const receipt=await requestLinuxHost({directory:channel.directory,scope,head,nonce,action:'delivery',driver,renderer,output:resolve(root,'carry-return')});
  assert.equal(receipt.exitCode,0);assert.equal(JSON.parse(await readFile(resolve(root,'carry-return/report.json'))).tested_head,head);
  assert.equal(receipt.sourceApproval.method,'reviewed-carry-forward');assert.equal(receipt.sourceApproval.reviewedHead,oldHead);
  assert.deepEqual(receipt.sourceApproval.changes.map(entry=>entry.path),['docs/progress.md']);
  const binding={id:'capture-test',command:['node','runner.mjs']};
  await assert.rejects(channel.authorize({...request,head,action:'capture'},binding),/approval|tree unreviewed/);
 }finally{await channel.close();}
});
test('new consumed file and stale branch HEAD fail independent source approval',async()=>{
 const {root,channel,request}=await fixture();try{
  await assert.rejects(channel.request({...request,nonce:'5'.repeat(32),head:'0'.repeat(40)}),/HEAD differs/);
  await mkdir(resolve(root,'src'));await writeFile(resolve(root,'src/new-header.h'),'unapproved');execFileSync('git',['-C',root,'add','.']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','added input']);
  const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();await assert.rejects(channel.request({...request,head,nonce:'6'.repeat(32)}),/approval/);assert.equal(channel.stats.executed,0);
 }finally{await channel.close();}
});
test('public file client roundtrip validates returned bytes and identity',async()=>{
 const {root,channel,request}=await fixture();try{
  const {version,head,nonce,action,driver,renderer,...scope}=request;
  const result=await requestLinuxHost({directory:channel.directory,scope,head,nonce,action,driver,renderer,output:resolve(root,'returned')});assert.equal(result.exitCode,0);
  const badNonce='7'.repeat(32),bad={...result,nonce:badNonce,files:{'report.json':Buffer.from('tampered').toString('base64')}};
  await writeFile(resolve(channel.directory,'results',badNonce+'.json'),JSON.stringify(bad));
  await assert.rejects(requestLinuxHost({directory:channel.directory,scope,head,nonce:badNonce,action,driver,renderer,output:resolve(root,'tampered-return')}),/incomplete\/changed/);
 }finally{await channel.close();}
});
test('queue rejects symlinks and does not follow request paths',async()=>{
 const {root,channel,request}=await fixture();try{
  await writeFile(resolve(root,'external.json'),JSON.stringify(request));await symlink(resolve(root,'external.json'),resolve(channel.directory,'requests',request.nonce+'.json'));
  await new Promise(r=>setTimeout(r,250));assert.equal(channel.stats.executed,0);
 }finally{await channel.close();}
});

test('incomplete and symlink artifacts fail; timeout terminates a bounded job',async()=>{
 for(const options of [
  {runner:"import {mkdir,writeFile} from 'node:fs/promises';await mkdir(process.argv[2]);await writeFile(process.argv[2]+'/other.json','incomplete');"},
  {runner:"import {mkdir,symlink} from 'node:fs/promises';await mkdir(process.argv[2]);await symlink('/etc/passwd',process.argv[2]+'/report.json');"},
  {runner:"setTimeout(()=>{},30000);",timeoutMs:50}
 ]){const {channel,request}=await fixture(options);try{await assert.rejects(channel.request(request),/incomplete|nonregular|timed out/);}finally{await channel.close();}}
});

test('controller capture uses real detached Git source, fresh inbox and independent tree approval',async()=>{
 const runner="import {mkdir,writeFile} from 'node:fs/promises';import {execFileSync} from 'node:child_process';const actual=execFileSync('git',['rev-parse','HEAD']).toString().trim();if(actual!==process.argv[3])throw Error('wrong actual Git target');await mkdir(process.argv[2]);await writeFile(process.argv[2]+'/report.json',JSON.stringify({tested_head:actual}));";
 const binding={id:'capture-test',command:['node','runner.mjs']};
 const {root,channel,request}=await fixture({capture:true,runner});try{
  const job={...request,action:'capture'};await assert.rejects(channel.request(job),/controller-only/);
  await assert.rejects(channel.authorize(job,{...binding,command:['node','unreviewed.mjs']}),/binding/);
  const output=resolve(root,'inbox');await mkdir(output);const receipt=await channel.request(job,{binding,output});assert.equal(receipt.exitCode,0);assert.equal(JSON.parse(await readFile(resolve(output,'report.json'))).tested_head,request.head);
 }finally{await channel.close();}
 const unreviewed=await fixture({capture:true,withoutTree:true,runner});try{await assert.rejects(unreviewed.channel.authorize({...unreviewed.request,action:'capture'},binding),/tree unreviewed/);assert.equal(unreviewed.channel.stats.executed,0);}finally{await unreviewed.channel.close();}
});

test('reviewed delivery job uses committed real Git source without frozen final binding',async()=>{
 const runner="import {mkdir,writeFile} from 'node:fs/promises';import {execFileSync} from 'node:child_process';const head=execFileSync('git',['rev-parse','HEAD']).toString().trim();if(head!==process.argv[3])throw Error('wrong source');await mkdir(process.argv[2]);await writeFile(process.argv[2]+'/report.json',JSON.stringify({head,finalAcceptance:false}));";
 const {channel,request}=await fixture({capture:true,delivery:true,runner});try{const receipt=await channel.request({...request,action:'delivery'});assert.equal(receipt.exitCode,0);assert.equal(JSON.parse(await readFile(resolve(receipt.output,'report.json'))).head,request.head);}finally{await channel.close();}
 const unapproved=await fixture({capture:true,delivery:true,withoutTree:true,runner});try{await assert.rejects(unapproved.channel.request({...unapproved.request,action:'delivery'}),/tree unreviewed/);assert.equal(unapproved.channel.stats.executed,0);}finally{await unapproved.channel.close();}
});

test('carry-forward refuses unreviewed code, new inputs, symlinks and executable metadata',async()=>{
 for(const change of ['outside','input','runner','new-input','symlink','executable','missing-delegation']){
  const {root,channel,request}=await fixture({capture:true,delivery:true,carryForward:change==='missing-delegation'?undefined:{version:1,metadataRoots:['docs']}});
  try{
   await mkdir(resolve(root,'docs'));
   if(change==='input')await writeFile(resolve(root,'input.txt'),'changed');
   else if(change==='runner')await writeFile(resolve(root,'runner.mjs'),'unreviewed producer');
   else if(change==='new-input'){await mkdir(resolve(root,'src'));await writeFile(resolve(root,'src/header.h'),'new consumed file');}
   else if(change==='symlink')await symlink('../runner.mjs',resolve(root,'docs/progress.md'));
   else if(change==='outside')await writeFile(resolve(root,'new-runner.py'),'unreviewed code outside inventory');
   else await writeFile(resolve(root,'docs/progress.md'),'bookkeeping',{mode:change==='executable'?0o755:0o644});
   // Never stage controller-owned policy/channel files into the test tree.
   execFileSync('git',['-C',root,'add','input.txt','runner.mjs','docs',...(change==='new-input'?['src']:[]),...(change==='outside'?['new-runner.py']:[])]);
   execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm',change]);
   const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();
   await assert.rejects(channel.request({...request,head,action:'delivery'}),error=>error.code==='HOST_APPROVAL_REQUIRED',change);
   assert.equal(channel.stats.executed,0);assert.equal(channel.stats.approvalFailure.code,'HOST_APPROVAL_REQUIRED');
  }finally{await channel.close();}
 }
});

test('controller files outside producer inputs need exact independently reviewed bytes and modes',async()=>{
 const code='reviewed controller version';
 const {root,channel,request}=await fixture({capture:true,delivery:true,carryForward:{version:1,reviewedFiles:{'.sandcastle/controller.mjs':[{mode:'100644',sha256:hash(Buffer.from(code))}]}}});
 try{
  await mkdir(resolve(root,'.sandcastle'));await writeFile(resolve(root,'.sandcastle/controller.mjs'),code);
  execFileSync('git',['-C',root,'add','.sandcastle/controller.mjs']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','reviewed controller']);
  const head=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();
  const approved=await channel.authorize({...request,head,action:'delivery'});assert.equal(approved.sourceApproval.changes[0].reason,'independently-reviewed-file');
  await writeFile(resolve(root,'.sandcastle/controller.mjs'),'worker modified controller');
  execFileSync('git',['-C',root,'add','.sandcastle/controller.mjs']);execFileSync('git',['-C',root,'-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','unreviewed controller']);
  const changed=execFileSync('git',['-C',root,'rev-parse','HEAD']).toString().trim();
  await assert.rejects(channel.authorize({...request,head:changed,action:'delivery'}),error=>error.code==='HOST_APPROVAL_REQUIRED');
 }finally{await channel.close();}
});
