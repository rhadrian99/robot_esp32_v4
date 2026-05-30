#include "WebControl.h"
#include <common.h>
#include "Brush.h"
#include "StepperX.h"
#include "ServoX.h"
#include "LEDdisplay.h"

extern void infrared_menu(uint32_t _var, char _mode);
extern char mode;
extern uint8_t servo_step;
extern Brush motor_up;
extern Brush motor_down;
extern StepperX feeder;
extern ServoX pan;
extern ServoX tilt;
extern LEDdisplay display;

WebControl *WebControl::_instance = nullptr;

// ── HTML page served from flash ─────────────────────────────────────────────
static const char HTML_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="ro">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Robot Control</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:sans-serif;background:#1a1a2e;color:#eee;
         display:flex;flex-direction:column;align-items:center;
         padding:16px;gap:14px;min-height:100vh}
    h2{letter-spacing:2px;font-size:18px}
    .card{background:#16213e;border-radius:14px;padding:14px;
          width:100%;max-width:320px}
    .row{display:flex;align-items:center;gap:8px}
    #spin-label{flex:1;text-align:center;padding:8px;
                background:#0f3460;border-radius:8px;
                font-weight:bold;color:#e94560;font-size:14px}
    .btn{flex:1;border:none;border-radius:10px;cursor:pointer;
         font-size:22px;color:#eee;padding:10px;}
    .btn:active{transform:scale(.92)}
    .btn-mute{background:#0f3460}
    .btn-power{background:#7b1f1f}
    .grid{display:grid;grid-template-columns:repeat(3,72px);
          grid-template-rows:repeat(3,72px);gap:8px;justify-content:center}
    .dpad{width:72px;height:72px;font-size:28px;border:none;
          border-radius:12px;background:#0f3460;color:#e94560;
          cursor:pointer;box-shadow:0 4px 8px #0005}
    .dpad:active{transform:scale(.92);background:#e94560;color:#fff}
    .empty{background:transparent;box-shadow:none;pointer-events:none}
    .slider-wrap{display:flex;flex-direction:column;gap:22px}
    .slider-item label{font-size:12px;color:#aaa;
                       display:flex;justify-content:space-between;margin-bottom:6px}
    .slider-item input[type=range]{width:100%;accent-color:#e94560;height:24px;cursor:pointer}
    .slider-item input[type=range]::-webkit-slider-runnable-track{height:12px;border-radius:6px;background:#0f3460}
    .slider-item input[type=range]::-moz-range-track{height:12px;border-radius:6px;background:#0f3460}
    .slider-item input[type=range]::-webkit-slider-thumb{margin-top:-4px}
    #status{font-size:12px;color:#555}
  </style>
</head>
<body>
  <h2>&#127926; Robot Control</h2>

  <div class="card">
    <div class="row">
      <button class="btn btn-mute" id="btn-mute" onclick="cmd('mute')">...</button>
      <button class="btn btn-power" onclick="cmd('power')">&#9211;</button>
    </div>
  </div>

  <div class="card">
    <div style="display:flex;justify-content:space-between;gap:8px;margin-bottom:6px">
      <button onclick="confirmSavePos()" style="border:none;border-radius:8px;background:#0f3460;color:#e94560;font-size:22px;padding:12px 20px;cursor:pointer;font-weight:bold">&#128190;</button>
      <button id="btn-step" onclick="toggleStep()" style="border:none;border-radius:8px;background:#0f3460;color:#e94560;font-size:22px;padding:12px 20px;cursor:pointer;font-weight:bold">6&#176;</button>
    </div>
    <div class="grid">
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('up')">&#9650;</button>
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('left')">&#9664;</button>
      <button class="dpad" onclick="cmd('home')" style="font-size:24px">&#127968;</button>
      <button class="dpad" onclick="cmd('right')">&#9654;</button>
      <div style="display:flex;flex-direction:column;align-items:center;justify-content:center;gap:2px;pointer-events:none;width:72px;height:72px">
        <span style="font-size:10px;color:#aaa">PAN</span>
        <span id="pan-val" style="font-size:18px;color:#e94560;font-weight:bold">-</span>
        <span style="font-size:9px;color:#555">[<span id="pan-min">-</span>..<span id="pan-max">-</span>]</span>
      </div>
      <button class="dpad" onclick="cmd('down')">&#9660;</button>
      <div style="display:flex;flex-direction:column;align-items:center;justify-content:center;gap:2px;pointer-events:none;width:72px;height:72px">
        <span style="font-size:10px;color:#aaa">TILT</span>
        <span id="tilt-val" style="font-size:18px;color:#e94560;font-weight:bold">-</span>
        <span style="font-size:9px;color:#555">[<span id="tilt-min">-</span>..<span id="tilt-max">-</span>]</span>
      </div>
    </div>
  </div>

  <div class="card slider-wrap">
    <div class="slider-item">
      <label><span id="m1label">MAIN</span> <span id="m1val">0</span>/8</label>
      <input type="range" min="0" max="8" value="0" id="m1"
             oninput="document.getElementById('m1val').textContent=this.value"
             onchange="setMotor(1,this.value)">
    </div>
    <div class="slider-item">
      <label>SUPPORT <span id="m2val">0</span>/8</label>
      <input type="range" min="0" max="8" value="0" id="m2"
             oninput="document.getElementById('m2val').textContent=this.value"
             onchange="setMotor(2,this.value)">
    </div>
  </div>

  <div class="card slider-wrap">
    <div class="slider-item">
      <label>Frecventa bile <span id="fval">0</span>/8</label>
      <input type="range" min="0" max="8" value="0" id="f"
             oninput="document.getElementById('fval').textContent=this.value"
             onchange="setFeeder(this.value)">
    </div>
  </div>

  <div id="status">ready</div>

  <div style="background:#16213e;border-radius:14px;padding:8px 14px;width:100%;max-width:320px;text-align:center;font-size:12px;color:#888">
    Firmware v<span id="fw-version">-</span>
  </div>

  <div style="display:flex;flex-direction:column;gap:8px;width:100%;max-width:320px">
    <div style="display:flex;gap:8px">
      <button onclick="stopPolling();window.location='/settings'" style="flex:1;width:100%;border:none;border-radius:10px;background:#0f3460;color:#aaa;
                       font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
        &#9881; Servo Settings
      </button>
      <button onclick="stopPolling();window.location='/motorsettings'" style="flex:1;width:100%;border:none;border-radius:10px;background:#0f3460;color:#aaa;
                       font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
        &#9881; Motor Settings
      </button>
    </div>
    <button onclick="stopPolling();window.location='/firmware'" style="width:100%;border:none;border-radius:10px;background:#1a3a1a;color:#66dd66;
                     font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
      &#11014; Firmware Update
    </button>
  </div>

  <div id="confirm-modal" style="display:none;position:fixed;inset:0;background:#0008;z-index:999;align-items:center;justify-content:center">
    <div style="background:#16213e;border-radius:14px;padding:24px 20px;max-width:280px;width:90%;text-align:center">
      <p style="margin-bottom:20px;font-size:14px;line-height:1.5">Salvezi pozitia curenta PAN/TILT ca pozitie HOME?</p>
      <div style="display:flex;gap:10px;justify-content:center">
        <button onclick="modalOk()" style="border:none;border-radius:8px;background:#e94560;color:#fff;font-size:14px;padding:10px 24px;cursor:pointer">Da</button>
        <button onclick="modalCancel()" style="border:none;border-radius:8px;background:#0f3460;color:#eee;font-size:14px;padding:10px 24px;cursor:pointer">Nu</button>
      </div>
    </div>
  </div>

  <script>
    function toggleStep(){
      fetch('/step')
        .then(r=>r.json())
        .then(d=>document.getElementById('btn-step').textContent=d.step+'\u00b0')
        .catch(()=>{});
    }
    function cmd(dir){
      document.getElementById('status').textContent=dir+'...';
      fetch('/'+dir)
        .then(r=>r.text())
        .then(t=>{document.getElementById('status').textContent=t;pollStatus();})
        .catch(()=>document.getElementById('status').textContent='error');
    }
    function setMotor(n,v){
      fetch('/motor'+n+'?v='+v)
        .then(r=>r.text())
        .then(t=>document.getElementById('status').textContent=t)
        .catch(()=>document.getElementById('status').textContent='error');
    }
    function confirmSavePos(){
      var m=document.getElementById('confirm-modal');
      m.style.display='flex';
    }
    function modalOk(){
      document.getElementById('confirm-modal').style.display='none';
      savePos();
    }
    function modalCancel(){
      document.getElementById('confirm-modal').style.display='none';
    }
    function savePos(){
      fetch('/savepos')
        .then(r=>r.text())
        .then(t=>document.getElementById('status').textContent=t)
        .catch(()=>document.getElementById('status').textContent='error');
    }
    function setFeeder(v){
      fetch('/feeder?v='+v)
        .then(r=>r.text())
        .then(t=>document.getElementById('status').textContent=t)
        .catch(()=>document.getElementById('status').textContent='error');
    }
    function pollStatus(){
      fetch('/status')
        .then(r=>r.json())
        .then(d=>{
          document.getElementById('btn-mute').textContent=d.spin;
          document.getElementById('m1label').textContent=d.spin;
          var m1=document.getElementById('m1');
          var m2=document.getElementById('m2');
          var nospin=(d.spin==='NOSPIN');
          m2.disabled=nospin;
          m2.style.opacity=nospin?'0.3':'1';
          if(document.activeElement!==m1){
            m1.value=d.m1;
            document.getElementById('m1val').textContent=d.m1;
          }
          if(document.activeElement!==m2){
            m2.value=nospin?d.m1:d.m2;
            document.getElementById('m2val').textContent=nospin?d.m1:d.m2;
          }
          var f=document.getElementById('f');
          var motorsOff=(d.m1===0 && d.m2===0);
          f.disabled=motorsOff;
          f.style.opacity=motorsOff?'0.3':'1';
          if(motorsOff){f.value=0;document.getElementById('fval').textContent=0;}
          if(document.activeElement!==f && !motorsOff){
            f.value=d.f;
            document.getElementById('fval').textContent=d.f;
          }
          if(d.pan!==undefined) document.getElementById('pan-val').textContent=d.pan;
          if(d.pan_min!==undefined) document.getElementById('pan-min').textContent=d.pan_min;
          if(d.pan_max!==undefined) document.getElementById('pan-max').textContent=d.pan_max;
          if(d.tilt!==undefined) document.getElementById('tilt-val').textContent=d.tilt;
          if(d.tilt_min!==undefined) document.getElementById('tilt-min').textContent=d.tilt_min;
          if(d.tilt_max!==undefined) document.getElementById('tilt-max').textContent=d.tilt_max;
          if(d.step!==undefined) document.getElementById('btn-step').textContent=d.step+'\u00b0';
          if(d.version!==undefined) document.getElementById('fw-version').textContent=d.version;
        }).catch(()=>{});
    }
    var pollInterval;
    function startPolling(){
      pollInterval=setInterval(pollStatus,500);
      pollStatus();
    }
    function stopPolling(){
      if(pollInterval) clearInterval(pollInterval);
    }
    window.addEventListener('beforeunload',stopPolling);
    startPolling();
  </script>
</body>
</html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

// ── Firmware Update page ──────────────────────────────────────────────────────
static const char FIRMWARE_UPDATE_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Firmware Update</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0f0f1a;color:#e0e0e0;padding:16px}
h2{color:#e94560;text-align:center;margin:12px 0 20px;font-size:18px}
.card{background:#16213e;border-radius:12px;padding:18px;margin-bottom:12px}
.lbl{font-size:12px;color:#888;margin-bottom:8px}
button{width:100%;padding:16px;font-size:16px;font-weight:700;border:none;border-radius:10px;cursor:pointer;touch-action:manipulation}
#otaStatus{margin-top:8px;font-size:13px;color:#aaa;min-height:18px}
#otaStatus.error{color:#e94560}
#otaStatus.success{color:#66dd66}
#otaStatus.info{color:#888}
progress{width:100%;height:8px;border-radius:4px;accent-color:#e94560;display:none;margin-top:6px}
.back{display:block;text-align:center;color:#888;text-decoration:none;margin-top:4px;padding:14px;border:1px solid #333;border-radius:10px;background:#1a1a2e;font-size:16px;font-weight:600;touch-action:manipulation}
</style>
</head>
<body>
<h2>Firmware Update</h2>
<div class="card">
  <div class="lbl">Selecteaza fisier .bin</div>
  <input type="file" id="fwFile" accept=".bin" onchange="readFwFile()" style="color:#e0e0e0;margin-bottom:8px;width:100%">
  <button onclick="uploadFw()" style="background:#4a148c;color:#e1bee7">Upload Firmware</button>
  <progress id="fwProg" value="0" max="100"></progress>
  <div id="otaStatus"></div>
</div>
<a href="/" class="back">&#8592; Inapoi</a>
<script>
function readFwFile(){
  var f=document.getElementById('fwFile').files[0];
  var st=document.getElementById('otaStatus');
  if(!f){st.textContent='';return;}
  if(!f.name.toLowerCase().endsWith('.bin')){st.className='error';st.textContent='Fisier invalid! Trebuie sa fie .bin';return;}
  st.className='info';st.textContent='Analizeaza...';
  var reader=new FileReader();
  reader.onload=function(e){
    var b=new Uint8Array(e.target.result);
    if(b[0]!==0xE9){st.className='error';st.textContent='Header invalid! Nu e firmware ESP32 (magic=0x'+b[0].toString(16).toUpperCase()+')';return;}
    var chipId=b[12]|(b[13]<<8);
    if(chipId!==0){st.className='error';st.textContent='Firmware pentru chip diferit (ID=0x'+chipId.toString(16).padStart(4,'0').toUpperCase()+', asteptam ESP32)';return;}
    var ver='unknown';
    for(var i=0;i<b.length-36;i++){
      if(b[i]===0xFE&&b[i+1]===0xED&&b[i+2]===0xBE&&b[i+3]===0xEF){
        var v='';
        for(var j=0;j<32;j++){if(b[i+4+j]===0)break;v+=String.fromCharCode(b[i+4+j]);}
        if(v)ver=v;
        break;
      }
    }
    st.className='success';
    st.textContent='Versiune: '+ver+' | '+Math.round(f.size/1024)+'KB | ESP32 (OK)';
  };
  reader.onerror=function(){st.className='error';st.textContent='Eroare citire fisier'};
  reader.readAsArrayBuffer(f);
}
function uploadFw(){
  var f=document.getElementById('fwFile').files[0];
  if(!f){document.getElementById('otaStatus').textContent='Selecteaza un fisier .bin';return;}
  if(!f.name.toLowerCase().endsWith('.bin')){document.getElementById('otaStatus').textContent='Fisier invalid!';return;}
  if(f.size<100000){document.getElementById('otaStatus').textContent='Fisier prea mic ('+Math.round(f.size/1024)+'KB)';return;}
  var prog=document.getElementById('fwProg');
  var st=document.getElementById('otaStatus');
  var fd=new FormData();fd.append('file',f,f.name);
  var xhr=new XMLHttpRequest();
  xhr.open('POST','/update');
  xhr.upload.onprogress=function(e){if(e.lengthComputable){var p=Math.round(e.loaded/e.total*100);prog.style.display='block';prog.value=p;st.className='info';st.textContent='Upload: '+p+'% ('+Math.round(f.size/1024)+'KB)';}};
  xhr.onload=function(){prog.style.display='none';if(xhr.status===200&&xhr.responseText==='OK'){st.className='success';st.textContent='Upload OK! Dispozitivul se restarteaza...';}else{st.className='error';st.textContent='Eroare: '+xhr.responseText;}};
  xhr.onerror=function(){st.className='error';st.textContent='Eroare conexiune';prog.style.display='none';};
  st.className='info';st.textContent='Uploading ('+Math.round(f.size/1024)+'KB)...';
  xhr.send(fd);
}
</script>
</body>
</html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

// ── Settings page ─────────────────────────────────────────────────────────────
static const char SETTINGS_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="ro">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Settings</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:sans-serif;background:#1a1a2e;color:#eee;
         display:flex;flex-direction:column;align-items:center;
         padding:20px;gap:16px;min-height:100vh}
    h2{letter-spacing:2px;font-size:18px;margin-bottom:4px}
    .card{background:#16213e;border-radius:14px;padding:16px 20px;
          width:100%;max-width:340px}
    .grid{display:grid;grid-template-columns:auto 1fr 1fr;gap:10px;align-items:center}
    .col-hdr{font-size:11px;color:#aaa;text-align:center;padding-bottom:2px}
    .row-lbl{font-size:13px;color:#e94560;font-weight:bold;padding-right:8px}
    .grid input{background:#0f3460;border:none;border-radius:8px;
                color:#eee;font-size:20px;padding:10px 6px;
                width:100%;text-align:center;font-weight:bold}
    .btn{width:100%;border:none;border-radius:10px;cursor:pointer;
         font-size:16px;color:#eee;padding:13px;font-weight:bold}
    .btn-save{background:#e94560}
    .btn-back{background:#0f3460;color:#aaa;margin-top:8px}
    #status{font-size:12px;color:#555}
    /* confirm modal */
    #modal{display:none;position:fixed;inset:0;background:#0008;
           z-index:999;align-items:center;justify-content:center}
    #modal.show{display:flex}
    .modal-box{background:#16213e;border-radius:14px;padding:24px 20px;
               max-width:280px;width:90%;text-align:center}
    .modal-box p{margin-bottom:20px;font-size:14px;line-height:1.5}
    .modal-btns{display:flex;gap:10px}
    .modal-btns button{flex:1;border:none;border-radius:10px;padding:12px;
                       font-size:15px;font-weight:bold;cursor:pointer}
    .btn-ok{background:#e94560;color:#eee}
    .btn-cancel{background:#0f3460;color:#aaa}
  </style>
</head>
<body>
  <h2>&#9881; Settings</h2>

  <div class="card">
    <div style="font-size:12px;color:#aaa;margin-bottom:8px;font-weight:bold;letter-spacing:1px">PAN</div>
    <div class="grid">
      <div></div>
      <div class="col-hdr">min</div>
      <div class="col-hdr">max</div>
      <div class="row-lbl">pan</div>
      <input type="number" id="pan_min" min="0" max="90" value="5">
      <input type="number" id="pan_max" min="0" max="90" value="50">
    </div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px">
      <button class="btn" style="background:#0f3460;font-size:14px" onclick="gotoLimit('pan','min')">&#9664; min</button>
      <button class="btn" style="background:#0f3460;font-size:14px" onclick="gotoLimit('pan','max')">max &#9654;</button>
    </div>
  </div>

  <div class="card">
    <div style="font-size:12px;color:#aaa;margin-bottom:8px;font-weight:bold;letter-spacing:1px">TILT</div>
    <div class="grid">
      <div></div>
      <div class="col-hdr">min</div>
      <div class="col-hdr">max</div>
      <div class="row-lbl">tilt</div>
      <input type="number" id="tilt_min" min="0" max="90" value="5">
      <input type="number" id="tilt_max" min="0" max="90" value="50">
    </div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px">
      <button class="btn" style="background:#0f3460;font-size:14px" onclick="gotoLimit('tilt','min')">&#9660; min</button>
      <button class="btn" style="background:#0f3460;font-size:14px" onclick="gotoLimit('tilt','max')">max &#9650;</button>
    </div>
  </div>

  <div style="width:100%;max-width:340px;display:flex;flex-direction:column;gap:8px">
    <button class="btn btn-save" onclick="confirmSave()">&#128190; Salveaza</button>
    <button class="btn btn-back" onclick="window.location='/'">&#8592; Inapoi</button>
  </div>

  <div id="status">Se incarca...</div>

  <!-- confirm modal -->
  <div id="modal">
    <div class="modal-box">
      <p>Salvezi limitele PAN/TILT?</p>
      <div class="modal-btns">
        <button class="btn-ok" onclick="doSave()">OK</button>
        <button class="btn-cancel" onclick="closeModal()">Cancel</button>
      </div>
    </div>
  </div>

  <script>
    var saved={pan_min:5,pan_max:50,tilt_min:5,tilt_max:50};

    function applyValues(v){
      document.getElementById('pan_min').value=v.pan_min;
      document.getElementById('pan_max').value=v.pan_max;
      document.getElementById('tilt_min').value=v.tilt_min;
      document.getElementById('tilt_max').value=v.tilt_max;
    }

    fetch('/status')
      .then(r=>r.json())
      .then(d=>{
        if(d.pan_min!==undefined)  saved.pan_min=d.pan_min;
        if(d.pan_max!==undefined)  saved.pan_max=d.pan_max;
        if(d.tilt_min!==undefined) saved.tilt_min=d.tilt_min;
        if(d.tilt_max!==undefined) saved.tilt_max=d.tilt_max;
        applyValues(saved);
        document.getElementById('status').textContent='ready';
      }).catch(()=>{document.getElementById('status').textContent='error loading';});

    function confirmSave(){
      var pm=parseInt(document.getElementById('pan_min').value);
      var pM=parseInt(document.getElementById('pan_max').value);
      var tm=parseInt(document.getElementById('tilt_min').value);
      var tM=parseInt(document.getElementById('tilt_max').value);
      var err='';
      if(pm<=0||tm<=0)         err='min trebuie sa fie > 0';
      else if(pM>=60||tM>=60)  err='max trebuie sa fie < 60';
      else if(pm>=pM)          err='PAN: min trebuie sa fie < max';
      else if(tm>=tM)          err='TILT: min trebuie sa fie < max';
      if(err){
        document.getElementById('status').textContent='\u26a0 '+err;
        applyValues(saved);
        return;
      }
      document.getElementById('modal').classList.add('show');
    }
    function closeModal()  { document.getElementById('modal').classList.remove('show'); }

    function gotoLimit(axis,lim){
      fetch('/'+axis+lim)
        .then(r=>r.text())
        .then(t=>document.getElementById('status').textContent=t)
        .catch(()=>document.getElementById('status').textContent='error');
    }

    function doSave(){
      closeModal();
      var pm=parseInt(document.getElementById('pan_min').value);
      var pM=parseInt(document.getElementById('pan_max').value);
      var tm=parseInt(document.getElementById('tilt_min').value);
      var tM=parseInt(document.getElementById('tilt_max').value);
      fetch('/setlimits?pan_min='+pm+'&pan_max='+pM+'&tilt_min='+tm+'&tilt_max='+tM)
        .then(r=>r.text())
        .then(t=>{
          saved={pan_min:pm,pan_max:pM,tilt_min:tm,tilt_max:tM};
          document.getElementById('status').textContent=t;
        })
        .catch(()=>{document.getElementById('status').textContent='error';});
    }
  </script>
</body>
</html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

// ── Motor Settings page ───────────────────────────────────────────────────────
static const char MOTOR_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="ro">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Motor Settings</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:sans-serif;background:#0f0f1a;color:#e0e0e0;padding:16px}
    h2{color:#e94560;text-align:center;margin:12px 0 20px;font-size:18px}
    .card{background:#16213e;border-radius:12px;padding:18px;margin-bottom:12px}
    .row{display:grid;grid-template-columns:1fr 1fr;gap:16px;margin-bottom:16px}
    .col{display:flex;flex-direction:column;align-items:center;gap:8px}
    .col-title{font-size:13px;color:#888;font-weight:bold;letter-spacing:1px}
    .btn{border:none;border-radius:10px;cursor:pointer;font-weight:bold;touch-action:manipulation;padding:12px 16px}
    .btn-up-down{font-size:28px;width:64px;height:64px;background:#0f3460;color:#e94560}
    .btn-up-down:active{transform:scale(0.9);background:#1a5480}
    button:disabled{cursor:not-allowed;opacity:0.5}
    .display{font-size:32px;font-weight:bold;color:#e94560;text-align:center;min-width:100px;padding:12px}
    .display-box{background:#0f0f1a;border-radius:8px;border:1px solid #333;padding:8px}
    .btn-save{width:100%;background:#4a148c;color:#e1bee7;font-size:14px}
    .table-wrap{overflow-x:auto}
    table{width:100%;border-collapse:collapse;font-size:12px}
    th{color:#888;font-weight:normal;padding:8px 4px;text-align:center;border-bottom:1px solid #333}
    td{padding:8px 4px;text-align:center;color:#e94560;font-weight:bold}
    td.idx{color:#555;font-size:10px}
    .const-row{display:flex;justify-content:space-between;font-size:12px;color:#888;padding:6px 0}
    .back{display:block;text-align:center;color:#888;text-decoration:none;padding:14px;border:1px solid #333;border-radius:10px;background:#1a1a2e;font-size:14px;font-weight:600;cursor:pointer;touch-action:manipulation;border:none}
  </style>
</head>
<body>
<h2>Motor Speed Setup</h2>

<div class="card">
  <div style="text-align:center;margin-bottom:16px">
    <div style="font-size:12px;color:#888;margin-bottom:8px">SPIN MODE</div>
    <button class="btn" id="spin-btn" style="background:#e94560;color:#0f0f1a;font-size:16px;font-weight:bold;padding:12px 24px;border-radius:10px;cursor:pointer" onclick="toggleSpin()">TOPSPIN</button>
  </div>
</div>

<div class="card">
  <div class="row">
    <div class="col">
      <span class="col-title">MAIN</span>
      <button class="btn btn-up-down" onclick="incMain()">▲</button>
      <div class="display-box">
        <div class="display" id="main-val">-</div>
      </div>
      <button class="btn btn-up-down" onclick="decMain()">▼</button>
      <button class="btn btn-save" onclick="saveMain()">SAVE T1</button>
    </div>
    <div class="col">
      <span class="col-title">SUPPORT</span>
      <button class="btn btn-up-down" id="pup-btn" onclick="incSupp()">▲</button>
      <div class="display-box">
        <div class="display" id="supp-val">-</div>
      </div>
      <button class="btn btn-up-down" id="pdown-btn" onclick="decSupp()">▼</button>
      <button class="btn btn-save" id="t2-btn" onclick="saveSupp()">SAVE T2</button>
    </div>
  </div>
</div>

<div class="card">
  <div style="font-size:12px;color:#888;margin-bottom:12px;font-weight:bold">Viteze Salvate</div>
  <div class="table-wrap">
    <table>
      <thead>
        <tr>
          <th>#</th>
          <th>MAIN (us)</th>
          <th>SUPPORT (us)</th>
        </tr>
      </thead>
      <tbody id="speeds-body">
        <tr><td colspan="3" style="color:#555">Se incarca...</td></tr>
      </tbody>
    </table>
  </div>
</div>

<div class="card">
  <div style="font-size:12px;color:#888;margin-bottom:12px;font-weight:bold">Constante</div>
  <div class="const-row"><span>MOTOR_STEP</span><span>30</span></div>
  <div class="const-row"><span>MOTOR_STEP_SETUP</span><span>10</span></div>
  <div class="const-row"><span>MOTOR_STEP_NOSPIN</span><span>4</span></div>
  <div class="const-row"><span>SUPPORT_STEP_SETUP</span><span>4</span></div>
  <div class="const-row"><span>SUPPORT_STEP</span><span>8</span></div>
</div>

<button class="back" onclick="window.location='/'">◄ Inapoi</button>
<div id="status" style="text-align:center;font-size:12px;color:#666;margin-top:16px">ready</div>

<script>
var mainIdx=0, suppIdx=0, currentSpin='TOPSPIN';

function toggleSpin(){
  fetch('/setspin')
    .then(r=>r.text())
    .then(t=>{
      poll();
    })
    .catch(e=>console.log(e));
}

function poll(){
  fetch('/mstatus')
    .then(r=>r.json())
    .then(d=>{
      currentSpin=d.spin;
      document.getElementById('spin-btn').textContent=currentSpin;
      document.getElementById('main-val').textContent=d.main_speed||'-';
      document.getElementById('supp-val').textContent=d.support_speed||'-';
      var tbody=document.getElementById('speeds-body');
      tbody.innerHTML='';
      for(var i=1;i<=8;i++){
        var tr=document.createElement('tr');
        tr.innerHTML='<td class="idx">'+i+'</td><td>'+d.up[i]+'</td><td>'+d.down[i]+'</td>';
        tbody.appendChild(tr);
      }
      // Disable P buttons in NOSPIN mode
      var isNospin = (d.spin === 'NOSPIN');
      document.getElementById('pup-btn').disabled = isNospin;
      document.getElementById('pdown-btn').disabled = isNospin;
      document.getElementById('t2-btn').disabled = isNospin;
      if(isNospin){
        document.getElementById('pup-btn').style.opacity='0.5';
        document.getElementById('pdown-btn').style.opacity='0.5';
        document.getElementById('t2-btn').style.opacity='0.5';
      }else{
        document.getElementById('pup-btn').style.opacity='1';
        document.getElementById('pdown-btn').style.opacity='1';
        document.getElementById('t2-btn').style.opacity='1';
      }
    }).catch(e=>document.getElementById('status').textContent='error');
}

function incMain(){
  fetch('/vup').then(()=>poll()).catch(e=>console.log(e));
}

function decMain(){
  fetch('/vdown').then(()=>poll()).catch(e=>console.log(e));
}

function incSupp(){
  fetch('/pup').then(()=>poll()).catch(e=>console.log(e));
}

function decSupp(){
  fetch('/pdown').then(()=>poll()).catch(e=>console.log(e));
}

function saveMain(){
  fetch('/t1save').then(r=>r.text()).then(t=>document.getElementById('status').textContent=t).catch(e=>document.getElementById('status').textContent='error');
  setTimeout(poll, 500);
}

function saveSupp(){
  fetch('/t2save').then(r=>r.text()).then(t=>document.getElementById('status').textContent=t).catch(e=>document.getElementById('status').textContent='error');
  setTimeout(poll, 500);
}

poll();
</script>
</body>
</html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

WebControl::WebControl() : _server(80)
{
  _instance = this;
}

void WebControl::_connect_wifi()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  IPAddress apIP(192, 168, 4, 1);
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apIP, apSubnet);
  
  // Start DNS server - redirects ALL DNS queries to AP IP
  _dnsServer.start(53, "*", apIP);
  
  Serial.printf("WiFi AP started — SSID: %s  http://%s\n",
                WIFI_SSID, WiFi.softAPIP().toString().c_str());
  Serial.println("DNS server started on port 53 (captive portal mode)");
}

void WebControl::_register_routes()
{
  _server.on("/",       HTTP_GET, _s_root);
  _server.on("/up",     HTTP_GET, _s_up);
  _server.on("/down",   HTTP_GET, _s_down);
  _server.on("/left",   HTTP_GET, _s_left);
  _server.on("/right",  HTTP_GET, _s_right);
  _server.on("/power",  HTTP_GET, _s_power);
  _server.on("/mute",   HTTP_GET, _s_mute);
  _server.on("/motor1",   HTTP_GET, _s_motor1);
  _server.on("/motor2",   HTTP_GET, _s_motor2);
  _server.on("/feeder",   HTTP_GET, _s_feeder);
  _server.on("/savepos",    HTTP_GET, _s_savepos);
  _server.on("/home",       HTTP_GET, _s_home);
  _server.on("/status",     HTTP_GET, _s_status);
  _server.on("/step",       HTTP_GET, _s_step);
  _server.on("/settings",      HTTP_GET, _s_settings);
  _server.on("/setlimits",     HTTP_GET, _s_setlimits);
  _server.on("/motorsettings", HTTP_GET, _s_motorsettings);
  _server.on("/mstatus",       HTTP_GET, _s_mstatus);
  _server.on("/t1save",        HTTP_GET, _s_t1save);
  _server.on("/t2save",        HTTP_GET, _s_t2save);
  _server.on("/setspin",       HTTP_GET, _s_setspin);
  _server.on("/vup",           HTTP_GET, _s_vup);
  _server.on("/vdown",         HTTP_GET, _s_vdown);
  _server.on("/pup",           HTTP_GET, _s_pup);
  _server.on("/pdown",         HTTP_GET, _s_pdown);
  _server.on("/panmin",  HTTP_GET, _s_panmin);
  _server.on("/panmax",  HTTP_GET, _s_panmax);
  _server.on("/tiltmin", HTTP_GET, _s_tiltmin);
  _server.on("/tiltmax", HTTP_GET, _s_tiltmax);
  _server.on("/firmware", HTTP_GET, _s_firmware);
  _server.on("/update", HTTP_POST, _s_update_done, _s_update_upload);
  _server.on("/favicon.ico", HTTP_GET, [this](){ _server.send(204, "text/plain", ""); });
  
  // Captive portal routes - iOS/Android/Windows probes
  _server.on("/hotspot-detect.html", HTTP_GET, _s_captive_portal);
  _server.on("/generate_204", HTTP_GET, _s_captive_portal);
  _server.on("/gen_204", HTTP_GET, _s_captive_portal);
  _server.on("/ncsi.txt", HTTP_GET, _s_captive_portal);
  _server.on("/connecttest.txt", HTTP_GET, _s_captive_portal);
  _server.on("/fwlink", HTTP_GET, _s_captive_portal);
  
  // Catch-all: redirect any unknown request to home page
  _server.onNotFound(_s_captive_portal);
}

// ── HTTP handlers ─────────────────────────────────────────────────────────────

void WebControl::_handle_root()
{
  _server.send_P(200, "text/html", HTML_PAGE);
}

void WebControl::_handle_up()
{
  infrared_menu(hTUP, mode);
  _server.send(200, "text/plain", "UP OK");
}

void WebControl::_handle_down()
{
  infrared_menu(hTDOWN, mode);
  _server.send(200, "text/plain", "DOWN OK");
}

void WebControl::_handle_left()
{
  infrared_menu(hTLEFT, mode);
  _server.send(200, "text/plain", "LEFT OK");
}

void WebControl::_handle_right()
{
  infrared_menu(hTRIGHT, mode);
  _server.send(200, "text/plain", "RIGHT OK");
}

void WebControl::_handle_power()
{
  infrared_menu(hPower, mode);
  _server.send(200, "text/plain", "POWER OK");
}

void WebControl::_handle_mute()
{
  infrared_menu(hMute, mode);
  _server.send(200, "text/plain", "MUTE OK");
}

void WebControl::_handle_step()
{
  if (servo_step == 4) servo_step = 6;
  else if (servo_step == 6) servo_step = 8;
  else servo_step = 4;
  
  char buffer[64] = {0};
  sprintf(buffer, "{\"step\":%d}", servo_step);
  _server.send(200, "application/json", buffer);
}

void WebControl::_handle_settings()
{
  _server.send_P(200, "text/html", SETTINGS_PAGE);
}

void WebControl::_handle_panmin()
{
  pan.startMove(pan.min_value);
  _server.send(200, "text/plain", "PAN -> min");
}

void WebControl::_handle_panmax()
{
  pan.startMove(pan.max_value);
  _server.send(200, "text/plain", "PAN -> max");
}

void WebControl::_handle_tiltmin()
{
  tilt.startMove(tilt.min_value);
  _server.send(200, "text/plain", "TILT -> min");
}

void WebControl::_handle_tiltmax()
{
  tilt.startMove(tilt.max_value);
  _server.send(200, "text/plain", "TILT -> max");
}

void WebControl::_handle_firmware()
{
  _server.send_P(200, "text/html", FIRMWARE_UPDATE_PAGE);
}

void WebControl::_handle_update_upload()
{
  HTTPUpload& upload = _server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    // Start of upload — validate header and begin update
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
      Update.printError(Serial);
      return;
    }
    Serial.printf("[OTA] Upload start: %s\n", upload.filename.c_str());
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Chunk received — validate magic byte and write
    if (upload.totalSize == 0 && upload.currentSize < 10) {
      // First chunk — validate ESP32 header (magic byte 0xE9)
      if (upload.buf[0] != 0xE9) {
        Update.abort();
        Serial.println("[OTA] ERROR: Invalid magic byte - not an ESP32 firmware!");
        return;
      }
    }
    
    // Write chunk to flash
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
      return;
    }
    Serial.printf("[OTA] Chunk: %u bytes (total: %u KB)\n", 
                  upload.currentSize, upload.totalSize / 1024);
  }
  else if (upload.status == UPLOAD_FILE_END) {
    // Upload complete — finalize and restart
    if (Update.end(true)) {
      Serial.printf("[OTA] SUCCESS: %u bytes uploaded. Restarting...\n", upload.totalSize);
      _server.send(200, "text/plain", "Update OK, rebooting...");
      delay(500);
      ESP.restart();
    } else {
      Update.printError(Serial);
      _server.send(400, "text/plain", "Update failed");
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    Serial.println("[OTA] Upload aborted");
  }
}

void WebControl::_handle_update_done()
{
  // Callback after POST - not used in this flow since restart happens in _handle_update_upload
  // But required to be defined for HTTP_POST with two parameters
}

void WebControl::_handle_motorsettings()
{
  _server.send_P(200, "text/html", MOTOR_PAGE);
}

void WebControl::_handle_mstatus()
{
  String spin;
  Brush* mainMotor = NULL;
  Brush* supportMotor = NULL;
  
  // Identify spin mode and get main/support motor pointers
  if (motor_up.spin == Brush::TOPSPIN) {
    spin = "TOPSPIN";
    mainMotor = &motor_up;
    supportMotor = &motor_down;
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN mode
    spin = "BACKSPIN";
    mainMotor = &motor_down;
    supportMotor = &motor_up;
  } else if (motor_up.spin == Brush::NOSPIN) {
    spin = "NOSPIN";
    mainMotor = &motor_up;
    supportMotor = &motor_down;
  } else {
    spin = "TOPSPIN";
    mainMotor = &motor_up;
    supportMotor = &motor_down;
  }

  char buffer[1024] = {0};
  
  // Build JSON with MAIN/SUPPORT speeds (not motor_up/motor_down fixed!)
  sprintf(buffer, "{\"spin\":\"%s\",\"main_speed\":%d,\"support_speed\":%d,\"up\":[" , 
          spin.c_str(), mainMotor->speed, supportMotor->speed);
  
  // Append MAIN motor speeds array
  for (int i = 0; i < 9; i++) {
    char num[10];
    sprintf(num, "%d", mainMotor->_SPEEDS[i]);
    strcat(buffer, num);
    if (i < 8) strcat(buffer, ",");
  }
  strcat(buffer, "],\"down\":[" );
  
  // Append SUPPORT motor speeds array
  for (int i = 0; i < 9; i++) {
    char num[10];
    sprintf(num, "%d", supportMotor->_SPEEDS[i]);
    strcat(buffer, num);
    if (i < 8) strcat(buffer, ",");
  }
  strcat(buffer, "]}");
  
  _server.send(200, "application/json", buffer);
}

void WebControl::_handle_t1save()
{
  // Save MAIN motor speed configuration (like T1 button on remote)
  // Identify main motor based on spin mode
  Brush* mainMotor = &motor_up; // default TOPSPIN/NOSPIN
  uint8_t spinMode = Brush::TOPSPIN;
  
  if (motor_up.spin == Brush::TOPSPIN) {
    mainMotor = &motor_up;
    spinMode = Brush::TOPSPIN;
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN: motor_down is MAIN
    mainMotor = &motor_down;
    spinMode = Brush::BACKSPIN;
  } else if (motor_up.spin == Brush::NOSPIN) {
    mainMotor = &motor_up;
    spinMode = Brush::NOSPIN;
  }
  
  // Update _SPEEDS array based on current spin mode
  if (spinMode == Brush::TOPSPIN) {
    mainMotor->update_speeds(mainMotor->_SPEEDS, mainMotor->speed, "TOPSPIN", MOTOR_STEP);
  } else if (spinMode == Brush::BACKSPIN) {
    mainMotor->update_speeds(mainMotor->_SPEEDS, mainMotor->speed, "BACKSPIN", MOTOR_STEP);
  } else if (spinMode == Brush::NOSPIN) {
    mainMotor->update_speeds_nospin(mainMotor->_SPEEDS, mainMotor->speed, "NOSPIN", SUPPORT_STEP);
    // NOSPIN: sync support motor with same speeds
    motor_down.update_speeds_nospin(motor_down._SPEEDS, mainMotor->speed, "NOSPIN", SUPPORT_STEP);
  }
  
  // Save to NVS and reload
  mainMotor->save_data_as();
  mainMotor->load_data_as();
  
  // NOSPIN: also save support motor
  if (spinMode == Brush::NOSPIN) {
    motor_down.save_data_as();
    motor_down.load_data_as();
  }
  
  // Display OK sign on LED matrix
  display.displayImage_async(IMAGES[12], 1); // ok save
  
  _server.send(200, "text/plain", "Main motor saved");
}

void WebControl::_handle_t2save()
{
  // Save SUPPORT motor speed configuration (like T2 button on remote)
  // Identify support motor based on spin mode
  Brush* supportMotor = &motor_down; // default TOPSPIN/NOSPIN
  uint8_t spinMode = Brush::TOPSPIN;
  
  if (motor_up.spin == Brush::TOPSPIN) {
    supportMotor = &motor_down;
    spinMode = Brush::TOPSPIN;
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN: motor_up is SUPPORT
    supportMotor = &motor_up;
    spinMode = Brush::BACKSPIN;
  } else if (motor_up.spin == Brush::NOSPIN) {
    supportMotor = &motor_down;
    spinMode = Brush::NOSPIN;
  }
  
  // Update _SPEEDS array based on current spin mode
  if (spinMode == Brush::TOPSPIN) {
    supportMotor->update_speeds(supportMotor->_SPEEDS, supportMotor->speed, "SUPPORT", SUPPORT_STEP);
  } else if (spinMode == Brush::BACKSPIN) {
    supportMotor->update_speeds(supportMotor->_SPEEDS, supportMotor->speed, "SUPPORT", SUPPORT_STEP);
  } else if (spinMode == Brush::NOSPIN) {
    // NOSPIN: T2 is disabled anyway, but sync both motors
    motor_up.update_speeds_nospin(motor_up._SPEEDS, supportMotor->speed, "NOSPIN", SUPPORT_STEP);
    motor_down.update_speeds_nospin(motor_down._SPEEDS, supportMotor->speed, "NOSPIN", SUPPORT_STEP);
  }
  
  // Save to NVS and reload
  supportMotor->save_data_as();
  supportMotor->load_data_as();
  
  // NOSPIN: also save the other motor
  if (spinMode == Brush::NOSPIN) {
    if (supportMotor == &motor_down) {
      motor_up.save_data_as();
      motor_up.load_data_as();
    } else {
      motor_down.save_data_as();
      motor_down.load_data_as();
    }
  }
  
  // Display OK sign on LED matrix
  display.displayImage_async(IMAGES[12], 1); // ok save
  
  _server.send(200, "text/plain", "Support motor saved");
}

void WebControl::_handle_setspin()
{
  // Change spin mode - mirrors _TMute() from infr_motor.h
  // MUTE button: stop_motors() + toggle_spin()
  
  // Stop motors first
  feeder.index = 0;
  feeder.enable = false;
  motor_down.index = 0;
  motor_up.index = 0;
  motor_up.speed = motor_up._SPEEDS[motor_up.index];
  motor_down.speed = motor_down._SPEEDS[motor_down.index];
  motor_down.set_speed();
  motor_up.set_speed();
  
  // Toggle spin mode: T->B->N->T
  if (motor_up.spin == Brush::TOPSPIN) {
    motor_up.spin = Brush::SUPPORT;
    motor_down.spin = Brush::BACKSPIN;
    motor_up.set_spin(Brush::SUPPORT);
    motor_down.set_spin(Brush::BACKSPIN);
    display.displayImage_async(IMAGES[12], 0.5); // Display on LED
  } else if (motor_up.spin == Brush::SUPPORT) {
    motor_up.spin = Brush::NOSPIN;
    motor_down.spin = Brush::NOSPIN;
    motor_up.set_spin(Brush::NOSPIN);
    motor_down.set_spin(Brush::NOSPIN);
    display.displayImage_async(IMAGES[12], 0.5);
  } else {
    motor_up.spin = Brush::TOPSPIN;
    motor_down.spin = Brush::SUPPORT;
    motor_up.set_spin(Brush::TOPSPIN);
    motor_down.set_spin(Brush::SUPPORT);
    display.displayImage_async(IMAGES[12], 0.5);
  }
  
  // Load speeds from NVS for the new spin mode
  motor_up.load_data_as();
  motor_down.load_data_as();
  
  _server.send(200, "text/plain", "Spin toggled");
}

void WebControl::_handle_vup()
{
  // V+ button: increase MAIN motor speed (V+/V- logic from infr_motor.h)
  // TOPSPIN: motor_up = main, use MOTOR_STEP_SETUP
  // BACKSPIN: motor_down = main, use MOTOR_STEP_SETUP
  // NOSPIN: both motors synchronized from main control
  
  if (motor_up.spin == Brush::TOPSPIN) {
    motor_up.increase_speed(MOTOR_STEP_SETUP);
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN case
    motor_down.increase_speed(MOTOR_STEP_SETUP);
  } else {
    // NOSPIN case: both motors controlled together
    motor_up.increase_speed(SUPPORT_STEP_SETUP);
    motor_down.increase_speed(SUPPORT_STEP_SETUP);
  }
  
  display.displayImage_async(IMAGES[10], 0.5);
  _server.send(200, "text/plain", "V+");
}

void WebControl::_handle_vdown()
{
  // V- button: decrease MAIN motor speed (V+/V- logic from infr_motor.h)
  // TOPSPIN: motor_up = main, use MOTOR_STEP_SETUP
  // BACKSPIN: motor_down = main, use MOTOR_STEP_SETUP
  // NOSPIN: both motors synchronized from main control
  
  if (motor_up.spin == Brush::TOPSPIN) {
    motor_up.decrease_speed(MOTOR_STEP_SETUP);
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN case
    motor_down.decrease_speed(MOTOR_STEP_SETUP);
  } else {
    // NOSPIN case: both motors controlled together
    motor_up.decrease_speed(SUPPORT_STEP_SETUP);
    motor_down.decrease_speed(SUPPORT_STEP_SETUP);
  }
  
  display.displayImage_async(IMAGES[8], 0.2);
  _server.send(200, "text/plain", "V-");
}

void WebControl::_handle_pup()
{
  // P+ button: increase SUPPORT motor speed (P+/P- logic from infr_motor.h)
  // TOPSPIN: motor_down = support
  // BACKSPIN: motor_up = support
  // NOSPIN: DISABLED - support controls inactive
  
  if (motor_up.spin == Brush::NOSPIN) {
    // NOSPIN: P controls are disabled
    _server.send(200, "text/plain", "P+ disabled (NOSPIN mode)");
    return;
  }
  
  if (motor_up.spin == Brush::TOPSPIN) {
    motor_down.increase_speed(SUPPORT_STEP_SETUP);
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN case
    motor_up.increase_speed(SUPPORT_STEP_SETUP);
  }
  
  display.displayImage_async(IMAGES[10], 0.2);
  _server.send(200, "text/plain", "P+");
}

void WebControl::_handle_pdown()
{
  // P- button: decrease SUPPORT motor speed (P+/P- logic from infr_motor.h)
  // TOPSPIN: motor_down = support
  // BACKSPIN: motor_up = support
  // NOSPIN: DISABLED - support controls inactive
  
  if (motor_up.spin == Brush::NOSPIN) {
    // NOSPIN: P controls are disabled
    _server.send(200, "text/plain", "P- disabled (NOSPIN mode)");
    return;
  }
  
  if (motor_up.spin == Brush::TOPSPIN) {
    motor_down.decrease_speed(SUPPORT_STEP_SETUP);
  } else if (motor_up.spin == Brush::SUPPORT) {
    // BACKSPIN case
    motor_up.decrease_speed(SUPPORT_STEP_SETUP);
  }
  
  display.displayImage_async(IMAGES[8], 0.2);
  _server.send(200, "text/plain", "P-");
}

void WebControl::_handle_setlimits()
{
  int pan_min  = _server.hasArg("pan_min")  ? _server.arg("pan_min").toInt()  : (int)pan.min_value;
  int pan_max  = _server.hasArg("pan_max")  ? _server.arg("pan_max").toInt()  : (int)pan.max_value;
  int tilt_min = _server.hasArg("tilt_min") ? _server.arg("tilt_min").toInt() : (int)tilt.min_value;
  int tilt_max = _server.hasArg("tilt_max") ? _server.arg("tilt_max").toInt() : (int)tilt.max_value;

  if (pan_min <= 0 || tilt_min <= 0)        { _server.send(400, "text/plain", "min trebuie sa fie > 0"); return; }
  if (pan_max >= 60 || tilt_max >= 60)      { _server.send(400, "text/plain", "max trebuie sa fie < 60"); return; }
  if (pan_min >= pan_max)                   { _server.send(400, "text/plain", "PAN: min trebuie sa fie < max"); return; }
  if (tilt_min >= tilt_max)                 { _server.send(400, "text/plain", "TILT: min trebuie sa fie < max"); return; }

  pan.min_value  = (uint8_t)pan_min;
  pan.max_value  = (uint8_t)pan_max;
  tilt.min_value = (uint8_t)tilt_min;
  tilt.max_value = (uint8_t)tilt_max;
  pan.save_limits();
  tilt.save_limits();
  _server.send(200, "text/plain", "Limits saved");
}

void WebControl::_handle_motor1()
{
  if (_server.hasArg("v"))
  {
    int v = _server.arg("v").toInt();
    if (v < 0) v = 0;
    if (v > 8) v = 8;
    // M1 = main motor (_VUP/_VDOWN logic)
    // TOPSPIN: motor_up = main
    // BACKSPIN (motor_up=SUPPORT): motor_down = main
    // NOSPIN: both synchronized
    if (motor_up.spin == Brush::TOPSPIN)
      motor_up.set_speed((uint8_t)v);
    else if (motor_up.spin == Brush::SUPPORT)
      motor_down.set_speed((uint8_t)v);
    else
    {
      motor_up.set_speed((uint8_t)v);
      motor_down.set_speed((uint8_t)v);
    }
    if (motor_up.index == 0 && motor_down.index == 0)
    {
      feeder.index = 0;
      feeder.stop();
    }
    display.status(motor_up.index, motor_down.index, feeder.index);
  }
  _server.send(200, "text/plain", "M1 OK");
}

void WebControl::_handle_motor2()
{
  if (_server.hasArg("v"))
  {
    int v = _server.arg("v").toInt();
    if (v < 0) v = 0;
    if (v > 8) v = 8;
    // M2 = support motor (_PUP/_PDOWN logic)
    // TOPSPIN: motor_down = support
    // BACKSPIN (motor_up=SUPPORT): motor_up = support
    // NOSPIN: inactive
    if (motor_up.spin == Brush::TOPSPIN)
      motor_down.set_speed((uint8_t)v);
    else if (motor_up.spin == Brush::SUPPORT)
      motor_up.set_speed((uint8_t)v);
    // NOSPIN: P slider inactive, do nothing
    if (motor_up.index == 0 && motor_down.index == 0)
    {
      feeder.index = 0;
      feeder.stop();
    }
    display.status(motor_up.index, motor_down.index, feeder.index);
  }
  _server.send(200, "text/plain", "M2 OK");
}

void WebControl::_handle_feeder()
{
  if (_server.hasArg("v"))
  {
    int v = _server.arg("v").toInt();
    if (v < 0) v = 0;
    if (v > 8) v = 8;
    // Don't start feeder when both motors are stopped
    if (v > 0 && motor_up.index == 0 && motor_down.index == 0)
    {
      _server.send(200, "text/plain", "F DISABLED");
      return;
    }
    feeder.index = (int8_t)v;
    if (v == 0)
      feeder.stop();
    else
      feeder.start();
    display.status(motor_up.index, motor_down.index, feeder.index);
  }
  _server.send(200, "text/plain", "F OK");
}

void WebControl::_handle_savepos()
{
  pan.save_pos((uint8_t)pan.read_pos());
  tilt.save_pos((uint8_t)tilt.read_pos());
  _server.send(200, "text/plain", "POS SAVED");
}

void WebControl::_handle_home()
{
  pan.startMove(pan.init_value);
  tilt.startMove(tilt.init_value);
  _server.send(200, "text/plain", "HOME OK");
}

void WebControl::_handle_status()
{
  String spin = motor_up.spintype;
  // translate internal spintype to user-friendly label
  if (motor_up.spin == Brush::TOPSPIN)     spin = "TOPSPIN";
  else if (motor_up.spin == Brush::SUPPORT) spin = "BACKSPIN";
  else if (motor_up.spin == Brush::NOSPIN)  spin = "NOSPIN";

  // La BACKSPIN: slider1 controleaza motor_down, slider2 controleaza motor_up
  // → swap m1/m2 in status ca sliderele sa reflecte ce controleaza
  int m1idx = motor_up.index;
  int m2idx = motor_down.index;
  if (motor_up.spin == Brush::SUPPORT) // BACKSPIN cycle
  {
    m1idx = motor_down.index;
    m2idx = motor_up.index;
  }

  char buffer[512] = {0};
  // Build JSON with sprintf - includes firmware version for OTA detection
  sprintf(buffer, "{\"spin\":\"%s\",\"m1\":%d,\"m2\":%d,\"f\":%d,\"pan\":%d,\"pan_min\":%d,\"pan_max\":%d,\"tilt\":%d,\"tilt_min\":%d,\"tilt_max\":%d,\"step\":%d,\"version\":\"%s\"}",
          spin.c_str(), m1idx, m2idx, feeder.index, 
          pan.read_pos(), pan.min_value, pan.max_value,
          tilt.read_pos(), tilt.min_value, tilt.max_value,
          servo_step, FW_VERSION);
  
  _server.send(200, "application/json", buffer);
}

void WebControl::_handle_captive_portal()
{
  // Redirect all captive portal probes to home page
  _server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  _server.sendHeader("Location", "http://192.168.4.1/", true);
  _server.send(302, "text/plain", "Redirecting to Robot Control");
}

// ── FreeRTOS task ─────────────────────────────────────────────────────────────

void WebControl::_task(void *param)
{
  WebControl *self = static_cast<WebControl *>(param);
  self->_connect_wifi();
  {
    self->_register_routes();
    self->_server.begin();
    Serial.println("WebServer started on port 80");
    while (true)
    {
      self->_dnsServer.processNextRequest();  // Process DNS queries for captive portal
      self->_server.handleClient();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(nullptr);
}

void WebControl::begin()
{
  // Core 0, priority 1 — same core as IRTask but lower priority
  xTaskCreatePinnedToCore(_task, "WebControl", 8192, this, 1, nullptr, 0);
}
