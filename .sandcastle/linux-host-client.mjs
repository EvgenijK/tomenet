// Tiny container client; no host filesystem/session environment required.
import { mkdir, readFile, writeFile, rename, lstat } from 'node:fs/promises';
import { resolve, dirname, isAbsolute } from 'node:path';
import { createHash, randomBytes } from 'node:crypto';
import { execFileSync } from 'node:child_process';
const hash=b=>createHash('sha256').update(b).digest('hex');
export async function requestLinuxHost({directory,scope,action='preflight',driver='wayland',renderer='software',output,nonce=randomBytes(16).toString('hex'),head}){
 if(!/^[a-f0-9]{32}$/.test(nonce)||!output||await lstat(output).catch(()=>null))throw new Error('fresh nonce/output required');
 const request={version:1,...scope,head,nonce,action,driver,renderer};
 const path=resolve(directory,'requests',nonce+'.json'),temporary=path+'.tmp';await writeFile(temporary,JSON.stringify(request),{flag:'wx',mode:0o600});await rename(temporary,path);
 const deadline=Date.now()+310000;let receipt;
 while(Date.now()<deadline){try{receipt=JSON.parse(await readFile(resolve(directory,'results',nonce+'.json')));break;}catch(error){if(error.code!=='ENOENT')throw error;}await new Promise(r=>setTimeout(r,100));}
 if(!receipt)throw new Error('host channel timeout');if(receipt.error){const error=new Error(receipt.error);if(receipt.diagnosticCode)Object.assign(error,{code:receipt.diagnosticCode,deterministic:true});throw error;}
 if(receipt.head!==head||receipt.contractDigest!==scope.contractDigest||receipt.nonce!==nonce||receipt.action!==action||JSON.stringify(receipt.scope)!==JSON.stringify(Object.fromEntries(Object.entries(scope).filter(([k])=>k!=='contractDigest'))))throw new Error('stale host receipt');
 await mkdir(output,{mode:0o700});
 for(const [name,entry]of Object.entries(receipt.artifacts)){
  if(isAbsolute(name)||name.split('/').some(v=>!v||v==='.'||v==='..'))throw new Error('artifact path escape');
  const bytes=Buffer.from(receipt.files[name]??'','base64');if(bytes.length!==entry.size||hash(bytes)!==entry.sha256)throw new Error('incomplete/changed host artifact');
  await mkdir(dirname(resolve(output,name)),{recursive:true});await writeFile(resolve(output,name),bytes,{mode:0o600});
 }await writeFile(resolve(output,'channel-receipt.json'),JSON.stringify({...receipt,files:undefined},null,2),{mode:0o600});return receipt;
}
if(process.argv[1]===import.meta.filename){
 const [scopeFile,action,renderer,output]=process.argv.slice(2),scope=JSON.parse(await readFile(scopeFile));
 const head=execFileSync('git',['rev-parse','HEAD']).toString().trim();
 const receipt=await requestLinuxHost({directory:'/home/agent/.sv-host-jobs',scope,head,action,renderer,output});console.log(JSON.stringify({head:receipt.head,exitCode:receipt.exitCode,nonce:receipt.nonce}));process.exitCode=receipt.exitCode;
}
