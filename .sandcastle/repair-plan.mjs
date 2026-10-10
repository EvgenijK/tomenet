import { digest } from './verification.mjs';
import { taskQuotaLimit } from './limits.mjs';
// Historical registration stopped after a genuine approved binding. This API
// only restores validation; it never registers tickets or approves acceptance.
export function resumeRepairPlanCheckpoint(state) {
  const review=state.planReviewHistory?.at(-1),binding=state.executionPlan;
  const require=(condition,message)=>{if(!condition)throw new Error(`PLAN_REPAIR_RESUME: ${message}`);};
  require(state.phase==='awaiting'&&state.resumePhase==='plan-validate'&&state.pauseReason==='Acceptance: repair ticket must name open ledger IDs','requires the saved historical registration rejection');
  require(state.verificationVersion===1&&binding?.digest===digest(binding?.plan)&&binding.plan.contractDigest===state.acceptance?.contractDigest&&review?.bindingDigest===binding.digest&&review.contractDigest===state.acceptance.contractDigest&&review.review.approved===true&&review.review.missingRequirements.length===0,'requires the saved independently approved exact binding');
  require(!state.wave&&!state.verificationTarget,'requires registration before workers or a frozen target');
  require(!state.quotaError&&(state.budget?.consumedPercent??0)<taskQuotaLimit(state),'cannot grant quota');
  (state.repairPlanResumes??=[]).push({at:new Date().toISOString(),checkpoint:'plan-validate',bindingDigest:binding.digest,reviewedHead:review.head,originalReason:state.pauseReason});
  state.phase='plan-validate';delete state.resumePhase;delete state.pauseReason;return state;
}
