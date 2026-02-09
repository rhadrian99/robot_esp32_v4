#ifndef infrared_template_h
#define infrared_template_h

#include "infrared_template_empty.h"
#define SERVO_STEP 6 //  degrees to move steper




class infrared_template: public infrared_template_empty
{
public:
  infrared_template()
  {
  }

    

  void virtual _TMute()
  {
    if (execute) return;
    stop_motors();
    toggle_spin();
  }
   
  void virtual _T1()
  {
    if (execute) return;
    //initial_position();
                

    target_point Point;

    Point._pan_pos=pan.read_pos();
    Point._tilt_pos=tilt.read_pos();
    Point._up=motor_up.speed;
    Point._down=motor_down.speed;
    Point.stepper_speed=feeder.speed;
    Point.stepper_index=feeder.index;
    
    Point.index_up=motor_up.index;
    Point.index_down=motor_down.index;
    Point.spin_up=motor_up.spin;
    Point.spin_down=motor_down.spin;
    //toggle_spin();


    Serial.print("MOTOR UP SPEED:"); Serial.println(Point._up,DEC);
    Serial.print("MOTOR DOWN SPEED:"); Serial.println(Point._down,DEC);
    
    Point.name="Point1";
    memcpy(&Point1,&Point,sizeof(Point));
    target_save_nvm(Point1);
    
    display.displayImage(IMAGES[12],1); // ok save
    show_display_status();

  }

  void virtual _T2()
  {
    if (execute) return;
    target_point Point;
    Point._pan_pos=pan.read_pos();
    Point._tilt_pos=tilt.read_pos();
    Point._up=motor_up.speed;
    Point._down=motor_down.speed;
    Point.stepper_speed=feeder.speed;
    Point.stepper_index=feeder.index;
    
    Point.index_up=motor_up.index;
    Point.index_down=motor_down.index;

    Serial.print("MOTOR UP SPEED:"); Serial.println(Point._up,DEC);
    Serial.print("MOTOR DOWN SPEED:"); Serial.println(Point._down,DEC);
    
    Point.name="Point2";
    memcpy(&Point2,&Point,sizeof(Point));
    target_save_nvm(Point2);

    display.displayImage(IMAGES[12],1); // ok save
    show_display_status();
  }

  void virtual _T3()
  {
    if (execute) return;
    target_point Point;
    Point._pan_pos=pan.read_pos();
    Point._tilt_pos=tilt.read_pos();
    Point._up=motor_up.speed;
    Point._down=motor_down.speed;
    Point.stepper_speed=feeder.speed;
    Point.stepper_index=feeder.index;
    
    Point.index_up=motor_up.index;
    Point.index_down=motor_down.index;

    Serial.print("MOTOR UP SPEED:"); Serial.println(Point._up,DEC);
    Serial.print("MOTOR DOWN SPEED:"); Serial.println(Point._down,DEC);
    
    Point.name="Point3";
    memcpy(&Point3,&Point,sizeof(Point));
    target_save_nvm(Point3);
    
    display.displayImage(IMAGES[12],1); // ok save
    show_display_status();
  }

  void load_target_point(String position)
  {
    target_point T;
    T.name=position;
    target_point POINT=target_load_nvm(T);
     
     motor_up.speed=POINT._up; 
     motor_down.speed=POINT._down; 
     motor_up.index=POINT.index_up; 
     motor_down.index=POINT.index_down; 
     motor_up.spin=POINT.spin_up;
     motor_down.spin=POINT.spin_down;
     
     if (motor_up.spin==Brush::TOPSPIN) {
      motor_down.spin=Brush::SUPPORT; 
      motor_up.set_spin_after_load(Brush::TOPSPIN);
      motor_down.set_spin_after_load(Brush::SUPPORT);
    } 
     else if (motor_up.spin==Brush::SUPPORT) {
      motor_down.spin=Brush::BACKSPIN;
      motor_up.set_spin_after_load(Brush::SUPPORT);
      motor_down.set_spin_after_load(Brush::BACKSPIN);
    } 
     else if (motor_up.spin==Brush::NOSPIN) {
      motor_down.spin=Brush::NOSPIN;
      motor_up.set_spin_after_load(Brush::NOSPIN );
      motor_down.set_spin_after_load(Brush::NOSPIN);
     } 
    
     motor_up.set_speed();
     motor_down.set_speed();

     pan.startMove(POINT._pan_pos);
     tilt.startMove(POINT._tilt_pos);
     tempo_empty(500);
     feeder.stop();
     feeder.speed= POINT.stepper_speed;
     feeder.index= POINT.stepper_index;
     show_display_status();
     feeder.start();
    
  }


  void virtual _T4()
  {
     if (execute) return;
     load_target_point("Point1");
     
  }

  void virtual _T5()
  {
     if (execute) return;
     load_target_point("Point2");
  }

  void virtual _T6()
  {
     if (execute) return;
     load_target_point("Point3");
  }

  

  void virtual _TOK()
  {
      if (feeder.index>0) return;
      if (motor_up.index>0) return;
      if (motor_down.index>0) return;

      if (execute) return;
   // if all above conditions are meet then abort entering in setup mode

      execute=false;
      switch_program(mode);
      tempo_empty(10);
      display.show_char(mode,0.5); 
      show_display_status();
  }
  void virtual _TTools()
  {         
      if (execute) return;
      
      if ((motor_up.index>0)  || (motor_down.index>0)) // allow feeder to feed balls only if one motor is active
      {
        feeder.increase_speed();
        show_display_status();
      }
      else 
      {
        feeder.stop();
        show_display_status();
      }
      
  }
      
