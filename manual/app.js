(() => {
  'use strict';
  // One source for the connector explorer, routes, checklist and reference table.
  // Pages are PDF viewer pages (the printed page number is one less).
  const SOURCE = 'ALS-SDA-ARMCTXM3-01-UM1';
  const CONNECTORS = {
    CNA: {page:24, note:'GPIO at power on; select alternate functions in PINSEL. Examples share lines with I2S, LEDs, relay, motors and displays.', contacts:[
      ['P0.4','I2SRX_CLK/RD2/CAP2.0',81],['P0.5','I2SRX_WS/TD2/CAP2.1',80],['P0.6','I2SRX_SDA/SSEL1/MAT2.0',79],['P0.7','I2STX_CLK/SCK1/MAT2.1',78],['P0.8','I2STX_WS/MISO1/MAT2.2',77],['P0.9','I2STX_SDA/MOSI1/MAT2.3',76],['P0.10','TXD2/SDA2/MAT3.0',48],['P0.11','RXD2/SCL2/MAT3.1',49],['No connection','',null],['Ground','',null]]},
    CNB: {page:25, note:'JP4 (1,2) is required for CNB 8. CNB 8 and CNC 9 share P2.13 through JP4. Examples use keypad, PWM, interrupts and display enables.', contacts:[
      ['P1.23','MCI1/PWM1.4/MISO0',37],['P1.24','MCI2/PWM1.5/MOSI0',38],['P1.25','MCOA1/MAT1.1',39],['P1.26','MCOB1/PWM1.6/CAP0.0',40],['P2.10','EINT0/NMI',53],['P2.11','EINT1/I2STX_CLK',52],['P2.12','EINT2/I2STX_WS',51],['P2.13','EINT3/I2STX_SDA',50],['No connection','',null],['Ground','',null]]},
    CNC: {page:25, note:'JP4 (2,3) is required for CNC 9 (p25). A ribbon to CNC1 can route these lines to U5 SPI EEPROM, U16 I²C RTC or U6 RS485; inspect whether it is fitted. JP12/JP13 select SPI versus RS485 at CNC1 (p35).', contacts:[
      ['P0.15','TXD1/SCK0/SCK',62],['P0.16','RXD1/SSEL0/SSEL',63],['P0.17','CTS1/MISO0/MISO',61],['P0.18','DCD1/MOSI0/MOSI',60],['P0.19','DSR1/SDA1',59],['P0.20','DTR1/SCL1',58],['P0.21','RI1/RD1',57],['P0.22','RTS1/TD1',56],['P2.13','I2STX_SDA',50],['Ground','',null]]},
    CND: {page:26, note:'JP5 is required for CND 4. CND 5 and 6 are unavailable when USB OTG uses them. Examples share CND with the LCD.', contacts:[
      ['P0.23','AD0.0/I2SRX_CLK/CAP3.0',9],['P0.24','AD0.1/I2SRX_WS/CAP3.1',8],['P0.25','AD0.2/I2SRX_SDA/TXD3',7],['P0.26','AD0.3/AOUT/RXD3',6],['P0.27','SDA0/USB/SDA',25],['P0.28','SCL0/USB_SCL',24],['P2.0','PWM1.1/TXD1',75],['P2.1','PWM1.2/RXD1',74],['No connection','',null],['Ground','',null]]}
  };
  const MODULES = {
    mpu:{name:'MPU6050',front:'assets/mpu-front.jpg',back:'assets/mpu-back.jpg',pins:[
      ['VCC','power','Supply input; regulator and safe rail unknown.'],['GND','ground','Common return; distribute through a reviewed breakout or adapter.'],
      ['SCL','clock','Primary host I²C clock, distinct from auxiliary XCL.'],['SDA','data','Primary host I²C data, distinct from auxiliary XDA.'],
      ['XDA','data','Auxiliary I²C data; host destination not established.'],['XCL','clock','Auxiliary I²C clock; host destination not established.'],
      ['AD0','control','I²C address selection needs the breakout circuit and address plan.'],['INT','control','Interrupt use and electrical characteristics need review.']]},
    tft:{name:'SPI TFT',front:'assets/tft-front.jpg',back:'assets/tft-back.jpg',pins:[
      ['VCC','power','Supply input; controller and safe rail unknown.'],['GND','ground','Common return; distribute through a reviewed breakout or adapter.'],
      ['CS','select','SPI chip select; shared device conflicts need review.'],['RESET','control','Reset line allocation and controller timing unknown.'],
      ['A0','control','Display data/command selection; GPIO allocation unknown.'],['SDA','data','SPI serial data input, not I²C SDA.'],
      ['SCK','clock','SPI clock input.'],['LED','power','Backlight supply and current limiting unknown.']]}
  };
  const ROUTES = [
    ['mpu','GND','CNC',10,'ground','Common reference','Review supply and ground distribution.'],
    ['mpu','SCL','CNC',6,'clock','Primary I²C clock','Review module logic levels and pull-up rail. If a ribbon joins CNC to CNC1, U16 RTC shares this line (manual p35); inspect actual cable and bus loading.'],
    ['mpu','SDA','CNC',5,'data','Primary I²C data','Review module logic levels and pull-up rail. If a ribbon joins CNC to CNC1, U16 RTC shares this line (manual p35); inspect actual cable and bus loading.'],
    ['tft','GND','CNC',10,'ground','Common reference','Review supply and ground distribution.'],
    ['tft','CS','CNC',2,'select','SPI chip select','Review controller input level and select allocation. If CNC is linked to CNC1, U5 EEPROM shares SPI; JP13 selects SPI versus RS485 there (manual p35).'],
    ['tft','SDA','CNC',4,'data','SPI MOSI to display data','Review controller input level. If CNC is linked to CNC1, U5 EEPROM shares MOSI (manual p35).'],
    ['tft','SCK','CNC',1,'clock','SPI clock','Review controller input level. If CNC is linked to CNC1, U5 EEPROM shares clock; JP12 selects SPI versus RS485 there (manual p35).']
  ].map(([module,pin,connector,contact,category,purpose,review],order)=>({
    id:`${module}-${pin}`,module,pin,connector,contact,category,purpose,review,order,
    method:'Compatible 10-pin FRC/IDC ribbon to a verified, labelled breakout; individual jumper from breakout to module header or breadboard',
    instruction:'With power off, identify connector pin 1 on the actual hardware and keyed cable. Verify the breakout contact by documentation or continuity. Connect one labelled module header to that breakout contact only after electrical review.',
    status:'awaiting',test:'Not physically tested'
  }));
  const STATUS = {
    documented:'Documented mapping',awaiting:'Awaiting electrical review',blocked:'Blocked',
    reviewed:'Electrically reviewed, physical wiring not approved',
    approved:'Approved for physical wiring',installed:'User-marked installed, not independently verified',
    tested:'Physically tested, with evidence recorded'
  };
  const CONTACTS = Object.fromEntries(Object.entries(CONNECTORS).flatMap(([name,c])=>c.contacts.map(([signal,alternate,packagePin],i)=>[
    `${name}-${i+1}`,{id:`${name}-${i+1}`,connector:name,number:i+1,signal,alternate,packagePin,page:c.page,
    warning:i===7&&name==='CNB'?'Requires JP4 (1,2).':i===8&&name==='CNC'?'Requires JP4 (2,3).':i===3&&name==='CND'?'Requires JP5.':name==='CND'&&(i===4||i===5)?'Unavailable while USB OTG uses these pins.':c.note}
  ])));
  const LESSONS = {
    physical:{label:'1 · Physical wire',detail:'#physical-wire',steps:[
      ['Identify both ends','Read the module label and the trainer connector table. The contact diagram is logical; find real pin 1 on the hardware before counting.','Gate: pin-1 orientation must be independently established.'],
      ['Choose a connection method','Use a compatible keyed 10-pin FRC/IDC ribbon and a labelled breakout or verified adapter. An ordinary jumper is not a known fit for a boxed connector.','Gate: inspect cable keying and verify breakout contact numbering.'],
      ['Review before power','Check module supply, logic levels, pull-ups, backlight current and shared onboard functions. Check continuity while power is off.','No proposed signal route is electrically approved yet.']]},
    identify:{label:'2 · Identify',detail:'#identify',steps:[
      ['Read the markings','Compare the board and piggyback photographs with the lab manual and record revisions.','Processor marking remains unreadable.'],
      ['Inspect connector tables','CNA, CNB, CNC and CND each have ten contacts in PDF pages 24–26. Page 39 identifies their board areas.','Placement does not settle the photographed contact-1 end.']]},
    mpu:{label:'3 · MPU',detail:'#mpu',steps:[
      ['Review the primary bus','MPU SCL and SDA are primary host I²C; XCL and XDA are auxiliary. CNC 6 and 5 offer documented I²C1 signals.','Supply, pull-ups, AD0 and shared RTC usage remain open.'],
      ['Test standalone','After reviewed wiring and flashing, read identity and timestamped raw axes.','Record actual logs, firmware and failures.']]},
    tft:{label:'4 · TFT',detail:'#tft',steps:[
      ['Review display labels','TFT SDA is SPI data. CNC 1, 2 and 4 document clock, select and MOSI candidates.','RESET, A0, VCC and LED remain unallocated.'],
      ['Test standalone','After reviewed wiring, show colors, text and corner markers.','Record actual photographs and firmware.']]},
    flash:{label:'5 · Flash',detail:'#program',steps:[['Verify programming path','Confirm the actual board, cable, clock and jumper settings.','Do not borrow another board’s settings.'],['Flash board only','Build, verify, flash and observe one board-only output.','No physical pass recorded.']]},
    integrate:{label:'6 · Integrate',detail:'#integrate',steps:[['Preserve standalone passes','Keep separate MPU and TFT evidence tied to firmware IDs.','Both passes are still pending.'],['Compare detectors','Use timestamped sessions, a threshold baseline and a held-out shallow tree comparison.','No dataset or measured result exists here.']]}
  };
  function routeFor(module,pin){return ROUTES.find(r=>r.module===module&&r.pin===pin);}
  function validateConnection(module,pin,endpoint,selected=[]){
    const r=routeFor(module,pin),c=CONTACTS[endpoint];
    if(!pin||!endpoint)return {ok:false,message:'Select a module pin and a connector contact.'};
    if(!c||c.signal==='No connection')return {ok:false,message:'That contact has no documented signal.'};
    if(!r||r.connector!==c.connector||r.contact!==c.number)return {ok:false,message:'No documented project route joins these endpoints. Do not improvise a wire.'};
    if(selected.includes(r.id))return {ok:false,message:'This connection is already in the guided list.'};
    return {ok:true,route:r,message:'Documented signal match. The line shown is a planning preview, not electrical approval or a physical wire.'};
  }
  if(typeof module!=='undefined'&&module.exports)module.exports={SOURCE,CONNECTORS,CONTACTS,MODULES,ROUTES,STATUS,validateConnection};
  if(typeof document==='undefined')return;
  const $=id=>document.getElementById(id),safe=v=>String(v??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const state={module:'mpu',connector:'CNC',pin:null,contact:null,selected:[],lesson:'physical',step:0,motion:0,fire:false,area:'CNC',photoSide:'front'};
  const moduleData=()=>MODULES[state.module];
  const currentRoute=()=>routeFor(state.module,state.pin);
  function renderBoard(){
    document.querySelectorAll('.board-hotspot').forEach(b=>b.setAttribute('aria-pressed',String(b.dataset.area===state.area)));
    $('board-info').innerHTML=`<strong>${safe(state.area)}</strong><p>${safe(CONNECTORS[state.area]?.note||'Area visible in the photograph; exact connector contacts are shown only in the logical diagram.')}</p><small>${SOURCE}, PDF p${CONNECTORS[state.area]?.page||39}. Placement diagram: p39. Photograph does not establish contact 1 reliably.</small>`;
    $('connector-tabs').innerHTML=Object.keys(CONNECTORS).map(id=>`<button type="button" data-connector="${id}" aria-pressed="${state.connector===id}">${id}</button>`).join('');
    const data=CONNECTORS[state.connector];
    $('connector-title').textContent=`${state.connector} · 10-contact FRC/IDC logical diagram`;
    $('connector-note').textContent=`${data.note} Source: ${SOURCE}, PDF p${data.page}. Positions below are numbered logically, not asserted to match the photo or cable view. Confirm physical pin 1 and keying.`;
    $('contact-grid').innerHTML=data.contacts.map((_,i)=>{const c=CONTACTS[`${state.connector}-${i+1}`];return `<button type="button" data-contact="${c.id}" class="contact ${c.signal==='No connection'?'is-nc':''}" aria-pressed="${state.contact===c.id}"><b>${c.number}</b><span>${safe(c.signal)}</span></button>`}).join('');
    const c=CONTACTS[state.contact];
    $('contact-info').innerHTML=c?`<strong>${c.id} · ${safe(c.signal)}</strong><p>${safe(c.alternate||'No alternate function listed.')}${c.packagePin?` · LPC1768 package pin ${c.packagePin} (not a connector contact)`:''}</p><small>${SOURCE}, PDF p${c.page}. ${safe(c.warning)}</small>`:'<strong>Choose a numbered contact</strong><p>The table and diagram share the documented contact data. Never count contacts from the board photo alone.</p>';
  }
  function renderModule(){
    $('choose-mpu').setAttribute('aria-pressed',String(state.module==='mpu'));$('choose-tft').setAttribute('aria-pressed',String(state.module==='tft'));
    $('module-photo').src=moduleData()[state.photoSide];$('module-photo').alt=`${state.photoSide==='front'?'Front':'Reverse'} photograph of ${moduleData().name}`;
    $('flip-module').textContent=state.photoSide==='front'?'Show reverse side':'Show front side';
    $('module-reverse').href=moduleData()[state.photoSide==='front'?'back':'front'];$('module-reverse').textContent=state.photoSide==='front'?'Open reverse photograph ↗':'Open front photograph ↗';
    $('module-pin-list').innerHTML=moduleData().pins.map(([id,category])=>`<button type="button" data-pin="${id}" class="pin-category-${category}" aria-pressed="${state.pin===id}"><b>${id}</b><span>${category}</span></button>`).join('');
    const p=moduleData().pins.find(x=>x[0]===state.pin),r=currentRoute(),c=r&&CONTACTS[`${r.connector}-${r.contact}`];
    $('pin-info').innerHTML=p?`<strong>${moduleData().name} ${p[0]}</strong><p>${safe(p[2])}</p><small>${c?`Candidate: ${c.id} → ${safe(c.signal)}. ${safe(r.review)} ${SOURCE}, PDF p${c.page}. ${safe(r.method)}`:'No trainer destination documented for this project pin. Obtain component circuit and reviewed allocation; do not improvise.'}</small>`:'<strong>Choose a module pin</strong><p>Inspect its function, candidate destination, method, prerequisite and evidence.</p>';
  }
  function selectRoute(r){
    state.module=r.module;state.connector=r.connector;state.area=r.connector;state.pin=r.pin;state.contact=`${r.connector}-${r.contact}`;renderAll();
    $('wire-feedback').textContent=`${MODULES[r.module].name} ${r.pin} → ${state.contact}: documented signal route only. ${r.review}`;
    document.querySelector(`#module-pin-list [data-pin="${r.pin}"]`)?.focus({preventScroll:true});
  }
  function renderWire(){
    const result=validateConnection(state.module,state.pin,state.contact,state.selected);
    $('wire-feedback').textContent=result.message;
    $('preview-detail').innerHTML=result.ok?`<strong>${moduleData().name} ${state.pin} → ${state.contact}</strong><p>${safe(result.route.purpose)}. ${safe(result.route.instruction)}</p><small>Electrical prerequisite: ${safe(result.route.review)} Source: ${SOURCE}, PDF p${CONTACTS[state.contact].page}. Status: ${STATUS.awaiting}.</small>`:'<strong>Connection preview</strong><p>Select a module header pin, then its documented trainer contact. A mismatched or unresolved pair cannot be added.</p>';
    $('connect-wire').disabled=!result.ok;
    $('connect-wire').textContent='Add documented plan';
    const rows=ROUTES.filter(r=>r.module===state.module);
    $('wire-list').innerHTML=rows.map((r,i)=>{const active=state.selected.includes(r.id),c=CONTACTS[`${r.connector}-${r.contact}`];return `<div class="wire-item ${active?'is-selected':''}"><span><b>${i+1}. ${moduleData().name} ${r.pin} → ${r.connector} ${r.contact} · ${safe(c.signal)}</b><small>${safe(r.purpose)} · ${active?'Added to local plan':'Available for review'} · ${STATUS.awaiting}</small></span><div><button type="button" data-view-route="${r.id}">Inspect</button>${active?`<button type="button" data-remove="${r.id}">Remove</button>`:''}</div><details class="route-detail"><summary>Method, safety and evidence</summary><p><b>Method:</b> ${safe(r.method)}.</p><p><b>Physical step:</b> ${safe(r.instruction)}</p><p><b>Electrical gate:</b> ${safe(r.review)} ${safe(c.warning)}</p><p><b>Source:</b> ${SOURCE}, PDF p${c.page}. <b>Physical test:</b> ${r.test}. Browser list status: ${active?'added to local plan':'not added'}.</p></details></div>`}).join('');
    $('wire-count').textContent=`${state.selected.filter(id=>ROUTES.find(r=>r.id===id)?.module===state.module).length} local plan connection(s)`;
    $('wire-checklist').textContent='Adding a route records only a browser planning choice. No route is electrically approved, installed or physically tested.';
    requestAnimationFrame(drawWires);
  }
  function drawWires(){
    const stage=$('wire-stage'),svg=$('wire-svg'),box=stage.getBoundingClientRect();if(!box.width||!box.height)return;
    svg.setAttribute('viewBox',`0 0 ${box.width} ${box.height}`);svg.replaceChildren();
    const visible=ROUTES.filter(r=>r.module===state.module&&r.connector===state.connector&&(state.selected.includes(r.id)||(state.pin===r.pin&&state.contact===`${r.connector}-${r.contact}`)));
    for(const r of visible){
      const a=stage.querySelector(`[data-stage-contact="${r.connector}-${r.contact}"]`),b=stage.querySelector(`[data-stage-pin="${r.pin}"]`);if(!a||!b)continue;
      const ar=a.getBoundingClientRect(),br=b.getBoundingClientRect(),x1=ar.right-box.left,y1=ar.top+ar.height/2-box.top,x2=br.left-box.left,y2=br.top+br.height/2-box.top;
      const path=document.createElementNS('http://www.w3.org/2000/svg','path');
      path.setAttribute('d',`M${x1},${y1} C${(x1+x2)/2},${y1} ${(x1+x2)/2},${y2} ${x2},${y2}`);
      path.setAttribute('class',`diagram-wire wire-${r.category} ${state.selected.includes(r.id)?'wire-added':'wire-preview'}`);
      svg.append(path);
    }
  }
  function renderStage(){
    $('host-column-title').textContent=`${state.connector} logical contacts`;
    $('host-endpoints').innerHTML=CONNECTORS[state.connector].contacts.map((_,i)=>{const c=CONTACTS[`${state.connector}-${i+1}`];return `<button type="button" class="endpoint" data-stage-contact="${c.id}" aria-pressed="${state.contact===c.id}"><b>${c.id}</b><small>${safe(c.signal)}</small></button>`}).join('');
    $('module-endpoints').innerHTML=moduleData().pins.map(([id,category])=>`<button type="button" class="endpoint pin-category-${category}" data-stage-pin="${id}" aria-pressed="${state.pin===id}"><b>${id}</b><small>${category}</small></button>`).join('');
    renderWire();
  }
  function renderTables(){
    const q=$('reference-search').value.trim().toLowerCase(),mf=$('filter-module').value,cf=$('filter-connector').value,sf=$('filter-status').value;
    $('mapping-status').textContent=`${SOURCE}, PDF pp24–26: connector contacts have Documented mapping evidence. Candidate module routes are Awaiting electrical review; unallocated pins are Blocked. Electrically reviewed: 0. Approved for physical wiring: 0. User-marked installed, not independently verified: 0. Physically tested, with evidence recorded: 0. Local plan choices do not change these statuses.`;
    const routeRows=ROUTES.map(r=>({r,c:CONTACTS[`${r.connector}-${r.contact}`]}));
    const unresolved=Object.entries(MODULES).flatMap(([module,m])=>m.pins.filter(p=>!routeFor(module,p[0])).map(([pin,category,description])=>({r:{module,pin,connector:'—',contact:'—',method:'Undetermined',status:'blocked',test:'Not physically tested',review:description,id:`${module}-${pin}`},c:null})));
    const rows=[...routeRows,...unresolved].filter(({r,c})=>(!mf||r.module===mf)&&(!cf||r.connector===cf)&&(!sf||r.status===sf)&&(!q||`${MODULES[r.module].name} ${r.pin} ${r.connector} ${r.contact} ${c?.signal||''} ${r.review}`.toLowerCase().includes(q)));
    $('mapping-rows').innerHTML=rows.map(({r,c})=>`<tr><td>${safe(MODULES[r.module].name)}</td><th scope="row"><button type="button" data-table-route="${r.id}">${safe(r.pin)}</button></th><td>${safe(r.connector)}</td><td>${safe(r.contact)}</td><td>${safe(c?.signal||'No allocation')}</td><td>${safe(r.method)}</td><td>${c?`${SOURCE}, PDF p${c.page}`:'Module documentation needed'}</td><td>${STATUS[r.status]}</td><td>${safe(r.test)}</td></tr>`).join('')||'<tr><td colspan="9">No matching connections.</td></tr>';
    $('connector-rows').innerHTML=Object.values(CONTACTS).filter(c=>!cf||c.connector===cf).filter(c=>!q||`${c.id} ${c.signal} ${c.alternate}`.toLowerCase().includes(q)).map(c=>`<tr><th scope="row"><button type="button" data-table-contact="${c.id}">${c.id}</button></th><td>${safe(c.signal)}</td><td>${safe(c.alternate||'—')}</td><td>${c.packagePin||'—'}</td><td>${SOURCE}, PDF p${c.page}</td><td>${safe(c.warning)}</td></tr>`).join('');
  }
  function renderLesson(){
    $('lesson-tabs').innerHTML=Object.entries(LESSONS).map(([id,l])=>`<button type="button" data-lesson="${id}" aria-pressed="${id===state.lesson}">${safe(l.label)}</button>`).join('');
    const l=LESSONS[state.lesson],s=l.steps[state.step];$('lesson-progress-label').textContent=`Learning step ${state.step+1} of ${l.steps.length} · no physical pass implied`;
    $('lesson-progress').max=l.steps.length;$('lesson-progress').value=state.step+1;$('lesson-step-title').textContent=s[0];$('lesson-step-body').textContent=s[1];$('lesson-step-gate').textContent=s[2];
    $('lesson-back').disabled=state.step===0;$('lesson-next').disabled=state.step===l.steps.length-1;$('lesson-detail').href=l.detail;
  }
  function renderSimulation(){
    const x=state.motion,command=state.fire?'FIRE':x < -45?'LEFT':x > 45?'RIGHT':'NONE';$('sim-motion').value=x;$('sim-motion-value').textContent=`${x} arbitrary units`;
    $('sim-uart').textContent=`SIMULATED — NOT A PHYSICAL HARDWARE TEST\nillustrative_x=${x} arbitrary_units\nillustrative_command=${command}\nsource=synthetic browser input`;
    $('sim-outcome').textContent=`SIMULATED output: ${command==='NONE'?'no command':command.toLowerCase()+' example'}. No real sensor or display result.`;
    $('mock-ship').style.left=`${50+x*.32}%`;$('mock-shot').hidden=!state.fire;
  }
  function renderAll(){renderBoard();renderModule();renderStage();renderTables();}
  document.querySelectorAll('.board-hotspot').forEach(b=>b.addEventListener('click',()=>{state.area=b.dataset.area;if(CONNECTORS[state.area]){state.connector=state.area;state.contact=null;}renderAll();}));
  $('connector-tabs').addEventListener('click',e=>{const b=e.target.closest('[data-connector]');if(b){state.connector=b.dataset.connector;state.area=state.connector;state.contact=null;renderAll();document.querySelector(`#connector-tabs [data-connector="${state.connector}"]`)?.focus({preventScroll:true});}});
  function chooseContact(id,where='contact-grid'){state.contact=id;state.connector=CONTACTS[id].connector;state.area=state.connector;renderAll();document.querySelector(`#${where} [data-contact="${id}"], #${where} [data-stage-contact="${id}"]`)?.focus({preventScroll:true});}
  $('contact-grid').addEventListener('click',e=>{const b=e.target.closest('[data-contact]');if(b)chooseContact(b.dataset.contact);});
  $('host-endpoints').addEventListener('click',e=>{const b=e.target.closest('[data-stage-contact]');if(b)chooseContact(b.dataset.stageContact,'host-endpoints');});
  function choosePin(id,where='module-pin-list'){state.pin=id;renderAll();document.querySelector(`#${where} [data-pin="${id}"], #${where} [data-stage-pin="${id}"]`)?.focus({preventScroll:true});}
  $('module-pin-list').addEventListener('click',e=>{const b=e.target.closest('[data-pin]');if(b)choosePin(b.dataset.pin);});
  $('module-endpoints').addEventListener('click',e=>{const b=e.target.closest('[data-stage-pin]');if(b)choosePin(b.dataset.stagePin,'module-endpoints');});
  for(const [id,module] of [['choose-mpu','mpu'],['choose-tft','tft']])$(id).addEventListener('click',()=>{state.module=module;state.photoSide='front';state.pin=null;state.contact=null;renderAll();});
  $('flip-module').addEventListener('click',()=>{state.photoSide=state.photoSide==='front'?'back':'front';renderModule();$('flip-module').focus({preventScroll:true});});
  $('connect-wire').addEventListener('click',()=>{const v=validateConnection(state.module,state.pin,state.contact,state.selected);if(v.ok){state.selected.push(v.route.id);renderAll();$('wire-feedback').textContent='Added to local plan. Awaiting electrical review; no physical installation or test recorded.';}});
  $('cancel-wire').addEventListener('click',()=>{state.pin=null;state.contact=null;renderAll();});
  $('wire-list').addEventListener('click',e=>{const b=e.target.closest('[data-remove],[data-view-route]');if(!b)return;const id=b.dataset.remove||b.dataset.viewRoute,r=ROUTES.find(x=>x.id===id);if(b.dataset.remove){state.selected=state.selected.filter(x=>x!==id);renderAll();}else if(r)selectRoute(r);});
  $('mapping-rows').addEventListener('click',e=>{const b=e.target.closest('[data-table-route]');if(!b)return;const r=ROUTES.find(x=>x.id===b.dataset.tableRoute);if(r)selectRoute(r);else{const [module,...pin]=b.dataset.tableRoute.split('-');state.module=module;state.pin=pin.join('-');state.contact=null;renderAll();$('wire-feedback').textContent='This module pin has no documented trainer destination. Do not improvise a connection.';}});
  $('connector-rows').addEventListener('click',e=>{const b=e.target.closest('[data-table-contact]');if(b)chooseContact(b.dataset.tableContact);});
  for(const id of ['reference-search','filter-module','filter-connector','filter-status'])$(id).addEventListener('input',renderTables);
  $('lesson-tabs').addEventListener('click',e=>{const b=e.target.closest('[data-lesson]');if(b){state.lesson=b.dataset.lesson;state.step=0;renderLesson();}});
  $('lesson-back').addEventListener('click',()=>{state.step=Math.max(0,state.step-1);renderLesson();});
  $('lesson-next').addEventListener('click',()=>{state.step=Math.min(LESSONS[state.lesson].steps.length-1,state.step+1);renderLesson();});
  $('lesson-restart').addEventListener('click',()=>{state.step=0;renderLesson();});
  $('sim-motion').addEventListener('input',e=>{state.motion=Number(e.target.value);state.fire=false;renderSimulation();});
  for(const [id,value] of [['sim-neutral',0],['sim-left',-75],['sim-right',75]])$(id).addEventListener('click',()=>{state.motion=value;state.fire=false;renderSimulation();});
  $('sim-fire').addEventListener('click',()=>{state.fire=!state.fire;renderSimulation();});
  window.addEventListener('resize',()=>requestAnimationFrame(drawWires));
  if(window.ResizeObserver)new ResizeObserver(()=>requestAnimationFrame(drawWires)).observe($('wire-stage'));
  $('print-record')?.addEventListener('click',()=>window.print());
  const links=[...document.querySelectorAll('.phase-nav a[href^="#"]')],sections=links.map(a=>document.getElementById(a.hash.slice(1))).filter(Boolean);
  function showCurrentSection(){const cutoff=Math.min(200,window.innerHeight*.3);let current=sections[0];for(const section of sections)if(section.getBoundingClientRect().top<=cutoff)current=section;for(const link of links){if(current&&link.hash===`#${current.id}`)link.setAttribute('aria-current','location');else link.removeAttribute('aria-current');}}
  let scheduled=false;function scheduleCurrentSection(){if(scheduled)return;scheduled=true;requestAnimationFrame(()=>{scheduled=false;showCurrentSection();});}
  window.addEventListener('scroll',scheduleCurrentSection,{passive:true});window.addEventListener('resize',scheduleCurrentSection);window.addEventListener('hashchange',scheduleCurrentSection);
  renderAll();renderLesson();renderSimulation();showCurrentSection();
})();
