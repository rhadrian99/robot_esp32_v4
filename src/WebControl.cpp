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
    <div style="display:flex;justify-content:flex-end;margin-bottom:6px">
      <button id="btn-step" onclick="toggleStep()" style="border:none;border-radius:8px;background:#0f3460;color:#e94560;font-size:22px;padding:12px 20px;cursor:pointer;font-weight:bold">6&#176;</button>
    </div>
    <div class="grid">
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('up')">&#9650;</button>
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('left')">&#9664;</button>
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('right')">&#9654;</button>
      <div class="empty"></div>
      <button class="dpad" onclick="cmd('down')">&#9660;</button>
      <div class="empty"></div>
    </div>
    <div style="margin-top:10px;font-size:12px;color:#aaa;display:flex;align-items:center;gap:8px">
      <div style="flex:1;display:flex;flex-direction:column;gap:4px">
        <span>PAN: <span id="pan-val" style="color:#e94560">-</span>&#176; <span style="color:#666;font-size:11px">[<span id="pan-min">-</span>..<span id="pan-max">-</span>]</span></span>
        <span>TILT: <span id="tilt-val" style="color:#e94560">-</span>&#176; <span style="color:#666;font-size:11px">[<span id="tilt-min">-</span>..<span id="tilt-max">-</span>]</span></span>
      </div>
      <button onclick="confirmSavePos()" style="border:none;border-radius:8px;background:#0f3460;color:#e94560;font-size:11px;padding:14px 8px;cursor:pointer;white-space:nowrap">&#128190; home</button>
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

  <a href="/settings" style="display:block;width:100%;max-width:320px;text-decoration:none">
    <button style="width:100%;border:none;border-radius:10px;background:#0f3460;color:#aaa;
                   font-size:14px;padding:12px;cursor:pointer;font-weight:bold">
      &#9881; Settings
    </button>
  </a>

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
         padding:16px;gap:14px;min-height:100vh}
    h2{letter-spacing:2px;font-size:18px}
    .card{background:#16213e;border-radius:14px;padding:14px;
          width:100%;max-width:320px}
    .sect{font-size:13px;color:#e94560;font-weight:bold;margin-bottom:10px}
    .field{display:flex;flex-direction:column;gap:6px;margin-bottom:12px}
    .field label{font-size:12px;color:#aaa}
    .field input{background:#0f3460;border:none;border-radius:8px;
                 color:#e94560;font-size:22px;padding:10px 14px;
                 width:100%;text-align:center;font-weight:bold}
    .btn{width:100%;border:none;border-radius:10px;cursor:pointer;
         font-size:16px;color:#eee;padding:13px;margin-top:6px;font-weight:bold}
    .btn-save{background:#e94560}
    .btn-back{background:#0f3460;color:#aaa}
    #status{font-size:12px;color:#555}
  </style>
</head>
<body>
  <h2>&#9881; Settings</h2>

  <div class="card">
    <div class="sect">PAN limits</div>
    <div class="field">
      <label>MIN (grade)</label>
      <input type="number" id="pan_min" min="0" max="90" value="5">
    </div>
    <div class="field">
      <label>MAX (grade)</label>
      <input type="number" id="pan_max" min="0" max="90" value="50">
    </div>
  </div>

  <div class="card">
    <div class="sect">TILT limits</div>
    <div class="field">
      <label>MIN (grade)</label>
      <input type="number" id="tilt_min" min="0" max="90" value="5">
    </div>
    <div class="field">
      <label>MAX (grade)</label>
      <input type="number" id="tilt_max" min="0" max="90" value="50">
    </div>
  </div>

  <div class="card">
    <button class="btn btn-save" onclick="saveLimits()">&#128190; Salveaza</button>
    <button class="btn btn-back" onclick="window.location='/'">&#8592; Inapoi</button>
  </div>

  <div id="status">Se incarca...</div>

  <script>
    fetch('/status')
      .then(r=>r.json())
      .then(d=>{
        if(d.pan_min!==undefined) document.getElementById('pan_min').value=d.pan_min;
        if(d.pan_max!==undefined) document.getElementById('pan_max').value=d.pan_max;
        if(d.tilt_min!==undefined) document.getElementById('tilt_min').value=d.tilt_min;
        if(d.tilt_max!==undefined) document.getElementById('tilt_max').value=d.tilt_max;
        document.getElementById('status').textContent='ready';
      }).catch(()=>{document.getElementById('status').textContent='error loading';});

    function saveLimits(){
      var pm=document.getElementById('pan_min').value;
      var pM=document.getElementById('pan_max').value;
      var tm=document.getElementById('tilt_min').value;
      var tM=document.getElementById('tilt_max').value;
      fetch('/setlimits?pan_min='+pm+'&pan_max='+pM+'&tilt_min='+tm+'&tilt_max='+tM)
        .then(r=>r.text())
        .then(t=>{document.getElementById('status').textContent=t;})
        .catch(()=>{document.getElementById('status').textContent='error';});
    }
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
  Serial.printf("WiFi AP started — SSID: %s  http://%s\n",
                WIFI_SSID, WiFi.softAPIP().toString().c_str());
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
  _server.on("/status",     HTTP_GET, _s_status);
  _server.on("/step",       HTTP_GET, _s_step);
  _server.on("/settings",   HTTP_GET, _s_settings);
  _server.on("/setlimits",  HTTP_GET, _s_setlimits);
  _server.on("/favicon.ico", HTTP_GET, [this](){ _server.send(204, "text/plain", ""); });
  _server.onNotFound([this](){ _server.send(404, "text/plain", ""); });
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
  String json = "{\"step\":" + String(servo_step) + "}";
  _server.send(200, "application/json", json);
}

void WebControl::_handle_settings()
{
  _server.send_P(200, "text/html", SETTINGS_PAGE);
}

void WebControl::_handle_setlimits()
{
  if (_server.hasArg("pan_min"))  pan.min_value  = (uint8_t)constrain(_server.arg("pan_min").toInt(),  0, 90);
  if (_server.hasArg("pan_max"))  pan.max_value  = (uint8_t)constrain(_server.arg("pan_max").toInt(),  0, 90);
  if (_server.hasArg("tilt_min")) tilt.min_value = (uint8_t)constrain(_server.arg("tilt_min").toInt(), 0, 90);
  if (_server.hasArg("tilt_max")) tilt.max_value = (uint8_t)constrain(_server.arg("tilt_max").toInt(), 0, 90);
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

  String json = "{\"spin\":\"" + spin +
                "\",\"m1\":" + String(m1idx) +
                ",\"m2\":" + String(m2idx) +
                ",\"f\":" + String(feeder.index) +
                ",\"pan\":" + String(pan.read_pos()) +
                ",\"pan_min\":" + String(pan.min_value) +
                ",\"pan_max\":" + String(pan.max_value) +
                ",\"tilt\":" + String(tilt.read_pos()) +
                ",\"tilt_min\":" + String(tilt.min_value) +
                ",\"tilt_max\":" + String(tilt.max_value) +
                ",\"step\":" + String(servo_step) + "}";
  _server.send(200, "application/json", json);
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
