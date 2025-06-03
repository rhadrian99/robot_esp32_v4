
#include "Brush.h"
#include "utils.h"
using namespace std;


uint16_t _TOPSPIN[]       =  {MOTOR_MIN,  1240, 1300, 1360, 1420, 1480, 1560, 1580, 1640};
uint16_t _BACKSPIN[]      =  {MOTOR_MIN,  1240, 1300, 1360, 1400, 1460, 1520, 1580, 1640};  // rapid


uint16_t _SUPPORT_UP[]    =  {MOTOR_MIN,  1180, 1188, 1196, 1204, 1212, 1220, 1228, 1236};  // support throw
uint16_t _SUPPORT_DOWN[]  =  {MOTOR_MIN,  1180, 1188, 1196, 1204, 1212, 1220, 1228, 1236};  // support throw

uint16_t _NOSPIN[]        =  {MOTOR_MIN,  1240, 1300, 1360, 1400, 1460, 1520, 1580, 1640};  // rapid


// 
Brush::Brush()
{
      
}

// initial_value == first value from series
void Brush::update_speeds(uint16_t _VAL[9],int initial_value, String spintype,int _step)
{
  _VAL[0]=MOTOR_MIN;
  
  if (initial_value<=MOTOR_MIN) initial_value=_VAL[1]; /// preserve old settings
  _VAL[0]=MOTOR_MIN;
  _VAL[1]=initial_value;
  for(int i=2;i<=8;i++)
  { 
    _VAL[i]=2*_step+_VAL[i-1];
  }

}

void Brush::update_speeds_nospin(uint16_t _VAL[9],int initial_value, String spintype,int _step)
{
  _VAL[0]=MOTOR_MIN;
  
  if (initial_value<=MOTOR_MIN) initial_value=_VAL[1]; /// preserve old settings
  _VAL[0]=MOTOR_MIN;
  _VAL[1]=initial_value;
  for(int i=2;i<=8;i++)
  { 
    _VAL[i]=2*_step+_VAL[i-1];
  }

}



void Brush::init(uint8_t _pin,_spinType _spin, String name)    
{
//pinMode(_pin, OUTPUT);  
update_speeds(_TOPSPIN,1240,"TOPSPIN",MOTOR_STEP);
update_speeds(_BACKSPIN,1240,"BACKSPIN",MOTOR_STEP);

update_speeds(_SUPPORT_UP,1180,"SUPORT_UP",SUPPORT_STEP);
update_speeds(_SUPPORT_DOWN,1180,"SUPORT_DOWN",SUPPORT_STEP);



motor.attach(_pin,1000,2000);
//motor.attach(_pin);

motor.setTimerWidth(10); /// mandatory to test for each ESC driver otherwise brushless motors do not move
//motor.setTimerWidth(12); /// mandatory to test for each ESC driver otherwise brushless motors do not move
motor.setPeriodHertz(50); ///  mandatory
//motor.setPeriodHertz(50); ///  mandatory

 index=0;
 spin = _spin;
 speed=MOTOR_MIN;
 motor_on=false;
 
 motor_position=name;
 set_spin(_spin);
 set_speed();
 return;   
}

