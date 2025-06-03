
#include "infrared_template_empty.h"


void update_motors()
{
  motor_up.set_speed();
  motor_down.set_speed(); 
}

void Beep_off()
{
  //executed once at 30 seconds for preventing beep sound

  if ((motor_up.index==0) )
  {
    for (long i=0;i<=4000;i++)
    {
      motor_up.speed=1200; 
      update_motors();
    }
    
    motor_up.speed=MOTOR_MIN; 
    motor_up.motor_on=false;
    update_motors();

    Serial.println("Timer executed on motor_up!");
   }

  if ((motor_down.index==0) )
  {
        
    for (long i=0;i<=4000;i++)
    {
      motor_down.speed=1200;
      update_motors();
    }
    
    motor_down.speed=MOTOR_MIN; 
    motor_down.motor_on=false;
    update_motors();

    Serial.println("Timer executed on motor_down!");
   }

}

void infrared_template_empty::stop_all()
{
   
  feeder.index=0;
  feeder.stop();
  motor_up.stop();
  motor_down.stop();
  motor_down.index=0;
  motor_up.index=0;

  motor_up.set_speed(0);
  motor_down.set_speed(0);

  tempo_empty(800);
  display.clear();
  //tempo_empty(10);
}
 
 infrared_template_empty::infrared_template_empty()
 {

 }
 
void infrared_template_empty::switch_program(char _mode)
{
  //int size = *(&modes + 1) - modes;
   switch (_mode)
    {
    case 'N': 
      mode='S';
      break;
    /*
    case 'P': // 
      mode='S';

      break;  
    */
    case 'S': // adjusting servo position
      mode='M';
      break;
    
    case 'M': // adjusting speed motors UP DOWN
      mode='N';
      break;  

    default: mode='N';
      break;

    }
   

  }
