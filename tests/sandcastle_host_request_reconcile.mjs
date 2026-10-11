import test from 'node:test';
import assert from 'node:assert/strict';
import {writeFile,readFile} from 'node:fs/promises';
import {join} from 'node:path';
import {prepareHostRequestReconcile,applyHostRequestReconcile} from '../.sandcastle/host-profile-upgrade.mjs';
import {prepareControllerHostReview} from '../.sandcastle/controller-host-review.mjs';
import {fixture,evidencePaths} from './sandcastle_host_request_fixture.mjs';
import {digest} from '../.sandcastle/verification.mjs';
const args=f=>({root:f.root,state:f.state,newPin:f.newPin,controllerDigest:f.controllerDigest});
test('actual unknown-action and known-action metadata barriers are removed only by pinned exact evidence correction',async t=>{
 const f=await fixture(t),before=structuredClone(f.state),check=structuredClone(f.state);check.phase='controller-host-review';delete check.resumePhase;
 const prepare=request=>prepareControllerHostReview({root:f.root,state:check,request,profile:f.profile,ops:{currentControllerDigest:async()=>f.controllerDigest}});
 await assert.rejects(prepare(f.state.hostReviewPending.request),/unknown action/);
 await assert.rejects(prepare({...f.state.hostReviewPending.request,...f.profile.defaultRequest}),/unlisted tree/);
 const p=await prepareHostRequestReconcile(args(f));assert.deepEqual(f.state,before);assert.equal(p.record.freshRequest.action,'capture-runtime-baseline');assert.equal(p.record.oldRequest.action,'entry-runtime-discovery');assert.equal(p.record.sourceProof.method,'reviewed-carry-forward');assert.deepEqual(p.record.evidenceReview.evidenceFiles,f.evidenceFiles);
 await writeFile(join(f.run,'run.lock'),String(process.pid));const oldPolicy=await readFile(f.profile.policyPath),oldSeal=await readFile(f.profile.sealPath);await applyHostRequestReconcile({...args(f),prepared:p});
 assert.equal(f.state.phase,'controller-host-review');assert.equal(f.state.hostReviewPending.request.action,'capture-runtime-baseline');assert.equal(f.state.hostReviewObserved.origin,'independently pinned controller default; historical request archived');for(const key of ['wave','budget','completedTickets','recoveryAttempts','buildAttempts','testAttempts','reviewLimit','acceptance','executionPlan','currentFailure'])assert.deepEqual(f.state[key],before[key]);const record=f.state.hostRequestReconciliations[0];assert.deepEqual(JSON.parse(await readFile(record.checkpointPath)),before);assert.equal(digest(await readFile(record.checkpointPath)),record.checkpointSha256);assert.deepEqual(await readFile(f.profile.policyPath),oldPolicy);assert.deepEqual(await readFile(f.profile.sealPath),oldSeal);
});
for(const [name,change]of [
 ['native argv',f=>f.updated.actions['capture-runtime-baseline'].argv.push('--skip')],
 ['jobs cap',f=>f.updated.maxJobs=9],
 ['timeout',f=>f.updated.actions['capture-runtime-baseline'].timeoutMs=600000],
 ['new action',f=>f.updated.actions['entry-runtime-discovery']={argv:['sh','-c','anything']}],
 ['metadata root',f=>f.updated.actions['collector-delivery-inspect'].carryForward.metadataRoots=['.scratch']],
 ['mutable evidence path',f=>f.updated.actions['collector-delivery-inspect'].carryForward.metadataPaths.push(evidencePaths[0])],
 ['unreviewed evidence blob',f=>f.updated.actions['collector-delivery-inspect'].carryForward.reviewedFiles[evidencePaths[0]][0].sha256='f'.repeat(64)],
 ['missing evidence path',f=>delete f.updated.actions['collector-delivery-inspect'].carryForward.reviewedFiles[evidencePaths[1]]],
 ['other evidence grant',f=>f.updated.actions['collector-delivery-inspect'].carryForward.reviewedFiles['.scratch/sandcastle-3fd6b311/evidence/other.json']=[{mode:'100644',sha256:'f'.repeat(64)}]],
 ['nonindependent evidence record',f=>f.next.requestReconciliationReview.reviewer=''],
 ['review different request',f=>f.next.requestReconciliationReview.oldRequestDigest='f'.repeat(64)],
 ['different default request',f=>f.next.defaultRequest.action='collector-delivery-inspect'],
 ['different source seal',f=>f.next.sealSha256='f'.repeat(64)],
 ['readonly proof change',f=>f.updated.readonlyDeliveries[0].sha256='f'.repeat(64)],
])test('reconciliation refuses '+name+' without checkpoint mutation',async t=>{const f=await fixture(t),before=structuredClone(f.state);change(f);await f.rewrite();await assert.rejects(prepareHostRequestReconcile(args(f)));assert.deepEqual(f.state,before);});
test('wrong checkpoint, known request, frozen target and exhausted quota are not this incident',async t=>{const f=await fixture(t);for(const change of [s=>s.resumePhase='parallel-work',s=>s.currentFailure.diagnosticCode='OTHER',s=>s.hostReviewPending.request.action='capture-runtime-baseline',s=>s.verificationTarget={head:f.head},s=>s.budget.consumedPercent=45]){const s=structuredClone(f.state);change(s);await assert.rejects(prepareHostRequestReconcile({...args(f),state:s}));}});
test('branch change and consumed source change refuse fresh observation',async t=>{const f=await fixture(t);await writeFile(join(f.root,'src/native-0.c'),'changed production');await f.git('add','src/native-0.c');await f.git('commit','-m','changed source');await assert.rejects(prepareHostRequestReconcile(args(f)),/branch changed/);const head=await f.git('rev-parse','HEAD');f.state.hostReviewPending.request.head=head;f.state.wave.members[0].failure.request.head=head;f.next.requestReconciliationReview.head=head;f.next.requestReconciliationReview.oldRequestDigest=digest(f.state.hostReviewPending.request);await f.rewrite();await assert.rejects(prepareHostRequestReconcile(args(f)),/changed producer/);});
test('held lock, raw checkpoint CAS and pinned preview bytes are rechecked before mutation',async t=>{const f=await fixture(t),p=await prepareHostRequestReconcile(args(f));await writeFile(join(f.run,'run.lock'),'99999');await assert.rejects(applyHostRequestReconcile({...args(f),prepared:p}),/held controller/);await writeFile(join(f.run,'run.lock'),String(process.pid));const external=structuredClone(f.state);external.budget.limitPercent=46;await writeFile(join(f.run,'state.json'),JSON.stringify(external));await assert.rejects(applyHostRequestReconcile({...args(f),prepared:p}),/checkpoint changed/);await writeFile(join(f.run,'state.json'),JSON.stringify(f.state));await writeFile(f.next.policyPath,JSON.stringify(f.updated)+' ');await assert.rejects(applyHostRequestReconcile({...args(f),prepared:p}),/bytes differ/);assert.equal(f.state.hostRequestReconciliations,undefined);});
test('workflow requires named reconciliation mode and applies fresh pinned profile before saved-phase execution',async()=>{const s=await readFile(new URL('../.sandcastle/workflow.mjs',import.meta.url),'utf8');assert.match(s,/SANDCASTLE_HOST_REQUEST_RECONCILE/);const lock=s.indexOf('const lockPath = await acquireLock(state)'),apply=s.indexOf('hostProfile=await applyHostRequestReconcile'),sandbox=s.indexOf('sandbox = await createSandbox',apply);assert.ok(lock<apply&&apply<sandbox);assert.match(s,/hostProfilePin=state.hostReviewProfile/);});
