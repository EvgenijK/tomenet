import { digest, verificationError } from './verification.mjs';
const need=(condition,code,message)=>{if(!condition)throw verificationError(code,message);};
const id=value=>typeof value==='string' && /^[a-zA-Z0-9_-]{1,100}$/.test(value);
const path=value=>typeof value==='string' && !value.startsWith('/') && /^[a-zA-Z0-9_.\/-]+$/.test(value) && !value.split('/').some(p=>!p||p==='.'||p==='..');
// This conservative guard supplements independent full-source verification;
// it does not infer absence of runtime obligations from phrasing alone.
export const runtimeRequirement=criterion=>/(?:runtime|controlled[- ]peer|production[- ]path) (?:evidence|capture|trace)|(?:runtime|trace).*fresh final|fresh.*runtime/i.test(criterion.requirement);
export function validateExecutionPlan(plan,contract,contractDigest,tickets=[]) {
  need(plan?.version===1 && plan.contractDigest===contractDigest,'PLAN_IDENTITY','binding must match immutable contract');
  need(Array.isArray(plan.criteria)&&Array.isArray(plan.artifactChecks),'PLAN_INVALID','criteria and artifact checks required');
  const checks=new Map();
  for(const check of plan.artifactChecks) {
    need(id(check.id)&&!checks.has(check.id),'PLAN_INVALID','unique artifact check ID required');
    need(check.owner==='controller'&&check.phase==='test-gate'&&check.target==='integrated-head','PLAN_STAGE_CYCLE','artifact checks belong after assembly');
    need(Array.isArray(check.command)&&['python3','node','bash'].includes(check.command[0])&&check.command.length>=2&&check.command.every(arg=>typeof arg==='string'&&/^[a-zA-Z0-9_.\/-]+$/.test(arg)),'PLAN_RUNNER','repository argv required');
    const runner=check.command[0]==='python3'&&check.command[1]==='-B'?check.command[2]:check.command[1];
    need(path(runner)&&runner.startsWith('tests/')&&check.command.slice(1).every(arg=>!arg.includes('..')),'PLAN_RUNNER','runner must be a repository test');
    need(check.syntheticOnly===true&&check.outputArgument===true&&path(check.manifestPath)&&Array.isArray(check.inputPaths)&&check.inputPaths.length>0&&check.inputPaths.every(path),'PLAN_EVIDENCE_POLICY','synthetic output, manifest and declared inputs required');
    need(check.expectedSuites&&Object.keys(check.expectedSuites).length>0&&Object.entries(check.expectedSuites).every(([key,count])=>id(key)&&Number.isInteger(count)&&count>0),'PLAN_EVIDENCE_POLICY','literal expected suite counts required');
    need(check.caseArtifactSuffix === undefined || ['.bin','.log'].includes(check.caseArtifactSuffix),'PLAN_EVIDENCE_POLICY','unsupported session artifact suffix');
    if(check.producedBy) need(tickets.some(t=>t.path===check.producedBy),'PLAN_RUNNER','runner delivery prerequisite is unknown');
    checks.set(check.id,check);
  }
  const mapped=new Set();
  for(const binding of plan.criteria) {
    const criterion=contract.criteria.find(c=>c.id===binding.id);
    need(criterion&&!mapped.has(binding.id),'PLAN_COVERAGE','one binding per existing criterion required');mapped.add(binding.id);
    need(['inspection','artifact','deferred'].includes(binding.evidence)&&Array.isArray(binding.artifactCheckIds),'PLAN_COVERAGE','typed evidence requirement required');
    if(criterion.applicability==='deferred') need(binding.evidence==='deferred'&&binding.artifactCheckIds.length===0,'PLAN_COVERAGE','deferred obligation must remain pending');
    else {
      need(binding.evidence!=='deferred','PLAN_COVERAGE','current obligation cannot be deferred');
      if(runtimeRequirement(criterion))need(binding.evidence==='artifact','PLAN_EVIDENCE_MISSING',`runtime criterion ${criterion.id} needs controller-owned artifact gate`);
      if(binding.evidence==='artifact')need(binding.artifactCheckIds.length>0&&binding.artifactCheckIds.every(key=>checks.has(key)),'PLAN_EVIDENCE_MISSING',`missing producer for ${criterion.id}`);
      else need(binding.artifactCheckIds.length===0,'PLAN_COVERAGE','inspection cannot claim artifacts');
    }
  }
  need(mapped.size===contract.criteria.length,'PLAN_COVERAGE','all immutable criteria must be represented');
  need([...checks.keys()].every(key=>plan.criteria.some(c=>c.artifactCheckIds.includes(key))),'PLAN_COVERAGE','unmapped artifact producer');
  for(const ticket of tickets) {
    need(ticket.delivery?.owner==='worker'&&ticket.delivery.phase==='parallel-work'&&Array.isArray(ticket.delivery.prerequisites),'PLAN_DELIVERY_MISSING',ticket.path);
    for(const prerequisite of ticket.delivery.prerequisites)need(prerequisite.owner==='worker'&&prerequisite.phase==='parallel-work','PLAN_STAGE_CYCLE',`${ticket.path} -> assembly -> ${prerequisite.phase} -> delivery`);
  }
  return plan;
}
export function acceptExecutionPlan(state,plan,review,tickets=[]) {
  validateExecutionPlan(plan,state.acceptance.contract,state.acceptance.contractDigest,tickets);
  need(review?.approved===true&&typeof review.summary==='string'&&review.summary.trim()&&Array.isArray(review.missingRequirements)&&review.missingRequirements.length===0,'PLAN_REVIEW','independent binding coverage verification required');
  const bindingDigest=digest(plan);
  if(state.executionPlan&&state.executionPlan.digest!==bindingDigest)need(!state.verificationTarget,'PLAN_FROZEN','cannot replace a frozen verification plan');
  state.verificationVersion=1;state.executionPlan={plan:structuredClone(plan),digest:bindingDigest,review:structuredClone(review)};
  return state.executionPlan;
}
export function assertExecutionPlan(state) {
  need(state.executionPlan&&digest(state.executionPlan.plan)===state.executionPlan.digest,'PLAN_IDENTITY','execution binding changed');
  validateExecutionPlan(state.executionPlan.plan,state.acceptance.contract,state.acceptance.contractDigest,state.tickets??[]);
  return state.executionPlan;
}
export function artifactRequirements(state,criterionId) { return assertExecutionPlan(state).plan.criteria.find(c=>c.id===criterionId)?.artifactCheckIds??[]; }

export function requiredArtifactChecks(state) {
  const plan = assertExecutionPlan(state).plan;
  const ids = new Set(plan.criteria.filter(binding => state.acceptance.contract.criteria.some(c => c.id === binding.id && c.mandatory && c.applicability === 'current')).flatMap(binding => binding.artifactCheckIds));
  return plan.artifactChecks.filter(check => ids.has(check.id));
}
