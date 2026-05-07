#ifndef WebControl_h
#define WebControl_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

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
  void _handle_motor2();
  void _handle_feeder();
  void _handle_savepos();
  void _handle_status();
  void _handle_step();
  void _handle_settings();
  void _handle_setlimits();

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
  static void _s_motor2()   { _instance->_handle_motor2(); }
  static void _s_feeder()   { _instance->_handle_feeder(); }
  static void _s_savepos()  { _instance->_handle_savepos(); }
  static void _s_status()   { _instance->_handle_status(); }
  static void _s_step()     { _instance->_handle_step(); }
  static void _s_settings() { _instance->_handle_settings(); }
  static void _s_setlimits(){ _instance->_handle_setlimits(); }
};

#endif
