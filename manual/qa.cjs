// Run with the bundled Node.js runtime. Uses the installed Edge browser offline.
const assert = require('node:assert/strict');
const path = require('node:path');
const { pathToFileURL } = require('node:url');
const { chromium } = require('C:/Users/Laptop/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const { CONNECTORS, CONTACTS, ROUTES, validateConnection } = require('./app.js');

(async () => {
  const browser = await chromium.launch({headless:true, executablePath:'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe'});
  try {
    const page = await browser.newPage({viewport:{width:1365,height:900}});
    const errors=[], requests=[];
    page.on('pageerror',e=>errors.push(e.message));
    page.on('request',r=>{if(!r.url().startsWith('file:'))requests.push(r.url());});
    await page.goto(pathToFileURL(path.join(__dirname,'index.html')).href);
    assert.equal(await page.title(),'Flick & Fly | Engineering lab manual');
    assert.deepEqual(errors,[]);
    assert.deepEqual(requests,[]);
    assert.equal(await page.locator('img').count()>=8,true);
    assert.equal(await page.locator('img').evaluateAll(xs=>xs.every(x=>x.complete&&x.naturalWidth>0)),true);
    for(const id of Object.keys(CONNECTORS)){
      await page.locator(`#connector-tabs [data-connector="${id}"]`).click();
      assert.equal(await page.locator('#contact-grid button').count(),10);
      for(let n=1;n<=10;n++){
        const text=await page.locator(`#contact-grid [data-contact="${id}-${n}"]`).innerText();
        assert(text.includes(CONTACTS[`${id}-${n}`].signal));
      }
    }
    await page.locator('#connector-tabs [data-connector="CNB"]').click();
    await page.locator('#contact-grid [data-contact="CNB-8"]').click();
    assert((await page.locator('#contact-info').innerText()).includes('JP4 (1,2)'));
    await page.locator('#connector-tabs [data-connector="CND"]').click();
    await page.locator('#contact-grid [data-contact="CND-4"]').click();
    assert((await page.locator('#contact-info').innerText()).includes('JP5'));
    await page.locator('#choose-tft').click();
    assert((await page.locator('#module-photo').getAttribute('src')).includes('tft-front'));
    await page.locator('#flip-module').click();
    assert((await page.locator('#module-photo').getAttribute('src')).includes('tft-back'));
    assert.equal(await page.locator('#module-pin-list button').count(),8);
    await page.locator('#choose-mpu').click();
    assert((await page.locator('#module-photo').getAttribute('src')).includes('mpu-front'));
    await page.locator('#module-pin-list [data-pin="SCL"]').click();
    await page.locator('#connector-tabs [data-connector="CNC"]').click();
    await page.locator('#contact-grid [data-contact="CNC-6"]').click();
    assert.equal(await page.locator('#connect-wire').isEnabled(),true);
    assert.equal(await page.locator('#wire-svg .wire-preview').count(),1);
    assert.equal(await page.evaluate(()=>document.activeElement?.getAttribute('data-contact')),'CNC-6');
    await page.locator('#connect-wire').click();
    assert.equal(await page.locator('#wire-svg .wire-added').count(),1);
    assert((await page.locator('#wire-list').innerText()).includes('Awaiting electrical review'));
    await page.locator('#wire-list [data-remove="mpu-SCL"]').click();
    assert.equal(await page.locator('#wire-svg .wire-added').count(),0);
    await page.locator('#module-pin-list [data-pin="SCL"]').click();
    await page.locator('#contact-grid [data-contact="CNC-5"]').click();
    assert.equal(await page.locator('#connect-wire').isEnabled(),false);
    assert.equal(validateConnection('tft','VCC','CNC-1').ok,false);
    assert.equal(ROUTES.length,7);
    assert.equal(await page.locator('#mapping-rows tr').count(),16);
    await page.locator('#reference-search').fill('SCL');
    assert((await page.locator('#mapping-rows').innerText()).includes('SCL'));
    await page.locator('#reference-search').fill('');
    assert.equal(await page.locator('#connector-rows tr').count(),40);
    await page.setViewportSize({width:390,height:844});
    assert.equal(await page.locator('body').evaluate(x=>x.scrollWidth<=window.innerWidth+2),true);
    await page.locator('#choose-tft').focus();
    await page.keyboard.press('Enter');
    assert.equal(await page.locator('#choose-tft').getAttribute('aria-pressed'),'true');
    assert.equal(await page.locator('#start').count(),1);
    assert.equal(await page.locator('#evidence').count(),1);
    assert.equal(await page.locator('.quiet-link[href="../index.html"]').count(),1);
    assert.deepEqual(errors,[]);
    console.log('PASS: offline assets, 40 PDF-backed contacts, module switching, preview/add/remove, invalid blocking, table, mobile width, keyboard, original sections');
  } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
