import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { mkdir, readFile, writeFile, readdir } from 'node:fs/promises';
import { resolve } from 'node:path';
import { randomUUID } from 'node:crypto';
import { assertTarget, digest, verificationError, publishEvidence, verifyEvidence, discoverEvidence, parseEvidenceJSON } from './verification.mjs';
import { assertExecutionPlan, requiredArtifactChecks } from './verification-plan.mjs';
const exec=promisify(execFile),git=async(cwd,...args)=>(await exec('git',args,{cwd})).stdout.trim();
const need=(condition,code,message)=>{if(!condition)throw verificationError(code,message);};
const quote=value=>`'${String(value).replaceAll("'","'\\''")}'`;
// Publication requires the current maintenance branch to be an ancestor. This
// guard is read-only: reconciling scope/history is a separate reviewed action.
export async function assertIntegrationBase(root,worktree,branch='modern_interface') {
  const mainHead=await git(root,'rev-parse',`refs/heads/${branch}`),integrationHead=await git(worktree,'rev-parse','HEAD');
  try { await git(worktree,'merge-base','--is-ancestor',mainHead,integrationHead); }
  catch(error) {
    if(error.code!==1)throw error;
    throw verificationError('INTEGRATION_BASE_DIVERGED',`${branch} ${mainHead} is not an ancestor of integration ${integrationHead}; separately review and merge maintenance history while stopped before freeze; preserve original baseCommit and contract`);
  }
  need(await git(root,'rev-parse',`refs/heads/${branch}`)===mainHead&&await git(worktree,'rev-parse','HEAD')===integrationHead,'INTEGRATION_BASE_CHANGED','publication revisions changed during preflight; repeat the read-only check');
  return {branch,mainHead,integrationHead};
}
export async function freezeVerificationTarget(state,worktree,{controllerDigest,runtime,publication}) {
  if(publication)await assertIntegrationBase(publication.root,worktree,publication.branch);
  need(!state.wave,'VERIFICATION_DELIVERY_PENDING','integrate all delivery before freeze');
  need(!(await git(worktree,'status','--porcelain')),'VERIFICATION_DIRTY','clean committed target required');
  const target=assertTarget({head:await git(worktree,'rev-parse','HEAD'),contractDigest:state.acceptance.contractDigest,bindingDigest:state.executionPlan.digest,controllerDigest,runtime});
  if(state.verificationTarget)need(digest(state.verificationTarget)===digest(target),'VERIFICATION_TARGET_CHANGED','frozen target differs; preserve old evidence and re-enter required gates');
  else state.verificationTarget=target;
  return target;
}
export async function assertVerificationTarget(state,worktree) {
  const target=assertTarget(state.verificationTarget);
  need(await git(worktree,'rev-parse','HEAD')===target.head&&!(await git(worktree,'status','--porcelain')),'VERIFICATION_TARGET_CHANGED','HEAD or committed inputs changed');
  need(state.acceptance.contractDigest===target.contractDigest&&state.executionPlan.digest===target.bindingDigest,'VERIFICATION_TARGET_CHANGED','contract or binding changed');
  return target;
}
export function releaseVerificationTarget(state) {
  if(state.verificationTarget)(state.verificationTargets??=[]).push(state.verificationTarget);
  delete state.verificationTarget;delete state.verifiedArtifactReferences;
}
export async function toolchainIdentity(sandbox) {
  const script=`const cp=require('node:child_process'),fs=require('node:fs'),crypto=require('node:crypto');const tools={};for(const [name,args] of [['python3',['-c','import sys,platform,json; print(json.dumps(dict(executable=sys.executable,version=sys.version,platform=platform.platform())))']],['cc',['--version']],[process.env.CC||'clang',['--version']],['pkg-config',['--modversion','sdl3','sdl3-ttf','freetype2','libsecret-1']]]){const r=cp.spawnSync(name,args,{encoding:'utf8'}),location=cp.spawnSync('which',[name],{encoding:'utf8'});if(r.status!==0||location.status!==0)process.exit(1);const path=fs.realpathSync(location.stdout.trim());tools[name]={version:r.stdout.trim(),path,sha256:crypto.createHash('sha256').update(fs.readFileSync(path)).digest('hex')};}console.log(JSON.stringify(tools));`;
  const output=await sandbox.exec('node -',{stdin:script});
  need(output.exitCode===0,'VERIFICATION_RUNTIME','required toolchain is unavailable');
  return JSON.parse(output.stdout);
}
async function inputIdentity(worktree,binding) {
  const paths=[...new Set(binding.inputPaths.concat(binding.command.filter(arg=>arg.startsWith('tests/'))))].sort();
  const inputs={};for(const path of paths)inputs[path]=digest(await readFile(resolve(worktree,path)));
  return inputs;
}
function validateCapture(manifest,binding,target) {
  need(manifest.tested_head===target.head&&manifest.contract_digest===target.contractDigest&&manifest.inputs_unchanged===true,'EVIDENCE_STALE','collector must attest unchanged exact target');
  need(Array.isArray(manifest.suites)&&manifest.suites.length===Object.keys(binding.expectedSuites).length,'EVIDENCE_INCOMPLETE','expected complete suite matrix');
  for(const [name,count] of Object.entries(binding.expectedSuites)) {
    const suite=manifest.suites.find(s=>s.name===name);need(suite?.returncode===0&&suite.status==='passed','EVIDENCE_INCOMPLETE',`suite ${name} did not pass`);
    // Case counts may be explicit in modern collectors. The existing B-024
    // collector has one log per session; no-send cases intentionally have no request.
    const cases=suite.caseCount??Object.keys(manifest.artifacts??{}).filter(path=>path.startsWith(name+'/')&&path.endsWith(binding.caseArtifactSuffix??'.bin')).length;
    need(cases===count,'EVIDENCE_INCOMPLETE',`suite ${name} count differs`);
  }
  need(manifest.sources&&manifest.artifacts&&Object.keys(manifest.artifacts).length>0,'EVIDENCE_INCOMPLETE','source/artifact inventory required');
}
export async function executeArtifactCheck(sandbox,state,binding,{runDirectory,save,assertRuntime}) {
  assertExecutionPlan(state);const target=await assertVerificationTarget(state,sandbox.worktreePath);
  if(assertRuntime)await assertRuntime(target);
  const store=resolve(runDirectory,'evidence');
  // A sealed orphan can be adopted after restart only after full identity/readback.
  const recovered=(await discoverEvidence(store,target,binding.id)).filter(r=>r.passed).at(-1);
  if(recovered)return recovered;
  const attemptId=randomUUID(),inbox=resolve(runDirectory,'evidence-inbox',attemptId);await mkdir(inbox,{recursive:true,mode:0o700});
  const inputs=await inputIdentity(sandbox.worktreePath,binding);
  state.artifactAttempt={checkId:binding.id,attemptId,target,inputs,status:'running'};await save(state);
  const command=[...binding.command,`/home/agent/.sandcastle-evidence-out/${attemptId}`].map(quote).join(' ');
  const result=await sandbox.exec(command);
  await assertVerificationTarget(state,sandbox.worktreePath);if(assertRuntime)await assertRuntime(target);
  need(digest(await inputIdentity(sandbox.worktreePath,binding))===digest(inputs),'EVIDENCE_STALE','runner inputs changed');
  // Do not retain arbitrary stdout/env/auth logs. Declared synthetic artifacts
  // are copied and inspected by the publisher; diagnostics remain code-only.
  const validateSnapshot = async snapshot => {
    const manifest=parseEvidenceJSON(await readFile(resolve(snapshot,binding.manifestPath),'utf8'));validateCapture(manifest,binding,target);
    for(const [path,expected]of Object.entries(manifest.sources)) {
      need(!path.startsWith('/')&&!path.split('/').includes('..'),'EVIDENCE_PATH','invalid source path');
      const bytes=await readFile(resolve(sandbox.worktreePath,path));need(bytes.length===expected.size&&digest(bytes)===expected.sha256,'EVIDENCE_STALE','collector source fingerprint differs');
    }
    for(const [path,expected]of Object.entries(manifest.artifacts)) {
      need(!path.startsWith('/')&&!path.split('/').includes('..'),'EVIDENCE_PATH','invalid artifact path');
      const bytes=await readFile(resolve(snapshot,path));need(bytes.length===expected.size&&digest(bytes)===expected.sha256,'EVIDENCE_INTEGRITY','collector artifact fingerprint differs');
    }
  };
  // Retain failed attempts even when a runner emitted no artifacts.
  if(!(await readdir(inbox)).length)await writeFile(resolve(inbox,'failure.json'),JSON.stringify({exitCode:result.exitCode})+'\n',{mode:0o600});
  const receipt=await publishEvidence({store,staging:inbox,target,checkId:binding.id,attemptId,producer:'controller',result:{exitCode:result.exitCode,command:binding.command,inputs,validation:result.exitCode===0?'passed':'failed'},artifactPolicy:{syntheticOnly:true},...(result.exitCode===0 ? {validateSnapshot} : {})});
  await assertVerificationTarget(state,sandbox.worktreePath);if(assertRuntime)await assertRuntime(target);
  await verifyEvidence(receipt,target);state.artifactAttempt={...state.artifactAttempt,status:'published',receipt};await save(state);
  return receipt;
}
export async function verifyRequiredArtifacts(state,worktree) {
  if(state.verificationVersion!==1)return;
  const target=await assertVerificationTarget(state,worktree);assertExecutionPlan(state);
  const references=[];
  for(const check of requiredArtifactChecks(state)) {
    const reference=(state.artifactEvidence??[]).filter(r=>r.checkId===check.id&&digest(r.target)===digest(target)).at(-1);
    need(reference,'EVIDENCE_MISSING',`required ${check.id} has no durable receipt`);
    const verified=await verifyEvidence(reference,target);need(verified.passed,'EVIDENCE_FAILED',`required ${check.id} failed`);references.push(verified);
  }
  await assertVerificationTarget(state,worktree);
  state.verifiedArtifactReferences=references;
  return references;
}
