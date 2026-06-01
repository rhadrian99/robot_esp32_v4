#ifndef infrared_template_h
#define infrared_template_h

#include "infrared_template_empty.h" // interface
#include <Ticker.h>
extern uint8_t servo_step;
#define SERVO_STEP servo_step //  degrees to move servo (toggleable 4/6/8 via web)

class infrared_template : public infrared_template_empty
{
private:
  // Sistem universal de confirmare pentru punctele 1-6
  int current_point_waiting = 0; // 0=none, 1-6 = care punct asteapta confirmare
  target_point pending_point;
  Ticker confirmation_ticker;
  Ticker blink_ticker;
  bool blink_state = false;

  void _confirm_timeout()
  {
    if (current_point_waiting > 0)
    {
      confirmation_ticker.detach();
      blink_ticker.detach();
      current_point_waiting = 0;
      blink_state = false;
      show_display_status();
    }
  }

  // function-local static to avoid in-class inline static incompatibility
  static infrared_template *&_instance_ptr_ref()
  {
    static infrared_template *ptr = nullptr;
    return ptr;
  }

  static void _static_confirm_timeout()
  {
    infrared_template *inst = _instance_ptr_ref();
    if (inst)
      inst->_confirm_timeout();
  }

  void _blink_toggle()
  {
    if (current_point_waiting <= 0)
      return;
    blink_state = !blink_state;
    char c = '1' + (current_point_waiting - 1);
    if (blink_state)
    {
      display.show_char(c, 0.5);
    }
    else
    {
      show_display_status();
    }
  }

  static void _static_blink_toggle()
  {
    infrared_template *inst = _instance_ptr_ref();
    if (inst)
      inst->_blink_toggle();
  }

public:
  infrared_template()
  {
    _instance_ptr_ref() = this;
  }

  void virtual _TMute()
  {
    if (execute)
      return;
    stop_motors();
    toggle_spin();
  }

  void virtual _T1()
  {
    if (execute)
      return;

    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Serial.print("MOTOR UP SPEED:");
    Serial.println(Point._up, DEC);
    Serial.print("MOTOR DOWN SPEED:");
    Serial.println(Point._down, DEC);

    Point.name = "Point1";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 1;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('1');
  }

  void virtual _T2()
  {
    if (execute)
      return;
    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Serial.print("MOTOR UP SPEED:");
    Serial.println(Point._up, DEC);
    Serial.print("MOTOR DOWN SPEED:");
    Serial.println(Point._down, DEC);

    Point.name = "Point2";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 2;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('2');
  }

  void virtual _T3()
  {
    if (execute)
      return;
    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Serial.print("MOTOR UP SPEED:");
    Serial.println(Point._up, DEC);
    Serial.print("MOTOR DOWN SPEED:");
    Serial.println(Point._down, DEC);

    Point.name = "Point3";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 3;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('3');
  }

  void virtual _T4()
  {
    if (execute)
      return;

    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Point.name = "Point4";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 4;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('4');
  }

  void virtual _T5()
  {
    if (execute)
      return;

    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Point.name = "Point5";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 5;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('5');
  }

  void virtual _T6()
  {
    if (execute)
      return;

    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;

    Point.name = "Point6";
    memcpy(&pending_point, &Point, sizeof(Point));

    current_point_waiting = 6;
    confirmation_ticker.once_ms(3000, _static_confirm_timeout);
    blink_state = false;
    blink_ticker.attach_ms(500, _static_blink_toggle);

    display.show_char_no_delay('6');
  }

