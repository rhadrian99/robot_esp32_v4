#include "LEDdisplay.h"
//#include <common.h>
extern LedControl lc;

//LedControl lc = LedControl(data_Pin, clk_Pin, cs_Pin, NBR_MTX);   

LEDdisplay::LEDdisplay(int dPin, int clPin, int csPin, int devices)
{
   lc = LedControl(dPin, clPin, csPin, devices);   
}

void LEDdisplay::timer(unsigned long reftime) {
  unsigned long ref = millis();
  while ((millis() - ref) < reftime)
  {
    motor_up.set_speed();
    motor_down.set_speed(); /// modificare recenta
    
  }
}

void LEDdisplay::clear(){
  for (int i = 0; i < NBR_MTX; i++)
  {
    lc.shutdown(i, false); //keep the screen on
    lc.setIntensity(i, 5); // set brightness to medium values
    lc.clearDisplay(i);    //clear the display after each letter
  }
}

void LEDdisplay::shutdown(){
  for (int i = 0; i < NBR_MTX; i++)
  {
    lc.clearDisplay(i);    //clear the display
    lc.shutdown(i, true);  //turn the display OFF
  }
}

// Draw image to LED matrix immediately (non-blocking) - NO clear here!
void LEDdisplay::display_now(uint64_t image) {
  // DON'T call clear() here - it would erase the display!
  // Just draw directly on top of whatever was there before.
  // Old display content (if any) will be overwritten.
  
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}

// Async display: set duration and return immediately (non-blocking)
void LEDdisplay::displayImage_async(uint64_t image, float seconds) {
  _pending_display.image = image;
  _pending_display.start_time = millis();
  _pending_display.duration_ms = (unsigned long)(1000.0 * seconds);
  _pending_display.active = true;
  
  // Display immediately
  display_now(image);
  
  Serial.printf("INFO: Display async started for %.1f seconds (%lu ms)\n", seconds, _pending_display.duration_ms);
}

// Call from main loop to check if display duration expired
void LEDdisplay::update() {
  if (!_pending_display.active) {
    return;  // No display active, nothing to do
  }
  
  unsigned long elapsed = millis() - _pending_display.start_time;
  
  if (elapsed >= _pending_display.duration_ms) {
    // Duration expired, restore bargraph
    Serial.printf("INFO: Display async expired (%lu ms elapsed >= %lu ms duration). Restoring bargraph.\n", 
                  elapsed, _pending_display.duration_ms);
    _pending_display.active = false;
    clear();
    status(motor_up.index, motor_down.index, feeder.index);
  }
}


void LEDdisplay::displayImage(uint64_t image,float seconds) {
  clear();
  unsigned long value = 1000 * seconds;
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
  
  timer(value);
  clear();
}


void LEDdisplay::barX(volatile int16_t val, int16_t x1, int16_t x2) {
  if (val == -1)
  {
    lc.setRow(0, x1, B00000000);
    lc.setRow(0, x2, B00000000);
    return;
  }

  if (val > 7)
  {
    val = 7;
    lc.setRow(0, x1, B00000001);
    lc.setRow(0, x2, B00000001);
    return;
  }

    lc.setRow(0, x1, 0);
    lc.setRow(0, x2, 0);
    
    //if (val==5) val=6;
    //byte rowState = (1 << (val + 1)) - 1; // Efficient way to set bits up to `val`
    
    // Update the rows with a single call
    //lc.setRow(0, x1, rowState);
    //lc.setRow(0, x2, rowState);

    
  for (int16_t i = 0; i <= val; i++)
  {
    lc.setLed(0, x1, i, B00000001);
    lc.setLed(0, x2, i, B00000001);
    
  }

  
}



void LEDdisplay::status( int16_t _S1, int16_t _S2, int16_t _F1)
{
  
  barX(_S1 - 1, 0, 1);
  barX(_S2 - 1, 6, 7);
  barX(_F1 - 1, 3, 4);
  //Serial.print("S1: "); Serial.print(_S1, DEC); Serial.print(" S2: "); Serial.print(_S2, DEC); Serial.print(" F1: "); Serial.println(_F1, DEC);
}

void LEDdisplay::show_char(char ch, float seconds){
  unsigned long value = 1000 * seconds;
  clear();
  lc.displayChar(0, lc.getCharArrayPosition(ch));
  //timer
  timer(value);
  clear();
}

void LEDdisplay::show_char_no_delay(char ch){
  clear();
  lc.displayChar(0, lc.getCharArrayPosition(ch));
}


