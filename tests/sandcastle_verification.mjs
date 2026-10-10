import assert from 'node:assert/strict';
import test from 'node:test';
import { mkdtemp, mkdir, writeFile, readFile, rm, chmod } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';

test('controller retains sealed evidence after producer teardown and rejects changed bytes', async t => {
  const { publishEvidence, verifyEvidence } = await import('../.sandcastle/verification.mjs');
  const dir = await mkdtemp(join(tmpdir(), 'sandcastle-evidence-')); t.after(() => rm(dir,{recursive:true,force:true}));
  const staging = join(dir,'producer'); await mkdir(staging); await writeFile(join(staging,'trace.txt'),'synthetic trace\n');
  const target = { head:'a'.repeat(40), contractDigest:'b'.repeat(64), bindingDigest:'c'.repeat(64), controllerDigest:'d'.repeat(64), runtime:{kind:'fixture',imageId:'sha256:'+ 'e'.repeat(64)} };
  const receipt = await publishEvidence({ store:join(dir,'store'), staging, target, checkId:'birth', attemptId:'first', result:{exitCode:0}, producer:'controller', artifactPolicy:{syntheticOnly:true} });
  await rm(staging,{recursive:true});
  assert.equal((await verifyEvidence(receipt,target)).passed,true);
  await chmod(join(receipt.bundlePath,'trace.txt'),0o600);
  await writeFile(join(receipt.bundlePath,'trace.txt'),'corrupted');
  await assert.rejects(verifyEvidence(receipt,target),/EVIDENCE_INTEGRITY/);
});

test('a runtime contract cannot be delivered as inspection-only acceptance', async () => {
  const { validateExecutionPlan } = await import('../.sandcastle/verification-plan.mjs');
  const contract={criteria:[{id:'RUNTIME',requirement:'Provide controlled-peer runtime evidence',mandatory:true,applicability:'current',checkIds:['controller-review']}],checks:[{id:'controller-review',kind:'review'}]};
  assert.throws(()=>validateExecutionPlan({version:1,contractDigest:'b'.repeat(64),criteria:[{id:'RUNTIME',evidence:'inspection',artifactCheckIds:[]}],artifactChecks:[]},contract,'b'.repeat(64)),/PLAN_EVIDENCE_MISSING/);
});

test('a later Git commit invalidates a frozen target before capture', async t => {
  const { freezeVerificationTarget, assertVerificationTarget }=await import('../.sandcastle/verification-gates.mjs');
  const { execFileSync }=await import('node:child_process');
  const dir=await mkdtemp(join(tmpdir(),'sandcastle-target-'));t.after(()=>rm(dir,{recursive:true,force:true}));
  const git=(...args)=>execFileSync('git',args,{cwd:dir,encoding:'utf8'}).trim();
  git('init','-q');git('config','user.name','Fixture');git('config','user.email','fixture@example.invalid');
  await writeFile(join(dir,'production.txt'),'first');git('add','.');git('commit','-qm','first');
  const state={acceptance:{contractDigest:'b'.repeat(64)},executionPlan:{digest:'c'.repeat(64)}};
  await freezeVerificationTarget(state,dir,{controllerDigest:'d'.repeat(64),runtime:{kind:'fixture'}});
  await writeFile(join(dir,'production.txt'),'changed');git('add','.');git('commit','-qm','changed');
  await assert.rejects(assertVerificationTarget(state,dir),/VERIFICATION_TARGET_CHANGED/);
});

test('a controller delivery receipt can advance assembly without falsely resolving acceptance', async () => {
  const { collectWave }=await import('../.sandcastle/parallel.mjs');
  const ticket={path:'.scratch/sandcastle-test/issues/09.md',blockedBy:[]};
  const state={id:'test',phase:'assemble',completedTickets:[],tickets:[ticket],ticketIndex:0,acceptance:{contractDigest:'b'.repeat(64),contract:{criteria:[{id:'AC',applicability:'current'}]}},executionPlan:{digest:'c'.repeat(64)}};
  const { acceptDelivery }=await import('../.sandcastle/delivery.mjs');
  const receipt=acceptDelivery(state,{revision:'a'.repeat(40),ticketPath:ticket.path,ticketDigest:'d'.repeat(64),changeDigest:'e'.repeat(64),review:{approved:true,localPredicatesPassed:true,head:'a'.repeat(40),ticketDigest:'d'.repeat(64),pendingCriterionIds:['AC']}});
  state.wave={id:1,after:'test-gate',members:[{ticket,status:'blocked',head:receipt.revision,result:{status:'blocked',summary:'independent acceptance pending'},deliveryReceipt:receipt}]};
  let assembled=0;
  await collectWave(state,{save:async()=>{},assemble:async()=>{assembled++;return{status:'completed',summary:'merged delivery'};},verify:async()=>{}});
  assert.equal(assembled,1);assert.equal(state.phase,'test-gate');assert.deepEqual(state.completedTickets,[]);
  assert.deepEqual(state.deliveredTickets,[ticket.path]);
});

