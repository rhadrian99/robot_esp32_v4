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

  <div style="display:flex;flex-direction:column;gap:8px;width:100%;max-width:320px">
    <div style="display:flex;gap:8px">
      <a href="/settings" style="flex:1;text-decoration:none">
        <button style="width:100%;border:none;border-radius:10px;background:#0f3460;color:#aaa;
                       font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
          &#9881; Servo Settings
        </button>
      </a>
      <a href="/motorsettings" style="flex:1;text-decoration:none">
        <button style="width:100%;border:none;border-radius:10px;background:#0f3460;color:#aaa;
                       font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
          &#9881; Motor Settings
        </button>
      </a>
    </div>
    <a href="/firmware" style="width:100%;text-decoration:none">
      <button style="width:100%;border:none;border-radius:10px;background:#1a3a1a;color:#66dd66;
                     font-size:13px;padding:12px;cursor:pointer;font-weight:bold">
        &#11014; Firmware Update
      </button>
    </a>
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
            m2.value=nospin?0:d.m2;
            document.getElementById('m2val').textContent=nospin?0:d.m2;
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
        }).catch(()=>{});
    }
    setInterval(pollStatus,500);
    pollStatus();
  </script>
</body>
</html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

// ── Firmware Update page ──────────────────────────────────────────────────────
static const char FIRMWARE_UPDATE_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="ro">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Firmware Update</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{font-family:sans-serif;background:#1a1a2e;color:#eee;
         display:flex;flex-direction:column;align-items:center;
         padding:20px;gap:16px;min-height:100vh}
    h2{letter-spacing:2px;font-size:18px;margin-bottom:4px;color:#66dd66}
    .card{background:#16213e;border-radius:14px;padding:16px 20px;
          width:100%;max-width:340px}
    .btn{width:100%;border:none;border-radius:10px;cursor:pointer;
         font-size:16px;color:#eee;padding:13px;font-weight:bold;margin-bottom:8px}
    .btn-upload{background:#1a3a1a;color:#66dd66}
    .btn-back{background:#0f3460;color:#aaa}
    #status{font-size:12px;color:#999;margin-top:12px;padding:10px;
            background:#0f3460;border-radius:8px;border-left:3px solid #555}
    #status.success{border-left-color:#66dd66;color:#66dd66}
    #status.error{border-left-color:#e94560;color:#e94560}
    #status.info{border-left-color:#e94560;color:#aaa}
    .progress{width:100%;height:24px;background:#0f3460;border-radius:8px;
              overflow:hidden;display:none;margin:12px 0}
    .progress.show{display:block}
    .progress-bar{height:100%;background:#66dd66;width:0%;transition:width 0.2s}
    .file-input-wrapper{position:relative;overflow:hidden;display:inline-block;width:100%}
    .file-input-wrapper input[type=file]{position:absolute;left:-9999px}
    .file-input-label{display:flex;align-items:center;justify-content:center;
                      background:#1a3a1a;color:#66dd66;padding:13px;
                      border-radius:10px;cursor:pointer;font-weight:bold;
                      border:2px dashed #66dd66}
    .file-input-label:hover{background:#0a2a0a}
    .file-name{font-size:12px;color:#aaa;margin-top:8px;text-align:center}
  </style>
</head>
<body>
  <h2>&#11014; Firmware Update</h2>

  <div class="card">
    <div style="font-size:12px;color:#aaa;margin-bottom:12px;line-height:1.6">
      <p>Selecteaza un fisier .bin de firmware din folderul <code>release/</code></p>
      <p style="margin-top:8px;font-size:10px;color:#666">Ex: firmware61.bin pentru versiunea 6.1</p>
    </div>

    <div class="file-input-wrapper">
      <label class="file-input-label" for="fw-file">Alege fisier...</label>
      <input type="file" id="fw-file" accept=".bin" onchange="handleFileSelect(event)">
    </div>
    <div class="file-name" id="file-name"></div>

    <div class="progress" id="progress">
      <div class="progress-bar" id="progress-bar"></div>
    </div>

    <button class="btn btn-upload" onclick="uploadFirmware()" id="upload-btn" disabled>
      &#128304; Upload Firmware
    </button>
  </div>

  <div id="status">...</div>

  <a href="/" style="width:100%;max-width:340px;text-decoration:none">
    <button class="btn btn-back">&#8592; Inapoi</button>
  </a>

  <script>
    let selectedFile = null;
    let fileSize = 0;

    function handleFileSelect(e){
      selectedFile = e.target.files[0];
      if(!selectedFile){
        document.getElementById('file-name').textContent = '';
        document.getElementById('upload-btn').disabled = true;
        return;
      }
      fileSize = selectedFile.size;
      const sizeKB = (fileSize / 1024).toFixed(1);
      document.getElementById('file-name').textContent = 
        selectedFile.name + ' (' + sizeKB + ' KB)';
      document.getElementById('upload-btn').disabled = false;
      setStatus('Fisier selectat, gata pentru upload', 'info');
    }

    function setStatus(msg, type='info'){
      const st = document.getElementById('status');
      st.textContent = msg;
      st.className = type;
    }

    function uploadFirmware(){
      if(!selectedFile){
        setStatus('Selecteaza un fisier!', 'error');
        return;
      }

      if(!selectedFile.name.endsWith('.bin')){
        setStatus('Doar fisiere .bin sunt acceptate!', 'error');
        return;
      }

      if(fileSize < 100000 || fileSize > 1500000){
        setStatus('Fisierul trebuie sa fie intre 100KB si 1.5MB!', 'error');
        return;
      }

      setStatus('Validare header...', 'info');
      
      // Citeste primii 4 bytes pentru header ESP32
      const reader = new FileReader();
      reader.onload = function(e){
        const arr = new Uint8Array(e.target.result, 0, 4);
        
        // ESP32 magic byte: 0xE9
        if(arr[0] !== 0xE9){
          setStatus('Header invalid! Nu e un firmware ESP32.', 'error');
          return;
        }

        setStatus('Header OK. Incepe upload...', 'info');
        sendFirmware();
      };
      reader.readAsArrayBuffer(selectedFile.slice(0, 4));
    }

    function sendFirmware(){
      const form = new FormData();
      form.append('file', selectedFile);

      const xhr = new XMLHttpRequest();
      xhr.upload.addEventListener('progress', function(e){
        if(e.lengthComputable){
          const pct = Math.round((e.loaded / e.total) * 100);
          document.getElementById('progress').classList.add('show');
          document.getElementById('progress-bar').style.width = pct + '%';
          setStatus('Uploading: ' + pct + '%', 'info');
        }
      });

      xhr.addEventListener('load', function(){
        if(xhr.status === 200){
          setStatus('Upload complet! Dispozitivul se restarteaza...', 'success');
          document.getElementById('progress-bar').style.width = '100%';
          setTimeout(() => {
            setStatus('Reconectare in progres...', 'info');
            setTimeout(() => window.location.href = '/', 3000);
          }, 1000);
        }else{
          setStatus('Eroare upload: ' + xhr.status, 'error');
        }
      });

      xhr.addEventListener('error', function(){
        setStatus('Eroare conexiune!', 'error');
      });

      xhr.open('POST', '/update');
      xhr.send(form);
    }

    setStatus('Gata. Selecteaza un fisier .bin', 'info');
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
    body{font-family:sans-serif;background:#1a1a2e;color:#eee;
         display:flex;flex-direction:column;align-items:center;
         padding:20px;gap:16px;min-height:100vh}
    h2{letter-spacing:2px;font-size:18px}
    .card{background:#16213e;border-radius:14px;padding:16px 20px;
          width:100%;max-width:340px}
    .btn-spin{width:100%;border:none;border-radius:10px;cursor:pointer;
              font-size:18px;font-weight:bold;padding:14px;color:#eee;
              background:#0f3460;letter-spacing:1px}
    .btn-spin.TOPSPIN  {background:#1a6b3a;color:#6fffaa}
    .btn-spin.BACKSPIN {background:#6b1a1a;color:#ffaaaa}
    .btn-spin.NOSPIN   {background:#0f3460;color:#aaa}
    table{width:100%;border-collapse:collapse;font-size:13px}
    th{color:#aaa;font-weight:normal;padding:6px 4px;text-align:center;border-bottom:1px solid #0f3460}
    td{padding:6px 4px;text-align:center;color:#e94560;font-weight:bold}
    td.idx{color:#555;font-size:11px}
    .btn-back{width:100%;border:none;border-radius:10px;background:#0f3460;
              color:#aaa;font-size:16px;padding:13px;cursor:pointer;font-weight:bold}
    #status{font-size:12px;color:#555}
  </style>
</head>
<body>
  <h2>&#9881; Motor Settings</h2>

  <div class="card">
    <button class="btn-spin" id="btn-spin" onclick="toggleSpin()">...</button>
  </div>

  <div class="card">
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

  <div style="width:100%;max-width:340px">
    <button class="btn-back" onclick="window.location='/'">&#8592; Inapoi</button>
  </div>

  <div id="status">ready</div>

  <script>
    function poll(){
      fetch('/mstatus')
        .then(r=>r.json())
        .then(d=>{
          var btn=document.getElementById('btn-spin');
          btn.textContent=d.spin;
          btn.className='btn-spin '+d.spin;
          var tbody=document.getElementById('speeds-body');
          tbody.innerHTML='';
          for(var i=1;i<=8;i++){
            var tr=document.createElement('tr');
            tr.innerHTML='<td class="idx">'+i+'</td><td>'+d.up[i]+'</td><td>'+d.down[i]+'</td>';
            tbody.appendChild(tr);
          }
        }).catch(()=>{document.getElementById('status').textContent='error';});
    }
    function toggleSpin(){
      fetch('/mute').then(()=>poll()).catch(()=>{});
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
  if      (motor_up.spin == Brush::TOPSPIN)  spin = "TOPSPIN";
  else if (motor_up.spin == Brush::SUPPORT)  spin = "BACKSPIN";
  else if (motor_up.spin == Brush::NOSPIN)   spin = "NOSPIN";
  else                                        spin = "TOPSPIN";

  char buffer[1024] = {0};
  
  // Build JSON with sprintf to avoid String fragmentation (18+ allocations reduced to 1)
  sprintf(buffer, "{\"spin\":\"%s\",\"up\":[" , spin.c_str());
  
  // Append UP array
  for (int i = 0; i < 9; i++) {
    char num[10];
    sprintf(num, "%d", motor_up._SPEEDS[i]);
    strcat(buffer, num);
    if (i < 8) strcat(buffer, ",");
  }
  strcat(buffer, "],\"down\":[" );
  
  // Append DOWN array
  for (int i = 0; i < 9; i++) {
    char num[10];
    sprintf(num, "%d", motor_down._SPEEDS[i]);
    strcat(buffer, num);
    if (i < 8) strcat(buffer, ",");
  }
  strcat(buffer, "]}");
  
  _server.send(200, "application/json", buffer);
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
