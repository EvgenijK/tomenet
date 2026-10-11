// One explicit allowance for the saved wave8 incident. No accounting is reset.
import {open,readFile} from 'node:fs/promises';
import {resolve} from 'node:path';
import {digest} from './verification.mjs';
import {taskQuotaLimit} from './limits.mjs';
const KEY='parallel-work:8:8:0:1:0';
const need=(ok,message)=>{if(!ok)throw new Error('Recovery allowance: '+message);};
export function recoveryAttemptLimit(state,key){
 const limit=state.recoveryAttemptLimits?.[key]??3;
 need(Number.isSafeInteger(limit)&&limit>=3&&limit<=100,'invalid saved checkpoint allowance');
 if(limit!==3)need(state.recoveryAllowanceHistory?.some(r=>r.checkpoint===key&&r.previousLimit===3&&r.limit===limit&&r.extraAttempts===limit-3&&r.authorizationId),'unrecorded checkpoint allowance');
 return limit;
}
export function validateRecoveryAllowanceCheckpoint(state,checkpoint=KEY){
 need(checkpoint===KEY&&state.id==='3fd6b311'&&state.phase==='awaiting'&&state.resumePhase==='parallel-work'&&state.pauseReason==='Automatic recovery made no progress after three decisions at this checkpoint'&&!state.recovery&&!state.verificationTarget,'exact saved exhausted wave8 checkpoint required');
 const actual=[state.resumePhase,state.wave?.id??0,state.completedTickets?.length,state.reviewRound,state.buildAttempts,state.testAttempts].join(':');
 need(actual===checkpoint&&state.recoveryAttempts?.[checkpoint]===3&&recoveryAttemptLimit(state,checkpoint)===3,'checkpoint/counter changed');
 need(!state.quotaError&&(state.budget?.consumedPercent??0)<taskQuotaLimit(state),'quota availability required; attempt allowance does not grant quota');
 return checkpoint;
}
export function readRecoveryGrant(env=process.env){
 if(env.SANDCASTLE_EXTRA_RECOVERY_ATTEMPTS===undefined)return undefined;
 need(env.SANDCASTLE_EXTRA_RECOVERY_ATTEMPTS==='3','this explicit incident allows exactly three additional decisions');
 need(env.SANDCASTLE_RECOVERY_CHECKPOINT===KEY,'exact checkpoint environment required');
 const authorizationId=env.SANDCASTLE_RECOVERY_AUTHORIZATION_ID,explanation=env.SANDCASTLE_RECOVERY_AUTHORIZATION_EXPLANATION;
 need(typeof authorizationId==='string'&&authorizationId.trim()&&typeof explanation==='string'&&explanation.trim(),'stable authorization ID and explanation required');
 return {checkpoint:KEY,extraAttempts:3,authorizationId,explanation};
}
export function prepareRecoveryAllowance({state,grant}){
 validateRecoveryAllowanceCheckpoint(state,grant?.checkpoint);
 need(grant?.checkpoint===KEY&&grant.extraAttempts===3&&typeof grant.authorizationId==='string'&&grant.authorizationId.trim()&&typeof grant.explanation==='string'&&grant.explanation.trim(),'explicit checkpoint authorization required');
 need(![...(state.recoveryAllowanceHistory??[]),...(state.attemptAllowanceHistory??[]),...(state.budget?.allowanceHistory??[])].some(r=>(r.authorizationId??r.id)===grant.authorizationId),'authorization already recorded');
 const record={version:1,kind:'recovery',checkpoint:grant.checkpoint,extraAttempts:grant.extraAttempts,authorizationId:grant.authorizationId,explanation:grant.explanation,consumedAttempts:3,previousLimit:3,limit:6};
 record.fingerprint=digest(record);return {record,checkpointDigest:digest(JSON.stringify(state))};
}
export async function applyRecoveryAllowance({root,state,prepared,profileRefresh,controllerDigest}){
 const run=resolve(root,'.sandcastle/runs',state.id),lock=resolve(run,'run.lock'),path=resolve(run,'state.json');
 const check=async()=>{need((await readFile(lock,'utf8')).trim()===String(process.pid),'held controller lock required');need(digest(JSON.stringify(state))===prepared.checkpointDigest&&digest(JSON.stringify(JSON.parse(await readFile(path,'utf8'))))===prepared.checkpointDigest,'checkpoint changed before allowance');};
 await check();const fresh=prepareRecoveryAllowance({state,grant:prepared.record});need(digest(fresh.record)===digest(prepared.record),'authorization preview changed');
 let refreshed;
 if(profileRefresh){
  const {prepareHostRecoveryProfileRefresh}=await import('./host-profile-upgrade.mjs');
  refreshed=await prepareHostRecoveryProfileRefresh({root,state,newPin:profileRefresh.record.newPin,controllerDigest});
  need(digest(refreshed.record)===digest(profileRefresh.record),'controller profile preview changed');
 }
 await check();
 const archive=resolve(run,`recovery-before-allowance-${fresh.record.fingerprint}.json`),raw=JSON.stringify(state)+'\n';
 try{const fd=await open(archive,'wx',0o600);try{await fd.writeFile(raw);await fd.sync();}finally{await fd.close();}}catch(error){if(error.code!=='EEXIST'||await readFile(archive,'utf8')!==raw)throw error;}
 const directory=await open(run,'r');try{await directory.sync();}finally{await directory.close();}
 const next=structuredClone(state);
 (next.recoveryAttemptLimits??={})[KEY]=6;
 (next.recoveryAllowanceHistory??=[]).push({...fresh.record,checkpointPath:archive,checkpointSha256:digest(raw),at:new Date().toISOString()});
 if(refreshed){
  next.hostReviewProfile=structuredClone(refreshed.record.newPin);
  next.activeHostPolicy={...next.activeHostPolicy,path:refreshed.profile.policyPath,sha256:refreshed.profile.policySha256};
  (next.hostRecoveryProfileRefreshes??=[]).push({...refreshed.record,checkpointPath:archive,checkpointSha256:digest(raw)});
 }
 next.phase=next.resumePhase;delete next.resumePhase;delete next.pauseReason;
 for(const key of Object.keys(state))if(!(key in next))delete state[key];Object.assign(state,next);
 return refreshed?.profile;
}