/// if force==true to save initial speeds in flash memory
void Brush::check_data(bool force)
{
  
  String dataType="nospin";
  brush_mem.begin(dataType.c_str(),false);
   
  size_t length=brush_mem.getBytesLength(dataType.c_str());
  char buffer1[length];
  size_t _length = brush_mem.getBytes(dataType.c_str(), buffer1, length);
  
  if (_length==0 || force)
  {
  uint8_t _size=sizeof(_NOSPIN);
  brush_mem.putBytes(dataType.c_str(), &_NOSPIN, _size);
  }
  brush_mem.end();
  
  ////////////////////////
  dataType="backspin";
  brush_mem.begin(dataType.c_str(),false);
   
  length=brush_mem.getBytesLength(dataType.c_str());
  char buffer2[length];
  _length = brush_mem.getBytes(dataType.c_str(), buffer2, length);
  
  if (_length==0 || force)
  {
  uint8_t _size=sizeof(_BACKSPIN);
  brush_mem.putBytes(dataType.c_str(), &_BACKSPIN, _size);
  }
  brush_mem.end();
  ///////////////////////////////
  dataType="support_up";
  
  brush_mem.begin(dataType.c_str(),false);

  length=brush_mem.getBytesLength(dataType.c_str());
  char buffer3[length];
  _length = brush_mem.getBytes(dataType.c_str(), buffer3, length);

  //_length = brush_mem.getBytes(dataType.c_str(), NULL, NULL);
  
  if (_length==0 || force)
  {
  uint8_t _size=sizeof(_SUPPORT_UP);
  brush_mem.putBytes(dataType.c_str(), &_SUPPORT_UP, _size);
  }
  brush_mem.end();
  ////////////////////////////

  dataType="support_down";
  
  brush_mem.begin(dataType.c_str(),false);
  length=brush_mem.getBytesLength(dataType.c_str());
  char buffer4[length];
  _length = brush_mem.getBytes(dataType.c_str(), buffer4, length);
  
  //_length = brush_mem.getBytes(dataType.c_str(), NULL, NULL);
  
  if (_length==0 || force)
  {
  uint8_t _size=sizeof(_SUPPORT_DOWN);
  brush_mem.putBytes(dataType.c_str(), &_SUPPORT_DOWN, _size);
  }
  brush_mem.end();
  //////////////////////////////////////////////
  dataType="topspin";
  
  brush_mem.begin(dataType.c_str(),false);

  length=brush_mem.getBytesLength(dataType.c_str());
  char buffer5[length];
  _length = brush_mem.getBytes(dataType.c_str(), buffer5, length);

  //_length = brush_mem.getBytes(dataType.c_str(), NULL, NULL);
  
  if (_length==0 || force)
  {
  uint8_t _size=sizeof(_TOPSPIN);
  brush_mem.putBytes(dataType.c_str(), &_TOPSPIN, _size);
  }
  brush_mem.end();

}


void Brush::save_data_as()
{
  String dataType="support";
  if (spin == _spinType::TOPSPIN)
  {
    dataType="topspin";
  }
  if (spin == _spinType::BACKSPIN)
  {
    dataType="backspin";
  }

  if ((spin == _spinType::SUPPORT) && (motor_position=="MOTOR UP"))
  {
    dataType="support_up";
  }

  if ((spin == _spinType::SUPPORT) && (motor_position=="MOTOR DOWN"))
  {
    dataType="support_down";
  }
  
  if ((spin == _spinType::NOSPIN))
  {
    dataType="nospin";
  }


  brush_mem.begin(dataType.c_str(),false);
  uint8_t _size=sizeof(_SPEEDS);
  brush_mem.putBytes(dataType.c_str(), &_SPEEDS, _size);  
  
  brush_mem.end();

  reporting();
}


void Brush::load_data_as()
{
  String dataType="support";
  if (spin == _spinType::TOPSPIN)
  {
    dataType="topspin";
    spintype="TOPSPIN";
  }
  if (spin == _spinType::BACKSPIN)
  {
    dataType="backspin";
    spintype="BACKSPIN";
  }
   

  if ((spin == _spinType::SUPPORT) && (motor_position=="MOTOR UP"))
  {
    dataType="support_up";
    spintype="SUPPORT";
  }

  if ((spin == _spinType::SUPPORT) && (motor_position=="MOTOR DOWN"))
  {
    dataType="support_down";
    spintype="SUPPORT";
  }
  
  if ((spin == _spinType::NOSPIN) )
  {
    dataType="nospin";
    spintype="NOSPIN";
  }

  brush_mem.begin(dataType.c_str(),false);

  size_t length=brush_mem.getBytesLength(dataType.c_str());
  char buffer3[length];
  size_t _length = brush_mem.getBytes(dataType.c_str(), buffer3, length);
  
  char buffer[_length]; 
  brush_mem.getBytes(dataType.c_str(), buffer, _length);  
  memcpy(_SPEEDS, buffer, _length);
  brush_mem.end();


  reporting();
  
}


void Brush::set_spin(uint8_t _spin)
{
this->spin = _spin;

if (spin==Brush::TOPSPIN) {spintype="TOPSPIN";}
if (spin==Brush::BACKSPIN) {spintype="BACKSPIN";}
if (spin==Brush::NOSPIN) {spintype="NOSPIN";}
if (spin==Brush::SUPPORT) {spintype="SUPPORT";}

//save_data_as();
    load_data_as();
    index=0;
    speed= _SPEEDS[index];
    set_speed();
 }


