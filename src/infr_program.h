#ifndef infr_program_h
#define infr_program_h

#include "infrared_template_empty.h"


class infr_program: public infrared_template_empty
{
public: 
  void virtual _Tdiez()
  {
    execute=false;
    mode='N';
    stop_all();
    initial_position();
    
    display.show_char(mode, 1);
  }

  void virtual _T0()
  {
      if (execute) return;
      display.show_char(mode,0.5);      
      show_display_status();
      
  }
  void _T1() override /// save position 1
    {
      if (execute==false) 
      {
        target1._pan_pos=pan.read_pos();
        target1._tilt_pos=tilt.read_pos();
        target1._up=motor_up.speed;
        target1._down=motor_down.speed;
        
        target1.index_up=motor_up.index;
        target1.index_down=motor_down.index;

        Serial.print("MOTOR UP SPEED:"); Serial.println(target1._up,DEC);
        Serial.print("MOTOR DOWN SPEED:"); Serial.println(target1._down,DEC);
        
        target1.name="target1";
        target_save_nvm(target1);

        display.displayImage(IMAGES[12],1); // ok save
      }
      
    }

  void _T2() override // save position 2
    {
      // save pan tilt postions 2 
      if (execute==false) 
      {
        target2._pan_pos=pan.read_pos();
        target2._tilt_pos=tilt.read_pos();
        
        target2._up=motor_up.speed;
        target2._down=motor_down.speed;
        target2.index_up=motor_up.index;
        target2.index_down=motor_down.index;
        
        target2.name="target2";
        target_save_nvm(target2);

        Serial.print("MOTOR UP SPEED:"); Serial.println(target2._up,DEC);
        Serial.print("MOTOR DOWN SPEED:"); Serial.println(target2._down,DEC);
        display.displayImage(IMAGES[12],1); // ok save
      }
      
    }

  void _T3() override 
    {
        // save pan tilt postions 2 
      if (execute==false) 
      {
        target3._pan_pos=pan.read_pos();
        target3._tilt_pos=tilt.read_pos();
        
        target3._up=motor_up.speed;
        target3._down=motor_down.speed;
        target3.name="target3";
        target_save_nvm(target3);
      }
      display.displayImage(IMAGES[12],1); // ok save
     
    }

    void _T9() override
    {
      if (execute==true)  {return;}
      points++;
      if (points==5) points++;  // skip 5, go to 6
      if (points>6) {points=2;}
      
      if (points==2) 
      {
        display.show_char('2',0.5);
      }
      if (points==3) 
      {
        display.show_char('3',0.5);
      }
      if (points==4) 
      {
        display.show_char('4',0.5);
      }
      if (points==6)
      {
        display.show_char('R',0.5);  // Random
      }
    }
  
  void _Tstar() override  // PRE-CH button start/stop selected program
  {
      mode='Q'; // prevent execution until all things are set up
      execute=!execute;
            
      if (execute==false)  //stop the programming mode
      {
        mode='N';
        
        feeder.index=0;
        feeder.stop();

        motor_up.index=0;
        motor_down.index=0;
        motor_down.stop();
        motor_up.stop();
        motor_down.set_speed();
        motor_up.set_speed(); 
                
        tempo_empty(500);

        initial_position(); // servo to neutral position
        display.displayImage(IMAGES[12],0.5); // ok save
      }
      else // start the program  execute=true
      {
          
          feeder.index=0;
          initial_position(); // move servos to neutral position
          tempo_empty(800);
          mode='N';
          
      }
            
  }

  void _TOK() override
  {
      if (execute==false) //if is not active programming mode
      {
        
      }
   
  }

void _TC() override
  {
     Serial.print(F("Mode: "));Serial.println(mode);  
    if (execute==false) //if is not active programming mode
      {
        switch_program(mode);
        tempo_empty(10);
        display.show_char(mode,0.5); // arata ca s-au memorat valorile
        show_display_status();
      }
   
  }


  void _TPower() override
  {
    Serial.println("Power off");
    execute=false;
    mode='N';
    feeder.index=0;
    feeder.stop();

    motor_up.index=0;
    motor_down.index=0;
    motor_down.stop();
    motor_up.stop();
    motor_down.set_speed();
    motor_up.set_speed(); 

    tempo_empty(500);
    
    initial_position(); // servo to neutral position
        
    display.show_char(mode, 0.5);
  }

  void virtual _TTools()
  {         
      if (execute) return;
      feeder.increase_speed();
      show_display_status();

  }
      
  void virtual _TInfo()
  {
      if (execute) return;
      feeder.decrease_speed();
      show_display_status();

  }

void virtual _TLEFT()
  {
      if (execute) return;
      
      pan.startMove(pan.read_pos()+SERVO_STEP);
      display.displayImage(IMAGES[11],0.5);
      show_display_status();
  }

  void virtual _TRIGHT()
  {
      if (execute) return;
      pan.startMove(pan.read_pos()-SERVO_STEP);
      display.displayImage(IMAGES[9],0.5);
      show_display_status();
  }

  void virtual _TDOWN()
  {
    if (execute) return;
    tilt.startMove(tilt.read_pos()-SERVO_STEP);
    display.displayImage(IMAGES[8],0.5);
    show_display_status();
  }

  void virtual _TUP()
  {
    if (execute) return;
    tilt.startMove(tilt.read_pos()+SERVO_STEP);
    display.displayImage(IMAGES[10],0.5);
    show_display_status();
  }

  
  void virtual _VUP()
  {
      if (execute) return;
      motor_up.increase_speed();
      show_display_status();
            
  }

  void virtual _VDOWN()
  {
    if (execute) return;
    motor_up.decrease_speed();
    show_display_status();
    
  }
  void virtual _PUP()
  {
      if (execute) return;
      motor_down.increase_speed();
      show_display_status();

    
  }

  void virtual _PDOWN()
  {
    if (execute) return;
    motor_down.decrease_speed();
    show_display_status();
    
  }
 
void _TMute() override //toggle TOPSPIN / BACKSPIN
  {
    if (execute) return;
    stop_motors();
    toggle_spin();
    
  }

  void program()
  {
    return;
    if (execute==false) return;

  }

  

};

#endif