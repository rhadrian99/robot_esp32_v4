#ifndef infr_motor_h
#define infr_motor_h

#include "infrared_template_empty.h"


class infr_motor: public infrared_template_empty
{
 
  
  void _T1() override /// save position 1
    {
      
      
      if (motor_up.spin==Brush::TOPSPIN)
      {
        
        motor_up.update_speeds(motor_up._SPEEDS,motor_up.speed,"TOPSPIN",MOTOR_STEP);
      }
      
      if (motor_up.spin==Brush::SUPPORT)
      {
        motor_up.update_speeds(motor_up._SPEEDS,motor_up.speed,"SUPPORT",SUPPORT_STEP);
      }

      if (motor_up.spin==Brush::NOSPIN)
      {
        motor_up.update_speeds_nospin(motor_up._SPEEDS,motor_up.speed,"NOSPIN",SUPPORT_STEP);
      }
    
      motor_up.save_data_as();
      motor_up.load_data_as(); // load and report
      
      display.displayImage(IMAGES[12],1); // ok save
      show_display_status();
      
    }

    void _T2() override /// save position
    {
      if (motor_down.spin==Brush::BACKSPIN)
      {
        motor_down.update_speeds(motor_down._SPEEDS,motor_down.speed,"BACKSPIN",MOTOR_STEP);
      }
      
      if (motor_down.spin==Brush::SUPPORT)
      {
        motor_down.update_speeds(motor_down._SPEEDS,motor_down.speed,"SUPPORT",SUPPORT_STEP);
      }
      
      motor_down.save_data_as();
      motor_down.load_data_as();
      
      display.displayImage(IMAGES[12],1); // ok save
      show_display_status();
      
    }

  void _T3() override
  {
    display.show_char('R',0.5);
    motor_up.check_data(false);
    motor_down.check_data(false);
    display.displayImage(IMAGES[12],1); // ok save

  }  
  
  void _T4() override
  {
    
  }  

  void _T5() override
  {
    
  }  
  void _T6() override
  {
    
  }  
 
  void _TOK() override
  {
      execute=false;
      switch_program(mode);
      //Serial.print(F("Mode: "));Serial.println(mode);
      tempo_empty(5);
      display.show_char(mode,0.5); //arata ca s-au memorat valorile
      show_display_status();
            
  }

  void _TPower() override
  {
    execute=false;
    mode='N';
    feeder.index=0;

    motor_down.stop();
    motor_up.stop();
    tempo_empty(500);
    
    initial_position(); // servo to neutral position
        
    display.show_char(mode, 0.5);
    
  }
  
  // motor_up increase speed
  void virtual _VUP()
  {
     if (motor_up.spin==Brush::TOPSPIN) motor_up.increase_speed(MOTOR_STEP_SETUP);
     if (motor_up.spin==Brush::SUPPORT) motor_up.increase_speed(SUPPORT_STEP_SETUP);
     if (motor_up.spin==Brush::NOSPIN) motor_up.increase_speed(SUPPORT_STEP_SETUP);
     display.displayImage(IMAGES[10],0.5);
      
  }

  void virtual _VDOWN()
  {
    
    if (motor_up.spin==Brush::TOPSPIN) motor_up.decrease_speed(MOTOR_STEP_SETUP);
    if (motor_up.spin==Brush::SUPPORT) motor_up.decrease_speed(SUPPORT_STEP_SETUP);
    if (motor_up.spin==Brush::NOSPIN) motor_up.decrease_speed(SUPPORT_STEP_SETUP);
    display.displayImage(IMAGES[8],0.2);

  }
  void virtual _PUP()
  {
      if (motor_down.spin==Brush::BACKSPIN) motor_down.increase_speed(MOTOR_STEP_SETUP);
      if (motor_down.spin==Brush::SUPPORT) motor_down.increase_speed(SUPPORT_STEP_SETUP);
      if (motor_down.spin==Brush::NOSPIN) motor_down.increase_speed(SUPPORT_STEP_SETUP);
      display.displayImage(IMAGES[10],0.2);

  }

  void virtual _PDOWN()
  {
    if (motor_down.spin==Brush::BACKSPIN) motor_down.decrease_speed(MOTOR_STEP_SETUP);
    if (motor_down.spin==Brush::SUPPORT) motor_down.decrease_speed(SUPPORT_STEP_SETUP);
    if (motor_down.spin==Brush::NOSPIN) motor_down.decrease_speed(SUPPORT_STEP_SETUP);
    display.displayImage(IMAGES[8],0.2);

  }



  void _TMute() override //disable key
  {
    stop_motors();
    toggle_spin();
  }

  void _T0() override // stop the motors
  {
    // stop both motors
    motor_down.index=0;
    motor_up.index=0;
    motor_up.speed=motor_up._SPEEDS[motor_up.index];
    motor_down.speed=motor_down._SPEEDS[motor_down.index];
    motor_down.set_speed();
    motor_up.set_speed();

    display.show_char('0',0.5);

    motor_up.reporting();
    motor_down.reporting();
    //feeder.play_music(); // removed: play_music() no longer exists in StepperX

  }
 
};

#endif