void infrared_template_empty::menu(uint32_t _var)
  {
    switch (_var)
    {

    case T1:        _T1();      break;
    case T2:        _T2();      break;
    case T3:        _T3();      break;
    case TOK:       _TOK();     break;
    case T4:        _T4();      break;
    case T7:        _T7();      break;    
    case T5:        _T5();      break;
    case T8:        _T8();      break;
    case T6:        _T6();      break;
    case T9:        _T9();      break;
    case T0:        _T0();      break;
    case TStar:     _Tstar();   break;
    case Tdiez:     _Tdiez();   break;
    case TUP:       _TUP();     break;
    case TDOWN:     _TDOWN();   break;
    case TRIGHT:    _TRIGHT();   break;
    case TLEFT:     _TLEFT();   break;

    case hT1:        _T1();      break;
    case hT2:        _T2();      break;
    case hT3:        _T3();      break;
    
        
    case hT4:        _T4();      break;
    case hT7:        _T7();      break;    
    
    case hT5:        _T5();      break;
    case hT8:        _T8();      break;
    
    case hT6:        _T6();      break;
    case hT9:        _T9();      break;
    
    case hT0:        _T0();      break;
    
    //case hTOK:       _TOk();     break;
    
    case hTStar:     _Tstar();   break;
    case hTdiez:     _Tdiez();   break;
    case hPower:     _TPower();   break;

    case hTUP:       _TUP();     break;
    case hTDOWN:     _TDOWN();   break;
    
    case hTRIGHT:    _TRIGHT();   break;
    case hTLEFT:     _TLEFT();   break;

    case hM1up:       _VUP();      break;  // + button
    case hM1down:     _VDOWN();      break;  // - button
    
    case hM2up:       _PUP();      break;  // Channel up button
    case hM2down:     _PDOWN();      break;  // channel down buton
    
    case hMute:       _TMute();  break;  // channel down buton
    case hReset:      _TReset();  break;  // reset
    case hTools:      _TTools();  break;  // reset
    case hGuide:       _TGuide();  break;  // reset
    
    case hB:       _TB();  break;  // reset
    case hC:       _TOK();  break;  // reset
    case hD:       _TD();  break;  // reset
  
    default:
      
      break;
    }
    
    }                  // end of switch

  ///////////////////////////////////////////////////////
  void infrared_template_empty:: _TTools(){}
  void infrared_template_empty:: _TGuide(){}
  void infrared_template_empty:: _TMute(){}
  void infrared_template_empty:: _T1(){}
  void infrared_template_empty:: _T2(){}
  void infrared_template_empty:: _T3(){}
  void infrared_template_empty:: _TOK(){}
  void infrared_template_empty:: _T4(){}
  void infrared_template_empty:: _T7(){}
  void infrared_template_empty:: _T5(){}
  void infrared_template_empty:: _T8(){}
  void infrared_template_empty:: _T6(){}
  void infrared_template_empty:: _T9(){}
  void infrared_template_empty:: _T0(){}
  void infrared_template_empty:: _Tstar(){}
  void infrared_template_empty:: _Tdiez(){}
  void infrared_template_empty:: _TLEFT(){}
  void infrared_template_empty:: _TRIGHT(){}
  void infrared_template_empty:: _TDOWN(){}
  void infrared_template_empty:: _TUP(){}
  
  void infrared_template_empty:: _VUP(){}
  void infrared_template_empty:: _VDOWN(){}
  
  void infrared_template_empty:: _PUP(){}
  void infrared_template_empty:: _PDOWN(){}
  void infrared_template_empty:: _TPower(){}
  
  void infrared_template_empty:: _TB(){}
  void infrared_template_empty:: _TC(){}
  void infrared_template_empty:: _TD(){}
  
  void infrared_template_empty:: _TReset()
  {
   

    display.show_char('R',1);
    execute=false;
    stop_feeder();
    stop_motors();
    mode='N';
    initial_position();
    ESP.restart();
  }


  void infrared_template_empty::show_display_status()
  {
    display.status(motor_up.index,motor_down.index,feeder.index);
  }
  // stop feeder for N and P modes
  void infrared_template_empty::stop_feeder()
  {
    feeder.stop();
    feeder.enable=false;
    execute=false;  
  }
  // stop motors
  void infrared_template_empty::stop_motors()
  {
    feeder.index=0;
    feeder.enable=false;
    motor_down.index=0;
    motor_up.index=0;
    motor_up.speed=motor_up._SPEEDS[motor_up.index];
    motor_down.speed=motor_down._SPEEDS[motor_down.index];
    motor_down.set_speed();
    motor_up.set_speed();
  }

  // load initial position of servos and execute
  void infrared_template_empty:: initial_position()
  {
     display.show_char('L',1);
     pan.load_pos();
     tilt.load_pos();
     
  }

  void infrared_template_empty::toggle_spin()
  {
    if (execute) return;
    
    // change spin sequence T->B->N->T

    if (motor_up.spin==Brush::TOPSPIN)
    {
      motor_up.spin=Brush::SUPPORT;
      motor_down.spin=Brush::BACKSPIN;
      display.show_char('B',0.5);
      Serial.println("B");
      motor_up.set_spin(Brush::SUPPORT);
      motor_down.set_spin(Brush::BACKSPIN);

    }
    else if (motor_up.spin==Brush::SUPPORT)
    {
      motor_up.spin=Brush::NOSPIN;
      motor_down.spin=Brush::NOSPIN;

      motor_up.set_spin(Brush::NOSPIN);
      motor_down.set_spin(Brush::NOSPIN);

      display.show_char('N',0.5);
      Serial.println("N");
    }
    else
    {
      motor_up.spin=Brush::TOPSPIN;
      motor_down.spin=Brush::SUPPORT;

      motor_up.set_spin(Brush::TOPSPIN);
      motor_down.set_spin(Brush::SUPPORT);

      display.show_char('T',0.5);
      Serial.println("T");
    }


  }
  



