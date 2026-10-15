const assert = require("node:assert/strict");
const {readFileSync} = require("node:fs");
const {test} = require("node:test");
const vm = require("node:vm");
const path = require("node:path");

function editor(picker) {
  const elements = new Map(), downloads = [], revoked = [], timers = [];
  const element = id => {
    if (!elements.has(id)) elements.set(id, {
      textContent: "", listeners: {}, classList: {add() {}, remove() {}},
      addEventListener(type, callback) { this.listeners[type] = callback; },
    });
    return elements.get(id);
  };
  const context = vm.createContext({
    document: {
      getElementById: element, querySelectorAll: () => [],
      createElement: () => ({click() { downloads.push({name: this.download, href: this.href}); }}),
    },
    window: {addEventListener() {}, ...(picker ? {showSaveFilePicker: picker} : {})},
    URL: {createObjectURL: () => "blob:template", revokeObjectURL: url => revoked.push(url)},
    Blob, setTimeout: callback => timers.push(callback),
  });
  vm.runInContext(readFileSync(path.join(__dirname, "../app.js"), "utf8"), context);
  vm.runInContext('model=newModel("test_room","Комната",1,1);setDirty(true);', context);
  return {
    context, downloads, revoked, timers,
    click: id => element(id).listeners.click(),
    dirty: () => vm.runInContext("dirty", context),
    toast: () => element("toast").textContent,
  };
}

test("repeated TXT and JSON exports use the save picker, without automatic downloads", async () => {
  const options = [], contents = [], events = [];
  const app = editor(async value => {
    options.push(value);
    return {async createWritable() { return {
      async write(text) { events.push("write"); contents.push(text); },
      async close() { events.push("close"); },
    }; }};
  });
  await app.click("export-game");
  await app.click("export-game");
  await app.click("save-draft");
  assert.equal(options.length, 3);
  assert.equal(options[0].suggestedName, "test_room.txt");
  assert.equal(options[2].suggestedName, "test_room.json");
  assert.deepEqual(Array.from(options[0].types[0].accept["text/plain"]), [".txt"]);
  assert.deepEqual(Array.from(options[2].types[0].accept["application/json"]), [".json"]);
  assert.equal(contents[0], vm.runInContext("gameText()", app.context));
  assert.equal(contents[1], contents[0]);
  assert.equal(JSON.parse(contents[2]).name, "Комната");
  assert.deepEqual(events, ["write", "close", "write", "close", "write", "close"]);
  assert.equal(app.downloads.length, 0);
  assert.equal(app.dirty(), false);
});

test("cancelling the picker preserves unsaved changes without a fallback download", async () => {
  const app = editor(async () => { throw Object.assign(new Error("Cancelled"), {name: "AbortError"}); });
  await app.click("export-game");
  assert.equal(app.dirty(), true);
  assert.equal(app.downloads.length, 0);
  assert.equal(app.toast(), "");
});

for (const stage of ["picker", "createWritable", "write", "close"]) {
  test(`a ${stage} failure reports the error and preserves unsaved changes`, async () => {
    const fail = () => { throw new Error("Диск недоступен"); };
    const app = editor(stage === "picker" ? fail : async () => ({
      createWritable: stage === "createWritable" ? fail : async () => ({
        write: stage === "write" ? fail : async () => {},
        close: stage === "close" ? fail : async () => {},
      }),
    }));
    await app.click("save-draft");
    assert.equal(app.dirty(), true);
    assert.equal(app.downloads.length, 0);
    assert.match(app.toast(), /Диск недоступен/);
  });
}

test("edits made during a save stay unsaved", async () => {
  let finish;
  const app = editor(async () => ({async createWritable() { return {
    async write() {}, close: () => new Promise(resolve => { finish = resolve; }),
  }; }}));
  const saving = app.click("save-draft");
  // Let the picker and write promises resolve, leaving close pending.
  await new Promise(resolve => setImmediate(resolve));
  vm.runInContext('model.name="Новое название";setDirty(true);', app.context);
  finish();
  await saving;
  assert.equal(app.dirty(), true);
});

test("browsers without the picker retain the download fallback", async () => {
  const app = editor();
  await app.click("export-game");
  assert.equal(app.downloads[0].name, "test_room.txt");
  assert.equal(app.downloads[0].href, "blob:template");
  app.timers.forEach(callback => callback());
  assert.deepEqual(app.revoked, ["blob:template"]);
});
