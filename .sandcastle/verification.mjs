import { createHash, randomUUID } from 'node:crypto';
import { constants } from 'node:fs';
import { mkdir, readdir, lstat, open, readFile, rename, rm, chmod } from 'node:fs/promises';
import { resolve, relative, sep } from 'node:path';

export const digest = value => createHash('sha256').update(typeof value === 'string' || Buffer.isBuffer(value) ? value : JSON.stringify(value)).digest('hex');
export function verificationError(code, message) { return Object.assign(new Error(`${code}: ${message}`), { code, deterministic:true }); }
const need = (condition,code,message) => { if (!condition) throw verificationError(code,message); };
export function parseEvidenceJSON(bytes) { try { return JSON.parse(bytes); } catch { throw verificationError('EVIDENCE_MANIFEST_INVALID','structured evidence is invalid'); } }
const name = value => typeof value === 'string' && /^[a-zA-Z0-9_-]{1,100}$/.test(value);
export function assertTarget(target) {
  need(/^[a-f0-9]{40,64}$/.test(target?.head) && ['contractDigest','bindingDigest','controllerDigest'].every(k => /^[a-f0-9]{64}$/.test(target[k])) && target.runtime && typeof target.runtime.kind === 'string', 'EVIDENCE_IDENTITY','invalid target');
  return target;
}
async function durableFile(path, bytes) { const h=await open(path,'wx',0o600); try { await h.writeFile(bytes); await h.sync(); } finally { await h.close(); } }
async function syncDirectory(path) { const h=await open(path,'r'); try {await h.sync();} finally {await h.close();} }
async function files(root,dir=root,out=[]) {
  for (const entry of await readdir(dir,{withFileTypes:true})) {
    const path=resolve(dir,entry.name),stat=await lstat(path);
    need(!stat.isSymbolicLink(),'EVIDENCE_PATH','symlinks are not evidence');
    if(stat.isDirectory()) await files(root,path,out);
    else { need(stat.isFile() && stat.nlink===1,'EVIDENCE_PATH','only regular private files are evidence'); out.push(relative(root,path).split(sep).join('/')); }
  }
  return out.sort();
}
function safeArtifact(path,bytes) {
  need(!/(?:^|\/)(?:auth\.json|\.env(?:\..*)?|credentials(?:\..*)?|id_rsa|id_ed25519)$/i.test(path),'EVIDENCE_SECRET','forbidden artifact class');
  need(!/(?:authorization\s*:\s*(?:bearer|basic)\s+\S+|-----BEGIN [A-Z ]*PRIVATE KEY-----|(?:OPENAI_API_KEY|AWS_SECRET_ACCESS_KEY)\s*[=:]\s*\S+)/i.test(bytes.toString('utf8')),'EVIDENCE_SECRET','sensitive artifact rejected');
}
async function snapshotFile(source,dest) {
  const h=await open(source,constants.O_RDONLY|constants.O_NOFOLLOW);
  try {
    const before=await h.stat(); need(before.isFile() && before.nlink===1 && before.size<=64*1024*1024,'EVIDENCE_PATH','invalid or oversized artifact');
    const bytes=await h.readFile();const after=await h.stat();
    need(before.size===after.size && before.mtimeMs===after.mtimeMs && before.ctimeMs===after.ctimeMs,'EVIDENCE_CHANGED','artifact changed while copying');
    safeArtifact(source,bytes);await mkdir(resolve(dest,'..'),{recursive:true,mode:0o700});await durableFile(dest,bytes);
    return {size:bytes.length,sha256:digest(bytes)};
  } finally {await h.close();}
}