test('incident migration preview keeps stopped checkpoint, contract, counters and open finding', async () => {
  const { previewIncidentMigration }=await import('../.sandcastle/verification-migration.mjs');
  const root=join(import.meta.dirname,'..');
  const artifact=JSON.parse(await readFile(join(import.meta.dirname,'fixtures/sandcastle-0241e0da-contract.json'),'utf8'));
  // Recorded read-only Git adapter: this regression must also run in a shallow
  // clone without the unmerged feature worker objects. Other tests use real Git.
  const ops={readTicket:async()=> 'Type: verification\nStatus: claimed\n## Answer\nCommitted documentation delivery; independent acceptance remains pending.\n',git:async(_root,...args)=>{
    if(args[0]==='rev-parse')return args[1].includes('wave-')?'48edfae35ce46996dd8c5337dacf6ecd5afe90b5':'72fac69a58b7875f1d812980312acaa63573d4c9';
    if(args[0]==='diff'&&args[1]==='--name-only')return '.scratch/sandcastle-0241e0da/issues/09-final-head-runtime-evidence.md';
    if(args[0]==='diff')return 'recorded synthetic documentation handoff delta';
    if(args[0]==='ls-tree')return 'src/client/sv/endpoint-run.c';
    return '';
  }};
  const ticket={path:'.scratch/sandcastle-0241e0da/issues/09-final-head-runtime-evidence.md',blockedBy:[]};
  const state={id:'0241e0da',phase:'awaiting',resumePhase:'parallel-work',pauseReason:'Recovery agent requested a stop',branch:'codex/sandcastle-dev-0241e0da',completedTickets:[],tickets:[ticket],budget:{consumedPercent:17,limitPercent:25,tokens:1234},buildAttempts:2,testAttempts:1,reviewRound:1,recoveryAttempts:{saved:2},acceptance:{contract:artifact.contract,contractDigest:artifact.digest,ledger:[{id:'D-9496281ca15e0741',status:'open',repairs:0}]},wave:{id:6,after:'tickets',baseCommit:'72fac69a58b7875f1d812980312acaa63573d4c9',members:[{ticket,status:'blocked',head:'48edfae35ce46996dd8c5337dacf6ecd5afe90b5',branch:'codex/sandcastle-0241e0da-wave-6-1',result:{status:'blocked',summary:'independent controller acceptance pending'}}]}};
  const original=structuredClone(state);const preview=await previewIncidentMigration(root,state,ops);
  assert.deepEqual(state,original);assert.equal(preview.version,1);assert.equal(preview.targetHead,state.wave.members[0].head);
  assert.equal(preview.executionPlan.contractDigest,artifact.digest);assert.equal(preview.executionPlan.artifactChecks[0].expectedSuites.versions,40);assert.equal(preview.executionPlan.artifactChecks[0].caseArtifactSuffix,'.log');
  assert.equal(preview.pendingCriterionIds.includes('P1-ac8'),true);
  const { applyIncidentMigration, resumeVerificationMigration }=await import('../.sandcastle/verification-migration.mjs');
  const bindingReview={approved:true,summary:'independent fixture binding coverage',missingRequirements:[],bindingDigest:preview.bindingDigest,head:preview.targetHead};
  const deliveryReview={approved:true,localPredicatesPassed:true,summary:'independent fixture local delivery',head:preview.targetHead,ticketDigest:preview.ticketDigest,pendingCriterionIds:preview.pendingCriterionIds};
  const prepared=await applyIncidentMigration(root,state,{authorizationId:'fixture-authorization',bindingReview,deliveryReview},ops);
  assert.equal(prepared.phase,'awaiting');assert.equal(prepared.resumePhase,'assemble');assert.deepEqual(prepared.budget,state.budget);
  for(const field of ['buildAttempts','testAttempts','reviewRound','recoveryAttempts'])assert.deepEqual(prepared[field],state[field]);
  assert.deepEqual(prepared.acceptance,state.acceptance);assert.equal(prepared.wave.members[0].status,'blocked');assert.deepEqual(prepared.completedTickets,state.completedTickets);
  assert.deepEqual(state,original);resumeVerificationMigration(prepared);assert.equal(prepared.phase,'assemble');
  await assert.rejects(applyIncidentMigration(root,state,{authorizationId:'fixture-authorization',bindingReview:{...bindingReview,bindingDigest:'f'.repeat(64)},deliveryReview},ops),/binding reviewer/);
  const { verificationCLI }=await import('../.sandcastle/verification-cli.mjs');
  const dir=await mkdtemp(join(tmpdir(),'sandcastle-migration-cli-'));
  try {
    const saved=join(dir,'.sandcastle/runs/0241e0da');await mkdir(saved,{recursive:true});const statePath=join(saved,'state.json');await writeFile(statePath,JSON.stringify(state));
    const br=join(dir,'binding-review.json'),dr=join(dir,'delivery-review.json');await writeFile(br,JSON.stringify(bindingReview));await writeFile(dr,JSON.stringify(deliveryReview));
    const originalLog=console.log;console.log=()=>{};
    try {
      await verificationCLI(['preview','0241e0da','--root',dir],ops);
      assert.deepEqual(JSON.parse(await readFile(statePath)),state);
      await verificationCLI(['apply','0241e0da','--root',dir,'--authorization','fixture-cli-authorization','--binding-review',br,'--delivery-review',dr],ops);
    } finally {console.log=originalLog;}
    const applied=JSON.parse(await readFile(statePath));assert.equal(applied.phase,'awaiting');assert.equal(applied.resumePhase,'assemble');assert.deepEqual(applied.budget,state.budget);
    assert.deepEqual(JSON.parse(await readFile(join(saved,'verification-before-fixture-cli-authorization.json'))),state);
    await assert.rejects(readFile(join(saved,'run.lock')),{code:'ENOENT'});
  } finally {await rm(dir,{recursive:true,force:true});}
  assert.deepEqual(state,original);

});

