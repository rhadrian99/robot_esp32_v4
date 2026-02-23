#ifndef LEDdisplay_h
#define LEDdisplay_h
#include <Arduino.h>
#include <StepperX.h>
#include <Brush.h>
#include <common.h>


extern StepperX feeder;
extern Brush motor_up;
extern Brush motor_down;
extern bool execute;

#define NBR_MTX 1 //was 2


//#include "devices.h"
#include <LedControlMS.h>

const uint64_t IMAGES[] PROGMEM= {
  0x5554545050404000,  //0
  0x3f21212121212121,  //1
  0x3f212d2121212121,  //2
  0x3f212d212d212121,  //3
  0x3f212d212d212d21,  //4
  0x3f212d2d2d212121,  //5
  0x3f212d2d2d2d2d2d,  //6
  0x00040a1120408000,  //7
  0x081c3e7f1c1c1c1c,  //8 /// sign ok index 12
  0x0010307fff7f3010,  //9
  0x1c1c1c1c7f3e1c08,  //10
  0x00080cfefffe0c08,  //11
  0x00040a1120408000,  //12  /// sign ok index 12
  0x000000001c080c08,  //13  /// digit 1 small    
  0x0000003c0c18301c,  //14   /// digit 2 small
  0x0066666666666600,  //15  // pause   ||
  0x00387c7c7c380000,  //16  dot
  
  0x0000663cff3c6600,  //17     // star
  0x00003a2a1a0b3a00,  //18     // 16
  0x00003a2a3a2b3a00,  //19     //18
  0x0000775152547700,  //20     //20
  0x0000771162447700,  //21     //22
  0x0000474172541700,  //22     //24
  0x0000775132143700,  //23     //26
  0x0000775172547700,   //24     //28 
  0x003c421a3a221c00   //25      // @ sign
 
};

//const int IMAGES_LEN = sizeof(IMAGES)/8;



class LEDdisplay
{


public:

LEDdisplay(int dPin, int clPin, int csPin, int devices);

void timer(unsigned long reftime);
void clear();
void displayImage(uint64_t image,float seconds);
void barX(volatile int16_t val, int16_t x1, int16_t x2);
void status(int16_t _S1, int16_t _S2, int16_t _F1);
void show_char(char ch, float seconds);
void show_char_no_delay(char ch);


}; // end class LED display
#endif