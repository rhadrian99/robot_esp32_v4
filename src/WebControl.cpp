#include "WebControl.h"
#include <common.h>   // hTUP, hTDOWN, hTLEFT, hTRIGHT, hPower, hMute
#include "Brush.h"

extern void infrared_menu(uint32_t _var, char _mode);
extern char mode;
extern Brush motor_up;
extern Brush motor_down;

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
    .slider-wrap{display:flex;flex-direction:column;gap:12px}
    .slider-item label{font-size:12px;color:#aaa;
                       display:flex;justify-content:space-between;margin-bottom:4px}
    .slider-item input[type=range]{width:100%;accent-color:#e94560;height:6px}
    #status{font-size:12px;color:#555}
  </style>
</head>
<body>
  <h2>&#127926; Robot Control</h2>

  <div class="card">
    <div class="row">
      <div id="spin-label">...</div>
      <button class="btn btn-mute" onclick="cmd('mute')">&#128263;</button>
      <button class="btn btn-power" onclick="cmd('power')">&#9211;</button>
    </div>
  </div>

  <div class="card">
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
  </div>

  <div class="card slider-wrap">
    <div class="slider-item">
      <label>Motor UP <span id="m1val">0</span>/8</label>
      <input type="range" min="0" max="8" value="0" id="m1"
             oninput="document.getElementById('m1val').textContent=this.value"
             onchange="setMotor(1,this.value)">
    </div>
    <div class="slider-item">
      <label>Motor DOWN <span id="m2val">0</span>/8</label>
      <input type="range" min="0" max="8" value="0" id="m2"
             oninput="document.getElementById('m2val').textContent=this.value"
             onchange="setMotor(2,this.value)">
    </div>
  </div>

  <div id="status">ready</div>

  <script>
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
    function pollStatus(){
      fetch('/status')
        .then(r=>r.json())
        .then(d=>{
          document.getElementById('spin-label').textContent=d.spin;
          var m1=document.getElementById('m1');
          var m2=document.getElementById('m2');
          if(document.activeElement!==m1){
            m1.value=d.m1;
            document.getElementById('m1val').textContent=d.m1;
          }
          if(document.activeElement!==m2){
            m2.value=d.m2;
            document.getElementById('m2val').textContent=d.m2;
          }
        }).catch(()=>{});
    }
    setInterval(pollStatus,2000);
    pollStatus();
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
  _server.on("/motor1", HTTP_GET, _s_motor1);
  _server.on("/motor2", HTTP_GET, _s_motor2);
  _server.on("/status", HTTP_GET, _s_status);
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

void WebControl::_handle_motor1()
{
  if (_server.hasArg("v"))
  {
    int v = _server.arg("v").toInt();
    if (v < 0) v = 0;
    if (v > 8) v = 8;
    motor_up.set_speed((uint8_t)v);
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
    motor_down.set_speed((uint8_t)v);
  }
  _server.send(200, "text/plain", "M2 OK");
}

void WebControl::_handle_status()
{
  String spin = motor_up.spintype;
  // translate internal spintype to user-friendly label
  if (motor_up.spin == Brush::TOPSPIN)   spin = "TOPSPIN";
  else if (motor_up.spin == Brush::SUPPORT) spin = "BACKSPIN";
  else if (motor_up.spin == Brush::NOSPIN)  spin = "NOSPIN";

  String json = "{\"spin\":\"" + spin +
                "\",\"m1\":" + String(motor_up.index) +
                ",\"m2\":" + String(motor_down.index) + "}";
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