test('publisher rejects escapes/secrets, retains honest failure, and recovers a sealed orphan', async t => {
  const { publishEvidence, verifyEvidence, discoverEvidence }=await import('../.sandcastle/verification.mjs');
  const { symlink }=await import('node:fs/promises');
  const dir=await mkdtemp(join(tmpdir(),'sandcastle-publish-negative-'));t.after(()=>rm(dir,{recursive:true,force:true}));
  const staging=join(dir,'producer');await mkdir(staging);await writeFile(join(dir,'outside.txt'),'synthetic outside');await symlink(join(dir,'outside.txt'),join(staging,'escape'));
  const target={head:'a'.repeat(40),contractDigest:'b'.repeat(64),bindingDigest:'c'.repeat(64),controllerDigest:'d'.repeat(64),runtime:{kind:'fixture'}};
  const args={store:join(dir,'store'),staging,target,checkId:'test',attemptId:'bad',result:{exitCode:0},artifactPolicy:{syntheticOnly:true}};
  await assert.rejects(publishEvidence(args),/EVIDENCE_PATH/);await rm(join(staging,'escape'));await writeFile(join(staging,'auth.json'),'synthetic forbidden class');
  await assert.rejects(publishEvidence(args),/EVIDENCE_SECRET/);await rm(join(staging,'auth.json'));await writeFile(join(staging,'trace.txt'),'synthetic failed trace');
  const receipt=await publishEvidence({...args,attemptId:'failure',result:{exitCode:1}});
  assert.equal((await verifyEvidence(receipt,target)).passed,false);
  assert.equal((await discoverEvidence(args.store,target,'test'))[0].passed,false);
  await assert.rejects(verifyEvidence(receipt,{...target,bindingDigest:'f'.repeat(64)}),/EVIDENCE_STALE/);
  await assert.rejects(publishEvidence({...args,attemptId:'failure'}),/EVIDENCE_EXISTS/);
});

