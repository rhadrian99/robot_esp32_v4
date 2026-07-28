
#ifndef StepperX_h
#define StepperX_h

#include <stdlib.h>

#include <Arduino.h>
#include <string.h>
#include <FastAccelStepper.h>
#include <Preferences.h>



// Stepper gear reduction ratio (physical gearbox) — DEFAULT value.
// The active value is the runtime member `gear_ratio`, adjustable 1..5 and
// persisted to NVS (see load_gear_ratio/save_gear_ratio).
//static constexpr float STEPPER_GEAR_RATIO = 2; //21T vs 42T
static constexpr float STEPPER_GEAR_RATIO = 4.36;   
// Steps per full revolution (1.8 deg/step motor)
static constexpr uint8_t STEPS_PER_REV = 50;
// Microstepping factor (TMC2208, MS1 jumper off = 1/8)
static constexpr uint8_t MICROSTEP = 1;

class StepperX
{

public:
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *_stepper = NULL;
Preferences stepper_mem;

uint8_t _stepPin;
uint8_t _dirPin;
uint8_t _stopPin;
uint8_t _FEEDER[9];
volatile bool enable;
int8_t directie;   // -1 or 1
float gear_ratio;  // physical gearbox ratio (1.0 - 5.0), persisted to NVS
String name;

volatile int8_t index; // accessed from IRTask (Core 0) and loop() (Core 1)
uint16_t speed;        // saved/restored with program points
uint16_t timeout_const;

StepperX(uint8_t stepPin, uint8_t dirPin, uint8_t stopPin);

// WARNING: DO NOT CALL init_pins() — pinMode() on step pin breaks FAS RMT/MCPWM GPIO routing
void init_pins();

void start();
void stop();

void increase_speed();
void decrease_speed();

void move_stepper(bool prog);

void load_direction();
void save_direction();
void save_timeout_const();
void load_timeout_const();
void save_gear_ratio();
void load_gear_ratio();

uint32_t getAcceleration();
uint32_t getSpeedInHz();
void setAcceleration(uint32_t accel);
void setSpeedInHz(uint32_t speed_hz);
void save_accel_speed();
void load_accel_speed();
// Saves accel, speed, timeout and direction in a single NVS open/close cycle
// (avoids 3-4 separate flash writes back-to-back, which briefly stalls the CPU
// and can disrupt WiFi timing when triggered directly from an HTTP handler).
void save_all_settings(uint32_t accel, uint32_t speed_hz, uint16_t timeout, int8_t direction, float gear);

private:


};

#endif 

