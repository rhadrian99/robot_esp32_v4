#ifndef infrared_template_empty_h
#define infrared_template_empty_h


#include <stdlib.h>
#include <Arduino.h>
#include <string.h>

#include <StepperX.h>
#include <Brush.h>
#include <ServoX.h>
#include <LEDdisplay.h>

extern void NL();
extern void DEBUG(String label, int64_t value, bool newline);
extern void DEBUG(int64_t value, bool newline);
extern void DEBUG(String label, bool newline);
extern void tempo_empty(unsigned long reftime);
extern char mode;

extern LEDdisplay display;
extern ServoX pan;
extern ServoX tilt;
extern ServoX scramble;
extern Brush motor_up;
extern Brush motor_down;
extern StepperX feeder;





class infrared_template_empty
{

public:
  infrared_template_empty();

void virtual stop_all();



void virtual switch_program(char _mode);

void virtual menu(uint32_t _var);
  

  ///////////////////////////////////////////////////////
  void virtual _TTools();
  void virtual _TGuide();

  void virtual _TMute();
  void virtual _T1();
  void virtual _T2();
  void virtual _T3();
  void virtual _TOK();
  void virtual _T4();
  void virtual _T7();
  void virtual _T5();
  void virtual _T8();
  void virtual _T6();
  void virtual _T9();
  void virtual _T0();
  void virtual _Tstar();
  void virtual _Tdiez();
  void virtual _TLEFT();
  void virtual _TRIGHT();
  void virtual _TDOWN();
  void virtual _TUP();
  void virtual _TReset();

  void virtual _VUP();
  void virtual _VDOWN();
  
  void virtual _PUP();
  void virtual _PDOWN();
  void virtual _TPower();
  
  void virtual _TB();
  void virtual _TC();
  void virtual _TD();
  
  
  void virtual show_display_status();
    
  void virtual stop_motors();
  void virtual stop_feeder();
  
  // load initial position of servos and execute
  void virtual initial_position();
  
  void virtual toggle_spin();
  

private:
};


#endif