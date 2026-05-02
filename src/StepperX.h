
#ifndef StepperX_h
#define StepperX_h

#include <stdlib.h>

#include <Arduino.h>
#include <string.h>
#include <FastAccelStepper.h>
#include <Preferences.h>



class StepperX
{

public:
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *_stepper = NULL;
Preferences stepper_mem;

//ESP_FlexyStepper _stepper;

unsigned long timer_stepper;
uint8_t _stepPin;
uint8_t _dirPin;
uint8_t _stopPin;
uint8_t _FEEDER[9];
volatile bool enable;
int16_t directie;
String name;

//ESP_FlexyStepper _stepper;
int8_t index;
uint16_t speed;
uint16_t timeout_const;
uint16_t steps;

StepperX(uint8_t stepPin, uint8_t dirPin, uint8_t stopPin);

void init_pins();

void start();
void stop();


void increase_speed();
void decrease_speed();

void move_stepper(int16_t _speed, bool prog);

void load_direction();
void save_direction();
void save_timeout_const();
void load_timeout_const();

private:


};

#endif 
