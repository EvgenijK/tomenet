import { resolve } from 'node:path';
import { readFile, open, unlink, rename } from 'node:fs/promises';
import { previewIncidentMigration, applyIncidentMigration } from './verification-migration.mjs';
import { digest } from './verification.mjs';

export async function verificationCLI(args=process.argv.slice(2),ops={}) {
  const [mode,runId,...flags]=args;
  if(!['preview','apply'].includes(mode)||!/^[a-f0-9]{8}$/.test(runId??''))throw new Error('Usage: sandbox:verification -- preview|apply <run-id> [--root <repository>] [--authorization <id> --binding-review <json> --delivery-review <json>]');
  const options={};for(let i=0;i<flags.length;i+=2){if(!['--root','--authorization','--binding-review','--delivery-review'].includes(flags[i])||!flags[i+1]||options[flags[i]])throw new Error('Invalid migration options');options[flags[i]]=flags[i+1];}
  const root=resolve(options['--root']??resolve(import.meta.dirname,'..'));
  const directory=resolve(root,'.sandcastle/runs',runId),path=resolve(directory,'state.json');
  const bytes=await readFile(path),state=JSON.parse(bytes);
  if(mode==='preview'){const preview=await previewIncidentMigration(root,state,ops);console.log(JSON.stringify(preview,null,2));return preview;}
  if(!options['--authorization']||!options['--binding-review']||!options['--delivery-review'])throw new Error('Apply requires explicit authorization and both independent review records; run remains stopped');
  const lockPath=resolve(directory,'run.lock');
  // Never remove an existing controller lock. A stale lock requires its existing
  // supported recovery path, not a migration that guesses process ownership.
  const lock=await open(lockPath,'wx',0o600);try{await lock.writeFile(String(process.pid)+'\n');await lock.sync();}finally{await lock.close();}
  try {
    if(digest(await readFile(path))!==digest(bytes))throw new Error('Saved checkpoint changed before migration lock');
    const next=await applyIncidentMigration(root,state,{authorizationId:options['--authorization'],bindingReview:JSON.parse(await readFile(resolve(options['--binding-review']),'utf8')),deliveryReview:JSON.parse(await readFile(resolve(options['--delivery-review']),'utf8'))},ops);
    const archive=resolve(directory,`verification-before-${options['--authorization']}.json`);
    const backup=await open(archive,'wx',0o600);try{await backup.writeFile(bytes);await backup.sync();}finally{await backup.close();}
    const temp=resolve(directory,'verification-state.tmp');const h=await open(temp,'wx',0o600);try{await h.writeFile(JSON.stringify(next,null,2)+'\n');await h.sync();}finally{await h.close();}
    await rename(temp,path);const d=await open(directory,'r');try{await d.sync();}finally{await d.close();}
    console.log('Prepared verification migration; run remains awaiting. Resume separately with SANDCASTLE_VERIFICATION_RESUME=1. Contract, finding and allowances remain unchanged.');return next;
  }finally{await unlink(lockPath);}
}
if(process.argv[1]&&resolve(process.argv[1])===resolve(import.meta.filename))await verificationCLI();
