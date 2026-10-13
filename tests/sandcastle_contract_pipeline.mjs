import assert from 'node:assert/strict';
import test from 'node:test';
import { mkdtemp, readFile, rm, stat, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { acceptContract, baselineChecks, contractStage, validateContract } from '../.sandcastle/acceptance.mjs';
import { resumeSemanticContractAmendment, validateContractAmendment } from '../.sandcastle/contract-amend.mjs';
import { planContractGroups, buildGroupedContractDraft } from '../.sandcastle/contract-groups.mjs';
import { reconcileContractDraft } from '../.sandcastle/contract-reconcile.mjs';
import { ensureFragmentCatalog, loadFragmentCatalog, readContractArtifact, writeContractArtifact } from '../.sandcastle/contract-storage.mjs';
import { readContractFragments } from '../.sandcastle/contract-fragments.mjs';

test('persisted fragments survive an interrupted grouped acceptance and publish only ID references', async t => {
  const directory = await mkdtemp(join(tmpdir(), 'contract-pipeline-'));
  t.after(() => rm(directory, { recursive: true, force: true }));
  const fragmentDirectory = join(directory, 'source-fragments');
  const sources = { 'spec.md': 'Implement the production outcome.\n', 'AGENTS.md': 'Preserve baseline behavior.\n' };
  const originalCatalog = await ensureFragmentCatalog({ directory: fragmentDirectory, sources });
  const originalFiles = await Promise.all(originalCatalog.fragments.map(async ({ id }) => ({
    id, bytes: await readFile(join(fragmentDirectory, id)), modified: (await stat(join(fragmentDirectory, id))).mtimeMs,
  })));
  const state = { id: 'fixture', phase: 'contract' };
  let catalog;
  let drafts = 0;
  let inspections = 0;
  let published = 0;
  let saved;
  const ops = {
    sources: async () => {
      catalog = await ensureFragmentCatalog({ directory: fragmentDirectory, sources, expectedDigest: originalCatalog.digest });
      return sources;
    },
    catalog: () => catalog,
    save: async current => { saved = JSON.parse(JSON.stringify(current)); },
    head: async () => 'a'.repeat(40),
    propose: async (input, _feedback, pending) => {
      const { parts, plan } = await planContractGroups(pending, input, async () => ({ groups: [
        { title: 'Production', partIndexes: [0] }, { title: 'Policy', partIndexes: [1] },
      ] }), async () => {});
      const draft = await buildGroupedContractDraft(pending, parts, plan, async group => {
        drafts++;
        const ids = group.sourceSpans.map(({ id }) => id);
        const fragments = await readContractFragments(fragmentDirectory, ids);
        return { version: 2, completionScope: 'implementation', checks: structuredClone(baselineChecks), criteria: [{
          id: 'AC1', requirement: `Satisfy ${fragments[0].sourcePath}`, sourceSpanIds: ids,
          mandatory: true, applicability: 'current', owner: '', deferralReason: '', checkIds: ['sv-core'],
        }] };
      }, async () => {}, { catalog });
      return reconcileContractDraft(pending, draft, input, async () => ({
        summary: 'All obligations represented', completionScope: null,
        criterionEdits: [], checkEdits: [], criteriaToAdd: [], checksToAdd: [],
      }), async () => {}, catalog);
    },
    verify: async proposal => {
      inspections++;
      validateContract(proposal, sources, catalog);
      if (inspections === 1) throw new Error('interrupted independent review');
      return { approved: true, summary: 'Complete independently verified contract', missingRequirements: [], findings: [] };
    },
    publish: async accepted => {
      published++;
      await writeContractArtifact(directory, {
        contract: accepted.contract, digest: accepted.contractDigest, verification: accepted.contractReview,
      });
    },
  };
  await assert.rejects(contractStage(state, ops), /interrupted independent review/);
  assert.equal(published, 0);
  const resumed = saved;
  await contractStage(resumed, ops);
  assert.equal(drafts, 2, 'saved group answers and assembled proposal are reused');
  assert.equal(inspections, 2);
  assert.equal(published, 1);
  assert.equal(resumed.phase, 'plan');
  assert.equal(resumed.acceptance.fragmentCatalogDigest, originalCatalog.digest);
  const artifact = await readContractArtifact(directory);
  assert.deepEqual(artifact.contract, resumed.acceptance.contract);
  const manifest = JSON.parse(await readFile(join(directory, 'acceptance-contract.json'), 'utf8'));
  assert.deepEqual(manifest.contract.criteria, ['P1-AC1', 'P2-AC1']);
  assert.deepEqual(manifest.contract.checks, ['sv-build', 'sv-core']);
  for (const criterion of artifact.contract.criteria) {
    const file = JSON.parse(await readFile(join(directory, 'criteria', criterion.id), 'utf8'));
    assert.deepEqual(file, criterion);
    assert.equal(file.source, undefined);
    assert.equal(file.quote, undefined);
    assert.ok(file.sourceSpanIds.every(id => originalCatalog.fragments.some(fragment => fragment.id === id)));
  }
  for (const check of artifact.contract.checks) {
    assert.deepEqual(JSON.parse(await readFile(join(directory, 'checks', check.id), 'utf8')), check);
  }
  for (const file of originalFiles) {
    assert.deepEqual(await readFile(join(fragmentDirectory, file.id)), file.bytes);
    assert.equal((await stat(join(fragmentDirectory, file.id))).mtimeMs, file.modified);
  }
  const appended = await ensureFragmentCatalog({ directory: fragmentDirectory,
    sources: { ...sources, 'spec.md': sources['spec.md'] + 'Support the newly requested outcome.\n' }, allowAppend: true });
  assert.equal(appended.fragments.length, originalCatalog.fragments.length + 1);
  for (const file of originalFiles) assert.deepEqual(await readFile(join(fragmentDirectory, file.id)), file.bytes);
  assert.deepEqual((await readContractArtifact(directory)).contract, artifact.contract, 'catalog append does not rewrite accepted criteria');
  await writeFile(join(fragmentDirectory, originalFiles[0].id), '{}');
  await assert.rejects(loadFragmentCatalog(fragmentDirectory), /integrity/);
});

test('authorized semantic amendment appends fragment files and publishes new criteria preserving the original contract', async t => {
  const directory = await mkdtemp(join(tmpdir(), 'contract-amend-pipeline-'));
  t.after(() => rm(directory, { recursive: true, force: true }));
  const fragmentDirectory = join(directory, 'source-fragments');
  const sources = { 'spec.md': 'Preserve the production outcome.\n' };
  const catalog = await ensureFragmentCatalog({ directory: fragmentDirectory, sources });
  const criterion = { id: 'AC1', requirement: 'Preserve production outcome', sourceSpanIds: [catalog.fragments[0].id],
    mandatory: true, applicability: 'current', owner: '', deferralReason: '', checkIds: ['sv-core'] };
  const contract = { version: 2, completionScope: 'implementation', criteria: [criterion], checks: structuredClone(baselineChecks) };
  const review = { approved: true, summary: 'Independently approved', missingRequirements: [], findings: [] };
  const state = { id: 'fixture', specPath: 'spec.md', spec: sources['spec.md'], phase: 'awaiting', resumePhase: 'plan',
    pauseReason: 'User-approved requirement amendment is not represented in the immutable acceptance contract: support reconnect.' };
  acceptContract(state, contract, sources, review, 'a'.repeat(40), catalog);
  const previous = structuredClone(state.acceptance);
  await writeContractArtifact(directory, { contract, digest: previous.contractDigest, verification: review });
  const originalCriterion = await readFile(join(directory, 'criteria', 'AC1'));
  const originalFragment = await readFile(join(fragmentDirectory, catalog.fragments[0].id));
  const additionalRequirement = 'Support reconnect.';
  const amendedSources = { 'spec.md': state.spec + additionalRequirement + '\n' };
  resumeSemanticContractAmendment(state, amendedSources['spec.md'], {
    authorizationId: 'user-add-reconnect', decision: 'Support reconnect', requiredQuotes: [additionalRequirement],
  }, catalog);
  let extended;
  await contractStage(state, {
    sources: async () => {
      extended = await ensureFragmentCatalog({ directory: fragmentDirectory, sources: amendedSources,
        allowAppend: true, expectedDigest: catalog.digest });
      return amendedSources;
    },
    catalog: () => extended,
    save: async () => {}, head: async () => 'b'.repeat(40),
    propose: async () => {
      const added = extended.fragments.find(fragment => fragment.quote === additionalRequirement);
      return { ...structuredClone(contract), criteria: [structuredClone(criterion), {
        ...structuredClone(criterion), id: 'AC2', requirement: 'Support reconnect', sourceSpanIds: [added.id],
      }] };
    },
    validateProposal: proposal => validateContractAmendment(previous.contract, proposal, state.contractAmendment, extended),
    verify: async () => review,
    publish: accepted => writeContractArtifact(directory, {
      contract: accepted.contract, digest: accepted.contractDigest, verification: accepted.contractReview,
    }, { allowAmendment: true }),
  });
  assert.equal(state.phase, 'plan');
  assert.deepEqual(state.contractAmendments[0].acceptance, previous);
  assert.equal(state.acceptance.contract.criteria.length, 2);
  assert.deepEqual((await readContractArtifact(directory)).contract, state.acceptance.contract);
  assert.deepEqual(await readFile(join(directory, 'criteria', 'AC1')), originalCriterion);
  assert.deepEqual(await readFile(join(fragmentDirectory, catalog.fragments[0].id)), originalFragment);
  assert.equal(state.acceptance.fragmentCatalogDigest, extended.digest);
});
