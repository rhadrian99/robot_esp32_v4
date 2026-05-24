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
      
      display.displayImage_async(IMAGES[12],1); // ok save
      // NO show_display_status() - avoid conflict with async display
      
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
      
      display.displayImage_async(IMAGES[12],1); // ok save
      // NO show_display_status() - avoid conflict with async display
      
    }

  void _T3() override
  {
    display.show_char('R',0.5);
    motor_up.check_data(false);
    motor_down.check_data(false);
    display.displayImage_async(IMAGES[12],1); // ok save
    // NO show_display_status() - avoid conflict with async display
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
  
  // ────────────────────────────────────────────────────────────────
  // Helper functions for motor control
  // ────────────────────────────────────────────────────────────────
  private:
  // Returns pointer to main-spin motor (controlled by V+/V-)
  // TOPSPIN → motor_up, BACKSPIN → motor_down, else → motor_up
  Brush* getMainMotor() {
    if(motor_up.spin == Brush::TOPSPIN) return &motor_up;
    if(motor_down.spin == Brush::BACKSPIN) return &motor_down;
    return &motor_up; // default NOSPIN
  }
  
  // Returns step size for main motor
  // TOPSPIN/BACKSPIN use MOTOR_STEP_SETUP, NOSPIN uses SUPPORT_STEP_SETUP
  int getMainStep() {
    if(motor_up.spin == Brush::TOPSPIN || motor_down.spin == Brush::BACKSPIN)
      return MOTOR_STEP_SETUP;
    return SUPPORT_STEP_SETUP;
  }
  
  // Returns pointer to support motor (controlled by P+/P-)
  // Inverse of main motor: if motor_down has SUPPORT → motor_down,
  // if motor_up has SUPPORT → motor_up, else → motor_down
  Brush* getSupportMotor() {
    if(motor_down.spin == Brush::SUPPORT) return &motor_down;
    if(motor_up.spin == Brush::SUPPORT) return &motor_up;
    return &motor_down; // default NOSPIN
  }
  
  public:
  // V+/V- controls the main-spin motor:
  //   TOPSPIN cycle (motor_up=TOPSPIN, motor_down=SUPPORT) → motor_up
  //   BACKSPIN cycle (motor_up=SUPPORT, motor_down=BACKSPIN) → motor_down
  //   NOSPIN cycle → motor_up (default)
  void virtual _VUP() {
    getMainMotor()->increase_speed(getMainStep());
    display.displayImage_async(IMAGES[10], 0.5);
  }

  void virtual _VDOWN() {
    getMainMotor()->decrease_speed(getMainStep());
    display.displayImage_async(IMAGES[8], 0.2);
  }

  // P+/P- controls the support motor (the one not controlled by V+/V-):
  //   TOPSPIN cycle → motor_down (support)
  //   BACKSPIN cycle → motor_up (support)
  //   NOSPIN cycle → motor_down (default)
  void virtual _PUP() {
    getSupportMotor()->increase_speed(SUPPORT_STEP_SETUP);
    display.displayImage_async(IMAGES[10], 0.2);
  }

  void virtual _PDOWN() {
    getSupportMotor()->decrease_speed(SUPPORT_STEP_SETUP);
    display.displayImage_async(IMAGES[8], 0.2);
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
  }
 
};

#endif
