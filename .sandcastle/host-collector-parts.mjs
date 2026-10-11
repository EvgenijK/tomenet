// Fixed parts belong to the private host policy; the accepted suite matrix is
// indivisible. Every part keeps its original capture identity in the publisher.
import {mkdir,open,readFile,writeFile,lstat} from 'node:fs/promises';
import {resolve,dirname,isAbsolute} from 'node:path';
import {constants} from 'node:fs';
import {digest} from './verification.mjs';
const need=(ok,message)=>{if(!ok)throw new Error('Host collector parts: '+message);};
const safe=path=>typeof path==='string'&&path&&!isAbsolute(path)&&path.split('/').every(p=>p&&p!=='.'&&p!=='..');
export function hostCollectorParts(policy,binding){
 const parts=policy.hostParts?.[binding.id];
 if(!parts){
  const found=Object.entries(policy.actions??{}).find(([,action])=>action.checkId===binding.id&&action.controllerOnly===true);
  need(found,'independent collector/source approval required');
  return [{action:found[0],driver:policy.captureDriver??'wayland',renderer:'software'}];
 }
 need(Array.isArray(parts)&&parts.length>=2&&parts.length<=8,'bounded fixed part list required');
 const labels=new Set(),suites=new Set();
 for(const part of parts){
  need(/^[a-z][a-z0-9-]{0,63}$/.test(part.label??'')&&!labels.has(part.label),'unique safe part label required');labels.add(part.label);
  need(['x11','wayland'].includes(part.driver)&&['software','opengl'].includes(part.renderer),'explicit driver/renderer required');
  const action=policy.actions?.[part.action];need(action?.controllerOnly===true&&action.checkId===binding.id&&digest(action.bindingCommand)===digest(binding.command),'part source/action binding differs');
  need(Array.isArray(part.suites)&&part.suites.length>0,'literal part suites required');
  for(const name of part.suites){need(Object.hasOwn(binding.expectedSuites,name)&&!suites.has(name),'missing/duplicate/unknown part suite');suites.add(name);}
 }
 need(suites.size===Object.keys(binding.expectedSuites).length,'complete accepted suite matrix required');
 return structuredClone(parts);
}
async function bytes(path){
 need(await lstat(path).then(s=>s.isFile()&&!s.isSymbolicLink()&&s.nlink===1),'regular private capture artifact required');
 const file=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);try{return await file.readFile();}finally{await file.close();}
}
export async function mergeHostCollectorParts({inbox,binding,target,parts}){
 const merged={tested_head:target.head,contract_digest:target.contractDigest,inputs_unchanged:true,
  complete_consumed_input_seal:true,status:'passed',physical_acceptance:false,milestone_acceptance:false,
  input_provenance:'programmatic SDL',sources:null,artifacts:{},suites:[],parts:[]};
 const seen=new Set(),labels=new Set(),nonces=new Set(),logicalReports=new Map();let logicalKeys;
 for(const {spec,receipt} of parts){
  need(/^[a-z][a-z0-9-]{0,63}$/.test(spec.label??'')&&!labels.has(spec.label),'unique safe part label');labels.add(spec.label);
  need(receipt.head===target.head&&receipt.contractDigest===target.contractDigest&&receipt.action===spec.action&&receipt.exitCode===0&&/^[a-f0-9]{32}$/.test(receipt.nonce??'')&&!nonces.has(receipt.nonce),'part receipt identity/failure');nonces.add(receipt.nonce);
  need(receipt.artifacts?.[binding.manifestPath],'part manifest absent');
  const documents={};
  for(const [name,identity] of Object.entries(receipt.artifacts)){
   need(safe(name),'artifact path escape');const data=await bytes(resolve(receipt.output,name));
   need(data.length===identity.size&&digest(data)===identity.sha256,'changed private part artifact');
   documents[name]=data;
  }
  const manifest=JSON.parse(documents[binding.manifestPath]);
  need(manifest.tested_head===target.head&&manifest.contract_digest===target.contractDigest&&manifest.nonce===receipt.nonce&&manifest.status==='passed'&&manifest.inputs_unchanged===true&&manifest.complete_consumed_input_seal===true&&manifest.physical_acceptance===false,'unsealed/stale/failed part capture');
  need(manifest.sources&&JSON.stringify(Object.keys(manifest.sources).sort())===JSON.stringify([...binding.inputPaths].sort()),'complete source inventory required');
  if(binding.id==='entry-automated'){
   const part=manifest.renderer_part;
   need(part?.renderer===spec.renderer&&part.logical_scenarios===18&&part.executions===18&&part.sessions===36&&part.full_execution_records===36&&part.final_acceptance===false&&JSON.stringify(part.required_parts)===JSON.stringify(['software','opengl']),'complete renderer provenance required');
   need(Array.isArray(manifest.logical_reports)&&manifest.logical_reports.length===18,'complete logical scenario pairing required');
   const keys=manifest.logical_reports.map(row=>{
    need(typeof row.route==='string'&&typeof row.category==='string'&&row.executions?.length===1&&row.executions[0].renderer===spec.renderer&&row.executions[0].sessions?.length===2,'renderer execution/session pairing required');
    return JSON.stringify([row.route,row.category]);
   }).sort();need(new Set(keys).size===18,'duplicate logical scenario');
   if(logicalKeys===undefined)logicalKeys=keys;
   need(JSON.stringify(keys)===JSON.stringify(logicalKeys),'renderer logical scenarios differ');
   for(const row of manifest.logical_reports){
    const key=JSON.stringify([row.route,row.category]),previous=logicalReports.get(key);
    if(previous)previous.executions.push(...structuredClone(row.executions));
    else logicalReports.set(key,structuredClone(row));
   }
  }
  if(merged.sources===null)merged.sources=manifest.sources;
  need(digest(merged.sources)===digest(manifest.sources),'part source identities differ');
  need(Array.isArray(manifest.suites)&&manifest.suites.length===spec.suites.length,'complete literal part suite matrix required');
  for(const suite of manifest.suites){
   need(spec.suites.includes(suite.name)&&!seen.has(suite.name)&&suite.status==='passed'&&suite.returncode===0,'duplicate/failed/unknown part suite');seen.add(suite.name);
   const count=binding.expectedSuites[suite.name];
   need(suite.caseCount===count&&Array.isArray(suite.caseLogs)&&suite.caseLogs.length===count&&new Set(suite.caseLogs).size===count,'complete case and no-send logs required');
   need(suite.caseLogs.every(path=>safe(path)&&path.endsWith(binding.caseArtifactSuffix??'.log')&&receipt.artifacts[path]),'missing case artifact');
   merged.suites.push({...suite,caseLogs:suite.caseLogs.map(path=>spec.label+'/'+path)});
  }
  for(const [name,data]of Object.entries(documents)){
   const path=spec.label+'/'+name;await mkdir(dirname(resolve(inbox,path)),{recursive:true,mode:0o700});
   await writeFile(resolve(inbox,path),data,{mode:0o600,flag:'wx'});merged.artifacts[path]={size:data.length,sha256:digest(data)};
  }
  merged.parts.push({label:spec.label,action:spec.action,driver:spec.driver,renderer:spec.renderer,nonce:receipt.nonce,policyDigest:receipt.policyDigest,manifest:spec.label+'/'+binding.manifestPath});
 }
 need(seen.size===Object.keys(binding.expectedSuites).length&&Object.keys(binding.expectedSuites).every(name=>seen.has(name)),'missing accepted suite');
 if(binding.id==='entry-automated'){
  need(parts.length===2&&new Set(parts.map(p=>p.spec.renderer)).size===2,'both accepted renderers required');
  merged.logical_reports=[...logicalReports.values()];
  need(merged.logical_reports.length===18&&merged.logical_reports.every(row=>row.executions.length===2),'complete paired renderer records required');
 }
 await writeFile(resolve(inbox,binding.manifestPath),JSON.stringify(merged,null,2)+'\n',{mode:0o600,flag:'wx'});
 return {exitCode:0};
}
