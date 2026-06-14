using namespace std;

#include <IRremote.h>
#include <common.h>
#include <Ticker.h>
#include <Preferences.h>
#include "WebControl.h"

// Version tag embedded in binary - readable by browser before OTA flash
// Magic: 0xFE 0xED 0xBE 0xEF followed by null-terminated version string
const char __attribute__((used)) FW_VERSION_TAG[] = "\xFE\xED\xBE\xEF" FW_VERSION;

char mode = 'N'; // initial normal program
uint8_t servo_step = 6; // degrees per arrow press (toggleable 4/6/8 via web)

#include <infrared.h>
Ticker BrushTimer;
Ticker BeepTimer;

extern void update_motors();
extern void Beep_off();

StepperX feeder(STEP_PIN, DIR_PIN, STOP_PIN);

// Initialize the LED matrix
LedControl lc = LedControl(data_Pin, clk_Pin, cs_Pin, NBR_MTX);
LEDdisplay display(data_Pin, clk_Pin, cs_Pin, NBR_MTX);

IRrecv irrecv(RECV_PIN);

WebControl webControl;

ServoX pan;
ServoX tilt;

Brush motor_up;
Brush motor_down;

static uint32_t last_ir_value = 0;
static unsigned long last_ir_time = 0;
const unsigned long IR_DEBOUNCE_MS = 700;
const unsigned long IR_DEBOUNCE_PROG_MS = 3000; // Samsung retransmits full code ~100ms; when program runs, block TStar retransmit

void receive_ir()
{
  if (irrecv.decode())
  {
    uint32_t receive_value = irrecv.decodedIRData.decodedRawData;
    irrecv.resume(); // always resume first

    // Ignora coduri cu protocol UNKNOWN — zgomot EMI de la WiFi
    if (irrecv.decodedIRData.protocol == UNKNOWN) { irrecv.resume(); return; }

    if (receive_value > 0)
    {
      unsigned long now = millis();
      bool same_code = (receive_value == last_ir_value);
      // When program is running, use longer debounce for start/stop codes to prevent
      // Samsung retransmit (every ~100ms) from accidentally stopping execution mid-cycle
      bool is_start_stop = (receive_value == TStar || receive_value == hTStar || receive_value == hPower);
      // Always use long debounce for TStar — Samsung retransmits every ~100ms.
      // Without this, after execute=false the debounce drops to 800ms and the
      // next Samsung retransmit (~100-800ms later) toggles execute back to true.
      unsigned long debounce = (same_code && is_start_stop) ? IR_DEBOUNCE_PROG_MS : IR_DEBOUNCE_MS;
      bool too_fast = (now - last_ir_time) < debounce;

      if (!(same_code && too_fast)) // block only if same code AND within debounce window
      {
        last_ir_value = receive_value;
        last_ir_time = now;
        DEBUG(F("IR value: "), receive_value, false);
        infrared_menu(receive_value, mode);
      }
    }
  }
}

void IRTask(void *parameter)
{
  while (true)
  {
    receive_ir();
    vTaskDelay(30 / portTICK_PERIOD_MS); // Adjust delay as needed
  }
}

void setup()
{

  SERIAL_BEGIN(9600);
  SERIAL_FLUSH();

  // Init ESC motors FIRST — motor.attach() starts the ESC boot sequence (~3-4s hardware beep).
  // By calling init() before NVS ops, ESC boot runs in parallel with the rest of setup().
  motor_up.init(MOT_UP, Brush::TOPSPIN, "MOTOR UP");
  motor_down.init(MOT_DOWN, Brush::SUPPORT, "MOTOR DOWN");

  irrecv.enableIRIn();

  xTaskCreatePinnedToCore(IRTask, "IR Task", 4096, NULL, 2, NULL, 0); // Core 0 — isolated from loop()/stepper

  target1.name = "target1";
  target2.name = "target2";
  target3.name = "target3";

  Point1.name = "Point1";
  Point2.name = "Point2";
  Point3.name = "Point3";

  // feeder.init_pins(); // DO NOT CALL: pinMode() on step pin breaks FAS RMT/MCPWM GPIO routing
  feeder.load_timeout_const(); // NVS now initialized — safe to load

  motor_up.check_data(false);   // ensure NVS defaults exist for next boot
  motor_down.check_data(false); // ensure NVS defaults exist for next boot

#define ROBOT_IRINEL 0
#define ROBOT_ADRIAN 1
#define ROBOT_NEW 0

  if (ROBOT_IRINEL)
  {
    pan.init(PAN, F("PAN"), 5, 55);
  }
  if (ROBOT_ADRIAN)
  {
    pan.init(PAN, F("PAN"), 5, 50);
  }
  if (ROBOT_NEW)
  {
    pan.init(PAN, F("PAN"), 0, 40);
  }
  pan.load_limits(); // override min/max from NVS (default 5, 50)
  pan.load_pos();
  if (ROBOT_IRINEL)
  {
    tilt.init(TILT, F("TILT"), 15, 60);
  } //
  if (ROBOT_ADRIAN)
  {
    tilt.init(TILT, F("TILT"), 5, 50);
  } //
  if (ROBOT_NEW)
  {
    tilt.init(TILT, F("TILT"), 0, 40);
  } // new join mechanism
  tilt.load_limits(); // override min/max from NVS (default 5, 50)
  tilt.load_pos();

  BrushTimer.attach_ms(50, update_motors);
  BeepTimer.attach_ms(1000 * 60 * 8, Beep_off); // ruleaza 8 minute apoi opreste beep-ul de avertizare dela motoarele brushless

  display.clear();
  feeder.enable = true;
  display.show_char('L', 1);

  webControl.begin(); // start WiFi + HTTP server on Core 0
}

// main program
//////////////////////////////////////////////////////
void loop()
{
  // Update async display: check if duration expired and clear if needed
  display.update();

  if (execute == true && mode == 'N')
  {
    infrared_normal.program();
  }

  if (execute == false && mode == 'N') // outside programming area
  {

    feeder.move_stepper(true); // true = delayed movement (timeout based on FEEDER index)
  }

} //////////////////////////////////////////////////// end loop
