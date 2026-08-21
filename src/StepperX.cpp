
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

static const uint8_t FEEDER_TABLE[9] = {0, 8, 7, 6, 5, 4, 3, 2, 1};
static constexpr bool USE_FAS_AUTO_ENABLE_TEST = true;


StepperX::StepperX(uint8_t stepPin, uint8_t dirPin, uint8_t stopPin )
{
  
   _stepPin = stepPin;
   _dirPin  = dirPin;
   _stopPin = stopPin;   

  // Default state for enable pin (active LOW driver): disabled.
   pinMode(_stopPin, OUTPUT);
   digitalWrite(_stopPin, HIGH); // start disabled (active LOW driver)

   engine.init();
  _stepper = engine.stepperConnectToPin(_stepPin);
  if (_stepper) {
    _stepper->setDirectionPin(_dirPin,true);

    if (USE_FAS_AUTO_ENABLE_TEST) {
      _stepper->setEnablePin(_stopPin, true);
      _stepper->setAutoEnable(true);
      _stepper->setDelayToDisable(3000);
    }

    //_stepper->setAcceleration(10000);
    //_stepper->setSpeedInHz(1000);
    
    _stepper->setAcceleration(8000); 
    _stepper->setSpeedInHz(800);
  }
   timeout_const=200;
   directie=1; // -1 = normal, 1 = reversed
   gear_ratio=STEPPER_GEAR_RATIO; // default; overridden from NVS via load_gear_ratio() in setup()

   index=0;
   memcpy(_FEEDER, FEEDER_TABLE, sizeof(FEEDER_TABLE));
   name="stepper";
   // load_timeout_const() intentionally NOT called here — NVS is not initialized yet
   // (global constructor runs before setup()). Call feeder.load_timeout_const() in setup().

  speed=0;
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

    if (!USE_FAS_AUTO_ENABLE_TEST && (digitalRead(_stopPin)==HIGH))
    {
      digitalWrite(_stopPin, LOW); // start the feeder
    }
    this->enable=true;
}

