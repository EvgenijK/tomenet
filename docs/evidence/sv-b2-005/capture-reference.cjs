// One-off replay of the approved MOTD C prototype, not a test suite.
const {chromium}=require('/home/svechnik/.nvm/versions/node/v26.2.0/lib/node_modules/@playwright/cli/node_modules/playwright');
const fs=require('fs'),path=require('path'),{pathToFileURL}=require('url');
const out=path.resolve(process.argv[2]||'docs/evidence/sv-b2-005');fs.mkdirSync(out,{recursive:true});
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'/home/svechnik/.cache/ms-playwright/chromium-1237/chrome-linux64/chrome',args:['--no-sandbox']});
 const page=await browser.newPage({viewport:{width:1440,height:900},deviceScaleFactor:1});
 await page.goto(pathToFileURL(path.resolve('.scratch/single-window-sdl3-client/motd-prototype.html')).href+'?variant=C');
 const fixtures={};
 for(const fixture of ['short','wide','notice','unknown','empty']){
  fixtures[fixture]=await page.evaluate(k=>{const rows=rowsFor(k);if(k==='wide')rows.forEach(row=>row.forEach(cell=>cell.color='w'));return rows;},fixture);
  for(const [width,height] of [[1440,900],[1024,768]]){
   await page.evaluate(({fixture,width,height})=>{document.getElementById('fixture').value=fixture;document.getElementById('size').value=`${width}×${height}`;document.getElementById('compare').checked=false;render();if(fixture==='wide'){const rows=rowsFor(fixture);rows.forEach(row=>row.forEach(cell=>cell.color='w'));window.prototypeState.reports=[renderScene(document.querySelector('canvas'),'C',rows,width,height,10,20,1)];}},{fixture,width,height});
   // Export the full logical canvas; the surrounding prototype controls are not part of MOTD.
   const capture=await page.evaluate(()=>({png:document.querySelector('canvas').toDataURL(),state:window.prototypeState}));
   fs.writeFileSync(`${out}/reference-${fixture}-${width}x${height}.png`,Buffer.from(capture.png.split(',')[1],'base64'));
   fs.writeFileSync(`${out}/reference-${fixture}-${width}x${height}.json`,JSON.stringify(capture.state,null,2));
  }
 }
 fs.writeFileSync(`${out}/fixtures.json`,JSON.stringify(fixtures));await browser.close();
})();
