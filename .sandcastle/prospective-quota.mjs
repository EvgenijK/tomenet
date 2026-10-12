// The human ceiling applies to this paused final-gate incident, not an AI retry.
import {readFile,open} from 'node:fs/promises';
import {resolve} from 'node:path';
import {digest} from './verification.mjs';
import {readPrivateHostJSON} from './controller-host-review-workflow.mjs';
import {recordTaskQuotaAllowance} from './limits.mjs';
const need=(ok,message)=>{if(!ok)throw Object.assign(new Error('Prospective quota grant: '+message),{code:'QUOTA_GRANT_REFUSED',deterministic:true});};
export async function prepareProspectiveQuotaGrant({root,state,authorizationPin}){
 need(state.id==='3fd6b311'&&state.phase==='awaiting'&&state.resumePhase==='controller-host-review'&&state.currentFailure?.fatalPreflight===true&&state.currentFailure.diagnosticCode==='HOST_REVIEW_REFUSED'&&state.hostReviewPending?.originalPhase==='test-gate'&&state.hostReviewPending.request?.head==='93780a282873dac77018831e3b1e9dd7f13bf03c'&&state.hostReviewPending.request.action==='capture-runtime-baseline'&&!state.verificationTarget,'exact saved final-gate host-review incident required');
 const authorization=await readPrivateHostJSON(root,state,authorizationPin),a=authorization;
 need(a.version===1&&a.runId===state.id&&a.previousLimit===55&&a.deltaPercent===5&&a.limitPercent===60&&typeof a.authorizationId==='string'&&a.authorizationId.trim()&&typeof a.explanation==='string'&&a.explanation.trim()&&typeof a.authorizedAt==='string'&&Number.isFinite(Date.parse(a.authorizedAt)),'independently pinned human55→60 authorization required');
 need(Object.keys(a).sort().join(',')==='authorizationId,authorizedAt,checkpointAtAuthorization,deltaPercent,explanation,limitPercent,previousLimit,runId,version','unknown authorization field');
 const c=a.checkpointAtAuthorization;need(c&&Object.keys(c).sort().join(',')==='consumedPercent,limitPercent,phase,resumePhase,stateSha256'&&c.phase===state.phase&&c.resumePhase===state.resumePhase&&c.limitPercent===55&&c.consumedPercent===54&&/^[a-f0-9]{64}$/.test(c.stateSha256),'original human authorization checkpoint required');
 need(state.budget?.limitPercent===55&&Number.isFinite(state.budget.consumedPercent)&&state.budget.consumedPercent>=54,'allowance/consumption changed incompatibly');
 need(![...(state.budget.allowanceHistory??[]),...(state.allowanceGrants??[]),...(state.recoveryAllowanceHistory??[])].some(r=>(r.id??r.authorizationId)===a.authorizationId),'authorization already recorded');
 const raw=await readFile(resolve(root,'.sandcastle/runs',state.id,'state.json'));need(JSON.stringify(JSON.parse(raw))===JSON.stringify(state),'saved checkpoint differs');
 const record={version:1,kind:'prospective-quota',authorizationPin:structuredClone(authorizationPin),authorization:structuredClone(a),rawStateSha256:digest(raw),checkpointDigest:digest(JSON.stringify(state))};record.fingerprint=digest(record);return{record};
}
export async function applyProspectiveQuotaGrant({root,state,prepared}){
 const run=resolve(root,'.sandcastle/runs',state.id),lock=resolve(run,'run.lock'),path=resolve(run,'state.json');
 const check=async()=>{need((await readFile(lock,'utf8')).trim()===String(process.pid),'held controller run lock required');need(digest(await readFile(path))===prepared.record.rawStateSha256&&digest(JSON.stringify(state))===prepared.record.checkpointDigest,'raw checkpoint changed');};
 await check();const fresh=await prepareProspectiveQuotaGrant({root,state,authorizationPin:prepared.record.authorizationPin});need(digest(fresh.record)===digest(prepared.record),'pinned authorization/preview changed');await check();
 const raw=await readFile(path),archive=resolve(run,`quota-before-prospective-${fresh.record.fingerprint}.json`);
 try{const fd=await open(archive,'wx',0o600);try{await fd.writeFile(raw);await fd.sync();}finally{await fd.close();}}catch(e){if(e.code!=='EEXIST'||digest(await readFile(archive))!==digest(raw))throw e;}
 const dir=await open(run,'r');try{await dir.sync();}finally{await dir.close();}
 const next=structuredClone(state),a=fresh.record.authorization;recordTaskQuotaAllowance(next,60,{authorizationId:a.authorizationId,explanation:a.explanation,extraPercent:5});Object.assign(next.budget.allowanceHistory.at(-1),{kind:'prospective-quota',authorizationPin:structuredClone(fresh.record.authorizationPin),authorizationCheckpoint:structuredClone(a.checkpointAtAuthorization),checkpointPath:archive,checkpointSha256:digest(raw),fingerprint:fresh.record.fingerprint});
 // No resume, failure clearing, telemetry update, counter reset or source approval.
 Object.assign(state,next);return fresh.record;
}
