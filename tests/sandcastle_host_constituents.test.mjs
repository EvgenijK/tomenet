import test from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,readFile,rm,symlink} from 'node:fs/promises';
import {join} from 'node:path';
import {tmpdir} from 'node:os';
import {gzipSync} from 'node:zlib';
import {digest} from '../.sandcastle/verification.mjs';
import {hostCollectorConstituents,mergeHostCollectorConstituents,assertHostCollectorAggregation} from '../.sandcastle/host-collector-constituents.mjs';
const canonical=x=>Array.isArray(x)?x.map(canonical):x&&typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,canonical(x[k])])):x;
async function fixture(t){
 const root=await mkdtemp(join(tmpdir(),'sv-fold-'));t.after(()=>rm(root,{recursive:true,force:true}));
 const sources=Object.fromEntries(Array.from({length:390},(_,i)=>['src/input-'+i,{size:1,sha256:digest('x')}]));
 const binding={id:'entry-runtime',manifestPath:'manifest.json',inputPaths:Object.keys(sources),command:['python3','runtime.py'],expectedSuites:{'executable-routes':3,'branch-reports':6}};
 const shards=Array.from({length:6},(_,i)=>'branch-'+i),tasks=[];
 for(const name of ['existing-native','existing-create-native','new-account-create-native'])tasks.push({id:'executable/'+name,kind:'executable-routes',shard:shards[0]});
 for(let i=0;i<6;i++)for(let j=0;j<94;j++)tasks.push({id:'branches/'+i+'-route/session-'+j,kind:'branch-reports',shard:shards[i],order:i<3?1:0,route:'route-'+i});
 const part={label:'executable',action:'capture-runtime-executable',driver:'x11',renderer:'software',suites:Object.keys(binding.expectedSuites),constituents:{reviewedSealPath:'/private/seal.json',kind:'executable',required_shards:shards,full_task_inventory:tasks,full_task_digest:digest(JSON.stringify(canonical(tasks))),selected_indices_by_shard:Object.fromEntries(shards.map(s=>[s,tasks.flatMap((r,i)=>r.shard===s?[i]:[])])),children:shards.map(s=>({label:s,action:'capture-'+s,driver:'x11',renderer:'software'}))}};
 const policy={actions:{[part.action]:{controllerOnly:true,checkId:binding.id,bindingCommand:binding.command}}};
 for(const c of part.constituents.children)policy.actions[c.action]={controllerOnly:true,checkId:binding.id,bindingCommand:binding.command,argv:['python3','runtime.py','--reviewed-seal','/private/seal.json','--runtime-part','executable','--constituent',c.label],inventory:{files:binding.inputPaths,roots:[]}};
 const target={head:'a'.repeat(40),contractDigest:'b'.repeat(64),inputHashes:structuredClone(sources),reviewedSealIdentity:{size:20,sha256:digest('seal')}};
 target.reviewedSeal={tested_head:target.head,contract_digest:target.contractDigest,complete_consumed_input_seal:true,sources:structuredClone(sources),independent_review:{reviewer:'independent-test'},external_files:{tool:{sha256:digest('t')}},compiler_dependencies:[{choice_order:'0'},{choice_order:'1'}]};
 const children=[];
 for(let i=0;i<6;i++){
  const spec=part.constituents.children[i],output=join(root,spec.label);await mkdir(output);const docs={},aliases={};
  const add=async(name,data)=>{data=Buffer.isBuffer(data)?data:Buffer.from(typeof data==='string'?data:JSON.stringify(data));await mkdir(join(output,name,'..'),{recursive:true});await writeFile(join(output,name),data);docs[name]={size:data.length,sha256:digest(data)};};
  const packed=async(name,data)=>{const raw=Buffer.from(typeof data==='string'?data:JSON.stringify(data));await add(name+'.gz',gzipSync(raw));aliases[name]={encoding:'gzip',artifact:name+'.gz',size:raw.length,sha256:digest(raw)};};
  const selected=part.constituents.selected_indices_by_shard[spec.label],logs=[];
  for(const index of selected){const task=tasks[index],path=task.id+'/native.log';logs.push(path);await packed(path,'FRAME 0\n');await packed(task.id+'/case.json',{status:'passed'});}
  await packed('branches/branch-report.json',{cases:[{order:i<3?1:0,route:'route-'+i,status:'passed',sessions:selected.filter(n=>tasks[n].kind==='branch-reports').map(n=>tasks[n].id.replace('branches/',''))}],failures:[],session_count:94,complete_matrix:false});
  // A real producer-shaped 4KiB block/image fixture with one frame position.
  const raw=Buffer.from('BM synthetic binary frame'),blockSha=digest(raw),packedBlock=gzipSync(raw),group='branches/';
  await add(group+'frame-blocks/'+blockSha+'.bin.gz',packedBlock);
  await add(group+'frame-block-catalog.json.gz',gzipSync(Buffer.from(JSON.stringify({schema:1,block_size:4096,blocks:[{id:0,size:raw.length,sha256:blockSha,artifact:'frame-blocks/'+blockSha+'.bin.gz',packed_size:packedBlock.length,packed_sha256:digest(packedBlock)}]}))));
  const rootBlock='frame-blocks/'+blockSha+'.bin.gz';await add(rootBlock,packedBlock);
  await add('frame-block-catalog.json.gz',gzipSync(Buffer.from(JSON.stringify({schema:1,block_size:4096,blocks:[{id:0,size:raw.length,sha256:blockSha,artifact:rootBlock,packed_size:packedBlock.length,packed_sha256:digest(packedBlock)}]}))));
  await add('wire-block-index.json.gz',gzipSync(Buffer.from(JSON.stringify({schema:1,catalog:'frame-block-catalog.json.gz',files:[{original:'wire.bin',size:raw.length,sha256:blockSha,blocks:[0]}]}))));
  aliases['wire.bin']={encoding:'content-blocks',index:'wire-block-index.json.gz',size:raw.length,sha256:blockSha};
  const task=tasks[selected.find(n=>tasks[n].kind==='branch-reports')];
  await add(task.id+'/frame-block-images.json.gz',gzipSync(Buffer.from(JSON.stringify({schema:1,catalog:'frame-block-catalog.json.gz',trace:'native.log',frames:[{sequence:0,original:'frame-000000.bmp',size:raw.length,sha256:blockSha,blocks:[0]}]}))));
  const graphLabels=part.constituents.kind==='executable'?['build-orders','branch-reports',...(spec.label===shards[0]?['executable-routes']:[])]:part.constituents.kind==='recovery'?['recovery-surfaces']:spec.label==='interaction-core'?['creation-cancel','name-recovery']:['async-routes'];
  const consumption_graphs=[];for(const label of graphLabels){const name=label+'.consumption.json.gz';await add(name,gzipSync(Buffer.from(JSON.stringify({repository:{'src/input-0':sources['src/input-0']},external_files:target.reviewedSeal.external_files,unresolved_inputs:[]}))));consumption_graphs.push({label,artifact:name});}
  const manifest={compiler_dependencies:structuredClone(target.reviewedSeal.compiler_dependencies),consumption_graphs,artifacts:structuredClone(docs),capture_scope:'reviewed-capture',independent_review:{reviewer:'independent-test'},tested_head:target.head,contract_digest:target.contractDigest,nonce:String(i).padStart(32,'0'),status:'constituent-passed',sources:structuredClone(sources),inputs_unchanged:true,complete_consumed_input_seal:true,physical_acceptance:false,milestone_acceptance:false,reviewed_seal_identity:target.reviewedSealIdentity,external_before:structuredClone(target.reviewedSeal.external_files),external_after:structuredClone(target.reviewedSeal.external_files),constituent_capture:{kind:'executable',required_shards:shards,full_task_inventory:structuredClone(tasks),full_task_digest:part.constituents.full_task_digest,shard_id:spec.label,selected_indices:[...selected],caseCount:selected.length,caseLogs:logs,artifact_aliases:aliases,expectedWholeSuites:binding.expectedSuites,whole_matrix_complete:false}};
  await add('manifest.json',manifest);children.push({spec,receipt:{scope:{runId:'3fd6b311',wave:0,ticket:'ticket08',branch:'codex/test',baseCommit:'a'.repeat(40)},output,head:target.head,contractDigest:target.contractDigest,action:spec.action,nonce:manifest.nonce,exitCode:0,policyDigest:'c'.repeat(64),artifacts:docs},manifest,add});
 }
 return {root,part,policy,binding,target,children,merge:()=>mergeHostCollectorConstituents({inbox:join(root,'fold'),part,binding,target,children})};
}
async function rewrite(v){v.manifest.artifacts=Object.fromEntries(Object.entries(v.receipt.artifacts).filter(([name])=>name!=='manifest.json'));const data=Buffer.from(JSON.stringify(v.manifest));await writeFile(join(v.receipt.output,'manifest.json'),data);v.receipt.artifacts['manifest.json']={size:data.length,sha256:digest(data)};}
test('finite registration preserves complete task/action plan and rejects unapproved selectors',async t=>{
 const f=await fixture(t);assert.equal(hostCollectorConstituents(f.policy,f.binding,f.part).length,6);assert.equal(hostCollectorConstituents(f.policy,f.binding,{suites:[]}),undefined);
 f.policy.actions['capture-branch-0'].argv.push('--discover');assert.throws(()=>hostCollectorConstituents(f.policy,f.binding,f.part),/sealed child argv/);
});
test('full sealed567 union reconstructs gzip and BMP and preserves child origins',async t=>{
 const f=await fixture(t),r=await f.merge();assert.equal(r.aggregation,true);assert.equal(r.children.length,6);assert.deepEqual(r.manifest.suites.map(s=>s.caseCount),[3,6]);assert.equal(r.manifest.aggregation.taskCount,567);assertHostCollectorAggregation({spec:f.part,receipt:r,manifest:r.manifest});
 for(const child of r.children)assert.ok(r.artifacts[child.manifest]);assert.notEqual(r.nonce,r.children[0].nonce);
});
for(const [name,change,error]of [
 ['discovery',v=>{v.manifest.status='discovery-proposal';},/unsealed/],
 ['source SHA',v=>{v.manifest.sources['src/input-0'].sha256=digest('bad');},/source bytes/],
 ['missing consumed seal',v=>{v.manifest.complete_consumed_input_seal=false;},/unsealed/],
 ['wrong seal SHA',v=>{v.manifest.reviewed_seal_identity={size:20,sha256:digest('other')};},/reviewed consumed/],
 ['wrong task index',v=>{v.manifest.constituent_capture.selected_indices[0]=566;},/scope\/index/],
 ['missing log',v=>{v.manifest.constituent_capture.caseLogs.pop();},/complete constituent/],
 ['changed compressed raw SHA',v=>{Object.values(v.manifest.constituent_capture.artifact_aliases)[0].sha256=digest('wrong');},/byte identity/],
 ['changed external input',v=>{v.manifest.external_after={'/lib/tool':{size:1,sha256:digest('changed')}};},/external consumed/]
])test('rejects '+name+' even with recomputed host artifact hash',async t=>{const f=await fixture(t);change(f.children[0]);await rewrite(f.children[0]);await assert.rejects(f.merge(),error);});
test('rejects missing/repeated children before publishing',async t=>{const f=await fixture(t);f.children.pop();await assert.rejects(f.merge(),/all finite/);});
test('rejects nonce replay and unlisted or symlinked artifacts',async t=>{const f=await fixture(t);f.children[1].receipt.nonce=f.children[0].receipt.nonce;await assert.rejects(f.merge(),/replayed/);f.children[1].receipt.nonce='1'.padStart(32,'0');await writeFile(join(f.children[0].receipt.output,'extra'),'x');await assert.rejects(f.merge(),/extra private/);await rm(join(f.children[0].receipt.output,'extra'));await symlink('/etc/passwd',join(f.children[0].receipt.output,'extra'));await assert.rejects(f.merge(),/symlink/);});
test('rejects changed block raw bytes despite new artifact and packed hashes',async t=>{const f=await fixture(t),v=f.children[0],catalogName='branches/frame-block-catalog.json.gz';const {gunzipSync}=await import('node:zlib');const catalog=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,catalogName))));const b=catalog.blocks[0],packed=gzipSync(Buffer.from('bad'));await v.add('branches/'+b.artifact,packed);b.packed_size=packed.length;b.packed_sha256=digest(packed);await v.add(catalogName,gzipSync(Buffer.from(JSON.stringify(catalog))));await rewrite(v);await assert.rejects(f.merge(),/byte identity/);});
test('rejects manifest frame index gap and stale HEAD',async t=>{const f=await fixture(t),v=f.children[0],name=Object.keys(v.receipt.artifacts).find(x=>x.endsWith('frame-block-images.json.gz'));const {gunzipSync}=await import('node:zlib');const index=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name))));index.frames[0].sequence=1;await v.add(name,gzipSync(Buffer.from(JSON.stringify(index))));await rewrite(v);await assert.rejects(f.merge(),/frame sequence/);v.receipt.head='d'.repeat(40);await assert.rejects(f.merge(),/receipt identity/);});
async function semanticFixture(t,kind){
 const root=await mkdtemp(join(tmpdir(),'sv-fold-'+kind+'-'));t.after(()=>rm(root,{recursive:true,force:true}));
 const sources=Object.fromEntries(Array.from({length:390},(_,i)=>['src/input-'+i,{size:1,sha256:digest('x')}]));
 const target={head:'a'.repeat(40),contractDigest:'b'.repeat(64),inputHashes:structuredClone(sources),reviewedSealIdentity:{size:20,sha256:digest('seal')}};
 const suites=kind==='recovery'?{'recovery-surfaces':16}:{'creation-cancel':18,'name-recovery':6,'async-routes':3};
 const binding={id:'entry-runtime',manifestPath:'manifest.json',inputPaths:Object.keys(sources),command:['python3','runtime.py'],expectedSuites:suites};
 const shards=kind==='recovery'?Array.from({length:8},(_,i)=>'recovery-'+i):['interaction-core','async-existing-selection','async-existing-account-creation','async-new-account-creation'];
 const tasks=[];
 if(kind==='recovery')for(let i=0;i<6690;i++)tasks.push({id:'recovery/case-'+i,kind:'recovery-surfaces',shard:shards[Math.floor(i*8/6690)],surface:2+i%15,edge:'cancel'});
 else{
  for(const[suite,count]of [['creation-cancel',18],['name-recovery',6]])for(let i=0;i<count;i++)tasks.push({id:suite+'/mode-'+i,kind:suite,shard:shards[0]});
  tasks.push({id:'asynchronous/retry-Test',kind:'async-routes',shard:shards[1]},{id:'asynchronous/retry-UnusedAccount',kind:'async-routes',shard:shards[3]});
  for(const[route,count]of [['existing-selection',5],['existing-account-creation',20],['new-account-creation',20]])for(let i=0;i<count;i++)tasks.push({id:'asynchronous/'+route+'/completion-'+i,kind:'async-routes',shard:'async-'+route,provider_wait:'wait-'+i,skip_motd:false});
 }
 const part={label:kind,action:'whole-'+kind,driver:'x11',renderer:'software',suites:Object.keys(suites),constituents:{reviewedSealPath:'/private/seal.json',kind,required_shards:shards,full_task_inventory:tasks,full_task_digest:digest(JSON.stringify(canonical(tasks))),selected_indices_by_shard:Object.fromEntries(shards.map(s=>[s,tasks.flatMap((r,i)=>r.shard===s?[i]:[])])),children:shards.map(s=>({label:s,action:'capture-'+s,driver:'x11',renderer:'software'}))}};
 target.reviewedSeal={tested_head:target.head,contract_digest:target.contractDigest,complete_consumed_input_seal:true,sources:structuredClone(sources),independent_review:{reviewer:'independent-test'},external_files:{tool:{sha256:digest('t')}},compiler_dependencies:[{choice_order:'0'},{choice_order:'1'}]};
 const children=[];
 for(const[ordinal,spec]of part.constituents.children.entries()){
  const output=join(root,spec.label);await mkdir(output);const docs={},aliases={};
  const add=async(name,data)=>{data=Buffer.isBuffer(data)?data:Buffer.from(typeof data==='string'?data:JSON.stringify(data));await mkdir(join(output,name,'..'),{recursive:true});await writeFile(join(output,name),data);docs[name]={size:data.length,sha256:digest(data)};};
  const packed=async(name,data)=>{const raw=Buffer.from(typeof data==='string'?data:JSON.stringify(data));await add(name+'.gz',gzipSync(raw));aliases[name]={encoding:'gzip',artifact:name+'.gz',size:raw.length,sha256:digest(raw)};};
  const selected=part.constituents.selected_indices_by_shard[spec.label],logs=[];
  for(const i of selected){const task=tasks[i],path=task.id+(kind==='recovery'?'.log.gz':task.id.includes('retry-')?'/result.log':['creation-cancel','name-recovery'].includes(task.kind)?'.log':'/native.log');logs.push(path);await packed(path,'observed production no-send evidence\n');}
  if(kind==='recovery'){
   const results=selected.map(i=>({id:'case-'+i,status:'passed',returncode:0,surface:2+i%15,edge:'cancel',log:'case-'+i+'.log.gz',transition_ids_by_surface:{'1':['start'],[2+i%15]:['active']},transition_ids_by_origin:{'1':['start'],[2+i%15]:['active']},observation_count:3,observer_wire_parity:true}));
   await packed('recovery/results.json',{cases:results});
   const names=['endpoint','contact','account','password','overview','name','sex','class','race','trait','body','stats','mode','acknowledgement','motd','failure'];
   for(let surface=1;surface<=16;surface++)await packed('recovery/S'+String(surface).padStart(2,'0')+'-'+names[surface-1]+'.json',{surface:'S'+String(surface).padStart(2,'0'),name:names[surface-1],cases:results.filter(c=>surface===1||c.surface===surface),unavailable:[],prefix_baseline:null,back_without_parent:null});
  }else if(ordinal>0){const route=spec.label.replace('async-',''),rows=selected.map(i=>tasks[i]).filter(t=>!t.id.includes('retry-')).map(task=>({status:'passed',asynchronous:true,provider_completion_wait:task.provider_wait,skip_motd:task.skip_motd}));await packed('asynchronous/'+route+'/report.json',{status:'passed',route,full_provider_wait_matrix:true,cases:rows,contract_digest:'original-baseline-kept'});}
  const graphLabels=part.constituents.kind==='executable'?['build-orders','branch-reports',...(spec.label===shards[0]?['executable-routes']:[])]:part.constituents.kind==='recovery'?['recovery-surfaces']:spec.label==='interaction-core'?['creation-cancel','name-recovery']:['async-routes'];
  const consumption_graphs=[];for(const label of graphLabels){const name=label+'.consumption.json.gz';await add(name,gzipSync(Buffer.from(JSON.stringify({repository:{'src/input-0':sources['src/input-0']},external_files:target.reviewedSeal.external_files,unresolved_inputs:[]}))));consumption_graphs.push({label,artifact:name});}
  const manifest={compiler_dependencies:structuredClone(target.reviewedSeal.compiler_dependencies),consumption_graphs,artifacts:structuredClone(docs),capture_scope:'reviewed-capture',independent_review:{reviewer:'independent-test'},tested_head:target.head,contract_digest:target.contractDigest,nonce:String(ordinal).padStart(32,'0'),status:'constituent-passed',sources:structuredClone(sources),inputs_unchanged:true,complete_consumed_input_seal:true,physical_acceptance:false,milestone_acceptance:false,reviewed_seal_identity:target.reviewedSealIdentity,external_before:structuredClone(target.reviewedSeal.external_files),external_after:structuredClone(target.reviewedSeal.external_files),constituent_capture:{kind,required_shards:shards,full_task_inventory:structuredClone(tasks),full_task_digest:part.constituents.full_task_digest,shard_id:spec.label,selected_indices:[...selected],caseCount:selected.length,caseLogs:logs,artifact_aliases:aliases,expectedWholeSuites:suites,whole_matrix_complete:false}};
  await add('manifest.json',manifest);children.push({spec,receipt:{scope:{runId:'3fd6b311',wave:0,ticket:'ticket08',branch:'codex/test',baseCommit:'a'.repeat(40)},output,head:target.head,contractDigest:target.contractDigest,action:spec.action,nonce:manifest.nonce,exitCode:0,policyDigest:'c'.repeat(64),artifacts:docs},manifest,add,packed});
 }
 return {root,part,binding,target,children,merge:()=>mergeHostCollectorConstituents({inbox:join(root,'fold'),part,binding,target,children})};
}
test('recovery exact6690 union preserves each surface transition and origin record',async t=>{
 const f=await semanticFixture(t,'recovery'),r=await f.merge();assert.equal(r.manifest.suites[0].caseCount,16);const row=JSON.parse(await readFile(join(r.output,'recovery-surfaces/S01.log')));assert.equal(row.result.cases.length,6690);assert.equal(row.result.cases[0].observer_wire_parity,true);assert.deepEqual(row.result.cases[0].transition_ids_by_origin['1'],['start']);assert.match(row.result.cases[0].log,/^recovery-0/);
});
test('recovery missing surface member rejected despite valid full task/log proof',async t=>{
 const f=await semanticFixture(t,'recovery'),v=f.children[0],{gunzipSync}=await import('node:zlib'),name='recovery/S01-endpoint.json';const row=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name+'.gz'))));row.cases.pop();await v.packed(name,row);await rewrite(v);await assert.rejects(f.merge(),/missing\/changed recovery/);
});
test('interaction preserves three complete async reports and retry constituent logs',async t=>{
 const f=await semanticFixture(t,'interaction'),r=await f.merge();assert.deepEqual(r.manifest.suites.map(s=>s.caseCount),[18,6,3]);const row=JSON.parse(await readFile(join(r.output,'async-routes/existing-selection.log')));assert.equal(row.result.contract_digest,'original-baseline-kept');assert.equal(row.result.cases.length,5);assert.ok(row.artifacts.some(x=>x.original.includes('retry-Test')));
});
test('interaction missing completion is rejected with child task indices intact',async t=>{
 const f=await semanticFixture(t,'interaction'),v=f.children[1],{gunzipSync}=await import('node:zlib'),name='asynchronous/existing-selection/report.json';const row=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name+'.gz'))));row.cases.pop();await v.packed(name,row);await rewrite(v);await assert.rejects(f.merge(),/incomplete async/);
});

