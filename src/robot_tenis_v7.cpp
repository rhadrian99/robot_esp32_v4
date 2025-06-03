using namespace std;

#include <IRremote.h>
#include <common.h>
#include <Ticker.h>
#include <Preferences.h>

#define VERSION 6         // added in april 2025
#define MINOR_VERSION 1 //

#define BRUSH_SPEED_STEP 30 // millisec
#define TIMEOUT_PROGRAM 200 // millisec
#define STEPPER_SPEED_PROGRAM 100 // millisec

#define ROBOT_IRINEL 0
#define ROBOT_ADRIAN 0
#define ROBOT_NEW 1


bool timeractive=true;
int var = 0;

char mode ='N'; // initial normal program
//char modes[] = {'N','P','S'};

#include <infrared.h>
Ticker BrushTimer;
Ticker IRTimer;
Ticker StatusTimer;
Ticker BeepTimer;
Ticker memory_timer;

 

extern void update_motors();
extern void Beep_off();



StepperX feeder(STEP_PIN, DIR_PIN, STOP_PIN );



// Initialize the LED matrix
LedControl lc = LedControl(data_Pin, clk_Pin, cs_Pin, NBR_MTX);
LEDdisplay display(data_Pin, clk_Pin, cs_Pin,NBR_MTX);

IRrecv irrecv(RECV_PIN);

ServoX pan;
ServoX tilt;

Brush motor_up;
Brush motor_down;


void status()
{
    
  DEBUG("********************** mode: ",true);
  motor_up.reporting();NL();
  motor_down.reporting();NL();
  
  DEBUG("STATUS: "+ motor_up.motor_position+"  -> "+motor_up.spintype+", speed = ",motor_up.speed,true) ;
  DEBUG("STATUS: "+ motor_down.motor_position+"-> "+motor_down.spintype+", speed = ",motor_down.speed,false) ;
  NL();
    
  DEBUG("STATUS:  SERVO PAN pos   ->" ,pan.read_pos(),false);
  DEBUG("         [---] PAN MIN   -> " ,pan.min_value,false);
  DEBUG("         [---] PAN MAX   -> " ,pan.max_value,false);
  NL();
  NL();
  DEBUG("STATUS:  SERVO TILT pos  ->",tilt.read_pos(),false) ;
  DEBUG("         [---] TILT MIN -> " ,tilt.min_value,false);
  DEBUG("         [---] TILT MAX -> " ,tilt.max_value,false);
  NL();
  NL();
  DEBUG("STATUS: FEEDER ENABLE    -> " ,feeder.enable,false);
  NL();
  DEBUG("               index     -> " ,feeder.index,false);
  NL();
  DEBUG("               speed     -> ",feeder.speed,false) ;
  NL();
  DEBUG("               direction -> " ,feeder.directie,false);
  NL();
  DEBUG("               timeout   -> " ,feeder.timeout_const,false);
  NL();


}

void printMemoryUsage() {
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
}

void printMinMemoryUsage() {
  Serial.print("Minimum Free Heap: ");
  Serial.print(ESP.getMinFreeHeap());
  Serial.println(" bytes");
}

void printMaxAllocHeap() {
  Serial.print("Max Allocatable Heap: ");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(" bytes");
}

void printTaskStackUsage() {
  Serial.print("Task Stack High Water Mark: ");
  Serial.print(uxTaskGetStackHighWaterMark(NULL)); // For the current task
  Serial.println(" bytes");
}

void printMemoryStats() {
  Serial.println("=== Memory Statistics ===");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");

  Serial.print("Minimum Free Heap: ");
  Serial.print(ESP.getMinFreeHeap());
  Serial.println(" bytes");

  Serial.print("Max Allocatable Heap: ");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(" bytes");

  Serial.print("Task Stack High Water Mark: ");
  Serial.print(uxTaskGetStackHighWaterMark(NULL));
  Serial.println(" bytes");
  Serial.println("=========================");
}


void receive_ir()
{
  
if (irrecv.decode())
  {
    uint32_t receive_value = irrecv.decodedIRData.decodedRawData ;
   
    if (receive_value>0)
    {
      DEBUG(F("IR value: "),receive_value,true); Serial.println();
      infrared_menu(receive_value, mode);
      //tempo_empty(100);
      status();
      
    }
    // Receive the next value
    irrecv.resume();
    
  }
  else
  {
      //bluetooh_receive(); 
  }
}

void IRTask(void* parameter) {
  while (true) {
      receive_ir();
      vTaskDelay(30 / portTICK_PERIOD_MS); // Adjust delay as needed
  }
}





void setup()
{
   

 irrecv.enableIRIn();
 
 xTaskCreatePinnedToCore(IRTask, "IR Task", 2048, NULL, 1, NULL, 1); // Run IR task on Core 0

 Serial.begin(9600);
 Serial.flush();

 target1.name="target1";
 target2.name="target2";
 target3.name="target3";

 Point1.name="Point1";
 Point2.name="Point2";
 Point3.name="Point3";

 //Point1=target_load_nvm(Point1);
 //Point2=target_load_nvm(Point2);
 //Point3=target_load_nvm(Point3);

 feeder.init_pins();

  motor_down.check_data(false); 

  motor_up.init(MOT_UP,Brush::TOPSPIN,"MOTOR UP") ;
  motor_down.init(MOT_DOWN,Brush::SUPPORT,"MOTOR DOWN");
  

  if (ROBOT_IRINEL) { pan.init(PAN, F("PAN"),5,55);}
  if (ROBOT_ADRIAN) { pan.init(PAN, F("PAN"),5,55);}
  if (ROBOT_NEW) { pan.init(PAN, F("PAN"),0,40);}
  pan.load_pos();
  
  //delay(200);
  if (ROBOT_IRINEL) { tilt.init(TILT,F("TILT"),15,60);} // 
  if (ROBOT_ADRIAN) { tilt.init(TILT,F("TILT"),5,60);} // 
  if (ROBOT_NEW) { tilt.init(TILT,F("TILT"),0,40);} // new join mechanism
  tilt.load_pos();

  
  BrushTimer.attach_ms(50, update_motors);  
  //IRTimer.attach_ms(50, receive_ir);  
  BeepTimer.attach_ms(1000*60*8, Beep_off);  // silent the beef from motors at every 8 min
  //memory_timer.attach_ms(1000*5, printMemoryStats);  // silent the beef from motors at every 8 min
  //BeepTimer.attach_ms(1000*20, Beep_off);  
  //StatusTimer.attach_ms(4000, status); 
  
  display.clear();
  feeder.enable=true;
  display.show_char('L',1);
 
    
  }


//main program
//////////////////////////////////////////////////////
void loop()
{
  
  if (execute==true && mode=='N')
  {
    infrared_normal.program();
  }

  if (execute==false && mode=='N') // outside programming area
  {
    //Serial.println("Normal mode");
    feeder.move_stepper(100,true); /// true is for delayed movement
    
  }
  
  if (feeder.enable==false && execute==false)
  {
    //feeder.stop();    
  }
  
  
  
  

} //////////////////////////////////////////////////// end loop
