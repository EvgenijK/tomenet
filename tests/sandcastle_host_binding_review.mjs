import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,readFile,symlink,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {resolve} from 'node:path';
import {execFileSync} from 'node:child_process';
import {digest} from '../.sandcastle/verification.mjs';
import {acceptExecutionPlan} from '../.sandcastle/verification-plan.mjs';
import {previewHostBinding,applyReviewedHostBinding} from '../.sandcastle/host-binding-review.mjs';
import {verificationCLI} from '../.sandcastle/verification-cli.mjs';

async function fixture(t){
 const root=await mkdtemp(resolve(tmpdir(),'sv-host-binding-'));t.after(()=>rm(root,{recursive:true,force:true}));
 for(const dir of ['tests','src','.sandcastle/runs/1234abcd'])await mkdir(resolve(root,dir),{recursive:true});
 await writeFile(resolve(root,'package.json'),'{}');await writeFile(resolve(root,'package-lock.json'),'{}');
 await writeFile(resolve(root,'tests/collector.py'),'# shipped collector\n');await writeFile(resolve(root,'src/input.h'),'#define INPUT 1\n');
 await symlink('collector.py',resolve(root,'tests/symlink.py'));
 const git=(...args)=>execFileSync('git',['-C',root,...args],{encoding:'utf8'}).trim();
 git('init','-q');git('add','tests','src');git('-c','user.name=Test','-c','user.email=test@example.invalid','commit','-qm','collector');
 const branch=git('branch','--show-current'),head=git('rev-parse','HEAD');
 const contract={criteria:[{id:'runtime',requirement:'Provide controlled-peer runtime evidence',mandatory:true,applicability:'current'}]};
 const state={id:'1234abcd',phase:'awaiting',resumePhase:'parallel-work',currentFailure:{diagnosticCode:'HOST_APPROVAL_REQUIRED'},
  completedTickets:['07.md'],tickets:[],budget:{consumedPercent:18,limitPercent:25,tokens:999},buildAttempts:1,testAttempts:0,reviewRound:0,recoveryAttempts:{saved:3},
  acceptance:{contract,contractDigest:digest(contract)},wave:{id:8,members:[{branch,head,status:'blocked',integrated:false}]}};
 acceptExecutionPlan(state,{version:1,contractDigest:state.acceptance.contractDigest,criteria:[{id:'runtime',evidence:'artifact',artifactCheckIds:['capture']}],artifactChecks:[{id:'capture',owner:'controller',phase:'test-gate',target:'integrated-head',command:['python3','-B','tests/collector.py'],outputArgument:true,manifestPath:'manifest.json',syntheticOnly:true,caseArtifactSuffix:'.log',expectedSuites:{routes:3},inputPaths:['tests/obsolete.py']}]},{approved:true,summary:'initial accepted plan',missingRequirements:[]});
 const plan=structuredClone(state.executionPlan.plan);plan.artifactChecks[0].inputPaths=['src/input.h','tests/collector.py'];
 const review={...await previewHostBinding(root,state),approved:true,reviewer:'controller-independent-review',branch,head,summary:'Verified delivered collector closure; preserve all runtime obligations',plan,inputs:{'src/input.h':digest(await readFile(resolve(root,'src/input.h'))),'tests/collector.py':digest(await readFile(resolve(root,'tests/collector.py')))}};
 return {root,state,review};
}
test('reviewed path reconciliation archives original checkpoint, preserves obligations and stays stopped',async t=>{
 const {root,state,review}=await fixture(t),before=structuredClone(state);
 const file=resolve(root,'.sandcastle/runs/1234abcd/state.json');await writeFile(file,JSON.stringify(state));
 const record=resolve(root,'review.json');await writeFile(record,JSON.stringify(review));
 const oldLog=console.log;console.log=()=>{};
 try{await verificationCLI(['binding-apply',state.id,'--root',root,'--authorization','reviewed-paths-1','--binding-review',record]);}finally{console.log=oldLog;}
 const next=JSON.parse(await readFile(file));assert.equal(next.phase,'awaiting');assert.equal(next.resumePhase,'parallel-work');
 for(const key of ['acceptance','completedTickets','wave','budget','buildAttempts','testAttempts','reviewRound','recoveryAttempts','currentFailure'])assert.deepEqual(next[key],before[key]);
 assert.deepEqual(next.executionPlan.plan.artifactChecks[0].inputPaths,review.plan.artifactChecks[0].inputPaths);
 assert.equal(next.hostBindingReviews[0].allowanceGranted,false);assert.equal(next.hostBindingReviews[0].finalAcceptance,false);
 assert.deepEqual(JSON.parse(await readFile(resolve(root,'.sandcastle/runs/1234abcd/verification-before-reviewed-paths-1.json'))),before);
 await assert.rejects(readFile(resolve(root,'.sandcastle/runs/1234abcd/run.lock')),error=>error.code==='ENOENT');
 assert.deepEqual(state,before);
});
test('binding reconciliation refuses scope weakening, forged inputs, stale review and frozen/quota checkpoints atomically',async t=>{
 const {root,state,review}=await fixture(t);
 for(const change of [
  r=>r.plan.criteria[0].evidence='inspection',
  r=>r.plan.artifactChecks[0].expectedSuites.routes=1,
  r=>r.plan.artifactChecks[0].command=['bash','tests/collector.py'],
  r=>r.inputs['src/input.h']='0'.repeat(64),
  r=>delete r.inputs['src/input.h'],
  r=>r.head='0'.repeat(40),
  r=>r.controllerDigest='0'.repeat(64),
  r=>{r.plan.artifactChecks[0].inputPaths.push('tests/symlink.py');r.inputs['tests/symlink.py']=digest('collector.py');}
 ]){
  const invalid=structuredClone(review),before=structuredClone(state);change(invalid);
  await assert.rejects(applyReviewedHostBinding(root,state,{authorizationId:'rejected-change',review:invalid}));assert.deepEqual(state,before);
 }
 for(const change of [s=>s.verificationTarget={head:'frozen'},s=>s.budget.consumedPercent=25,s=>s.phase='test-gate']){
  const invalid=structuredClone(state);change(invalid);const before=structuredClone(invalid);
  await assert.rejects(applyReviewedHostBinding(root,invalid,{authorizationId:'rejected-checkpoint',review}));assert.deepEqual(invalid,before);
 }
});
