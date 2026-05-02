
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

uint8_t FEEDER[]={0, 9, 8, 7, 6, 5, 4, 3, 2};


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
   directie=-1; //1  for normal
   //load_direction();
      
   
  // if (directie ==-1 ) {_stepper->setDirectionPin(_dirPin,false);}
   //else {_stepper->setDirectionPin(_dirPin,true);}
   
   //_stepper->setDirectionPin(_dirPin,false);

   
   //save_direction();
  
  //_stepper.setAccelerationInStepsPerSecondPerSecond(500);

   index=0;
   memcpy(_FEEDER, FEEDER, sizeof(FEEDER));
   name="directie";
   
  stop();
  this->enable=true;
}


void StepperX::init_pins()
{
   //_stepper.startAsService(1);   
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

  void StepperX::move_stepper(int16_t _speed, bool prog)  //numar de pasi de executat
  {
  // start
  // move 50 steps with current speed
  // stop
  // timeout based on FEEDER index value
  
    _speed=50*2.75*8; //tmc2208 v1 ms1 jumper offa4938

  if (prog==true)
  { 
    if (index==0) {stop(); return;}
    if (index>0) {start();}
  }
  else {start();}
        
  
    
  _stepper->setCurrentPosition(0);
  _stepper->move(directie*_speed);
  

  while (_stepper->isRunning())
  {
    //_stepper->setSpeedInHz(300);
    
    yield();
  }

   if (prog==true)
   { 
      load_timeout_const(); 
      uint16_t timeout = FEEDER[index] *timeout_const;  // 160 ---> 280 msec
      tempo_empty(timeout);
   }
      
   
  }

  void StepperX::load_direction()
  {
    stepper_mem.begin(name.c_str(),false);
    directie = stepper_mem.getInt("directie");  
    if (directie >1) {directie=1;}
    if (directie <=0) {directie=-1;}
    stepper_mem.end();

  }  

void StepperX::save_direction()
  {
    
    if (directie >1) {directie=1;}
    if (directie <=0) {directie=-1;}

    stepper_mem.begin(name.c_str(),false);
    stepper_mem.putInt("directie",directie); 
    
    stepper_mem.end();

  }


void StepperX::save_timeout_const()
  {
    stepper_mem.begin(name.c_str(),false);
    stepper_mem.putInt("timeout",timeout_const); 
    
    stepper_mem.end();

  }


  void StepperX::load_timeout_const()
  {
    stepper_mem.begin(name.c_str(),false);
    timeout_const = stepper_mem.getInt("timeout");  
    stepper_mem.end();

    if (timeout_const>280) {timeout_const=200; save_timeout_const();}
    if (timeout_const<160) {timeout_const=200; save_timeout_const();}

  }  






  