  // Tasta 7 - Salvează punctul în așteptare
  void virtual _T7()
  {
    if (execute)
      return;

    if (current_point_waiting > 0 && current_point_waiting <= 6)
    {
      confirmation_ticker.detach();
      blink_ticker.detach();
      blink_state = false;

      pending_point.name = "Point" + String(current_point_waiting);

      switch (current_point_waiting)
      {
      case 1:
        memcpy(&Point1, &pending_point, sizeof(target_point));
        target_save_nvm(Point1);
        break;
      case 2:
        memcpy(&Point2, &pending_point, sizeof(target_point));
        target_save_nvm(Point2);
        break;
      case 3:
        memcpy(&Point3, &pending_point, sizeof(target_point));
        target_save_nvm(Point3);
        break;
      case 4:
        memcpy(&Point4, &pending_point, sizeof(target_point));
        target_save_nvm(Point4);
        break;
      case 5:
        memcpy(&Point5, &pending_point, sizeof(target_point));
        target_save_nvm(Point5);
        break;
      case 6:
        memcpy(&Point6, &pending_point, sizeof(target_point));
        target_save_nvm(Point6);
        break;
      }

      current_point_waiting = 0;
      display.displayImage_async(IMAGES[12], .5); // ok save — bargraph restored by update() after 500ms
    }
  }

  // Tasta 8 - Încarcă punctul în aștuptare
  void virtual _T8()
  {
    if (execute)
      return;

    if (current_point_waiting > 0 && current_point_waiting <= 6)
    {
      confirmation_ticker.detach();
      blink_ticker.detach();
      blink_state = false;

      String point_name = "Point" + String(current_point_waiting);
      load_target_point(point_name);

      current_point_waiting = 0;
      display.displayImage_async(IMAGES[25], .5); // ok load — bargraph restored by update() after 500ms
    }
  }

  void load_target_point(String position)
  {
    target_point T;
    T.name = position;
    target_point POINT = target_load_nvm(T);

    motor_up.speed = POINT._up;
    motor_down.speed = POINT._down;
    motor_up.index = POINT.index_up;
    motor_down.index = POINT.index_down;
    motor_up.spin = (Brush::_spinType)POINT.spin_up;
    motor_down.spin = (Brush::_spinType)POINT.spin_down;

    if (motor_up.spin == Brush::TOPSPIN)
    {
      motor_down.spin = Brush::SUPPORT;
      motor_up.set_spin_after_load(Brush::TOPSPIN);
      motor_down.set_spin_after_load(Brush::SUPPORT);
    }
    else if (motor_up.spin == Brush::SUPPORT)
    {
      motor_down.spin = Brush::BACKSPIN;
      motor_up.set_spin_after_load(Brush::SUPPORT);
      motor_down.set_spin_after_load(Brush::BACKSPIN);
    }
    else if (motor_up.spin == Brush::NOSPIN)
    {
      motor_down.spin = Brush::NOSPIN;
      motor_up.set_spin_after_load(Brush::NOSPIN);
      motor_down.set_spin_after_load(Brush::NOSPIN);
    }
    else
    {
      // spin_up=0 (BACKSPIN) or unknown — old NVS data saved before spin was tracked.
      // Fall back to TOPSPIN (safest default for motor_up).
      motor_up.spin  = Brush::TOPSPIN;
      motor_down.spin = Brush::SUPPORT;
      motor_up.set_spin_after_load(Brush::TOPSPIN);
      motor_down.set_spin_after_load(Brush::SUPPORT);
    }

    motor_up.set_speed();
    motor_down.set_speed();

    pan.startMove(POINT._pan_pos);
    tilt.startMove(POINT._tilt_pos);
    tempo_empty(500);
    feeder.stop();
    feeder.speed = POINT.stepper_speed;
    feeder.index = POINT.stepper_index;
    show_display_status();
    feeder.start();
  }

  void load_point_from_nvm(int point_num)
  {
    target_point *pPoint = nullptr;
    switch (point_num)
    {
    case 1:
      pPoint = &Point1;
      break;
    case 2:
      pPoint = &Point2;
      break;
    case 3:
      pPoint = &Point3;
      break;
    case 4:
      pPoint = &Point4;
      break;
    case 5:
      pPoint = &Point5;
      break;
    case 6:
      pPoint = &Point6;
      break;
    }

    if (pPoint)
    {
      pPoint->name = "Point" + String(point_num);
      target_point temp = target_load_nvm(*pPoint);
      memcpy(pPoint, &temp, sizeof(target_point));
    }
  }

