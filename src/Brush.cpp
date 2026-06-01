
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





void Brush::updateSpinType()
{
  // Convert enum spin value to string spintype
  switch(spin) {
    case TOPSPIN:
      spintype = "TOPSPIN";
      break;
    case BACKSPIN:
      spintype = "BACKSPIN";
      break;
    case NOSPIN:
      spintype = "NOSPIN";
      break;
    case SUPPORT:
      spintype = "SUPPORT";
      break;
    default:
      spintype = "UNKNOWN";
  }
}

void Brush::init(uint8_t _pin, _spinType _spin, String name)    
{
  //pinMode(_pin, OUTPUT);  
  update_speeds(_TOPSPIN, 1240, "TOPSPIN", MOTOR_STEP);
  update_speeds(_BACKSPIN, 1240, "BACKSPIN", MOTOR_STEP);
  update_speeds(_SUPPORT_UP, 1180, "SUPORT_UP", SUPPORT_STEP);
  update_speeds(_SUPPORT_DOWN, 1180, "SUPORT_DOWN", SUPPORT_STEP);

  motor.attach(_pin, 1000, 2000);
  motor.setTimerWidth(10);
  motor.setPeriodHertz(50);

  index = 0;
  spin = _spin;
  speed = MOTOR_MIN;
  motor_on = false;
  motor_position = name;
  
  set_spin((uint8_t)_spin);
  set_speed();
}

/// if force==true to save initial speeds in flash memory
void Brush::check_data(bool force)
{
  // Validate and initialize all motor speed profiles in NVS
  const struct {
    const char* key;
    uint16_t* data;
  } profiles[] = {
    {"nospin", _NOSPIN},
    {"backspin", _BACKSPIN},
    {"support_up", _SUPPORT_UP},
    {"support_down", _SUPPORT_DOWN},
    {"topspin", _TOPSPIN}
  };
  
  for (int i = 0; i < 5; i++) {
    const char* dataType = profiles[i].key;
    uint16_t* defaultData = profiles[i].data;
    
    if (!brush_mem.begin(dataType, false)) {
      Serial.printf("ERROR: Failed to open NVS namespace '%s'\n", dataType);
      continue;
    }
    
    size_t length = brush_mem.getBytesLength(dataType);
    
    // Validate length: must be either 0 (empty) or exactly right size
    if (length > 0 && length != MAX_MOTOR_SPEEDS_SIZE) {
      Serial.printf("WARNING: NVS '%s' corrupted (size=%u, expected=%u). Resetting.\n", 
                    dataType, length, MAX_MOTOR_SPEEDS_SIZE);
      brush_mem.remove(dataType);
      length = 0;
    }
    
    if (length == 0 || force) {
      // Initialize with default values
      uint8_t _size = sizeof(_TOPSPIN);  // All profiles are 9x uint16_t
      if (!brush_mem.putBytes(dataType, defaultData, _size)) {
        Serial.printf("ERROR: Failed to write NVS '%s'\n", dataType);
      } else {
        Serial.printf("INFO: Initialized NVS '%s' with defaults\n", dataType);
      }
    }
    
    brush_mem.end();
  }
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

  if (!brush_mem.begin(dataType.c_str(), false)) {
    Serial.printf("ERROR: Failed to open NVS namespace '%s' for write\n", dataType.c_str());
    return;
  }
  
  uint8_t _size = sizeof(_SPEEDS);
  // Safety check: size must be within bounds
  if (_size != MAX_MOTOR_SPEEDS_SIZE) {
    Serial.printf("ERROR: Speed array size mismatch (got %u, expected %u)\n", _size, MAX_MOTOR_SPEEDS_SIZE);
    brush_mem.end();
    return;
  }
  
  if (!brush_mem.putBytes(dataType.c_str(), &_SPEEDS, _size)) {
    Serial.printf("ERROR: Failed to write motor speeds to NVS '%s'\n", dataType.c_str());
  } else {
    Serial.printf("INFO: Motor speeds saved for '%s'\n", dataType.c_str());
  }
  
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

  if (!brush_mem.begin(dataType.c_str(), false)) {
    Serial.printf("ERROR: Failed to open NVS namespace '%s' for read\n", dataType.c_str());
    return;
  }

  size_t length = brush_mem.getBytesLength(dataType.c_str());
  
  // Validate size: must be either 0 (use defaults) or exactly correct size
  if (length > 0 && length != MAX_MOTOR_SPEEDS_SIZE) {
    Serial.printf("ERROR: Corrupted NVS '%s' (size=%u, expected=%u). Using defaults.\n", 
                  dataType.c_str(), length, MAX_MOTOR_SPEEDS_SIZE);
    brush_mem.end();
    return;
  }
  
  if (length == 0) {
    Serial.printf("WARNING: NVS '%s' empty. Using defaults.\n", dataType.c_str());
    brush_mem.end();
    return;
  }
  
  // Use fixed-size buffer instead of VLA
  uint8_t buffer[MAX_MOTOR_SPEEDS_SIZE] = {0};
  size_t _length = brush_mem.getBytes(dataType.c_str(), buffer, length);
  
  if (_length != length) {
    Serial.printf("ERROR: Failed to read full motor speeds (read %u, expected %u)\n", _length, length);
    brush_mem.end();
    return;
  }
  
  memcpy(_SPEEDS, buffer, _length);
  Serial.printf("INFO: Motor speeds loaded from '%s'\n", dataType.c_str());
  
  brush_mem.end();

  reporting();
  
}