test('rejects altered async wait order with unchanged count and passed statuses',async t=>{const f=await semanticFixture(t,'interaction'),v=f.children[1],{gunzipSync}=await import('node:zlib'),name='asynchronous/existing-selection/report.json';const row=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name+'.gz'))));[row.cases[0],row.cases[1]]=[row.cases[1],row.cases[0]];await v.packed(name,row);await rewrite(v);await assert.rejects(f.merge(),/completion wait\/order/);});
test('rejects changed recovery edge without relabeling task id',async t=>{const f=await semanticFixture(t,'recovery'),v=f.children[0],{gunzipSync}=await import('node:zlib'),name='recovery/results.json';const row=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name+'.gz'))));row.cases[0].edge='disconnect';await v.packed(name,row);await rewrite(v);await assert.rejects(f.merge(),/foreign recovery result/);});
for(const [name,change,error]of [
 ['library omitted from both maps',v=>{v.manifest.external_before={};v.manifest.external_after={};},/omitted from reviewed seal/],
 ['library changed consistently in both maps',v=>{v.manifest.external_before.tool.sha256=digest('different-library');v.manifest.external_after.tool.sha256=digest('different-library');},/omitted from reviewed seal/],
 ['source size changed with original SHA',v=>{v.manifest.sources['src/input-0'].size=2;},/source size\/identity/],
 ['compiler dependency omitted',v=>{v.manifest.compiler_dependencies.pop();},/compiler dependencies/],
 ['independent reviewer changed',v=>{v.manifest.independent_review={reviewer:'self'};},/independent review differs/]
])test('rejects resealed '+name+' while claiming original seal SHA',async t=>{const f=await fixture(t),v=f.children[0];change(v);await rewrite(v);await assert.rejects(f.merge(),error);});
for(const [name,change,error]of [
 ['unknown shared library',g=>{g.external_files['/unapproved/library.so']={size:5,sha256:digest('lib')};},/unsealed\/changed external/],
 ['repository input identity mismatch',g=>{g.repository['src/input-0'].size=3;},/unsealed\/changed repository/],
 ['unresolved consumed input',g=>{g.unresolved_inputs.push('/unknown');},/closed consumption graph/]
])test('rejects recomputed graph SHA with '+name,async t=>{const f=await fixture(t),v=f.children[0],name='build-orders.consumption.json.gz', {gunzipSync}=await import('node:zlib');const body=JSON.parse(gunzipSync(await readFile(join(v.receipt.output,name))));change(body);await v.add(name,gzipSync(Buffer.from(JSON.stringify(body))));await rewrite(v);await assert.rejects(f.merge(),error);});
test('requires parsed reviewed seal body, not hash-only child claim',async t=>{const f=await fixture(t);delete f.target.reviewedSeal;await assert.rejects(f.merge(),/reviewed seal body required/);});
const scopedInputs={version:2,linker_external_data_reads:'excluded',repository_and_generated_inputs:'checked',executable_mappings:'checked',non_linker_external_reads:'checked'};
async function addScope(f){f.target.reviewedSeal.consumed_input_scope=structuredClone(scopedInputs);for(const v of f.children){v.manifest.consumed_input_scope=structuredClone(scopedInputs);for(const g of v.manifest.consumption_graphs){const body={consumed_input_scope:scopedInputs,repository:{'src/input-0':f.target.reviewedSeal.sources['src/input-0']},external_files:f.target.reviewedSeal.external_files,unresolved_inputs:[]};await v.add(g.artifact,gzipSync(Buffer.from(JSON.stringify(body))));}await rewrite(v);}}
test('constituent fold preserves exact reviewed linker input scope and rejects resealed child/graph scope mismatch',async t=>{
 const f=await fixture(t);await addScope(f);const r=await f.merge();assert.deepEqual(r.manifest.consumed_input_scope,scopedInputs);
 const g=await fixture(t);await addScope(g);delete g.children[0].manifest.consumed_input_scope;await rewrite(g.children[0]);await assert.rejects(g.merge(),/scope differs/);
 const h=await fixture(t);await addScope(h);const v=h.children[0],graph=v.manifest.consumption_graphs[0];await v.add(graph.artifact,gzipSync(Buffer.from(JSON.stringify({repository:{},external_files:{},unresolved_inputs:[]}))));await rewrite(v);await assert.rejects(h.merge(),/graph\/reviewed input scope differs/);
});