test('controller artifact gate runs a real collector at the frozen revision and reuses sealed evidence after restart', async t => {
  const { execFileSync, execFile }=await import('node:child_process');const { promisify }=await import('node:util');
  const { digest }=await import('../.sandcastle/verification.mjs');
  const { acceptExecutionPlan }=await import('../.sandcastle/verification-plan.mjs');
  const { freezeVerificationTarget, executeArtifactCheck, verifyRequiredArtifacts }=await import('../.sandcastle/verification-gates.mjs');
  const dir=await mkdtemp(join(tmpdir(),'sandcastle-real-gate-'));t.after(()=>rm(dir,{recursive:true,force:true}));
  const worktree=join(dir,'repo'),runDirectory=join(dir,'run');await mkdir(join(worktree,'tests'),{recursive:true});await mkdir(join(runDirectory,'evidence-inbox'),{recursive:true});
  const git=(...args)=>execFileSync('git',args,{cwd:worktree,encoding:'utf8'}).trim();git('init','-q');git('config','user.name','Gate fixture');git('config','user.email','fixture@example.invalid');
  const collector=`import { readFileSync, writeFileSync } from 'node:fs';import { execFileSync } from 'node:child_process';import { createHash } from 'node:crypto';const out=process.argv[2];const hash=b=>createHash('sha256').update(b).digest('hex');const source=readFileSync('tests/capture.mjs');const trace=Buffer.from('synthetic controlled trace');writeFileSync(out+'/trace.txt',trace);writeFileSync(out+'/manifest.json',JSON.stringify({tested_head:execFileSync('git',['rev-parse','HEAD'],{encoding:'utf8'}).trim(),contract_digest:'${'b'.repeat(64)}',inputs_unchanged:true,sources:{'tests/capture.mjs':{size:source.length,sha256:hash(source)}},artifacts:{'trace.txt':{size:trace.length,sha256:hash(trace)}},suites:[{name:'ordinary',returncode:0,status:'passed',caseCount:1}]}));`;
  await writeFile(join(worktree,'tests/capture.mjs'),collector);git('add','.');git('commit','-qm','collector');
  const contract={criteria:[{id:'RUNTIME',requirement:'Provide controlled-peer runtime evidence',applicability:'current',mandatory:true,checkIds:['controller-review']}],checks:[{id:'controller-review',kind:'review'}]};
  const binding={id:'runtime',owner:'controller',phase:'test-gate',target:'integrated-head',command:['node','tests/capture.mjs'],outputArgument:true,manifestPath:'manifest.json',syntheticOnly:true,inputPaths:['tests/capture.mjs'],expectedSuites:{ordinary:1}};
  const state={tickets:[],acceptance:{contract,contractDigest:'b'.repeat(64)}};acceptExecutionPlan(state,{version:1,contractDigest:'b'.repeat(64),criteria:[{id:'RUNTIME',evidence:'artifact',artifactCheckIds:['runtime']}],artifactChecks:[binding]},{approved:true,summary:'fixture scope independently declared',missingRequirements:[]});
  await freezeVerificationTarget(state,worktree,{controllerDigest:'d'.repeat(64),runtime:{kind:'fixture'}});
  let calls=0;const shell=promisify(execFile);const sandbox={worktreePath:worktree,exec:async command=>{calls++;const r=await shell('sh',['-c',command.replace('/home/agent/.sandcastle-evidence-out/',join(runDirectory,'evidence-inbox')+'/')],{cwd:worktree});return{exitCode:0,stdout:r.stdout,stderr:r.stderr};}};
  const receipt=await executeArtifactCheck(sandbox,state,binding,{runDirectory,save:async()=>{}});assert.equal(receipt.passed,true);assert.equal(calls,1);
  state.artifactEvidence=[receipt];await verifyRequiredArtifacts(state,worktree);
  const restarted=structuredClone(state);delete restarted.artifactAttempt;
  const recovered=await executeArtifactCheck(sandbox,restarted,binding,{runDirectory,save:async()=>{}});assert.equal(recovered.receiptDigest,receipt.receiptDigest);assert.equal(calls,1);
  assert.equal(state.verifiedArtifactReferences[0].integrityVerified,true);
});
