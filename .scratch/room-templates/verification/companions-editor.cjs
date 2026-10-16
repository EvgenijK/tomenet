const assert = require('node:assert/strict');
const {readFileSync} = require('node:fs');
const vm = require('node:vm');
const editorPath = process.argv[2] || '/home/svechnik/Projects/github_site/tomenet_room_template_editor/app.js';
const elements = new Map();
const element = id => {
  if (!elements.has(id)) elements.set(id, {value:'', textContent:'', listeners:{}, classList:{add(){},remove(){},toggle(){}}, addEventListener(t,fn){this.listeners[t]=fn;}, showModal(){},close(){},replaceChildren(){},append(){},querySelectorAll(){return [];}});
  return elements.get(id);
};
const context=vm.createContext({document:{getElementById:element,querySelectorAll:()=>[],createElement:()=>element(Symbol())},window:{addEventListener(){}},setTimeout(){}});
vm.runInContext(readFileSync(editorPath,'utf8'),context);
const run = code => vm.runInContext(code,context);
run(`model=newModel('test_room','Test',1,1);model.monsterProfiles={a:emptyMonsterProfile(),b:emptyMonsterProfile()};model.monsterProfiles.a.companions='none';model.maps.monsters=[['a']];`);
assert.match(run('gameText()'),/M:a:companions:none/);
assert.doesNotMatch(run('monsterLines("b",model.monsterProfiles.b).join("\\n")'),/companions/);
assert.equal(run('parseGameText(gameText()).monsterProfiles.a.companions'),'none');
assert.equal(run('normalizeModel(JSON.parse(JSON.stringify(model))).monsterProfiles.a.companions'),'none');
assert.equal(run('normalizeMonsterProfile({}).companions'),null);
for (const value of ['', 'default', 'NONE', 'none:extra']) {
  assert.throws(()=>run(`parseProfileLine(${JSON.stringify('M:x:companions:'+value)},1,{},'M')`),/invalid companions mode/);
}
assert.throws(()=>run(`{const profiles={};parseProfileLine('M:x:companions:none',1,profiles,'M');parseProfileLine('M:x:companions:none',2,profiles,'M');}`),/duplicate field/);
assert.ok(run(`{const errors=[];races=new Map([[1,{id:1,flags:new Set(),level:1}]]);const p=emptyMonsterProfile();p.companions='wrong';validateMonsterProfile('a',p,message=>errors.push(message));errors.some(message=>message.includes('companions'));}`));
run(`catalogsReady=true;openMonsterDialog('a');`);
assert.equal(element('monster-companions').value,'none');
run(`checkpoint=()=>{};render=()=>{};`);
element('monster-form').listeners.submit({preventDefault(){}});
assert.equal(run('model.monsterProfiles.a.companions'),'none');
element('monster-companions').value='';
element('monster-form').listeners.submit({preventDefault(){}});
assert.equal(run('model.monsterProfiles.a.companions'),null);
console.log('PASS: TXT/JSON round trips, defaults, invalid values, duplicates, validation and profile form');
