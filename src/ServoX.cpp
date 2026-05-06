#include "ServoX.h"
#include <Arduino.h>
using namespace std;

extern void NL();
extern void DEBUG(String label, int64_t value, bool newline);
extern void DEBUG(int64_t value, bool newline);
extern void DEBUG(String label, bool newline);
extern void tempo_empty(unsigned long reftime);

#define MAX_SERVO 46
#define MIN_SERVO 0
#define MOVE_STEP 12


// if val = 255 the read current postion
void ServoX::save_pos(uint8_t val)
{
  // memorize to flash the actual position of servo 
  uint8_t _input  = (byte) _servo.read();
  if (val<255)
  {
    if (val<10) {_input=30;}
    if (val>max_value) {_input=30;}
  }
  else
  {
    _input=30;
  }
  servo_mem.begin(name.c_str(),false);

  servo_mem.putUInt("position",_input);  
  servo_mem.end();
}

void ServoX::load_pos()
{
  servo_mem.begin(name.c_str(),false);
  uint16_t test=0;
  test=(uint8_t)servo_mem.getUInt("max");  
  Serial.print("Max: ");Serial.println(test,DEC);
  init_value = (uint8_t)servo_mem.getUInt("position",20);  
  
  servo_mem.end();
  
     
  if (init_value<min_value) {init_value=30;}
  if (init_value>max_value) {init_value=30;}
  startMove(init_value);
  

}


int ServoX::read_pos()
{
  return _targetAngle;
}

// read current stepper position

void ServoX::init(int _pin, String _name,int _min, int _max)    
{
 
 name = _name;
 PIN=_pin;
 //pinMode(PIN, OUTPUT);
 //this->_servo.attach(_pin);
 this->_servo.attach(_pin, 544, 2550);
 delayMs=6;
 // initial angle = 0 
 _servo.write(0);
 moving=0;
 _targetAngle=0;
 min_value=_min;
 max_value=_max;
 
 timer_const=MOVE_STEP;

 angleQueue = xQueueCreate(5, sizeof(int)); // Queue size: 5, item size: int

    // Create a persistent task
 xTaskCreatePinnedToCore(taskWrapper, name.c_str(), 2048, this, 1, &myTaskHandle, 1);

 
 return;   
}



 


void ServoX::task() {
  int targetAngle;

  while (true) {
      // Wait for a new target angle from the queue
      if (xQueueReceive(angleQueue, &targetAngle, portMAX_DELAY) == pdTRUE) {
          int currentAngle = _servo.read(); // Read the current angle
          moveServo(currentAngle, targetAngle); // Move to the target angle
      }
  }
}


// Wrapper static pentru a putea folosi metoda ca task FreeRTOS
void ServoX::taskWrapper(void* parameter) {
  ServoX* _this = static_cast<ServoX*>(parameter);
  _this->task();
 
}


// Funcție de ajutor pentru a mișca servomotorul între două unghiuri
void ServoX::moveServo(int startAngle, int endAngle) {
  //used for identify when servo is still moving
  this->moving=1;
  if (startAngle < endAngle) {
      for (int angle = startAngle; angle <= endAngle; angle++) {
          _servo.write(angle);
          vTaskDelay(delayMs / portTICK_PERIOD_MS);
      }
  } else if (startAngle > endAngle) {
      for (int angle = startAngle; angle >= endAngle; angle--) {
          _servo.write(angle);
          vTaskDelay(delayMs / portTICK_PERIOD_MS);
      }
  }
  this->moving=0;
}


void ServoX::startMove(int targetAngle) {
  // Ensure the target angle is within the valid range
  if (targetAngle < min_value) {
      targetAngle = min_value;
  }
  if (targetAngle > max_value) {
      targetAngle = max_value;
  }
  _targetAngle = targetAngle;

  // Send the target angle to the queue
  if (angleQueue != NULL) {
      xQueueSend(angleQueue, &targetAngle, portMAX_DELAY);
  }
}   
    




