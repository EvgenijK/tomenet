const { chromium } = require(process.env.ROOM_TEMPLATE_PLAYWRIGHT || 'playwright');
const fs = require('node:fs/promises');
const path = require('node:path');
const assert = require('node:assert/strict');
const root = path.resolve(process.argv[2] || path.join(__dirname, '../../..'));
const cell = (page, x, y) => page.locator(`#grid .cell[data-x="${x}"][data-y="${y}"]`);
async function drag(page, start, end) {
  const a = await start.boundingBox(), b = await end.boundingBox();
  await page.mouse.move(a.x + a.width / 2, a.y + a.height / 2);
  await page.mouse.down();
  await page.mouse.move(b.x + b.width / 2, b.y + b.height / 2, { steps: 8 });
  await page.mouse.up();
}
async function download(page, selector, name) {
  const promise = page.waitForEvent('download');
  await page.locator(selector).click();
  const result = await promise;
  const output = `/tmp/${name}`;
  await result.saveAs(output);
  return { output, text: await fs.readFile(output, 'utf8') };
}
(async () => {
  const browser = await chromium.launch({ channel: process.env.ROOM_TEMPLATE_BROWSER_CHANNEL || 'chrome', headless: true });
  try {
    const page = await browser.newPage({ viewport: { width: 1500, height: 1100 } });
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('dialog', dialog => dialog.accept());
    await page.goto(`file://${root}/tools/room_template_editor/index.html`);
    assert.match(await page.title(), /TomeNET/);
    await page.locator('#catalog-input').setInputFiles(path.join(root, 'lib/game'));
    await page.waitForFunction(() => document.querySelector('#catalog-status').classList.contains('ok'));
    await page.locator('#new-code').fill('browser_resume');
    await page.locator('#new-width').fill('4');
    await page.locator('#new-height').fill('4');
    await page.locator('#create-button').click();
    await page.locator('#add-terrain').click();
    await page.locator('#terrain-symbol').fill('.');
    await page.locator('#terrain-feature').selectOption('1');
    await page.locator('#terrain-form button.primary').click();
    await page.locator('[data-tool="rectangle"]').click();
    await cell(page, 0, 0).scrollIntoViewIfNeeded();
    await drag(page, cell(page, 0, 0), cell(page, 2, 2));
    for (let y = 0; y < 3; y++) for (let x = 0; x < 3; x++) {
      assert.match(await cell(page, x, y).getAttribute('title'), /T:\./);
    }
    await page.mouse.move(...await cell(page, 3, 3).evaluate(node => { const b = node.getBoundingClientRect(); return [b.x + b.width / 2, b.y + b.height / 2]; }));
    assert.match(await cell(page, 3, 3).getAttribute('title'), /T:_/);
    await page.locator('#add-monster').click();
    await page.locator('#monster-symbol').fill('m');
    await page.locator('#monster-r-idx').fill('29');
    await page.locator('#monster-ego-mode').selectOption('none');
    await page.locator('#monster-form button.primary').click();
    await page.locator('[data-layer="monsters"]').click();
    await page.locator('[data-tool="brush"]').click();
    await cell(page, 1, 1).click();
    await page.locator('#add-item').click();
    await page.locator('#item-symbol').fill('i');
    await page.locator('#item-tval').fill('80');
    await page.locator('#item-sval').fill('1');
    await page.locator('#item-form button.primary').click();
    await page.locator('[data-layer="items"]').click();
    await cell(page, 1, 1).click();
    await page.locator('[data-layer="entrances"]').click();
    await cell(page, 0, 1).click();
    assert.match(await page.locator('#validation-count').innerText(), /Готово/);
    const draft = await download(page, '#save-draft', 'room-templates-resumed-draft.json');
    const saved = JSON.parse(draft.text);
    assert.equal(saved.maps.monsters[1][1], 'm');
    assert.equal(saved.maps.items[1][1], 'i');
    assert.deepEqual(saved.entrances, [{ x: 0, y: 1 }]);
    const undoBefore = await page.locator('#undo').isDisabled();
    await cell(page, 3, 3).click();
    assert.equal(await page.locator('#dirty-status').innerText(), 'Изменения сохранены');
    assert.equal(await page.locator('#undo').isDisabled(), undoBefore);
    const game = await download(page, '#export-game', 'browser_resume.txt');
    assert.match(game.text, /M:m:r_idx:29/);
    assert.match(game.text, /I:i:tval:80/);
    assert.match(game.text, /E:0:1/);
    await page.locator('#template-input').setInputFiles(game.output);
    await page.waitForFunction(() => document.querySelector('#validation-count').textContent === 'Готово');
    const roundtrip = await download(page, '#save-draft', 'room-templates-resumed-roundtrip.json');
    assert.deepEqual(JSON.parse(roundtrip.text), saved);
    await page.locator('#orientations').click();
    assert.equal(await page.locator('.orientation-card').count(), 8);
    assert.equal(await page.locator('.orientation-card .entities').count(), 8);
    await page.locator('#orientation-dialog button').click();
    await page.locator('#server-profile').selectOption('arcade');
    await page.waitForFunction(() => document.querySelector('#catalog-status').classList.contains('ok'));
    await page.locator('#server-profile').selectOption('main');
    await page.waitForFunction(() => document.querySelector('#catalog-status').classList.contains('ok'));
    assert.deepEqual(errors, []);
    console.log('PASS: file:// catalog folder, UI T/M/I forms, rectangle/brush, pointer release, entrance dirty-state, JSON/TXT roundtrip, eight orientations, server profile reload; no page errors.');
    console.log(`Exported fixture: ${game.output}`);
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
