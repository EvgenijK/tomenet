import {normalizeControllerHostRequest} from './host-capture-failure.mjs';
// Historical dependency evidence is readable before new host code is approved.
// It retains its original source identity and never authorizes host execution.
export function workerHostDelivery(state,member,wave){
 const records=state.hostApprovalResumes??[];
 return records.findLast(record=>record.wave===wave.id&&record.branch===member.branch)
  ??records.findLast(record=>member.ticket.blockedBy.includes(record.ticket)&&state.completedTickets.includes(record.ticket));
}

// A provider may request controller review, but only the private host channel
// can establish that approval is actually missing. Never classify prose.
export async function workerHostFailure(channel,result,base){
 if(result.status!=='blocked'||!channel)return undefined;
 if(channel.stats.approvalFailure)return structuredClone(channel.stats.approvalFailure);
 const request=result.hostApprovalRequest;
 if(!request)return undefined;
 if(Object.keys(request).sort().join(',')!=='action,driver,renderer'||
  !/^[a-z][a-z0-9-]{0,63}$/.test(request.action)||
  !['wayland','x11'].includes(request.driver)||!['software','opengl'].includes(request.renderer))throw new Error('Invalid structured host approval request');
 try{await channel.observeApprovalRequest({...base,action:request.action,driver:request.driver,renderer:request.renderer});}
 catch(error){if(error.code==='HOST_APPROVAL_REQUIRED')return {code:error.code,deterministic:true,request:normalizeControllerHostRequest({...base,...request})};throw error;}
 return undefined;
}