  void virtual _TGuide()
  {
      if (execute) return;
      feeder.decrease_speed();
      show_display_status();

  }

  void virtual _VUP()
  {
      if (execute) return;
      motor_up.increase_speed();
      if (motor_up.spin==Brush::NOSPIN) {motor_down.increase_speed();}
      show_display_status();
      
  }

  void virtual _VDOWN()
  {
    if (execute) return;
    motor_up.decrease_speed();
    if (motor_up.spin==Brush::NOSPIN) {motor_down.decrease_speed();}
    show_display_status();

  }
  void virtual _PUP()
  {
      if (execute) return;
      motor_down.increase_speed();
      if (motor_down.spin==Brush::NOSPIN) {motor_up.increase_speed();}
      show_display_status();

  }

  void virtual _PDOWN()
  {
    if (execute) return;
    motor_down.decrease_speed();
    if (motor_down.spin==Brush::NOSPIN) {motor_up.decrease_speed();}
    show_display_status();

  }

  void virtual _T0()
  {
      if (execute) return;

       /*
  0x00003a2a1a0b3a00,  //18     //16
  0x00003a2a3a2b3a00,  //19     //18
  0x0000775152547700,  //20     //20
  0x0000771162447700,  //21     //22
  0x0000474172541700,  //22     //24
  0x0000775132143700,  //23     //26
  0x0000775172547700   //24     //28 
  */
    if (feeder.index>0) return;
    feeder.load_timeout_const();   
    feeder.timeout_const+=20;
    if (feeder.timeout_const>280) feeder.timeout_const=160;

    if (feeder.timeout_const==160) display.displayImage(IMAGES[18],0.4);
    if (feeder.timeout_const==180) display.displayImage(IMAGES[19],0.4);
    if (feeder.timeout_const==200) display.displayImage(IMAGES[20],0.4);
    if (feeder.timeout_const==220) display.displayImage(IMAGES[21],0.4);
    if (feeder.timeout_const==240) display.displayImage(IMAGES[22],0.4);
    if (feeder.timeout_const==260) display.displayImage(IMAGES[23],0.4);
    if (feeder.timeout_const==280) display.displayImage(IMAGES[24],0.4);

    feeder.save_timeout_const();   
      
 
  }

  void _T9() override
    {
      if (execute==true)  {return;}
      points++;
      if (points>4) {points=2;}
      
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

      ///if (points==5) 
      //{
      //  display.show_char('5',0.5);
      //}


    }


  void virtual _Tstar()  // pause feeder
  {
      mode='Q'; // prevent execution until all things are set
      execute=!execute;
            
      if (execute==false)  //stop the programming mode
      {
        mode='N';
        display.displayImage(IMAGES[12],1); // ok save
        feeder.index=0;
        feeder.stop();

        motor_down.stop();
        motor_up.stop();
        motor_down.index=0;
        motor_up.index=0;

        motor_up.set_speed(0);
        motor_down.set_speed(0);

        initial_position(); // servo to neutral position
      }
      else // start the program  execute=true
      {
        
          feeder.index=0;
          feeder.stop();
          initial_position(); // move servos to neutral position
          tempo_empty(800);
          mode='N';
          

      }

      
           
  }

  void _TPower() override
  {
    feeder.index=0;
    feeder.stop();

    execute=false;
    mode='N';
    
    motor_down.index=0;
    motor_up.index=0;

    motor_up.set_speed(0);
    motor_down.set_speed(0);

    tempo_empty(500);
    
    initial_position(); // servo to neutral position
        
    display.show_char(mode, 0.5);

    motor_down.index=0;
    motor_up.index=0;
    motor_up.set_speed(0);
    motor_down.set_speed(0);

  }

  void virtual _Tdiez()
  {
    execute=false;
    mode='N';
    stop_all();
    

    tempo_empty(30);
    initial_position();
    display.show_char(mode, 0.5);
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

  
  void program()
  {
    if (execute==false) return;
    //int time=500; 
    int time=1000; // varianta irinel
    
    Point1.name="Point1";
    target_point POINT1=target_load_nvm(Point1);
    memcpy(&Point1, &POINT1, sizeof(target_point));
        
    Point2.name="Point2";
    target_point POINT2=target_load_nvm(Point2);
    memcpy(&Point2, &POINT2, sizeof(target_point));
    
    Point3.name="Point3";
    target_point POINT3=target_load_nvm(Point3);
    memcpy(&Point3, &POINT3, sizeof(target_point));
    

    if (mode=='N')
    {
        Serial.print("PROGRAM N: " );Serial.print(points,DEC);Serial.println("points");
        if (points==2)
        {
          pos(Point1,time);
          pos(Point3,time);
        }
        

        if (points==3)
        {
          pos(Point1,time);
          pos(Point2,time);
          pos(Point3,time);
          pos(Point2,time);  
        }

        if (points==4)
        {
          pos(Point1,600);
          pos(Point1,time);
          
          pos(Point3,600);
          pos(Point3,time);
          
        }
    }

  }

  
  void pos(target_point P,uint16_t timeout_throw)
  {

    motor_up.speed=P._up;
    motor_down.speed=P._down;

    motor_up.index=P.index_up;
    motor_down.index=P.index_down;
    show_display_status();
    
    if (execute==false) {return;}
    pan.startMove(P._pan_pos);
    tilt.startMove(P._tilt_pos);
    
    if (execute==false) {return;}
    
    tempo_empty(timeout_throw);
    feeder.move_stepper(100,false);

  }

private:

};

#endif