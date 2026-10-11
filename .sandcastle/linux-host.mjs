// Controller-owned file IPC. The worker chooses identities, never code/argv/env.
import { constants } from 'node:fs';
import { mkdir, open, readFile, readdir, lstat, realpath, rename, writeFile, rm } from 'node:fs/promises';
import { resolve, relative, dirname, isAbsolute } from 'node:path';
import { execFile, spawn } from 'node:child_process';
import { promisify } from 'node:util';
import { createHash, randomUUID } from 'node:crypto';
const exec=promisify(execFile), hash=b=>createHash('sha256').update(b).digest('hex');
const need=(ok,message)=>{if(!ok){const approval=['unapproved action','source inventory has no independent approval','full execution source tree unreviewed','independent policy review required'].includes(message);const error=new Error(approval?`Execution preflight [HOST_APPROVAL_REQUIRED]: ${message}`:`Linux host: ${message}`);if(approval)Object.assign(error,{code:'HOST_APPROVAL_REQUIRED',deterministic:true});throw error;}};
const safe=p=>typeof p==='string'&&p.length>0&&!isAbsolute(p)&&!p.split('/').some(v=>v==='..'||v==='.'||!v);
async function regular(path,max=24*1024*1024){
 const file=await open(path,constants.O_RDONLY|constants.O_NOFOLLOW);
 try{const stat=await file.stat();need(stat.isFile()&&stat.size<=max&&stat.nlink===1,'nonregular/oversized file');return await file.readFile();}finally{await file.close();}
}
async function atomic(path,value){
 const temporary=path+'.'+randomUUID()+'.tmp',file=await open(temporary,'wx',0o600);
 try{await file.writeFile(JSON.stringify(value)+'\n');await file.sync();}finally{await file.close();}
 await rename(temporary,path);
}
async function artifacts(path){
 const inventory={};let total=0;
 async function visit(base){for(const entry of await readdir(base,{withFileTypes:true})){
  const full=resolve(base,entry.name),name=relative(path,full);need(safe(name),'artifact path escape');
  if(entry.isDirectory())await visit(full);else{need(entry.isFile(),'symlink/nonregular artifact');const bytes=await regular(full);total+=bytes.length;need(total<=24*1024*1024,'artifact output limit');inventory[name]={size:bytes.length,sha256:hash(bytes)};}
 }}await visit(path);return inventory;
}
async function invoke(argv,cwd,env,timeoutMs){
 return await new Promise((accept,reject)=>{
  const child=spawn(argv[0],argv.slice(1),{cwd,env,detached:true,stdio:['ignore','pipe','pipe']});let stdout='',stderr='',size=0,failure;
  const kill=reason=>{failure??=new Error(reason);try{process.kill(-child.pid,'SIGKILL');}catch{}};
  const timer=setTimeout(()=>kill('host job timed out'),timeoutMs);
  for(const [stream,name]of [[child.stdout,'stdout'],[child.stderr,'stderr']])stream.on('data',chunk=>{size+=chunk.length;if(size>1024*1024)kill('host diagnostic output limit');else if(name==='stdout')stdout+=chunk;else stderr+=chunk;});
  child.once('error',error=>{clearTimeout(timer);reject(error);});child.once('close',code=>{clearTimeout(timer);if(failure)reject(failure);else accept({exitCode:code??1,stdout,stderr});});
 });
}
export async function startLinuxHostChannel({root,policyPath,scope,directory,inspectOnly=false}){
 root=await realpath(root);policyPath=await realpath(policyPath);
 const policyStat=await lstat(policyPath);need(policyStat.isFile()&&(policyStat.mode&0o077)===0,'policy must be private regular file');
 const policyBytes=await regular(policyPath),policy=JSON.parse(policyBytes),policyDigest=hash(policyBytes);
 need(policy.version===1&&policy.review?.reviewer&&policy.review?.reference,'independent policy review required');
 need(policy.runId===scope.runId&&/^[a-f0-9]{64}$/.test(policy.contractDigest),'run/digest policy mismatch');
 need(policy.maxJobs>0&&policy.maxJobs<=64&&Array.isArray(policy.approvals),'bounded approvals required');
 const launcher=await realpath(policy.launcherCheckout),git=async(...args)=>(await exec('git',['-C',root,...args],{maxBuffer:32*1024*1024})).stdout.trim();
 async function runtime(){
  need(hash(await regular(policyPath))===policyDigest,'reviewed policy changed');
  need((await exec('git',['-C',launcher,'rev-parse','HEAD'])).stdout.trim()===policy.launcherHead,'launcher HEAD changed');
  need(!(await exec('git',['-C',launcher,'status','--porcelain','--untracked-files=no'])).stdout.trim(),'launcher checkout dirty');
  const files={};for(const [path,expected]of Object.entries(policy.launcherFiles??{})){
   need(safe(path),'launcher path escape');const full=resolve(launcher,path);need(await realpath(full)===full,'launcher symlink');files[path]=hash(await regular(full));need(files[path]===expected,'launcher code changed');
  }
  need(Object.keys(files).length>0,'launcher code approval absent');
  const tools={};for(const [path,expected]of Object.entries(policy.runtimeFiles??{})){tools[path]=hash(await regular(await realpath(path),256*1024*1024));need(tools[path]===expected,'host toolchain changed');}need(Object.keys(tools).length>0,'host toolchain approval absent');
  const desktop={};for(const key of ['DISPLAY','WAYLAND_DISPLAY','XDG_RUNTIME_DIR','DBUS_SESSION_BUS_ADDRESS']){need(process.env[key],'host desktop environment missing');desktop[key]=process.env[key];}
  // X11 authentication stays solely in the host process environment. Neither
  // cookie bytes nor the authority file enter policy, artifacts or Docker.
  if(process.env.XAUTHORITY)desktop.XAUTHORITY=process.env.XAUTHORITY;
  return {kind:'bounded-linux-host',policyDigest,launcherHead:policy.launcherHead,files,tools,desktopDigest:hash(JSON.stringify(desktop))};
 }
 const identity=await runtime();if(inspectOnly)return {identity,authorize};
 directory=resolve(directory);need(!await lstat(directory).catch(()=>null),'fresh channel directory required');
 await mkdir(directory,{recursive:true,mode:0o700});await mkdir(resolve(directory,'requests'),{mode:0o700});await mkdir(resolve(directory,'results'),{mode:0o700});
 const privateDir=resolve(dirname(policyPath),'host-jobs');await mkdir(privateDir,{recursive:true,mode:0o700});need(await realpath(privateDir)===privateDir,'private job directory symlink');
 const stats={executed:0,rejected:0};let closed=false,busy=false,count=0,serial=Promise.resolve();
 async function authorize(request,binding){
  await runtime();need(request&&Object.keys(request).sort().join(',')===['version','runId','wave','ticket','branch','baseCommit','head','contractDigest','nonce','action','driver','renderer'].sort().join(','),'unknown request fields');
  need(request.version===1&&['runId','wave','ticket','branch','baseCommit'].every(k=>request[k]===scope[k]),'scope/run/wave/ticket/branch mismatch');
  need(request.contractDigest===policy.contractDigest,'wrong digest');need(/^[a-f0-9]{40}$/.test(request.head)&&/^[a-f0-9]{32}$/.test(request.nonce),'invalid HEAD/nonce');
  need(['wayland','x11'].includes(request.driver)&&['software','opengl'].includes(request.renderer),'unsupported driver/renderer');
  const action=policy.actions?.[request.action];need(action&&Array.isArray(action.argv),'unapproved action');
  need(!(action.controllerOnly||action.checkId)||binding,'capture is controller-only');
  if(binding){need(action.checkId===binding.id&&JSON.stringify(action.bindingCommand)===JSON.stringify(binding.command),'unreviewed final collector binding');}
  need(await git('rev-parse',`refs/heads/${scope.branch}`)===request.head,'branch HEAD differs');await git('merge-base','--is-ancestor',scope.baseCommit,request.head);
  let approval;
  for(const candidate of policy.approvals){
   if(!candidate.actions?.includes(request.action))continue;
   try{await git('merge-base','--is-ancestor',candidate.base,request.head);}catch{continue;}
   need(action.inventory?.files&&action.inventory?.roots,'action inventory required');
   const consumed=(await git('ls-tree','-r','--name-only',request.head,'--',...action.inventory.roots,...action.inventory.files)).split('\n').filter(Boolean).sort();
   if(JSON.stringify(consumed)!==JSON.stringify(Object.keys(candidate.inputs??{}).sort()))continue;
   let matches=true;for(const [path,expected]of Object.entries(candidate.inputs??{})){
    need(safe(path),'approval input path escape');const mode=await git('ls-tree',request.head,'--',path);if(!/^(100644|100755) blob /.test(mode)){matches=false;break;}
    const bytes=(await exec('git',['-C',root,'show',`${request.head}:${path}`],{encoding:'buffer',maxBuffer:24*1024*1024})).stdout;
    if(hash(bytes)!==expected){matches=false;break;}
   }if(matches&&Object.keys(candidate.inputs??{}).length){approval=candidate;break;}
  }need(approval,'source inventory has no independent approval');
  // Exhaustive capture inventory: reviewed whole tree, not guessed roots. Any
  // producer change needs a new independent approval before host execution.
  if(binding||action.sourceWorktree)need(approval.tree===await git('rev-parse',`${request.head}^{tree}`),'full execution source tree unreviewed');
  return {action,approval};
 }
 async function perform(request,{binding,output}={}){
  const {action}=await authorize(request,binding);
  need(++count<=policy.maxJobs,'job limit reached');
  const job=resolve(privateDir,request.nonce);try{await mkdir(job,{mode:0o700});}catch{throw new Error('Linux host: replayed nonce');}
  const resultOut=resolve(job,'output');
  let source=launcher;
  const materialized=Boolean(binding||action.sourceWorktree);
  if(materialized){
   source=resolve(job,'source');await mkdir(source,{mode:0o700});
   await rm(source,{recursive:true});await exec('git',['-C',root,'worktree','add','--detach',source,request.head]);
  }
  const placeholders={$OUTPUT:resultOut,$HEAD:request.head,$DIGEST:request.contractDigest,$NONCE:request.nonce,$DRIVER:request.driver,$RENDERER:request.renderer,$SOURCE:source};
  const argv=action.argv.map(value=>placeholders[value]??value.replace(/^\$SOURCE\//,source+'/'));need(action.timeoutMs>0&&action.timeoutMs<=300000,'invalid timeout');
  const env={PATH:'/usr/local/bin:/usr/bin:/bin',LANG:'C.UTF-8',HOME:resolve(job,'home')};for(const key of ['DISPLAY','WAYLAND_DISPLAY','XDG_RUNTIME_DIR','DBUS_SESSION_BUS_ADDRESS'])env[key]=process.env[key];
  if(request.driver==='x11'&&process.env.XAUTHORITY)env.XAUTHORITY=process.env.XAUTHORITY;
  await mkdir(env.HOME,{mode:0o700});stats.executed++;
  let result;
  try{result=await invoke(argv,source,env,action.timeoutMs);if(materialized)need(!(await exec('git',['-C',source,'status','--porcelain','--untracked-files=no'])).stdout.trim()&&(await exec('git',['-C',source,'rev-parse','HEAD'])).stdout.trim()===request.head,'capture source changed');}
  finally{if(materialized)await exec('git',['-C',root,'worktree','remove','--force',source]);}
  await runtime();
  const inventory=await artifacts(resultOut);for(const required of action.requiredArtifacts??[])need(inventory[required],'incomplete artifact return');need(Object.keys(inventory).length,'empty artifact return');
  // Private results remain authoritative; shared copies are verified on read.
  if(output){const stat=await lstat(output).catch(()=>null);need(!stat||(stat.isDirectory()&&!stat.isSymbolicLink()&&!(await readdir(output)).length),'fresh empty final output required');if(!stat)await mkdir(output,{mode:0o700});for(const name of Object.keys(inventory)){await mkdir(dirname(resolve(output,name)),{recursive:true});await writeFile(resolve(output,name),await regular(resolve(resultOut,name)),{mode:0o600});}}
  const receipt={version:1,scope,head:request.head,contractDigest:request.contractDigest,nonce:request.nonce,action:request.action,policyDigest,exitCode:result.exitCode,artifacts:inventory,output:output??resultOut};
  await atomic(resolve(job,'receipt.json'),receipt);return receipt;
 }
 const request=(job,options)=>{const result=serial.then(()=>perform(job,options));serial=result.catch(()=>{});return result;};
 async function pump(){if(closed||busy)return;busy=true;try{
  for(const name of (await readdir(resolve(directory,'requests'))).sort().slice(0,16)){
   if(!/^[a-f0-9]{32}\.json$/.test(name))continue;
   const full=resolve(directory,'requests',name),claimed=full+'.claimed';
   try{need(stats.rejected<64,'channel rejected-request limit reached');await rename(full,claimed);const job=JSON.parse(await regular(claimed,4096));need(job.nonce+'.json'===name,'filename nonce mismatch');
    const receipt=await request(job);const returned={...receipt,output:undefined};
    const files={};for(const [path,entry]of Object.entries(receipt.artifacts)){const bytes=await regular(resolve(receipt.output,path));need(hash(bytes)===entry.sha256,'changed return artifact');files[path]=bytes.toString('base64');}
    await atomic(resolve(directory,'results',name),{...returned,files});
   }catch(error){stats.rejected++;await atomic(resolve(directory,'results',name),{error:String(error.message),diagnosticCode:error.code});}
   finally{await rm(claimed,{force:true});}if(stats.rejected>=64){closed=true;clearInterval(timer);break;}
  }
 }finally{busy=false;}}
 const timer=setInterval(()=>pump().catch(()=>{stats.rejected++;}),100);timer.unref();
 return {directory,stats,request,runtime,authorize,mount:{hostPath:directory,sandboxPath:'/home/agent/.sv-host-jobs',readonly:false},async close(){closed=true;clearInterval(timer);while(busy)await new Promise(r=>setTimeout(r,20));await serial;}};
}