void Brush::set_spin_after_load(uint8_t _spin)
{
this->spin = _spin;

if (spin==Brush::TOPSPIN) {spintype="TOPSPIN";}
if (spin==Brush::BACKSPIN) {spintype="BACKSPIN";}
if (spin==Brush::NOSPIN) {spintype="NOSPIN";}
if (spin==Brush::SUPPORT) {spintype="SUPPORT";}

    load_data_as();
    set_speed();
 }


// maximum 8 levels of speed
void Brush::increase_speed()
{
  int microstep=MOTOR_STEP;

  if (spin==Brush::SUPPORT) {microstep=SUPPORT_STEP;}
  if (spin==Brush::NOSPIN)  {microstep=SUPPORT_STEP;}
  //index++;
  speed+= microstep;
  if (index==0) {index++;this->speed=_SPEEDS[index]; set_speed();return;} // takes first value from array 

  if (speed==_SPEEDS[index+1])
  {
    index++; /// show active the next led
    if (index>8) {
      index=8;
      speed= _SPEEDS[8]; 
    }
  }

  if (speed>_SPEEDS[8])
  {
    index=8;
    speed= _SPEEDS[8]; 
  }
 
 

 set_speed();     
 Serial.print(motor_position + F(" speed = "));
 Serial.println(speed, DEC);
 Serial.print(motor_position+F(" ") +spintype+F(" index = ") );Serial.println(index, DEC);
 Serial.print(F("Motor step= "));Serial.println(microstep, DEC);
}

void Brush:: decrease_speed()
{
  int microstep=MOTOR_STEP;

  if (spin==Brush::SUPPORT) {microstep=SUPPORT_STEP;}
  if (spin==Brush::NOSPIN)  {microstep=SUPPORT_STEP;}
    speed-=microstep;
    
    if (speed<_SPEEDS[1])
      {
        index=0;
        speed= _SPEEDS[0]; 
        set_speed();
        return;

      }
      
    if (speed<_SPEEDS[index])
    {
      index--; /// show active the next led
      
    }

   
   set_speed();
   Serial.print(motor_position + F(" speed = "));
   Serial.println(speed, DEC);
   Serial.print(motor_position+F(" ") +spintype+F(" index = ") );Serial.println(index, DEC);
   Serial.print(F("Motor step= "));Serial.println(microstep, DEC);
}



void Brush:: set_speed()
{
      
    if (speed>MOTOR_MIN) 
    {
            
      motor.writeMicroseconds(speed);
      motor_on=true;
    }
    else
    {
      
      //if (motor_on==true)
      //{
        motor.writeMicroseconds(MOTOR_MIN);
        motor_on=false;
      //}
    }
}

void Brush::set_speed2(uint16_t ispeed)
{
    //if (motor.hertz<50) {motor.setPeriodHertz(50);}
    speed=ispeed;
    set_speed();
}


void Brush:: set_speed(uint8_t _val)
{
    if (_val>8) {_val=8;}
    if (_val<0) {_val=0;}
    index=_val;
    speed= _SPEEDS[index];

    set_speed();
}

void Brush:: stop()
{
    index=0;
    set_speed(index);
}


void Brush:: increase_speed(uint8_t val)
{
  if (index==0) { index=1; this->speed=_SPEEDS[index];}
  
  this->speed+=val;
  if (this->speed>MOTOR_MAX) {this->speed=MOTOR_MAX;}
    
   set_speed2(this->speed);
   DEBUG(motor_position + F(" speed = "),this->speed,true );
   Serial.println();
      
}

void Brush:: decrease_speed(uint8_t val)
{
    if (index==0) { index=1; this->speed=_SPEEDS[index];}
    
    this->speed-=val;
    if (this->speed<MOTOR_MIN) {this->speed=MOTOR_MIN;}
   set_speed2(this->speed);
   
   DEBUG(motor_position + F(" speed = "),this->speed,true );
   Serial.println();
      
}


void Brush::reporting()
{
  NL();
  DEBUG(motor_position,true);DEBUG(spintype,false);
  for (uint8_t i=0;i<9;i++)
  {
    DEBUG(_SPEEDS[i],false);
   
  }
  
  
}
