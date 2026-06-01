#ifndef infrared_h
#define infrared_h

#include <Preferences.h>

// NVS safety limits
#define MAX_TARGET_POINT_SIZE sizeof(target_point)
#define MAX_NVS_BLOB_SIZE 512

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
  
  if (!point.begin((P.name).c_str(), false)) {
    Serial.printf("ERROR: Failed to open NVS namespace for save '%s'\n", (P.name).c_str());
    return;
  }
  
  size_t size = sizeof(P);
  if (size > MAX_NVS_BLOB_SIZE) {
    Serial.printf("ERROR: target_point too large (%u > %u)\n", size, MAX_NVS_BLOB_SIZE);
    point.end();
    return;
  }
  
  if (!point.putBytes((P.name).c_str(), &P, size)) {
    Serial.printf("ERROR: Failed to write target point to NVS '%s'\n", (P.name).c_str());
  }
  
  point.end();
   
}

// load data from eeprom for each point
target_point target_load_nvm(target_point P)
{
  Preferences point;
  Serial.print("Load: ");Serial.println(P.name);
  
  target_point result = P;  // Start with defaults
  
  if (!point.begin((P.name).c_str(), false)) {
    Serial.printf("ERROR: Failed to open NVS namespace for load '%s'\n", (P.name).c_str());
    return result;
  }

  size_t _length = point.getBytesLength((P.name).c_str());
  
  // Validate size
  if (_length == 0) {
    Serial.printf("WARNING: No data in NVS '%s'. Using defaults.\n", (P.name).c_str());
    point.end();
    return result;
  }
  
  if (_length != MAX_TARGET_POINT_SIZE) {
    Serial.printf("ERROR: Corrupted NVS '%s' (size=%u, expected=%u). Using defaults.\n", 
                  (P.name).c_str(), _length, MAX_TARGET_POINT_SIZE);
    point.end();
    return result;
  }
  
  // Use fixed-size buffer instead of VLA
  uint8_t buffer[MAX_NVS_BLOB_SIZE] = {0};
  if (!point.getBytes((P.name).c_str(), buffer, _length)) {
    Serial.printf("ERROR: Failed to read target point from NVS '%s'\n", (P.name).c_str());
    point.end();
    return result;
  }

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

volatile bool execute=false;





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
    
    case 'P': // program mode
      infrared_program.menu(_var);
      
      break;
       
    case 'M': // program topspin motor
      infrared_brush.menu(_var);
       
      break;  

    default:
      break;

    }

}

void infrared_web_save_point(int poz)
{
  infrared_normal.web_save_point(poz);
}

void infrared_web_run_point(int poz)
{
  infrared_normal.web_run_point(poz);
}

int infrared_get_current_point()
{
  return infrared_normal.get_current_point_waiting();
}

int infrared_get_selected_program()
{
  return points;
}

bool infrared_get_execute_state()
{
  extern volatile bool execute;
  return execute;
}

String infrared_get_pozdata_json(int poz)
{
  if (poz < 1 || poz > 6) return "{\"error\":\"Invalid poz\"}";
  
  target_point pt;
  pt.name = "Point" + String(poz);
  
  // Check if data actually exists in NVS
  Preferences check_prefs;
  bool data_exists = false;
  if (check_prefs.begin((pt.name).c_str(), false)) {
    size_t len = check_prefs.getBytesLength((pt.name).c_str());
    data_exists = (len == MAX_TARGET_POINT_SIZE);
    check_prefs.end();
  }
  
  if (!data_exists) {
    return "{\"exists\":false}";
  }
  
  target_point loaded = target_load_nvm(pt);
  
  String spin_up_str = "UNKNOWN";
  String spin_down_str = "UNKNOWN";
  
  switch (loaded.spin_up)
  {
    case Brush::TOPSPIN: spin_up_str = "TOPSPIN"; break;
    case Brush::BACKSPIN: spin_up_str = "BACKSPIN"; break;
    case Brush::SUPPORT: spin_up_str = "SUPPORT"; break;
    case Brush::NOSPIN: spin_up_str = "NOSPIN"; break;
  }
  
  switch (loaded.spin_down)
  {
    case Brush::TOPSPIN: spin_down_str = "TOPSPIN"; break;
    case Brush::BACKSPIN: spin_down_str = "BACKSPIN"; break;
    case Brush::SUPPORT: spin_down_str = "SUPPORT"; break;
    case Brush::NOSPIN: spin_down_str = "NOSPIN"; break;
  }

  char buf[512];
  sprintf(buf,
    "{\"exists\":true,\"poz\":%d,\"m1_spin\":\"%s\",\"m1_index\":%d,\"m2_spin\":\"%s\",\"m2_index\":%d,\"feeder_index\":%d,\"feeder_speed\":%d}",
    poz,
    spin_up_str.c_str(), loaded.index_up,
    spin_down_str.c_str(), loaded.index_down,
    loaded.stepper_index, loaded.stepper_speed);
  
  return String(buf);
}



#endif