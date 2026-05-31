
#ifndef StepperX_h
#define StepperX_h

#include <stdlib.h>

#include <Arduino.h>
#include <string.h>
#include <FastAccelStepper.h>
#include <Preferences.h>



// Stepper gear reduction ratio (physical gearbox)
static constexpr float STEPPER_GEAR_RATIO = 2.75f;
// Steps per full revolution (1.8 deg/step motor)
static constexpr uint8_t STEPS_PER_REV = 50;
// Microstepping factor (TMC2208, MS1 jumper off = 1/8)
static constexpr uint8_t MICROSTEP = 8;

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

private:


};

#endif 
