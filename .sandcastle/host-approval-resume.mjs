// One corrected controller-owned prerequisite, not another recovery allowance.
import { readFile, writeFile, mkdir, lstat, realpath, open } from 'node:fs/promises';
import { resolve, relative, isAbsolute, dirname } from 'node:path';
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { createHash, randomUUID } from 'node:crypto';
import { taskQuotaLimit } from './limits.mjs';
const exec=promisify(execFile),hash=bytes=>createHash('sha256').update(bytes).digest('hex');
export const hostCheckpointFingerprint=state=>hash(JSON.stringify(state));
const need=(ok,message)=>{if(!ok)throw new Error(`Host approval resume: ${message}`);};
async function regular(path){const stat=await lstat(path);need(stat.isFile()&&!stat.isSymbolicLink()&&stat.nlink===1&&await realpath(path)===resolve(path),'nonregular/symlink input');return readFile(path);}
const routes=['existing-character','existing-account-create','new-account-create'];
const categories=['keyboard-text','composition','mouse','resize','minimize','restore'];
const validated=new WeakSet();
export async function validateHostApprovalResume(root,state,proof,{controllerDigest,policyPath}){
 need(proof?.version===1&&proof.authorization==='user-requested-host-approval-correction','explicit scoped authorization required');
 need(proof.runId===state.id&&proof.stateFingerprint===hostCheckpointFingerprint(state),'checkpoint fingerprint changed');
 need(hash(await regular(resolve(root,'.sandcastle/runs',state.id,'state.json')))===proof.rawStateSha256,'raw saved checkpoint changed');
 need(state.phase==='awaiting'&&state.resumePhase==='parallel-work'&&state.pauseReason==='Automatic recovery made no progress after three decisions at this checkpoint'&&!state.recovery,'requires the stopped exhausted worker checkpoint');
 need(state.wave?.id===7&&state.wave.members.length===1,'requires original single-member wave7');
 const member=state.wave.members[0],key=['parallel-work',7,state.completedTickets.length,state.reviewRound,state.buildAttempts,state.testAttempts].join(':');
 need(state.recoveryAttempts?.[key]===3&&member.status==='blocked'&&!member.integrated&&hash(member.result?.summary??'')==='d621dbcc233e25130a65de1741c94139f1031f79f8b4d66ddef6ab56554dc552','requires the recorded controller approval blocker, not arbitrary exhausted work');
 need(!state.quotaError&&(state.budget?.consumedPercent??0)<taskQuotaLimit(state),'remaining quota required');
 need(proof.controllerDigest===controllerDigest&&proof.contractDigest===state.acceptance?.contractDigest&&proof.bindingDigest===state.executionPlan?.digest,'controller/contract/binding identity differs');
 const branchHead=(await exec('git',['-C',root,'rev-parse',`refs/heads/${member.branch}`])).stdout.trim();
 need(proof.previousHead===member.head&&proof.head===branchHead,'retained worker HEAD changed');
 await exec('git',['-C',root,'merge-base','--is-ancestor',member.head,proof.head]);
 await exec('git',['-C',root,'merge-base','--is-ancestor',state.wave.baseCommit,proof.head]);
 const policyBytes=await regular(policyPath),policy=JSON.parse(policyBytes);
 need(hash(policyBytes)===proof.policyDigest&&policy.runId===state.id&&policy.contractDigest===proof.contractDigest&&policy.review?.reviewer&&policy.review?.reference,'independent active policy identity differs');
 for(const [path,expected]of Object.entries(policy.runtimeFiles??{}))need(hash(await regular(await realpath(path)))===expected,'approved host tool changed');
 for(const [path,expected]of Object.entries(policy.launcherFiles??{}))need(!isAbsolute(path)&&path.split('/').every(part=>part&&part!=='.'&&part!=='..')&&hash(await regular(resolve(policy.launcherCheckout,path)))===expected,'approved launcher changed');
 if(policy.launcherCheckout)need((await exec('git',['-C',policy.launcherCheckout,'rev-parse','HEAD'])).stdout.trim()===policy.launcherHead,'approved launcher HEAD changed');
 need(policy.actions?.interaction?.sourceWorktree===true,'reviewed delivery action absent');
 const git=async(...args)=>(await exec('git',['-C',root,...args])).stdout.trim();
 const tree=await git('rev-parse',`${proof.head}^{tree}`),action=policy.actions.interaction;
 need(action.inventory?.roots&&action.inventory?.files,'delivery input inventory absent');
 const consumed=(await git('ls-tree','-r','--name-only',proof.head,'--',...action.inventory.roots,...action.inventory.files)).split('\n').filter(Boolean).sort();
 let approvedInputs;
 for(const candidate of policy.approvals??[]){
  if(!candidate.actions?.includes('interaction')||candidate.tree!==tree)continue;
  try{await git('merge-base','--is-ancestor',candidate.base,proof.head);}catch{continue;}
  if(JSON.stringify(consumed)!==JSON.stringify(Object.keys(candidate.inputs??{}).sort()))continue;
  let matches=consumed.length>0;
  for(const path of consumed){
   need(!isAbsolute(path)&&path.split('/').every(part=>part&&part!=='.'&&part!=='..'),'delivery input path escape');
   if(!/^(100644|100755) blob /.test(await git('ls-tree',proof.head,'--',path))){matches=false;break;}
   const bytes=(await exec('git',['-C',root,'show',`${proof.head}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;
   if(hash(bytes)!==candidate.inputs[path]){matches=false;break;}
  }
  if(matches){approvedInputs=candidate.inputs;break;}
 }
 need(approvedInputs,'exact reviewed whole tree/input approval absent');
 const reviewPath=resolve(root,'.sandcastle/runs',state.id,proof.reviewFile??'');
 need(relative(resolve(root,'.sandcastle/runs',state.id),reviewPath)===proof.reviewFile&&proof.reviewFile&&!proof.reviewFile.includes('..'),'review path escape');
 const reviewBytes=await regular(reviewPath),review=JSON.parse(reviewBytes);
 need(hash(reviewBytes)===proof.reviewDigest&&review.approved===true&&review.reviewer&&review.head===proof.head&&review.policyDigest===proof.policyDigest,'independent concrete source/policy review required');
 need(!state.hostApprovalResumes?.some(record=>record.head===proof.head&&record.wave===7),'correction already used');
 need(Array.isArray(proof.receipts)&&proof.receipts.length===2,'software and OpenGL delivery receipts required');
 need(['wayland','x11'].includes(proof.driver),'actual approved Linux video driver required');
 const seen=new Set(),records=[];
 for(const entry of proof.receipts){
  need(/^[a-f0-9]{32}$/.test(entry.nonce),'invalid receipt nonce');const job=resolve(root,'.sandcastle/runs',state.id,'host-jobs',entry.nonce);
  const bytes=await regular(resolve(job,'receipt.json')),receipt=JSON.parse(bytes);
  need(hash(bytes)===entry.sha256&&receipt.head===proof.head&&receipt.contractDigest===proof.contractDigest&&receipt.nonce===entry.nonce&&receipt.action==='interaction'&&receipt.exitCode===0&&receipt.policyDigest===proof.policyDigest,'delivery receipt identity differs');
  need(receipt.scope.runId===state.id&&receipt.scope.wave===7&&receipt.scope.branch===member.branch&&receipt.scope.ticket===member.ticket.path&&receipt.scope.baseCommit===state.wave.baseCommit,'receipt scope differs');
  const output=resolve(job,'output');need(receipt.output===output,'private receipt output differs');
  for(const [path,identity]of Object.entries(receipt.artifacts??{})){
   need(!isAbsolute(path)&&path.split('/').every(part=>part&&part!=='.'&&part!=='..'),'artifact path escape');const artifact=await regular(resolve(output,path));need(artifact.length===identity.size&&hash(artifact)===identity.sha256,'returned artifact changed');
  }
  need(receipt.artifacts?.['interaction-report.json'],'interaction report absent');
  const report=JSON.parse(await regular(resolve(output,'interaction-report.json')));
  need(report.kind==='linux-synthetic-interaction-producer'&&report.head===proof.head&&report.contract_digest===proof.contractDigest&&report.nonce===entry.nonce&&report.status==='passed'&&report.physical_acceptance===false&&report.milestone_acceptance===false,'passed delivery report required');
  need(report.inputs_before&&typeof report.inputs_before==='object'&&Object.keys(report.inputs_before).length>0&&report.inputs_after&&report.runtime_before&&report.runtime_after&&report.runtime_before.tools&&report.runtime_before.libraries&&Object.keys(report.runtime_before.tools).length>0&&Object.keys(report.runtime_before.libraries).length>0,'source/runtime identity absent');
  for(const [path,expected]of Object.entries({...report.runtime_before.tools,...report.runtime_before.libraries}))need(hash(await regular(await realpath(path)))===expected,'captured runtime changed');
  need(JSON.stringify(Object.keys(report.inputs_before).sort())===JSON.stringify(Object.keys(approvedInputs).sort())&&Object.entries(approvedInputs).every(([path,value])=>report.inputs_before[path]===value),'captured source differs from reviewed Git inputs');
  need(JSON.stringify(report.inputs_before)===JSON.stringify(report.inputs_after)&&JSON.stringify(report.runtime_before)===JSON.stringify(report.runtime_after),'source/runtime changed');
  need(report.reports?.length===18,'complete renderer delivery scenarios required');let renderer;
  const scenarios=new Set();for(const row of report.reports){
   need(routes.includes(row.route)&&categories.includes(row.category)&&row.executions?.length===1,'unexpected route/category/renderer');
   const execution=row.executions[0];renderer??=execution.renderer;need(execution.renderer===renderer&&execution.status==='passed'&&execution.sessions?.length===2,'incomplete renderer/build execution');
   need(JSON.stringify(execution.sessions.map(session=>session.order).sort())==='[0,1]'&&execution.sessions.every(session=>session.status==='passed'&&session.video===proof.driver&&session.renderer===renderer&&session.route===row.route&&session.category===row.category),'both legal build sessions and actual video required');
   scenarios.add(row.route+'/'+row.category);
  }
  need(scenarios.size===18&&['software','opengl'].includes(renderer)&&!seen.has(renderer),'missing/repeated renderer matrix');seen.add(renderer);records.push({nonce:entry.nonce,receiptDigest:entry.sha256,renderer});
 }
 const record={wave:7,head:proof.head,previousHead:proof.previousHead,driver:proof.driver,branch:member.branch,ticket:member.ticket.path,policyDigest:proof.policyDigest,reviewDigest:proof.reviewDigest,controllerDigest,contractDigest:proof.contractDigest,bindingDigest:proof.bindingDigest,stateFingerprint:proof.stateFingerprint,rawStateSha256:proof.rawStateSha256,receipts:records,allowanceGranted:false};
 for(const receipt of record.receipts)Object.freeze(receipt);Object.freeze(record.receipts);Object.freeze(record);validated.add(record);return record;
}
export async function applyHostApprovalResume(root,state,record){
 need(validated.has(record)&&hostCheckpointFingerprint(state)===record.stateFingerprint,'validated correction/checkpoint required');
 const directory=resolve(root,'.sandcastle/runs',state.id);need(hash(await regular(resolve(directory,'state.json')))===record.rawStateSha256,'raw checkpoint changed before archive');const archive=resolve(directory,`host-approval-before-${randomUUID()}.json`),file=await open(archive,'wx',0o600);
 try{await file.writeFile(JSON.stringify(state,null,2)+'\n');await file.sync();}finally{await file.close();}
 const dir=await open(directory,'r');try{await dir.sync();}finally{await dir.close();}
 // Publish only synthetic declared artifacts for the readonly worker handoff.
 // Private source workspaces/policy/other run metadata never enter this mount.
 const deliveryDirectory=resolve(directory,'host-delivery',randomUUID());await mkdir(deliveryDirectory,{recursive:true,mode:0o700});
 const published=[];
 for(const entry of record.receipts){
  const job=resolve(directory,'host-jobs',entry.nonce),receiptBytes=await regular(resolve(job,'receipt.json'));need(hash(receiptBytes)===entry.receiptDigest,'receipt changed before handoff');const receipt=JSON.parse(receiptBytes);
  const destination=resolve(deliveryDirectory,entry.renderer);await mkdir(destination,{mode:0o700});
  for(const [path,expected]of Object.entries(receipt.artifacts)){
   const bytes=await regular(resolve(job,'output',path));need(hash(bytes)===expected.sha256&&bytes.length===expected.size,'artifact changed before handoff');const target=resolve(destination,path);await mkdir(dirname(target),{recursive:true,mode:0o700});await writeFile(target,bytes,{mode:0o600});
  }await writeFile(resolve(destination,'channel-receipt.json'),receiptBytes,{mode:0o600});published.push({...entry,report:entry.renderer+'/interaction-report.json',receipt:entry.renderer+'/channel-receipt.json'});
 }
 await writeFile(resolve(deliveryDirectory,'index.json'),JSON.stringify({...record,receipts:published,deliveryOnly:true,finalAcceptance:false},null,2)+'\n',{mode:0o600});
 (state.hostApprovalResumes??=[]).push({...record,archive,deliveryDirectory,verifiedAt:new Date().toISOString()});state.phase='parallel-work';delete state.resumePhase;delete state.pauseReason;delete state.currentFailure;state.updatedAt=new Date().toISOString();return state;
}