  target_point *get_point_by_number(int point_num)
  {
    switch (point_num)
    {
    case 1:
      return &Point1;
    case 2:
      return &Point2;
    case 3:
      return &Point3;
    case 4:
      return &Point4;
    case 5:
      return &Point5;
    case 6:
      return &Point6;
    }
    return nullptr;
  }

  // Web UI: save current state as Point N (no timer — immediate save)
  void web_save_point(int poz)
  {
    if (execute) return;
    if (poz < 1 || poz > 6) return;

    confirmation_ticker.detach();
    blink_ticker.detach();
    current_point_waiting = 0;
    blink_state = false;

    target_point Point;
    Point._pan_pos = pan.read_pos();
    Point._tilt_pos = tilt.read_pos();
    Point._up = motor_up.speed;
    Point._down = motor_down.speed;
    Point.stepper_speed = feeder.speed;
    Point.stepper_index = feeder.index;
    Point.index_up = motor_up.index;
    Point.index_down = motor_down.index;
    Point.spin_up = motor_up.spin;
    Point.spin_down = motor_down.spin;
    Point.name = "Point" + String(poz);

    target_point *pPoint = get_point_by_number(poz);
    if (pPoint)
    {
      memcpy(pPoint, &Point, sizeof(target_point));
      target_save_nvm(*pPoint);
    }
    display.displayImage_async(IMAGES[12], .5);
  }

  // Web UI: load and execute Point N (no timer check)
  void web_run_point(int poz)
  {
    if (execute) return;
    if (poz < 1 || poz > 6) return;

    confirmation_ticker.detach();
    blink_ticker.detach();
    current_point_waiting = 0;
    blink_state = false;

    String point_name = "Point" + String(poz);
    load_target_point(point_name);
    display.displayImage_async(IMAGES[25], .5);
  }

  void virtual _TOK()
  {
    if (feeder.index > 0)
      return;
    if (motor_up.index > 0)
      return;
    if (motor_down.index > 0)
      return;

    if (execute)
      return;
    // if all above conditions are meet then abort entering in setup mode

    execute = false;
    switch_program(mode);
    tempo_empty(10);
    display.show_char(mode, 0.5);
    show_display_status();
  }

  // decrease feeder speed
  void virtual _TTools()
  {
    if (execute)
      return;
    feeder.decrease_speed();
    show_display_status();
  }

  void virtual _TInfo()
  {

    if (execute)
      return;

    if ((motor_up.index > 0) || (motor_down.index > 0)) // allow feeder to feed balls only if one motor is active
    {
      feeder.increase_speed();
    }
    else
    {
      feeder.index = 0;
      feeder.stop();
    }
    show_display_status();
  }

  void virtual _VUP() // main motor up
  {
    if (execute)
      return;
    if (motor_up.spin == Brush::TOPSPIN)
    {
      motor_up.increase_speed(); // topspin: motor_up = main
    }
    else if (motor_up.spin == Brush::SUPPORT)
    {
      motor_down.increase_speed(); // backspin: motor_down = main
    }
    else
    { // nospin: both same
      motor_up.increase_speed();
      motor_down.increase_speed();
    }
    
    show_display_status();
  }

