#ifndef infrared_h
#define infrared_h

#include <Preferences.h>

uint8_t points=2;

struct target_point{
  uint8_t index_up;
  uint8_t index_down;
  uint16_t _up;  // speed
  uint16_t _down; // speed
  uint8_t _pan_pos; // position
  uint8_t _tilt_pos; //position
  String name;
  uint8_t stepper_speed; //speed
  uint8_t stepper_index; //index
  uint8_t spin_up; // 0=TOPSPIN, 1=BACKSPIN, 2=NOSPIN
  uint8_t spin_down; // 0=TOPSPIN, 1=BACKSPIN, 2=NOSPIN
  
  //String spintype; /// de refacut contextul in care a fost salvata pozitia 
  //uint8_t spin;
  
  
} target1, target2,target3,Point1,Point2,Point3,Point4,Point5,Point6;


void target_save_nvm(target_point P)
{
  Preferences point;
  Serial.print("Saved: ");Serial.println(P.name);
  point.begin((P.name).c_str(),false);
  point.putBytes((P.name).c_str(), &P, sizeof(P));  
  
  point.end();
   
}

// load data from eeprom for each point
target_point target_load_nvm(target_point P)
{
  Preferences point;
  Serial.print("Load: ");Serial.println(P.name);
  point.begin((P.name).c_str(),false);

  size_t _length = point.getBytesLength((P.name).c_str());
  char buffer[_length]; 
  point.getBytes((P.name).c_str(), buffer, _length);  

  
  target_point result;
  memcpy(&result, buffer, _length);
  
  point.end();

  Serial.println(result.name);
     
  return result;
   
}


#include "infrared_template_empty.h"
#include "infrared_template.h"
#include "infrared_template_servo.h"
#include "infr_program.h"
#include "infr_motor.h"

bool execute=false;





infrared_template infrared_normal;
infrared_template_servo infrared_servo;
infr_program infrared_program;
infr_motor infrared_brush;

void infrared_menu(uint32_t _var, char _mode)
{
    switch (_mode)
    {
    case 'N': // mode normal
      
      infrared_normal.menu(_var);
      
      break;
    
    case 'S': // adjusting servo
      infrared_servo.menu(_var);
      
      break;  
    /*
    case 'P': // program left right
      infrared_program.menu(_var);
      
      break;
   */   
    case 'M': // program topspin motor
      infrared_brush.menu(_var);
       
      break;  



    default:
      break;

    }

}



#endif