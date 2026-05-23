#ifndef infrared_template_servo_h
#define infrared_template_servo_h

#include "infrared_template_empty.h"

#define SERVO_SETUP_SPEED 2 


class infrared_template_servo: public infrared_template_empty
{
    void _T1() override // save init position
    {
      uint8_t val= pan.read_pos();
      pan.save_pos(val);
      val = tilt.read_pos();
      tilt.save_pos(val);

      display.displayImage_async(IMAGES[12],1); // ok save
    }
    
  void _T2() override  // save pan minim
    {
      pan.startMove(pan.min_value);
      display.displayImage_async(IMAGES[9],2.0);
      // NO show_display_status() - avoid conflict with async display
      
    }
  void _T3() override // save pan maxim
    {
      pan.startMove(pan.max_value);
      display.displayImage_async(IMAGES[11],2.0);
      // NO show_display_status() - avoid conflict with async display
    }

void _T5() override
    {
      tilt.startMove(tilt.min_value);
      display.displayImage_async(IMAGES[8],2.0);
      // NO show_display_status() - avoid conflict with async display
    }
  void _T6() override // save tilt max
    {
      tilt.startMove(tilt.max_value);
      display.displayImage_async(IMAGES[10],2.0);
      // NO show_display_status() - avoid conflict with async display
    }
  void _TOK() override
  {
      switch_program(mode);
      tempo_empty(5);
      display.show_char(mode,0.5); // arata ca s-au memorat valorile
      show_display_status();
  }
  void virtual _TLEFT()
  {
      pan.startMove(pan.read_pos()+SERVO_SETUP_SPEED);
      display.displayImage_async(IMAGES[11],1.2);  // Optimized timeout
      // NO show_display_status() here - avoid conflict with async display
  }

  void virtual _TRIGHT()
  {
      pan.startMove(pan.read_pos()-SERVO_SETUP_SPEED);
      display.displayImage_async(IMAGES[9],1.2);  // Optimized timeout
      // NO show_display_status() here - avoid conflict with async display
  }

  void virtual _TDOWN()
  {
    tilt.startMove(tilt.read_pos()-SERVO_SETUP_SPEED);
    display.displayImage_async(IMAGES[8],1.2);  // Optimized timeout
    // NO show_display_status() here - avoid conflict with async display
  }

  void virtual _TUP()
  {
    tilt.startMove(tilt.read_pos()+SERVO_SETUP_SPEED);
    display.displayImage_async(IMAGES[10],1.2);  // Optimized timeout
    // NO show_display_status() here - avoid conflict with async display
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

  void virtual _TB()
  {
    // active only in mode "S"
    //pan.move(0,true,true);
    //tilt.move(0,true,true);
    //display.displayImage(IMAGES[10],0.5);
    //show_display_status();
    
    if (feeder.directie==-1) {feeder.directie=1; display.show_char('1', 0.5);}
    else {feeder.directie=-1;display.show_char('2', 0.5);}
    feeder.save_direction();
    
    for (int i=0; i<=1000;i++)
    {
      if ((i % 10) ==0)
      {
        motor_up.speed=i;
        motor_down.speed=i;
        for (int j=1;j<=3;j++)
        {
          motor_up.set_speed();
          motor_down.set_speed();
          tempo_empty(10);
        }
      }
    }
    
        
  }
  
  void virtual _TC()
  {
    
    
    //pan.startMove(0);
    //tilt.startMove(0);
    //tilt.move(0,true,true);
    //display.displayImage(IMAGES[10],0.5);
    //show_display_status();
    
  
   
  }

  void virtual _TD()
  {
    
    //pan.startMove(23);
    //tilt.startMove(23);
    //return;

    //if (pan.read_pos()<pan.max_value )  pan.startMove(pan.max_value ); else pan.startMove(pan.min_value );
    
    //if (tilt.read_pos()<tilt.max_value )  tilt.startMove(tilt.max_value) ; else tilt.startMove(tilt.min_value );
    
    // scramble servo removed from robot_tenis_v7.cpp
    //scramble.startMove(0);
    //scramble.startMove(180);
    //scramble.startMove(0);
    //scramble.startMove(180);
    //scramble.startMove(0);

    //pan.move(60,true,true);
    //tilt.move(60,true,true);
    //display.displayImage(IMAGES[10],0.5);
    //show_display_status();
  }

};

#endif