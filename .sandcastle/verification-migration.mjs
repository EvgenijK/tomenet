import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { digest, verificationError } from './verification.mjs';
import { acceptExecutionPlan } from './verification-plan.mjs';
import { acceptDelivery } from './delivery.mjs';
import { taskQuotaLimit } from './limits.mjs';
const exec=promisify(execFile),git=async(root,...args)=>(await exec('git',args,{cwd:root,maxBuffer:8*1024*1024})).stdout.trim();
const need=(condition,message)=>{if(!condition)throw verificationError('VERIFICATION_MIGRATION_INVALID',message);};
export const incidentCheckpoint=Object.freeze({id:'0241e0da',integrationHead:'72fac69a58b7875f1d812980312acaa63573d4c9',workerHead:'48edfae35ce46996dd8c5337dacf6ecd5afe90b5',contractDigest:'0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef',findingId:'D-9496281ca15e0741',ticketPath:'.scratch/sandcastle-0241e0da/issues/09-final-head-runtime-evidence.md'});
export async function previewIncidentMigration(root,state,ops={}) {
  const readGit=ops.git??git;
  const readTicket=ops.readTicket??(async (root,revision,path)=>(await exec('git',['show',`${revision}:${path}`],{cwd:root,maxBuffer:8*1024*1024})).stdout);
  const expected=incidentCheckpoint;
  need(!state.verificationMigration&&state.id===expected.id&&state.phase==='awaiting'&&state.resumePhase==='parallel-work'&&state.wave?.id===6&&state.wave.members.length===1,'requires the unmigrated saved stopped incident');
  need(state.acceptance.contractDigest===expected.contractDigest&&digest(state.acceptance.contract)===expected.contractDigest,'immutable contract differs');
  const member=state.wave.members[0];need(member.status==='blocked'&&!member.integrated&&member.head===expected.workerHead&&member.ticket.path===expected.ticketPath,'blocked delivery checkpoint differs');
  need(state.acceptance.ledger.some(f=>f.id===expected.findingId&&f.status==='open'),'evidence finding must remain open');
  need(await readGit(root,'rev-parse',state.branch)===expected.integrationHead&&await readGit(root,'rev-parse',member.branch)===expected.workerHead,'branch revisions changed; review a new migration');
  await readGit(root,'merge-base','--is-ancestor',expected.integrationHead,expected.workerHead);
  const allowed=[expected.ticketPath,'.scratch/sandcastle-0241e0da/evidence/09/README.md','.scratch/sandcastle-0241e0da/map.md'];
  const paths=(await readGit(root,'diff','--name-only',expected.integrationHead,expected.workerHead)).split('\n').filter(Boolean);
  need(paths.length>0&&paths.every(path=>allowed.includes(path)),'only the reviewed documentation handoff delta is supported; implementation blockers cannot migrate');
  const ticket=await readTicket(root,expected.workerHead,expected.ticketPath);
  need(/^Status: claimed$/m.test(ticket)&&/^## Answer$/m.test(ticket),'preserve historical claimed ticket with Answer');
  const inputPaths=(await readGit(root,'ls-tree','-r','--name-only',expected.workerHead,'--','src/client/sv','src/common/md5.c','tests/sv','tests/sv_birth*','tests/sv_guide_motd_checks.py','.sandcastle/checks.sh')).split('\n').filter(Boolean);
  // git ls-tree path arguments do not expand globs consistently; include every
  // declared suite and collector explicitly, and validate their committed blobs.
  const runnerPaths=['tests/sv_birth_runtime_evidence.py','tests/sv_birth_complete_checks.py','tests/sv_birth_version_checks.py','tests/sv_birth_dedicated_checks.py','tests/sv_birth_single_submission_checks.py','tests/sv_birth_reply_checks.py','tests/sv_guide_motd_checks.py'];
  for(const path of runnerPaths)await readGit(root,'cat-file','-e',`${expected.workerHead}:${path}`);
  const pendingCriterionIds=['P1-ac3','P1-ac6','P1-ac8','P2-4'];
  const executionPlan={version:1,contractDigest:expected.contractDigest,criteria:state.acceptance.contract.criteria.map(c=>({id:c.id,evidence:c.applicability==='deferred'?'deferred':pendingCriterionIds.includes(c.id)?'artifact':'inspection',artifactCheckIds:pendingCriterionIds.includes(c.id)?['birth-runtime']:[]})),artifactChecks:[{id:'birth-runtime',owner:'controller',phase:'test-gate',target:'integrated-head',command:['python3','-B','tests/sv_birth_runtime_evidence.py'],outputArgument:true,manifestPath:'manifest.json',syntheticOnly:true,caseArtifactSuffix:'.log',inputPaths:[...new Set(inputPaths.concat(runnerPaths))].sort(),expectedSuites:{ordinary:3,versions:40,dedicated:20,submission:18,replies:17,'guide-motd':10}}]};
  return {version:1,runId:state.id,stateDigest:digest(state),sourceHead:expected.integrationHead,targetHead:expected.workerHead,ticketPath:expected.ticketPath,ticketDigest:digest(ticket),changeDigest:digest(await readGit(root,'diff',expected.integrationHead,expected.workerHead)),changedPaths:paths,pendingCriterionIds,executionPlan,bindingDigest:digest(executionPlan),preservedFinding:expected.findingId,resumeCheckpoint:'assemble',reviewRequirements:'Independent binding coverage review and exact-revision worker-local delivery review are required; no controller acceptance is implied.'};
}
export async function applyIncidentMigration(root,state,{authorizationId,bindingReview,deliveryReview},ops={}) {
  need(typeof authorizationId==='string'&&/^[a-zA-Z0-9_-]{8,100}$/.test(authorizationId),'unique explicit authorization ID required');
  const preview=await previewIncidentMigration(root,state,ops);
  need(bindingReview?.bindingDigest===preview.bindingDigest&&bindingReview.head===preview.targetHead,'binding reviewer must name exact preview identity');
  need(Array.isArray(deliveryReview?.pendingCriterionIds) && JSON.stringify([...deliveryReview.pendingCriterionIds].sort()) === JSON.stringify([...preview.pendingCriterionIds].sort()),'delivery review must retain every pending runtime criterion');
  const next=structuredClone(state);
  const tickets=next.tickets.map(ticket=>({...ticket,delivery:{owner:'worker',phase:'parallel-work',prerequisites:[]}}));
  acceptExecutionPlan(next,preview.executionPlan,bindingReview,tickets);next.tickets=tickets;
  const receipt=acceptDelivery(next,{revision:preview.targetHead,ticketPath:preview.ticketPath,ticketDigest:preview.ticketDigest,changeDigest:preview.changeDigest,review:deliveryReview});
  next.wave.members[0].ticket=tickets.find(t=>t.path===preview.ticketPath);next.wave.members[0].deliveryReceipt=receipt;
  next.wave.after='test-gate';
  next.verificationMigration={version:1,authorizationId,at:new Date().toISOString(),preview,originalCheckpoint:{phase:state.phase,resumePhase:state.resumePhase,pauseReason:state.pauseReason,wave:structuredClone(state.wave)},bindingReview:structuredClone(bindingReview)};
  next.resumePhase='assemble'; // Remains stopped; a separate explicit resume is required.
  return next;
}
export function resumeVerificationMigration(state) {
  need(state.phase==='awaiting'&&state.resumePhase==='assemble'&&state.verificationMigration?.version===1&&state.wave?.members.length===1,'requires a prepared stopped migration');
  need(state.wave.members[0].deliveryReceipt?.digest===state.deliveryReceipts?.at(-1)?.digest,'validated delivery receipt required');
  need(!state.quotaError && (state.budget?.consumedPercent ?? 0) < taskQuotaLimit(state),'quota guard requires its separate authorized allowance; verification resume grants nothing');
  state.phase='assemble';delete state.resumePhase;delete state.pauseReason;return state;
}
