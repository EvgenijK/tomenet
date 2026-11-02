const {chromium}=require('/home/svechnik/.nvm/versions/node/v26.2.0/lib/node_modules/@playwright/cli/node_modules/playwright');
const fs=require('fs');
const path=require('path');
const os=require('os');
const {pathToFileURL}=require('url');
const resources=path.resolve(__dirname,'../../../lib/xtra/sv/login');
// Use the external prototype checkout; TOMENET_INTERFACE_ROOT can select a pinned revision.
const reference=path.resolve(process.env.TOMENET_INTERFACE_ROOT || path.resolve(__dirname,'../../../../github_site/tomenet_interface'));
for(const name of ['login.html','css','js']) {
 if(!fs.existsSync(path.join(reference,name))) throw new Error(`Missing prototype ${name} in ${reference}; set TOMENET_INTERFACE_ROOT to the tomenet_interface checkout.`);
}
const snapshot=fs.mkdtempSync(path.join(os.tmpdir(),'sv-b2-reference-'));
for(const name of ['login.html','css','js']) fs.symlinkSync(path.join(reference,name),path.join(snapshot,name));
fs.mkdirSync(path.join(snapshot,'assets'));
fs.symlinkSync(resources,path.join(snapshot,'assets/log_in_backgrounds'));
const output=process.argv[2] || path.join(snapshot,'captures');
fs.mkdirSync(output,{recursive:true});
(async()=>{const b=await chromium.launch({headless:true,executablePath:'/home/svechnik/.cache/ms-playwright/chromium-1237/chrome-linux64/chrome',args:['--no-sandbox']});
for(const [width,height] of [[1440,900],[1024,768]]) for(const state of ["overview","name","edit","error"]){
 const page=await b.newPage({viewport:{width,height},deviceScaleFactor:1,reducedMotion:'reduce'});
 await page.addInitScript(()=>Math.random=()=>.999999);
 await page.goto(pathToFileURL(path.join(snapshot,'login.html')).href+'?variant=D');
 await page.waitForTimeout(1500);
 await page.locator(".server-enter").first().click();
 await page.locator("#accountInput").fill("TestAccount");
 await page.locator("#passwordInput").fill("sample");
 await page.locator("#passwordInput").press("Enter"); await page.waitForTimeout(800);
 if(state!=="overview") await page.locator("#createCharacterButton").click();
 if(state==="edit") {await page.locator("#characterNameInput").fill("Newhero"); await page.locator("#characterNameInput").evaluate(e=>e.setSelectionRange(2,5));}
 if(state==="error") {await page.locator("#characterNameInput").fill("Invalid!"); await page.locator("#primaryButton").click();}
 const data=await page.evaluate(()=>{
 const selectors=['body','.login-shell','.login-hero','.hero-content','#accountLogo','.login-panel','.world-chronicle','.realm-links','.realm-link','.realm-link-label','.account-torches','.server-option','.server-enter','.step-title','.server-name','.server-meta','.server-title','.chronicle-copy','.chronicle-empty','.server-controls','.account-welcome','.panel-header h2','#directBackdrop','.manual-card','#accountStep','.account-form','#accountInput','#passwordInput','#characterStep','#characterList','.character-option','.character-level','.character-identity','.character-meta','.character-slot-placeholder','#characterCreateStep','.creation-name-layout','#characterNameInput','.panel-footer','#characterNameError'];
 const props=['display','fontFamily','fontSize','fontWeight','lineHeight','color','background','border','padding','gap','gridTemplateColumns','gridTemplateRows','transform','filter','opacity','width','height'];
 return {url:location.href,viewport:{width:innerWidth,height:innerHeight,devicePixelRatio},reducedMotion:matchMedia('(prefers-reduced-motion: reduce)').matches,bodyVariant:document.body.dataset.variant,styles:selectors.map(s=>{const e=document.querySelector(s);if(!e)return {selector:s,missing:true};const c=getComputedStyle(e),r=e.getBoundingClientRect();return {selector:s,rect:{x:r.x,y:r.y,width:r.width,height:r.height},styles:Object.fromEntries(props.map(p=>[p,c[p]]))};}),backgrounds:[...document.querySelectorAll('[data-background-layer]')].map(e=>({class:e.className,background:getComputedStyle(e).backgroundImage})),logo:document.querySelector('#accountLogo').outerHTML};});
 const cdp=await page.context().newCDPSession(page); await cdp.send('DOM.enable'); await cdp.send('CSS.enable'); const doc=await cdp.send('DOM.getDocument'); const node=await cdp.send('DOM.querySelector',{nodeId:doc.root.nodeId,selector:'.realm-link-label'}); data.renderedFonts=await cdp.send('CSS.getPlatformFontsForNode',{nodeId:node.nodeId});
 fs.writeFileSync(`${output}/reference-${state}-${width}x${height}.json`,JSON.stringify(data,null,2));
 await page.screenshot({path:`${output}/reference-${state}-${width}x${height}.png`});
 console.log(JSON.stringify({size:[width,height],variant:data.bodyVariant,background:data.backgrounds,body:data.styles[0],logo:data.styles[4],panel:data.styles[5]}));
 await page.close();
}await b.close(); console.log('Captures: '+output);})();
