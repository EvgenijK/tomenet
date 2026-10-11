// Controller-only fold of a reviewed finite constituent plan. A child is never
// evidence that the accepted suite passed; only the exact union can be published.
import {open,lstat,readdir,mkdir,writeFile,rename,rm} from 'node:fs/promises';
import {constants} from 'node:fs';
import {resolve,dirname,isAbsolute} from 'node:path';
import {gunzipSync} from 'node:zlib';
import {createHash,randomBytes} from 'node:crypto';
import {digest} from './verification.mjs';
const need=(ok,message)=>{if(!ok)throw new Error('Host collector constituents: '+message);};
const safe=p=>typeof p==='string'&&p&&!isAbsolute(p)&&p.split('/').every(x=>x&&x!=='.'&&x!=='..');
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
const same=(a,b)=>digest(canonical(a))===digest(canonical(b));
const sha=x=>typeof x==='string'&&/^[a-f0-9]{64}$/.test(x);
const label=x=>typeof x==='string'&&/^[a-z][a-z0-9-]{0,63}$/.test(x);
const SHAPES={executable:{children:6,tasks:567,suites:{'executable-routes':3,'branch-reports':6}},recovery:{children:8,tasks:6690,suites:{'recovery-surfaces':16}},interaction:{children:4,tasks:71,suites:{'creation-cancel':18,'name-recovery':6,'async-routes':3}}};
function planOf(binding,part){
 const p=part.constituents,s=SHAPES[p?.kind];need(s,'known finite constituent kind required');
 need(same(Object.fromEntries(part.suites.map(k=>[k,binding.expectedSuites[k]])),s.suites),'accepted whole suite counts differ');
 need(Array.isArray(p.required_shards)&&p.required_shards.length===s.children&&new Set(p.required_shards).size===s.children&&p.required_shards.every(label),'complete fixed child list required');
 need(Array.isArray(p.full_task_inventory)&&p.full_task_inventory.length===s.tasks,'full constituent task matrix required');
 need(sha(p.full_task_digest)&&digest(JSON.stringify(canonical(p.full_task_inventory)))===p.full_task_digest,'full task digest differs');
 const ids=new Set(),indices=[];
 for(const child of p.required_shards){
  const selected=p.selected_indices_by_shard?.[child];need(Array.isArray(selected)&&selected.length>0,'fixed child indices required');
  need(selected.every((i,n)=>Number.isSafeInteger(i)&&i>=0&&i<s.tasks&&(n===0||i>selected[n-1])&&p.full_task_inventory[i].shard===child),'invalid selected task indices');indices.push(...selected);
 }
 need(indices.length===s.tasks&&new Set(indices).size===s.tasks,'missing/duplicate constituent index');
 for(const task of p.full_task_inventory){need(safe(task.id)&&!ids.has(task.id)&&p.required_shards.includes(task.shard),'safe unique canonical task identity required');ids.add(task.id);}
 need(same(Object.keys(p.selected_indices_by_shard).sort(),[...p.required_shards].sort()),'unknown child index map');
 return p;
}
export function hostCollectorConstituents(policy,binding,part){
 if(!part.constituents)return undefined;
 const p=planOf(binding,part);need(Array.isArray(p.children)&&p.children.length===p.required_shards.length,'complete child action list required');
 const parent=policy.actions?.[part.action];need(parent?.controllerOnly===true&&parent.checkId===binding.id&&same(parent.bindingCommand,binding.command),'whole action binding differs');
 for(const [i,child]of p.children.entries()){
  need(child.label===p.required_shards[i]&&child.driver===part.driver&&child.renderer===part.renderer,'fixed child ordering/desktop differs');
  const action=policy.actions?.[child.action];need(action?.controllerOnly===true&&action.checkId===binding.id&&same(action.bindingCommand,binding.command),'child source/action binding differs');
  need(Array.isArray(action.argv)&&action.argv.filter(x=>x==='--constituent').length===1&&action.argv[action.argv.indexOf('--constituent')+1]===child.label&&action.argv.filter(x=>x==='--runtime-part').length===1&&action.argv[action.argv.indexOf('--runtime-part')+1]===p.kind&&!action.argv.some(x=>typeof x==='string'&&x.startsWith('--discover'))&&action.argv.filter(x=>x==='--reviewed-seal').length===1&&action.argv[action.argv.indexOf('--reviewed-seal')+1]===p.reviewedSealPath,'fixed sealed child argv required');
  need(same(action.inventory?.files?.slice().sort(),binding.inputPaths.slice().sort())&&action.inventory?.roots?.length===0,'exact child inventory required');
  const flags=new Set(['--head','--contract-digest','--nonce','--tracer','--reviewed-seal','--runtime-part','--constituent']);need(action.argv.filter(x=>typeof x==='string'&&x.startsWith('--')).every(x=>flags.has(x)),'arbitrary child filter prohibited');
 }
 need(new Set(p.children.map(c=>c.action)).size===p.children.length,'duplicate child action');
 return structuredClone(p.children);
}
async function walk(root,dir=root,out=[]){
 for(const entry of await readdir(dir,{withFileTypes:true})){
  const path=resolve(dir,entry.name),s=await lstat(path);need(!s.isSymbolicLink(),'artifact symlink');
  if(s.isDirectory())await walk(root,path,out);else{need(s.isFile()&&s.nlink===1,'regular private artifact required');out.push(path.slice(root.length+1));}
 }return out.sort();
}
async function bytes(root,name){
 need(safe(name),'artifact path escape');let current=root;
 for(const piece of name.split('/')){current=resolve(current,piece);need(!(await lstat(current)).isSymbolicLink(),'artifact parent symlink');}
 const f=await open(current,constants.O_RDONLY|constants.O_NOFOLLOW);try{
  const before=await f.stat();need(before.isFile()&&before.nlink===1&&before.size<=64*1024*1024,'regular bounded artifact required');
  const data=await f.readFile(),after=await f.stat();need(before.size===after.size&&before.mtimeMs===after.mtimeMs&&before.ctimeMs===after.ctimeMs,'artifact changed during read');return data;
 }finally{await f.close();}
}
const unzip=data=>gunzipSync(data,{maxOutputLength:64*1024*1024});
function identity(data,id){need(id&&data.length===id.size&&digest(data)===id.sha256,'artifact byte identity differs');}
function inputs(value){need(value&&Object.keys(value).length===390,'complete exact390 source inventory required');return Object.fromEntries(Object.entries(value).map(([path,id])=>{need(safe(path)&&sha(typeof id==='string'?id:id?.sha256),'source SHA required');return[path,typeof id==='string'?id:id.sha256];}));}
async function readChild(receipt,binding,target,spec,part){
 need(receipt.head===target.head&&receipt.contractDigest===target.contractDigest&&receipt.action===spec.action&&receipt.exitCode===0&&/^[a-f0-9]{32}$/.test(receipt.nonce??'')&&sha(receipt.policyDigest),'child receipt identity/failure');
 const root=resolve(receipt.output);need((await lstat(root)).isDirectory()&&!(await lstat(root)).isSymbolicLink(),'private child root required');
 need(same(await walk(root),Object.keys(receipt.artifacts??{}).sort()),'missing/extra private child artifact');
 const docs=new Map();for(const [name,id]of Object.entries(receipt.artifacts)){const data=await bytes(root,name);identity(data,id);docs.set(name,data);}
 const manifest=JSON.parse(docs.get(binding.manifestPath));
 need(manifest.capture_scope==='reviewed-capture'&&manifest.independent_review&&Object.keys(manifest.independent_review).length>0,'independent reviewed capture required');
 need(same(manifest.artifacts,Object.fromEntries(Object.entries(receipt.artifacts).filter(([name])=>name!==binding.manifestPath))),'manifest/receipt artifact inventory differs');
 need(manifest.status==='constituent-passed'&&manifest.tested_head===target.head&&manifest.contract_digest===target.contractDigest&&manifest.nonce===receipt.nonce&&manifest.inputs_unchanged===true&&manifest.complete_consumed_input_seal===true&&manifest.physical_acceptance===false&&manifest.milestone_acceptance===false,'unsealed/discovery/stale child');
 need(same(inputs(manifest.sources),inputs(target.inputHashes))&&same(Object.keys(manifest.sources).sort(),[...binding.inputPaths].sort()),'source bytes differ from frozen input identities');
 need(sha(target.reviewedSealIdentity?.sha256)&&Number.isSafeInteger(target.reviewedSealIdentity.size)&&same(manifest.reviewed_seal_identity,target.reviewedSealIdentity),'reviewed consumed seal differs');
 need(manifest.external_before&&Object.keys(manifest.external_before).length>0&&same(manifest.external_before,manifest.external_after),'external consumed inputs changed');
 const p=planOf(binding,part),c=manifest.constituent_capture;
 need(c&&c.kind===p.kind&&c.shard_id===spec.label&&same(c.required_shards,p.required_shards)&&same(c.full_task_inventory,p.full_task_inventory)&&c.full_task_digest===p.full_task_digest&&same(c.selected_indices,p.selected_indices_by_shard[spec.label])&&same(c.expectedWholeSuites,SHAPES[p.kind].suites)&&c.whole_matrix_complete===false,'child scope/index proof differs');
 need(c.caseCount===c.selected_indices.length&&Array.isArray(c.caseLogs)&&c.caseLogs.length===c.caseCount&&new Set(c.caseLogs).size===c.caseCount,'complete constituent case logs required');
 const get=name=>{need(safe(name),'original artifact path escape');if(docs.has(name))return docs.get(name);const a=c.artifact_aliases?.[name];need(a,'missing original artifact');if(a.encoding==='gzip'){need(docs.has(a.artifact),'missing gzip alias');const raw=unzip(docs.get(a.artifact));identity(raw,a);return raw;}
  need(a.encoding==='content-blocks','unknown lossless alias');const index=JSON.parse(unzip(docs.get(a.index)));const row=index.files?.find(r=>r.original===name);need(row,'missing binary alias row');const raw=reconstruct('',index,row);identity(raw,a);return raw;};
 const cache=new Map();
 function reconstruct(group,index,row,materialize=true){
  const key=group+index.catalog;let blocks=cache.get(key);
  if(!blocks){need(index.schema===1&&safe(index.catalog)&&docs.has(key),'missing block catalog');const catalog=JSON.parse(unzip(docs.get(key)));need(catalog.schema===1&&catalog.block_size===4096&&Array.isArray(catalog.blocks),'unknown block codec');blocks=catalog.blocks.map((id,i)=>{need(id.id===i&&safe(id.artifact)&&docs.has(group+id.artifact),'missing/duplicate block');const packed=docs.get(group+id.artifact);identity(packed,{size:id.packed_size,sha256:id.packed_sha256});const raw=unzip(packed);identity(raw,id);need(raw.length>0&&raw.length<=4096,'invalid block length');return raw;});cache.set(key,blocks);}
  need(Array.isArray(row.blocks)&&row.blocks.every(i=>Number.isSafeInteger(i)&&i>=0&&i<blocks.length),'invalid block sequence');const h=createHash('sha256');let length=0;for(const i of row.blocks){h.update(blocks[i]);length+=blocks[i].length;}need(length===row.size&&h.digest('hex')===row.sha256,'reconstructed original bytes differ');return materialize?Buffer.concat(row.blocks.map(i=>blocks[i])):undefined;
 }
 for(const [name,alias]of Object.entries(c.artifact_aliases??{})){need(safe(name)&&!docs.has(name),'ambiguous original alias');get(name);}
 for(const [name,data]of docs){if(!name.endsWith('frame-block-images.json.gz')&&!name.endsWith('wire-block-index.json.gz'))continue;
  const index=JSON.parse(unzip(data)),group=name.endsWith('wire-block-index.json.gz')?'':name.split('/')[0]+'/';
  const rows=index.frames??index.files;need(Array.isArray(rows)&&rows.length>0,'missing original image/binary inventory');const seen=new Set();
  for(const [i,row]of rows.entries()){need(safe(row.original)&&!seen.has(row.original),'duplicate/unsafe original image');seen.add(row.original);if(index.frames)need(row.sequence===i,'frame sequence gap');reconstruct(group,index,row,false);}
  if(index.frames){const trace=get(dirname(name)+'/'+index.trace).toString('utf8');const frames=[...trace.matchAll(/^FRAME (\d+)$/gm)].map(m=>Number(m[1]));need(same(frames,rows.map(r=>r.sequence)),'frame positions differ from native trace');}
 }
 for(const [n,i]of c.selected_indices.entries()){
  const task=p.full_task_inventory[i];const suffix=p.kind==='recovery'?'.log.gz':task.kind==='async-routes'&&task.id.split('/').at(-1).startsWith('retry-')?'/result.log':['creation-cancel','name-recovery'].includes(task.kind)?'.log':'/native.log';
  need(c.caseLogs[n]===task.id+suffix,'constituent task/log pairing differs');need(get(c.caseLogs[n]).length>0,'empty no-send log');
 }
 return {spec,receipt,manifest,docs,get};
}
export function assertHostCollectorAggregation({spec,receipt,manifest}){
 need(receipt.aggregation===true&&manifest.aggregation?.kind==='controller-constituent-union'&&manifest.nonce===receipt.nonce&&same(manifest.aggregation.origins,receipt.children),'aggregate origins differ');
 need(manifest.aggregation.full_task_digest===spec.constituents.full_task_digest&&manifest.aggregation.whole_matrix_complete===true,'incomplete aggregate scope');
 need(receipt.children.length===spec.constituents.required_shards.length&&same(receipt.children.map(c=>c.label),spec.constituents.required_shards),'missing aggregate child');
}
export async function mergeHostCollectorConstituents({inbox,binding,target,part,children}){
 const plan=planOf(binding,part);need(children.length===plan.required_shards.length,'all finite children required');
 need(target.inputHashes&&target.reviewedSealIdentity,'controller frozen source and reviewed seal required');
 const verified=[],nonces=new Set();
 for(const [i,item]of children.entries()){
  const spec=item.spec??plan.children[i];need(same(spec,plan.children[i]),'child spec differs from reviewed plan');
  need(!nonces.has(item.receipt.nonce),'replayed child nonce');nonces.add(item.receipt.nonce);verified.push(await readChild(item.receipt,binding,target,spec,part));
 }
 const foldId=randomBytes(16).toString('hex');need(!nonces.has(foldId),'fold nonce collision');
 need(verified.every(v=>v.receipt.scope&&same(v.receipt.scope,verified[0].receipt.scope)),'child controller scopes differ');
 const origins=verified.map(v=>({scope:v.receipt.scope,sourceApproval:v.receipt.sourceApproval??null,label:v.spec.label,action:v.receipt.action,driver:v.spec.driver,renderer:v.spec.renderer,head:v.receipt.head,contractDigest:v.receipt.contractDigest,nonce:v.receipt.nonce,policyDigest:v.receipt.policyDigest,manifest:v.spec.label+'/'+binding.manifestPath,manifestSha256:v.receipt.artifacts[binding.manifestPath].sha256,exitCode:v.receipt.exitCode}));
 const out=resolve(inbox),temp=out+'.fold-'+foldId;need(!(await lstat(out).catch(()=>null)),'fresh aggregate output required');await mkdir(temp,{mode:0o700,recursive:false});const artifacts={};
 const put=async(name,data)=>{need(safe(name),'aggregate path escape');await mkdir(dirname(resolve(temp,name)),{recursive:true,mode:0o700});await writeFile(resolve(temp,name),data,{mode:0o600,flag:'wx'});artifacts[name]={size:data.length,sha256:digest(data)};};
 try{
  for(const v of verified)for(const[name,data]of v.docs)await put(v.spec.label+'/'+name,data);
  const suites=[];const caseLog=async(suite,name,result,refs)=>{const path=suite+'/'+name+'.log';await put(path,Buffer.from(JSON.stringify({result,artifacts:refs})+'\n'));return path;};
  const ref=(v,path)=>({artifact:v.spec.label+'/'+(v.manifest.constituent_capture.artifact_aliases?.[path]?.artifact??path),original:path,originalIdentity: {size:v.get(path).length,sha256:digest(v.get(path))},nonce:v.receipt.nonce});
  if(plan.kind==='executable'){
   const baseline=verified[0],routes=[];for(const task of plan.full_task_inventory.filter(t=>t.kind==='executable-routes')){const row=JSON.parse(baseline.get(task.id.replace('executable/','executable/')+'/case.json'));need(row.status==='passed','failed executable route');routes.push(await caseLog('executable-routes',task.id.split('/').at(-1),row,[ref(baseline,task.id+'/case.json')]));}
   suites.push({name:'executable-routes',status:'passed',returncode:0,caseCount:3,caseLogs:routes});
   const logs=[];for(const v of verified){const report=JSON.parse(v.get('branches/branch-report.json'));need(report.failures?.length===0&&report.cases?.length===1&&report.session_count===94&&report.complete_matrix===false,'failed/incomplete branch child');const row=report.cases[0],tasks=plan.full_task_inventory.filter(t=>t.kind==='branch-reports'&&t.shard===v.spec.label);need(row.status==='passed'&&same(row.sessions,tasks.map(t=>t.id.replace(/^branches\//,'')))&&row.order===tasks[0].order&&row.route===tasks[0].route,'branch session matrix differs');const refs=[];for(const t of tasks){const path=t.id+'/case.json',r=JSON.parse(v.get(path));need(r.status==='passed','failed branch session');refs.push(ref(v,path));}logs.push(await caseLog('branch-reports',row.order+'-'+row.route,row,refs));}suites.push({name:'branch-reports',status:'passed',returncode:0,caseCount:6,caseLogs:logs});
  }else if(plan.kind==='recovery'){
   const allCases=new Map(),caseOwners=new Map();
   for(const v of verified){const result=JSON.parse(v.get('recovery/results.json'));need(Array.isArray(result.cases)&&result.cases.length===v.manifest.constituent_capture.caseCount,'missing recovery result cases');
    for(const c of result.cases){const index=plan.full_task_inventory.findIndex(t=>t.id==='recovery/'+c.id);need(v.manifest.constituent_capture.selected_indices.includes(index)&&c.status==='passed'&&c.returncode===0&&!allCases.has(c.id)&&['order','variant','route','surface','edge'].every(k=>!Object.hasOwn(plan.full_task_inventory[index],k)||c[k]===(k==='surface'&&plan.full_task_inventory[index][k]===114?14:plan.full_task_inventory[index][k]))&&c.log===c.id+'.log.gz'&&c.transition_ids_by_surface&&c.transition_ids_by_origin,'failed/duplicate/foreign recovery result');allCases.set(c.id,{index,value:c});caseOwners.set(c.id,v);}}
   need(allCases.size===6690,'full recovery result matrix missing');
   const surfaceNames=['endpoint','contact','account','password','overview','name','sex','class','race','trait','body','stats','mode','acknowledgement','motd','failure'];
   const logs=[];for(let surface=1;surface<=16;surface++){
    const tag='S'+String(surface).padStart(2,'0'),filename='recovery/'+tag+'-'+surfaceNames[surface-1]+'.json';
    let baseline;const cases=[],seen=new Set(),refs=[];
    for(const v of verified){const row=JSON.parse(v.get(filename)),{cases:rows,...scalar}=row;need(row.surface===tag&&row.name===surfaceNames[surface-1]&&Array.isArray(rows),'recovery surface identity differs');if(!baseline)baseline=scalar;need(same(baseline,scalar),'recovery surface metadata differs');
     const expected=v.manifest.constituent_capture.selected_indices.map(i=>allCases.get(plan.full_task_inventory[i].id.replace(/^recovery\//,''))?.value).filter(c=>c&&(Object.hasOwn(c.transition_ids_by_origin,String(surface))||Object.hasOwn(c.transition_ids_by_surface,String(surface))||c.surface===surface||(c.surface===0&&[1,3,4,16].includes(surface))||(surface===16&&['disconnect','rejection','retrydisconnect','retryrejection'].includes(c.edge))));
     need(same(rows,expected),'missing/changed recovery surface observations');
     for(const c of rows){need(!seen.has(c.id),'duplicate recovery surface case');seen.add(c.id);cases.push({index:allCases.get(c.id).index,value:{...c,log:v.spec.label+'/recovery/'+c.log}});refs.push(ref(v,'recovery/'+c.log));}}
    need(cases.length>0,'missing recovery surface');cases.sort((a,b)=>a.index-b.index);logs.push(await caseLog('recovery-surfaces',tag,{...baseline,cases:cases.map(c=>c.value)},refs));
   }suites.push({name:'recovery-surfaces',status:'passed',returncode:0,caseCount:16,caseLogs:logs});
  }else{
   const core=verified[0];for(const suite of ['creation-cancel','name-recovery']){const logs=[];for(const task of plan.full_task_inventory.filter(t=>t.kind===suite)){const path=task.id+'.log';logs.push(await caseLog(suite,task.id.split('/').at(-1),{status:'passed',task:task.id,original_log:ref(core,path)},[ref(core,path)]));}suites.push({name:suite,status:'passed',returncode:0,caseCount:logs.length,caseLogs:logs});}
   const logs=[];for(const v of verified.slice(1)){const route=v.spec.label.replace(/^async-/,''),path='asynchronous/'+route+'/report.json',row=JSON.parse(v.get(path));need(row.status==='passed'&&row.route===route&&row.full_provider_wait_matrix===true&&Array.isArray(row.cases)&&row.cases.length===(route==='existing-selection'?5:20)&&row.cases.every(c=>c.status==='passed'&&c.asynchronous===true),'failed/incomplete async route');const completionTasks=plan.full_task_inventory.filter(task=>task.shard===v.spec.label&&!task.id.split('/').at(-1).startsWith('retry-'));need(row.cases.every((c,i)=>c.provider_completion_wait===completionTasks[i].provider_wait&&c.skip_motd===completionTasks[i].skip_motd),'async completion wait/order differs');logs.push(await caseLog('async-routes',route,row,[ref(v,path),...v.manifest.constituent_capture.caseLogs.map(p=>ref(v,p))]));}suites.push({name:'async-routes',status:'passed',returncode:0,caseCount:3,caseLogs:logs});
  }
  need(same(Object.fromEntries(suites.map(s=>[s.name,s.caseCount])),SHAPES[plan.kind].suites),'whole suite count differs');
  const manifest={tested_head:target.head,contract_digest:target.contractDigest,nonce:foldId,status:'passed',inputs_unchanged:true,complete_consumed_input_seal:true,physical_acceptance:false,milestone_acceptance:false,input_provenance:'programmatic SDL',sources:verified[0].manifest.sources,suites,artifacts:{...artifacts},reviewed_seal_identity:target.reviewedSealIdentity,aggregation:{kind:'controller-constituent-union',origins,full_task_digest:plan.full_task_digest,whole_matrix_complete:true,taskCount:plan.full_task_inventory.length}};
  await put(binding.manifestPath,Buffer.from(JSON.stringify(manifest)+'\n'));await rename(temp,out);
  return {aggregation:true,exitCode:0,output:out,artifacts,head:target.head,contractDigest:target.contractDigest,action:part.action,nonce:foldId,children:origins,manifest};
 }catch(error){await rm(temp,{recursive:true,force:true});throw error;}
}
