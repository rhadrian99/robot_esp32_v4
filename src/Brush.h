
#ifndef Brush_h
#define Brush_h

#include <stdlib.h>
#include <EEPROM.h>
#include <ESP32Servo.h>
#include <Arduino.h>
#include <string.h>
#include <Preferences.h>

#define MOTOR_MIN 900
#define MOTOR_MAX  1750

#define SUPPORT_STEP 8
#define MOTOR_STEP 30

#define SUPPORT_STEP_SETUP 4
#define MOTOR_STEP_SETUP 10
#define MOTOR_STEP_NOSPIN 10


class Brush
{

public:
Preferences brush_mem;

uint16_t _SPEEDS[9];

uint16_t speed_step;
uint16_t slow_speed_step;
uint16_t motor_min;
uint16_t motor_max;

Servo motor;

String motor_position;
String spintype;


typedef enum
    {
        TOPSPIN  = 1,
        BACKSPIN = 0,
        SUPPORT  = 2,
        NOSPIN   = 3
    } _spinType;

_spinType _spin;
uint8_t spin;
int8_t index;
uint16_t speed;
bool motor_on;

Brush();

void set_spin_after_load(uint8_t _spin);
void check_data(bool force);
void update_speeds(uint16_t _VAL[9],int initial_value, String spintype,int _step);
void update_speeds_nospin(uint16_t _VAL[9],int initial_value, String spintype,int _step);
void save_data_as();
void load_data_as();

void reporting();

void init(uint8_t _pin,_spinType _spin,String name);   
void increase_speed();
void decrease_speed();
void stop();
void set_speed();
void set_speed(uint8_t val);
void set_speed2(uint16_t ispeed);
void set_spin(uint8_t _spin);
void increase_speed(uint8_t val);
void decrease_speed(uint8_t val);



private:


};

#endif 