// Writable producer staging never becomes the sealed bundle. The controller
// copies into a new private directory and seals its own immutable snapshot.
export async function publishEvidence({store,staging,target,checkId,attemptId=randomUUID(),result,producer='controller',artifactPolicy,validateSnapshot}) {
  assertTarget(target);need(name(checkId)&&name(attemptId),'EVIDENCE_IDENTITY','invalid check/attempt');
  need(artifactPolicy?.syntheticOnly===true,'EVIDENCE_POLICY','only declared synthetic artifacts can be retained');
  need(Number.isInteger(result?.exitCode),'EVIDENCE_RESULT','actual exit code required');
  const parent=resolve(store,target.contractDigest,target.head,checkId);await mkdir(parent,{recursive:true,mode:0o700});
  const bundlePath=resolve(parent,attemptId),temp=resolve(parent,`.publish-${randomUUID()}`);
  const receiptPath=resolve(bundlePath,'receipt.json');
  try { await lstat(bundlePath); throw verificationError('EVIDENCE_EXISTS','attempt cannot be overwritten'); } catch(e) {if(e.code!=='ENOENT') throw e;}
  await mkdir(temp,{mode:0o700});
  try {
    const paths=await files(resolve(staging)); need(paths.length>0 && !paths.includes('receipt.json'),'EVIDENCE_INCOMPLETE','artifacts required; receipt name reserved');
    const artifacts={};for(const path of paths) artifacts[path]=await snapshotFile(resolve(staging,path),resolve(temp,path));
    const verifiedResult=structuredClone(result);
    if(validateSnapshot) { try { await validateSnapshot(temp); } catch(error) { verifiedResult.validation='failed'; verifiedResult.validationError=error.code ?? 'EVIDENCE_INCOMPLETE'; } }
    const receipt={version:1,target:structuredClone(target),checkId,attemptId,producer,result:verifiedResult,artifacts,publishedAt:new Date().toISOString()};
    const bytes=JSON.stringify(receipt,null,2)+'\n';await durableFile(resolve(temp,'receipt.json'),bytes);await syncDirectory(temp);
    for(const path of paths) await chmod(resolve(temp,path),0o400);await chmod(resolve(temp,'receipt.json'),0o400);
    await rename(temp,bundlePath);await syncDirectory(parent);
    return {version:1,bundlePath,receiptPath,receiptDigest:digest(bytes),target:structuredClone(target),checkId,attemptId,passed:verifiedResult.exitCode===0 && verifiedResult.validation!=='failed'};
  } catch(e) {await rm(temp,{recursive:true,force:true});throw e;}
}
export async function verifyEvidence(reference,target) {
  assertTarget(target);need(reference?.version===1 && digest(reference.target)===digest(target),'EVIDENCE_STALE','receipt target differs');
  const bytes=await readFile(reference.receiptPath);need(digest(bytes)===reference.receiptDigest,'EVIDENCE_INTEGRITY','receipt changed');
  const receipt=parseEvidenceJSON(bytes);need(digest(receipt.target)===digest(target)&&receipt.checkId===reference.checkId&&receipt.attemptId===reference.attemptId,'EVIDENCE_IDENTITY','receipt identity differs');
  const paths=await files(reference.bundlePath);need(JSON.stringify(paths)===JSON.stringify([...Object.keys(receipt.artifacts),'receipt.json'].sort()),'EVIDENCE_INTEGRITY','missing or extra artifact');
  for(const [path,expected] of Object.entries(receipt.artifacts)) {
    need(!path.startsWith('/') && !path.split('/').some(part => !part || part === '..' || part === '.'), 'EVIDENCE_PATH', 'invalid artifact path');
    const actual=await readFile(resolve(reference.bundlePath,path));need(actual.length===expected.size&&digest(actual)===expected.sha256,'EVIDENCE_INTEGRITY','artifact changed');
  }
  need(receipt.producer==='controller','EVIDENCE_IDENTITY','author evidence is not a controller gate');
  return {...reference,passed:receipt.result.exitCode===0 && receipt.result.validation!=='failed',integrityVerified:true};
}

export async function discoverEvidence(store,target,checkId) {
  assertTarget(target);const parent=resolve(store,target.contractDigest,target.head,checkId),found=[];
  let entries;try{entries=await readdir(parent);}catch(e){if(e.code==='ENOENT')return [];throw e;}
  for(const attemptId of entries.filter(name).sort()) {
    const bundlePath=resolve(parent,attemptId),receiptPath=resolve(bundlePath,'receipt.json');
    const bytes=await readFile(receiptPath),receipt=parseEvidenceJSON(bytes);
    const reference={version:1,bundlePath,receiptPath,receiptDigest:digest(bytes),target:receipt.target,checkId,attemptId,passed:receipt.result.exitCode===0 && receipt.result.validation!=='failed'};
    found.push(await verifyEvidence(reference,target));
  }
  return found;
}
