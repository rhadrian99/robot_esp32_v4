#ifndef WebControl_h
#define WebControl_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Update.h>

// ── WiFi Access Point credentials ────────────────────────────
#define WIFI_SSID "TTROBOT"
#define WIFI_PASS "barcelona2935"
// IP implicit AP: 192.168.4.1
// ──────────────────────────────────────────────────────────────

class WebControl
{
public:
  WebControl();

  // Call once from setup() — connects WiFi and starts the HTTP server task
  void begin();

private:
  WebServer _server;
  DNSServer _dnsServer;

  void _connect_wifi();
  void _register_routes();

  // HTTP handlers
  void _handle_root();
  void _handle_up();
  void _handle_down();
  void _handle_left();
  void _handle_right();
  void _handle_power();
  void _handle_mute();
  void _handle_motor1();
  void _handle_motor1_up();
  void _handle_motor1_down();
  void _handle_motor2();
  void _handle_motor2_up();
  void _handle_motor2_down();
  void _handle_feeder();
  void _handle_savepos();
  void _handle_home();
  void _handle_status();
  void _handle_step();
  void _handle_settings();
  void _handle_setlimits();
  void _handle_motorsettings();
  void _handle_mstatus();
  void _handle_t1save();
  void _handle_t2save();
  void _handle_savepoint();
  void _handle_runpoint();
  void _handle_pozinfo();
  void _handle_pozdata();
  void _handle_setspin();
  void _handle_vup();
  void _handle_vdown();
  void _handle_pup();
  void _handle_pdown();
  void _handle_panmin();
  void _handle_panmax();
  void _handle_tiltmin();
  void _handle_tiltmax();
  void _handle_firmware();
  void _handle_update_upload();
  void _handle_update_done();
  void _handle_captive_portal();

  // FreeRTOS task
  static void _task(void *param);

  // Static trampoline helpers (WebServer callbacks must be plain functions)
  static WebControl *_instance;
  static void _s_root()   { _instance->_handle_root(); }
  static void _s_up()     { _instance->_handle_up(); }
  static void _s_down()   { _instance->_handle_down(); }
  static void _s_left()   { _instance->_handle_left(); }
  static void _s_right()  { _instance->_handle_right(); }
  static void _s_power()  { _instance->_handle_power(); }
  static void _s_mute()   { _instance->_handle_mute(); }
  static void _s_motor1()   { _instance->_handle_motor1(); }
  static void _s_motor1_up() { _instance->_handle_motor1_up(); }
  static void _s_motor1_down() { _instance->_handle_motor1_down(); }
  static void _s_motor2()   { _instance->_handle_motor2(); }
  static void _s_motor2_up() { _instance->_handle_motor2_up(); }
  static void _s_motor2_down() { _instance->_handle_motor2_down(); }
  static void _s_feeder()   { _instance->_handle_feeder(); }
  static void _s_savepos()  { _instance->_handle_savepos(); }
  static void _s_home()     { _instance->_handle_home(); }
  static void _s_status()   { _instance->_handle_status(); }
  static void _s_step()     { _instance->_handle_step(); }
  static void _s_settings()      { _instance->_handle_settings(); }
  static void _s_setlimits()     { _instance->_handle_setlimits(); }
  static void _s_motorsettings() { _instance->_handle_motorsettings(); }
  static void _s_mstatus()       { _instance->_handle_mstatus(); }
  static void _s_t1save()        { _instance->_handle_t1save(); }
  static void _s_t2save()        { _instance->_handle_t2save(); }
  static void _s_savepoint()     { _instance->_handle_savepoint(); }
  static void _s_runpoint()      { _instance->_handle_runpoint(); }
  static void _s_pozinfo()       { _instance->_handle_pozinfo(); }
  static void _s_pozdata()       { _instance->_handle_pozdata(); }
  static void _s_setspin()       { _instance->_handle_setspin(); }
  static void _s_vup()           { _instance->_handle_vup(); }
  static void _s_vdown()         { _instance->_handle_vdown(); }
  static void _s_pup()           { _instance->_handle_pup(); }
  static void _s_pdown()         { _instance->_handle_pdown(); }
  static void _s_panmin()        { _instance->_handle_panmin(); }
  static void _s_panmax()        { _instance->_handle_panmax(); }
  static void _s_tiltmin()       { _instance->_handle_tiltmin(); }
  static void _s_tiltmax()       { _instance->_handle_tiltmax(); }
  static void _s_firmware()      { _instance->_handle_firmware(); }
  static void _s_update_upload() { _instance->_handle_update_upload(); }
  static void _s_update_done()   { _instance->_handle_update_done(); }
  static void _s_captive_portal() { _instance->_handle_captive_portal(); }
};

#endif
