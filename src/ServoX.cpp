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


// if val = 255 use default position (30)
void ServoX::save_pos(uint8_t val)
{
  // memorize to flash the actual position of servo
  // Use val directly to avoid rounding errors from _servo.read() roundtrip:
  // write(angle) -> us -> ticks -> us -> read() can return angle-1 due to integer truncation
  uint8_t _input;
  if (val < 255)
  {
    if (val < 10 || val > max_value)
      _input = 30;
    else
      _input = val;
  }
  else
  {
    _input = 30;
  }
  
  if (!servo_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open servo NVS namespace '%s'\n", name.c_str());
    return;
  }

  if (!servo_mem.putUInt("position", _input)) {
    Serial.printf("ERROR: Failed to save servo position for '%s'\n", name.c_str());
  } else {
    Serial.printf("INFO: Servo '%s' position saved: %u\n", name.c_str(), _input);
    init_value = _input;   // tine cache-ul RAM sincronizat cu NVS
    _pos_cached = true;
  }
  
  servo_mem.end();
}

void ServoX::save_limits()
{
  if (!servo_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open servo NVS namespace '%s' for limits\n", name.c_str());
    return;
  }
  
  if (!servo_mem.putUInt("min", min_value) || !servo_mem.putUInt("max", max_value)) {
    Serial.printf("ERROR: Failed to save servo limits for '%s'\n", name.c_str());
  } else {
    Serial.printf("INFO: Servo '%s' limits saved: min=%u, max=%u\n", name.c_str(), min_value, max_value);
  }
  
  servo_mem.end();
}

void ServoX::load_limits(uint8_t default_min, uint8_t default_max)
{
  if (!servo_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open servo NVS namespace '%s' for limits read\n", name.c_str());
    min_value = default_min;
    max_value = default_max;
    return;
  }
  
  min_value = (uint8_t)servo_mem.getUInt("min", default_min);
  max_value = (uint8_t)servo_mem.getUInt("max", default_max);
  
  // Validate limits are reasonable
  if (min_value >= max_value) {
    Serial.printf("WARNING: Servo '%s' has invalid limits (min=%u >= max=%u). Using defaults.\n", 
                  name.c_str(), min_value, max_value);
    min_value = default_min;
    max_value = default_max;
  }
  
  Serial.printf("INFO: Servo '%s' limits loaded: min=%u, max=%u\n", name.c_str(), min_value, max_value);
  servo_mem.end();
}

void ServoX::load_pos()
{
  // Cache in RAM: citeste NVS o singura data. Citirile din flash dezactiveaza
  // temporar cache-ul CPU si pot lasa goluri in beacon-ul AP -> homing repetat
  // in program (initial_position la fiecare punct/TStar) nu mai atinge flash-ul.
  if (_pos_cached)
  {
    startMove(init_value);
    return;
  }

  if (!servo_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open servo NVS namespace '%s' for position read\n", name.c_str());
    init_value = 30;  // Use safe default
    startMove(init_value);
    return;
  }
  
  // First load and verify max value exists
  uint16_t test = servo_mem.getUInt("max", 0);  // 0 = key not found
  if (test == 0) {
    Serial.printf("WARNING: Servo '%s' max value not found in NVS\n", name.c_str());
  } else {
    Serial.printf("INFO: Servo '%s' max value: %u\n", name.c_str(), test);
  }
  
  // Load position with safe default
  init_value = (uint8_t)servo_mem.getUInt("position", 20);
  
  servo_mem.end();
  
  // Validate position is within limits
  if (init_value < min_value) {
    Serial.printf("WARNING: Servo '%s' position %u below min %u. Using min.\n", 
                  name.c_str(), init_value, min_value);
    init_value = min_value;
  }
  if (init_value > max_value) {
    Serial.printf("WARNING: Servo '%s' position %u above max %u. Using max.\n", 
                  name.c_str(), init_value, max_value);
    init_value = max_value;
  }
  
  Serial.printf("INFO: Servo '%s' loaded position: %u (min=%u, max=%u)\n", 
                name.c_str(), init_value, min_value, max_value);
  
  _pos_cached = true;
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
 //this->_servo.attach(_pin);d
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
  
  // Timeout protection: prevent jam hangs (5 second max movement)
  unsigned long startTime = millis();
  const unsigned long MOVE_TIMEOUT = 5000; // 5000ms = 5 seconds
  
  if (startAngle < endAngle) {
      for (int angle = startAngle; angle <= endAngle; angle++) {
          // Check timeout: if servo is stuck, break out of loop
          if (millis() - startTime > MOVE_TIMEOUT) {
              Serial.printf("ERROR: Servo '%s' TIMEOUT during movement from %d to %d (stuck at %d)\n", 
                           name.c_str(), startAngle, endAngle, angle);
              break;
          }
          _servo.write(angle);
          vTaskDelay(delayMs / portTICK_PERIOD_MS);
      }
  } else if (startAngle > endAngle) {
      for (int angle = startAngle; angle >= endAngle; angle--) {
          // Check timeout: if servo is stuck, break out of loop
          if (millis() - startTime > MOVE_TIMEOUT) {
              Serial.printf("ERROR: Servo '%s' TIMEOUT during movement from %d to %d (stuck at %d)\n", 
                           name.c_str(), startAngle, endAngle, angle);
              break;
          }
          _servo.write(angle);
          vTaskDelay(delayMs / portTICK_PERIOD_MS);
      }
  }
  
  this->moving=0;
  Serial.printf("INFO: Servo '%s' movement completed: %d -> %d (elapsed: %lu ms)\n", 
               name.c_str(), startAngle, endAngle, millis() - startTime);
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
    