void StepperX::stop(){

  if (_stepper && _stepper->isRunning())
  {
    _stepper->forceStop(); // cancel immediately any move already queued in FastAccelStepper
  }

  if (!USE_FAS_AUTO_ENABLE_TEST && (digitalRead(_stopPin)==LOW))
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

    int16_t _speed = (int16_t)(STEPS_PER_REV * gear_ratio * MICROSTEP);

  if (prog==true)
  { 
    if (index==0) {
      if (enable) stop();
      return;
    }
    if (index>0) {start();}
  }
  else {start();}

  // Wait on the TARGET POSITION, not on isRunning().
  // Rationale: _stepper->move() is asynchronous and there is a brief window
  // right after it returns where isRunning() is still false (the ramp/RMT
  // command has not been picked up yet). A "while(isRunning())" loop can exit
  // in that window, returning while the wheel is still turning. On the next
  // position, move() is then called while the previous move is still active;
  // FastAccelStepper ADDS the relative moves, so the wheel does ~180° at once
  // → the previous position throws no ball and the next one throws two.
  // Waiting until the current position reaches the computed target avoids this.
  int32_t _target = _stepper->getCurrentPosition() + (int32_t)directie * (int32_t)_speed;
  _stepper->moveTo(_target);

  // Safety timeout: 5s max — prevents infinite block if FAS gets stuck
  unsigned long _move_deadline = millis() + 5000UL;
  while (enable && _stepper->getCurrentPosition() != _target && millis() < _move_deadline)
  {
    yield();
  }
  if (enable && _stepper->getCurrentPosition() != _target) {
    _stepper->forceStop();
    Serial.printf("WARNING: move_stepper() timeout — stepper force-stopped (pos=%ld target=%ld)\n",
                  (long)_stepper->getCurrentPosition(), (long)_target);
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
    if (timeout_const < 50 || timeout_const > 400) {
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
    if (timeout_const > 400) {
      Serial.printf("WARNING: Timeout %u too high. Setting to 200.\n", timeout_const);
      timeout_const = 200;
      save_timeout_const();
    }
    if (timeout_const < 50) {
      Serial.printf("WARNING: Timeout %u too low. Setting to 200.\n", timeout_const);
      timeout_const = 200;
      save_timeout_const();
    }
    
    Serial.printf("INFO: Stepper timeout loaded: %u\n", timeout_const);

  }


void StepperX::save_gear_ratio()
  {
    // Validate before saving
    if (gear_ratio < 1.0f || gear_ratio > 5.0f) {
      Serial.printf("WARNING: Invalid gear ratio %.2f. Not saving.\n", gear_ratio);
      gear_ratio = STEPPER_GEAR_RATIO;
      return;
    }

    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for gear save\n");
      return;
    }

    if (!stepper_mem.putFloat("gear", gear_ratio)) {
      Serial.printf("ERROR: Failed to save stepper gear ratio\n");
    } else {
      Serial.printf("INFO: Stepper gear ratio saved: %.2f\n", gear_ratio);
    }

    stepper_mem.end();
  }


  void StepperX::load_gear_ratio()
  {
    if (!stepper_mem.begin(name.c_str(), false)) {
      Serial.printf("ERROR: Failed to open stepper NVS namespace for gear load\n");
      gear_ratio = STEPPER_GEAR_RATIO;
      return;
    }

    gear_ratio = stepper_mem.getFloat("gear", STEPPER_GEAR_RATIO);
    stepper_mem.end();

    // Validate and correct if out of range
    if (gear_ratio < 1.0f || gear_ratio > 5.0f) {
      Serial.printf("WARNING: Gear ratio %.2f out of range. Setting to %.2f.\n", gear_ratio, (float)STEPPER_GEAR_RATIO);
      gear_ratio = STEPPER_GEAR_RATIO;
      save_gear_ratio();
    }

    Serial.printf("INFO: Stepper gear ratio loaded: %.2f\n", gear_ratio);
  }


uint32_t StepperX::getAcceleration()
{
  if (_stepper) return _stepper->getAcceleration();
  return 0;
}

uint32_t StepperX::getSpeedInHz()
{
  if (_stepper) return _stepper->getSpeedInMilliHz() / 1000;
  return 0;
}

void StepperX::setAcceleration(uint32_t accel)
{
  if (accel < 400) accel = 400;
  if (accel > 12000) accel = 12000;
  if (_stepper) _stepper->setAcceleration((int32_t)accel);
}

void StepperX::setSpeedInHz(uint32_t speed_hz)
{
  if (speed_hz < 200) speed_hz = 200;
  if (speed_hz > 1600) speed_hz = 1600;
  if (_stepper) _stepper->setSpeedInHz(speed_hz);
}

void StepperX::save_accel_speed()
{
  if (!stepper_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open stepper NVS namespace for accel/speed save\n");
    return;
  }

  uint32_t accel = getAcceleration();
  uint32_t speed = getSpeedInHz();

  if (!stepper_mem.putUInt("accel", accel) || !stepper_mem.putUInt("speed", speed)) {
    Serial.printf("ERROR: Failed to save stepper accel/speed\n");
  } else {
    Serial.printf("INFO: Stepper accel/speed saved: accel=%u speed=%u\n", accel, speed);
  }

  stepper_mem.end();
}

void StepperX::load_accel_speed()
{
  if (!stepper_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open stepper NVS namespace for accel/speed load\n");
    setAcceleration(8000);
    setSpeedInHz(800);
    return;
  }

  uint32_t accel = stepper_mem.getUInt("accel", 8000);
  uint32_t speed = stepper_mem.getUInt("speed", 800);
  stepper_mem.end();

  setAcceleration(accel);
  setSpeedInHz(speed);

  Serial.printf("INFO: Stepper accel/speed loaded: accel=%u speed=%u\n", getAcceleration(), getSpeedInHz());
}

void StepperX::save_all_settings(uint32_t accel, uint32_t speed_hz, uint16_t timeout, int8_t direction, float gear)
{
  setAcceleration(accel);
  setSpeedInHz(speed_hz);

  if (timeout < 50 || timeout > 400) {
    Serial.printf("WARNING: Invalid timeout %u. Using 200.\n", timeout);
    timeout = 200;
  }
  timeout_const = timeout;

  if (direction != 1 && direction != -1) {
    Serial.printf("WARNING: Invalid direction %d. Using -1.\n", direction);
    direction = -1;
  }
  directie = direction;

  if (gear < 1.0f || gear > 5.0f) {
    Serial.printf("WARNING: Invalid gear ratio %.2f. Using %.2f.\n", gear, (float)STEPPER_GEAR_RATIO);
    gear = STEPPER_GEAR_RATIO;
  }
  gear_ratio = gear;

  if (!stepper_mem.begin(name.c_str(), false)) {
    Serial.printf("ERROR: Failed to open stepper NVS namespace for combined save\n");
    return;
  }

  bool ok = stepper_mem.putUInt("accel", getAcceleration()) &&
            stepper_mem.putUInt("speed", getSpeedInHz()) &&
            stepper_mem.putInt("timeout", timeout_const) &&
            stepper_mem.putInt("directie", directie) &&
            stepper_mem.putFloat("gear", gear_ratio);

  uint32_t t0 = micros();
  stepper_mem.end();
  uint32_t commit_us = micros() - t0;

  if (ok) {
    Serial.printf("INFO: Stepper settings saved: accel=%u speed=%u timeout=%u direction=%d gear=%.2f (NVS commit=%u us)\n",
                  getAcceleration(), getSpeedInHz(), timeout_const, directie, gear_ratio, commit_us);
  } else {
    Serial.printf("ERROR: Failed to save one or more stepper settings\n");
  }
}