  void virtual _VDOWN() // main motor down
  {
    if (execute)
      return;

    if (motor_up.spin == Brush::TOPSPIN)
    {
      motor_up.decrease_speed(); // topspin: motor_up = main
    }
    else if (motor_up.spin == Brush::SUPPORT)
    {
      motor_down.decrease_speed(); // backspin: motor_down = main
    }
    else
    { // nospin: both same
      motor_up.decrease_speed();
      motor_down.decrease_speed();
    }

    if (motor_up.index == 0 && motor_down.index == 0)
    {
      feeder.index = 0;
      feeder.stop();
    }
    show_display_status();
  }
  void virtual _PUP() // support motor up
  {
    if (execute)
      return;
    if (motor_up.spin == Brush::TOPSPIN)
    {
      motor_down.increase_speed(); // topspin: motor_down = support
    }
    else if (motor_up.spin == Brush::SUPPORT)
    {
      motor_up.increase_speed(); // backspin: motor_up = support
    }
    // nospin: P buttons inactive (both motors controlled via V)

    show_display_status();
  }

  void virtual _PDOWN() // support motor down
  {
    if (execute)
      return;
    if (motor_up.spin == Brush::TOPSPIN)
    {
      motor_down.decrease_speed(); // topspin: motor_down = support
    }
    else if (motor_up.spin == Brush::SUPPORT)
    {
      motor_up.decrease_speed(); // backspin: motor_up = support
    }
    // nospin: P buttons inactive
    if (motor_up.index == 0 && motor_down.index == 0)
    {
      feeder.index = 0;
      feeder.stop();
    }
    show_display_status();
  }

  void virtual _T0()
  {
    if (execute)
      return;
    if (feeder.index > 0)
      return;
    feeder.load_timeout_const();
    feeder.timeout_const += 20;
    if (feeder.timeout_const > 280)
      feeder.timeout_const = 160;

    if (feeder.timeout_const == 160)
      display.displayImage_async(IMAGES[18], 0.4);
    if (feeder.timeout_const == 180)
      display.displayImage_async(IMAGES[19], 0.4);
    if (feeder.timeout_const == 200)
      display.displayImage_async(IMAGES[20], 0.4);
    if (feeder.timeout_const == 220)
      display.displayImage_async(IMAGES[21], 0.4);
    if (feeder.timeout_const == 240)
      display.displayImage_async(IMAGES[22], 0.4);
    if (feeder.timeout_const == 260)
      display.displayImage_async(IMAGES[23], 0.4);
    if (feeder.timeout_const == 280)
      display.displayImage_async(IMAGES[24], 0.4);

    feeder.save_timeout_const();
  }

  void _T9() override
  {
    if (execute == true)
    {
      return;
    }

    // Ciclu: 2 -> 3 -> 4 -> 6 -> 2
    switch (points)
    {
    case 2:
      points = 3;
      break;
    case 3:
      points = 4;
      break;
    case 4:
      points = 6;
      break;
    case 6:
      points = 2;
      break;
    default:
      points = 2;
      break; // inițial
    }

    display.show_char('0' + points, 0.5);
  }

  void virtual _Tstar() // pause feeder
  {
    mode = 'Q'; // prevent execution until all things are set
    execute = !execute;

    if (execute == false) // stop the programming mode
    {
      // stop motors FIRST — before any display/delay calls
      // BrushTimer fires every 50ms and calls update_motors() → set_speed() with old pos() speed
      // if we delay here (display), motors keep running for that duration
      feeder.index = 0;
      feeder.stop();
      motor_up.stop();
      motor_down.stop();
      motor_up.set_speed(0);
      motor_down.set_speed(0);

      mode = 'N';
      display.displayImage_async(IMAGES[12], 1); // ok save
      initial_position();                  // servo to neutral position
    }
    else // start the program  execute=true
    {

      feeder.index = 0;
      feeder.stop();
      initial_position(); // move servos to neutral position
      tempo_empty(800);
      mode = 'N';
    }
  }

  void _TPower() override
  {
    // stop motors FIRST — before any blocking delay/display
    feeder.index = 0;
    feeder.stop();
    motor_up.stop();
    motor_down.stop();
    motor_up.set_speed(0);
    motor_down.set_speed(0);

    execute = false;
    mode = 'N';

    tempo_empty(500);
    initial_position(); // servo to neutral position
    display.show_char(mode, 0.5);
  }

