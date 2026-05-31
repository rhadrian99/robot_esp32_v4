
#include "StepperX.h"
using namespace std;

extern void tempo_empty(unsigned long reftime);
extern void NL();
extern void DEBUG(String label, int64_t value, bool newline);
extern void DEBUG(int64_t value, bool newline);
extern void DEBUG(String label, bool newline);


// 200 steps per rotation  1.8 degrees per step
// 50 steps/s => complete rotation in 4 s => 1 ball/ 1sec // 4 wings 
// 25 steps/s ==> complete rotation in 8 s => 1 ball / 2sec
// formula  ==>  200/Steps 

// connect and configure the stepper motor to its IO pins

static const uint8_t FEEDER_TABLE[9] = {0, 9, 8, 7, 6, 5, 4, 3, 2};


StepperX::StepperX(uint8_t stepPin, uint8_t dirPin, uint8_t stopPin )
{
  
   _stepPin = stepPin;
   _dirPin  = dirPin;
   _stopPin = stopPin;   

   // Enable pin managed manually via start()/stop() — NOT via FAS setAutoEnable,
   // because setAutoEnable + manual digitalWrite on the same pin causes conflicts.
   pinMode(_stopPin, OUTPUT);
   digitalWrite(_stopPin, HIGH); // start disabled (active LOW driver)

   engine.init();
  _stepper = engine.stepperConnectToPin(_stepPin);
  if (_stepper) {
    _stepper->setDirectionPin(_dirPin,true);

    _stepper->setAcceleration(10000);
    _stepper->setSpeedInHz(1000);
    
  }
   timeout_const=200;
   directie=-1; // -1 = normal, 1 = reversed

   index=0;
   memcpy(_FEEDER, FEEDER_TABLE, sizeof(FEEDER_TABLE));
   name="stepper";
   // load_timeout_const() intentionally NOT called here — NVS is not initialized yet
   // (global constructor runs before setup()). Call feeder.load_timeout_const() in setup().

  stop();
  this->enable=true;
}


void StepperX::init_pins()
{
   // WARNING: DO NOT CALL — pinMode() on step pin breaks FAS RMT/MCPWM GPIO routing
   pinMode(_stepPin, OUTPUT);
   pinMode(_dirPin,  OUTPUT);
   pinMode(_stopPin, OUTPUT);

  
}


void StepperX::start(){
    //_stepper.deactivateBrake();
    
     
    if (digitalRead(_stopPin)==HIGH)
    {
      digitalWrite(_stopPin, LOW); // start the feeder
      
    }
    this->enable=true;
}

void StepperX::stop(){
  
  if (digitalRead(_stopPin)==LOW)
  {
    digitalWrite(_stopPin, HIGH); // stop the feeder
      }
  this->enable=false;
  this->index=0;
  this->speed=0;

}


void StepperX::increase_speed()
{
  this->enable=true;
  this->index++;
  if (this->index > 7)
      this->index = 8;
  
  if (index>0)
  {
    start();
 }
        
}


void StepperX::decrease_speed()
{
  this->enable=true;
  this->index--;
  if (this->index <0)
      {this->index = 0; speed=0;}
  
  if (index==0) 
  {  stop(); speed=0;}
   else
   {  start();}
  
}

/*
steps/sec					
			      timeout	    ball/sec
100		0.5		0.2	        0.7
100		0.5		0.3	        0.8
100		0.5		0.4	        0.9
100		0.5		0.5	          1
100		0.5		0.6	        1.1
100		0.5		0.7	        1.2
100		0.5		0.8	        1.3
100		0.5		0.9	        1.4
100		0.5		  1         1.5
*/

  void StepperX::move_stepper(bool prog)
  {
  // move one feeder step (STEPS_PER_REV * GEAR_RATIO * MICROSTEP steps)
  // then optionally wait timeout based on FEEDER index value

    int16_t _speed = (int16_t)(STEPS_PER_REV * STEPPER_GEAR_RATIO * MICROSTEP);

  if (prog==true)
  { 
    if (index==0) {stop(); return;}
    if (index>0) {start();}
  }
  else {start();}
        
  _stepper->move(directie*_speed);
  
  // Safety timeout: 5s max — prevents infinite block if FAS gets stuck
  unsigned long _move_deadline = millis() + 5000UL;
  while (_stepper->isRunning() && millis() < _move_deadline)
  {
    yield();
  }
  if (_stepper->isRunning()) {
    _stepper->stopMove();
    Serial.printf("WARNING: move_stepper() timeout — stepper force-stopped\n");
  }

   if (prog==true)
   { 
      uint16_t timeout = _FEEDER[index] *timeout_const;  // 160 ---> 280 msec
      tempo_empty(timeout);
   }
  }

  void StepperX::load_direction()
  {
    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for direction\n");
      directie = -1;  // Safe default
      return;
    }
    
    directie = stepper_mem.getInt("directie", -1);  // Default to -1 if not found
    
    // Validate direction
    if (directie != 1 && directie != -1) {
      Serial.printf("WARNING: Invalid stepper direction %d. Using default (-1).\n", directie);
      directie = -1;
    }
    
    Serial.printf("INFO: Stepper direction loaded: %d\n", directie);
    stepper_mem.end();

  }  

void StepperX::save_direction()
  {
    // Validate before saving
    if (directie != 1 && directie != -1) {
      Serial.printf("ERROR: Invalid stepper direction %d. Not saving.\n", directie);
      directie = -1;
      return;
    }

    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for direction save\n");
      return;
    }
    
    if (!stepper_mem.putInt("directie", directie)) {
      Serial.printf("ERROR: Failed to save stepper direction\n");
    } else {
      Serial.printf("INFO: Stepper direction saved: %d\n", directie);
    }
    
    stepper_mem.end();

  }


void StepperX::save_timeout_const()
  {
    // Validate timeout before saving
    if (timeout_const < 160 || timeout_const > 280) {
      Serial.printf("WARNING: Invalid timeout %u. Not saving.\n", timeout_const);
      timeout_const = 200;  // Safe default
      return;
    }
    
    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for timeout save\n");
      return;
    }
    
    if (!stepper_mem.putInt("timeout", timeout_const)) {
      Serial.printf("ERROR: Failed to save stepper timeout\n");
    } else {
      Serial.printf("INFO: Stepper timeout saved: %u\n", timeout_const);
    }
    
    stepper_mem.end();

  }


  void StepperX::load_timeout_const()
  {
    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for timeout load\n");
      timeout_const = 200;  // Safe default
      return;
    }
    
    timeout_const = stepper_mem.getInt("timeout", 200);  // Default to 200 if not found
    stepper_mem.end();

    // Validate and correct if out of range
    if (timeout_const > 280) {
      Serial.printf("WARNING: Timeout %u too high. Setting to 200.\n", timeout_const);
      timeout_const = 200;
      save_timeout_const();
    }
    if (timeout_const < 160) {
      Serial.printf("WARNING: Timeout %u too low. Setting to 200.\n", timeout_const);
      timeout_const = 200;
      save_timeout_const();
    }
    
    Serial.printf("INFO: Stepper timeout loaded: %u\n", timeout_const);

  }  






  