void Brush::set_spin(uint8_t _spin)
{
  this->spin = (_spinType)_spin;
  updateSpinType();
  load_data_as();
  index = 0;
  speed = _SPEEDS[index];
  set_speed();
}


void Brush::set_spin_after_load(uint8_t _spin)
{
  this->spin = (_spinType)_spin;
  updateSpinType();
  load_data_as();
  set_speed();
}


// maximum 8 levels of speed
void Brush::increase_speed()
{
  int microstep = MOTOR_STEP;

  if (spin == SUPPORT) { microstep = SUPPORT_STEP; }
  if (spin == NOSPIN)  { microstep = MOTOR_STEP / 2; }
  
  speed += microstep;
  if (index == 0) { 
    index++; 
    this->speed = _SPEEDS[index]; 
    set_speed();
    return;
  }

  // Fixed: bounds check before accessing array
  if (index < 8 && speed >= _SPEEDS[index + 1]) {
    index++;
    if (index > 8) {
      index = 8;
      speed = _SPEEDS[8]; 
    }
  }

  if (speed > _SPEEDS[8]) {
    index = 8;
    speed = _SPEEDS[8]; 
  }

  set_speed();     
  Serial.print(motor_position + F(" speed = "));
  Serial.println(speed, DEC);
  Serial.print(motor_position + F(" ") + spintype + F(" index = "));
  Serial.println(index, DEC);
  Serial.print(F("Motor step= "));
  Serial.println(microstep, DEC);
}

void Brush::decrease_speed()
{
  int microstep = MOTOR_STEP;

  if (spin == SUPPORT) { microstep = SUPPORT_STEP; }
  if (spin == NOSPIN)  { microstep = MOTOR_STEP / 2; }
  
  speed -= microstep;
    
  if (speed < _SPEEDS[1]) {
    index = 0;
    speed = _SPEEDS[0]; 
    set_speed();
    return;
  }
      
  if (speed < _SPEEDS[index]) {
    index--;
  }

  set_speed();
  Serial.print(motor_position + F(" speed = "));
  Serial.println(speed, DEC);
  Serial.print(motor_position + F(" ") + spintype + F(" index = "));
  Serial.println(index, DEC);
  Serial.print(F("Motor step= "));
  Serial.println(microstep, DEC);
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


void Brush::increase_speed(uint8_t val)
{
  if (index == 0) { index = 1; this->speed = _SPEEDS[index]; }
  
  this->speed += val;
  if (this->speed > MOTOR_MAX) { this->speed = MOTOR_MAX; }
  
  // Update index to match current speed value
  for (int i = 1; i <= 8; i++) {
    if (this->speed >= _SPEEDS[i]) {
      index = i;
    } else {
      break;
    }
  }
    
  set_speed2(this->speed);
  DEBUG(motor_position + F(" speed = "), this->speed, true);
  Serial.println();
}

void Brush::decrease_speed(uint8_t val)
{
  if (index == 0) { index = 1; this->speed = _SPEEDS[index]; }
  
  this->speed -= val;
  if (this->speed < MOTOR_MIN) { this->speed = MOTOR_MIN; }
  
  // Update index to match current speed value
  for (int i = 8; i >= 1; i--) {
    if (this->speed <= _SPEEDS[i]) {
      index = i;
    } else {
      break;
    }
  }
  
  set_speed2(this->speed);
  DEBUG(motor_position + F(" speed = "), this->speed, true);
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

float Brush::getVirtualPosition()
{
  // Returns virtual position 0.0-8.0 with decimal interpolation
  if (index == 0) return 0.0f;
  if (index >= 8) return 8.0f;
  
  uint16_t lower = _SPEEDS[index];
  uint16_t upper = _SPEEDS[index + 1];
  
  if (speed <= lower) return (float)index;
  if (speed >= upper) return (float)(index + 1);
  
  float frac = (float)(speed - lower) / (float)(upper - lower);
  return (float)index + frac;
}