  void virtual _Tdiez()
  {
    execute = false;
    mode = 'N';
    stop_all();

    tempo_empty(30);
    initial_position();
    display.show_char(mode, 0.5);
  }

  void virtual _TLEFT()
  {
    if (execute)
      return;
    pan.startMove(pan.read_pos() + SERVO_STEP);
    Serial.printf("PAN  pos=%d  min=%d  max=%d\n", pan.read_pos(), pan.min_value, pan.max_value);
    display.displayImage_async(IMAGES[11], 1.2);
    // NO show_display_status() - avoid conflict with async display
  }

  void virtual _TRIGHT()
  {
    if (execute)
      return;
    pan.startMove(pan.read_pos() - SERVO_STEP);
    Serial.printf("PAN  pos=%d  min=%d  max=%d\n", pan.read_pos(), pan.min_value, pan.max_value);
    display.displayImage_async(IMAGES[9], 1.2);
    // NO show_display_status() - avoid conflict with async display
  }

  void virtual _TDOWN()
  {
    if (execute)
      return;
    tilt.startMove(tilt.read_pos() - SERVO_STEP);
    Serial.printf("TILT pos=%d  min=%d  max=%d\n", tilt.read_pos(), tilt.min_value, tilt.max_value);
    display.displayImage_async(IMAGES[8], 1.2);
    // NO show_display_status() - avoid conflict with async display
  }

  void virtual _TUP()
  {
    if (execute)
      return;
    tilt.startMove(tilt.read_pos() + SERVO_STEP);
    Serial.printf("TILT pos=%d  min=%d  max=%d\n", tilt.read_pos(), tilt.min_value, tilt.max_value);
    display.displayImage_async(IMAGES[10], 1.2);
    // NO show_display_status() - avoid conflict with async display
  }

  void program()
  {
    if (execute == false)
      return;
    int time = 700;

    for (int i = 1; i <= 6; i++)
    {
      load_point_from_nvm(i);
    }

    if (mode == 'N')
    {
      Serial.print("PROGRAM N: ");
      Serial.print(points, DEC);
      Serial.println("points");

      switch (points)
      {
      case 2:
        pos(*get_point_by_number(1), time);
        pos(*get_point_by_number(3), time);
        break;

      case 3:
        pos(*get_point_by_number(1), time);
        pos(*get_point_by_number(2), time);
        pos(*get_point_by_number(3), time);
        pos(*get_point_by_number(2), time);
        break;

      case 4:
        pos(*get_point_by_number(1), 600);
        pos(*get_point_by_number(1), time);
        pos(*get_point_by_number(3), 600);
        pos(*get_point_by_number(3), time);
        break;

      case 6:
        // Executa random sequence cu 4 pozitii aleatorii din Point1-Point6
        // fara a repeta pozitia anterioara consecutiv
        {
          int lastPoint = 0;
          for (int i = 0; i < 4; i++)
          {
            int randPoint;
            do
            {
              randPoint = random(1, 7);
            } while (randPoint == lastPoint);

            target_point *p = get_point_by_number(randPoint);
            if (p)
              pos(*p, time);
            lastPoint = randPoint;
          }
        }
        break;
      }
    }
  }

  void pos(target_point P, uint16_t timeout_throw)
  {
    // Check execute FIRST — before setting any motor speeds.
    // BrushTimer fires every 50ms calling update_motors() → set_speed().
    // If we set motor_up.speed here and then return on execute==false,
    // BrushTimer will use the new speed and restart the motors.
    if (execute == false)
    {
      return;
    }

    motor_up.speed = P._up;
    motor_down.speed = P._down;

    motor_up.index = P.index_up;
    motor_down.index = P.index_down;
    show_display_status();

    pan.startMove(P._pan_pos);
    tilt.startMove(P._tilt_pos);

    if (execute == false)
    {
      return;
    }

    tempo_empty(timeout_throw);
    feeder.move_stepper(false);
  }

private:
};

#endif
