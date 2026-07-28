#ifndef common_h
#define common_h
#include <Arduino.h>

// Global serial output switch:
// 1 = enabled, 0 = disabled
#ifndef SERIAL_OUTPUT_ENABLED
#define SERIAL_OUTPUT_ENABLED 1
#endif

#if SERIAL_OUTPUT_ENABLED
#define SERIAL_BEGIN(baud) Serial.begin(baud)
#define SERIAL_FLUSH() Serial.flush()
#else
#define SERIAL_BEGIN(baud) do { } while (0)
#define SERIAL_FLUSH() do { } while (0)
#endif

// #define HUAYU
#define SAMSUNG

#define MINIREMOTE3

// mini remote
#ifdef MINIREMOTE

#define T0 3860463360
#define T1 3125149440
#define T2 3108437760
#define T3 3091726080
#define T4 3141861120
#define T5 3208707840
#define T6 3158572800
#define T7 4161273600
#define T8 3927310080
#define T9 4127850240

#define Tdiez 4061003520
#define TStar 3910598400

#define TOK 3810328320

#define TLEFT 4144561920
#define TRIGHT 2774204160

#define TUP 3877175040
#define TDOWN 2907897600
#endif

#ifdef SAMSUNG

#define hT0 3994093319
#define hT1 4211345159
#define hT2 4194633479
#define hT3 4177921799
#define hT4 4144498439
#define hT5 4127786759
#define hT6 4111075079 // mode up
#define hT7 4077651719
#define hT8 4060940039
#define hT9 4044228359

#define hTdiez 3542877959 //  -/-- button
#define hTStar 3960669959 // pre-ch button

#define hTOK 3927246599 // original 2540177159

#define hTLEFT 2590312199
#define hTRIGHT 2640447239

#define hTUP 2673870599
#define hTDOWN 2657158919

#define hM1up 4161210119
#define hM1down 4094363399

#define hM2up 3977381639
#define hM2down 4010804999
#define hMute 4027516679
#define hPower 4244768519
#define hReset 2473330439
#define hTools 3024815879
#define hInfo 2807564039
#define hGuide 3760129799

#define hB 3943958279
#define hC 3927246599 // original 3927246599 
#define hD 3910534919
#define pause 3041527559
#define play 3091662599

#endif

#define ESPDUINO
// #define ESP32

#ifdef ESPDUINO

////////////////////// definitie pini
// infrared
#define RECV_PIN 12 // IR input on GPIO12 (strap pin): if boot issues appear, move back to GPIO23

// ─── Selectie model robot ─────────────────────────────────────────────
// Lasa definit ROBOT_MODEL_OLD pentru robotul VECHI (MOT_UP=13, servo 10..110)
// Comenteaza linia pentru robotul NOU              (MOT_UP=16, servo 5..50)
//#define ROBOT_MODEL_OLD

#ifdef ROBOT_MODEL_OLD
  #define MOT_UP 13     // model VECHI: motor 1 pe GPIO13
  #define SERVO_MIN 10  // limite servo model VECHI
  #define SERVO_MAX 110
#else
  #define MOT_UP 16     // model NOU: motor 1 pe GPIO16
  #define SERVO_MIN 5   // limite servo model NOU
  #define SERVO_MAX 50
#endif
// ──────────────────────────────────────────────────────────────────────

// versiunea 2025
//  brushless
#define MOT_DOWN 27 // motor 2 //

// servo motors
#define PAN 18      // servo 1 //26 esp32
#define TILT 19     // servo 2 //12 esp32

// stepper
#define STEP_PIN 4  // 19 //esp32
#define DIR_PIN 5   // 18 //esp32
#define STOP_PIN 14 // 22 //esp32

// display
#define clk_Pin 26  // 32  //esp32
#define cs_Pin 25   // 33  //esp32
#define data_Pin 17 // 25  //esp32

// Firmware version
#define FW_VERSION "6.7" // 6.5 selectie programe, setare pozitii

#endif

#endif
