//Notes: do not use delay - use _delay & _delayMicroseconds
#define ANYTIME true
#define TEMP false

#define Z_SENSE_M Pin(24)

#define Z_ERROR 200

void act_report(OSCMessage &msg, int addrOffset)
{
  report_status();
}

void act_init(OSCMessage &msg, int addrOffset)
{
  Z_SENSE_M.setInput();
  Z_SENSE_M.setPullupOn();
  
  M_LOCK_ALL();
  M_ACTIVATE(M_CLAW_L);
  M_ACTIVATE(M_CLAW_R);
  _wait_timeout(!(check_motor_flag(motors[M_CLAW_L], MOTOR_CLEAN) && check_motor_flag(motors[M_CLAW_R], MOTOR_CLEAN)), 30000, 1)
  {
    tick_motors();
  }
  _end
  M_SLEEP(M_CLAW_L);
  M_SLEEP(M_CLAW_R);

  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  _wait_timeout(!check_motor_flag(motors[MOTOR_Z], MOTOR_CLEAN), 60000, 2)
  {
    tick_motors();
  }
  _end

  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Y);
  _wait_timeout(!check_motor_flag(motors[MOTOR_Y], MOTOR_CLEAN), 60000, 3)
  {
    tick_motors();
  }
  _end

  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_X);
  _wait_timeout(!check_motor_flag(motors[MOTOR_X], MOTOR_CLEAN), 60000, 4)
  {
    tick_motors();
  }
  _end
}

void act_moveTo(OSCMessage &msg, int addrOffset)
{
  M_LOCK_ALL();
  if (msg.isInt(0) && msg.isInt(1)) //check method parameter
  {
    M_ACTIVATE(msg.getInt(0));
    M_TARGET(msg.getInt(0), msg.getInt(1));
    _wait_timeout(!M_ARRIVED(msg.getInt(0)), 60000, 2)
    {
      tick_motors();
    }
  }
  _end
}


void act_grab(OSCMessage &msg, int addrOffset)
{
  int x = 0;
  int y = 0;
  int z = 0;
  int zref = 0;
  if (!(msg.isInt(0) && msg.isInt(1) && msg.isInt(2) && msg.isInt(3)))
  {
    return DEAD(100);
  }
  else
  {
    x = msg.getInt(0);
    y = msg.getInt(1);
    z = msg.getInt(2);
    zref = msg.getInt(3);
  }


  //reset claw
  M_LOCK_ALL();
  M_ACTIVATE(M_CLAW_L);
  M_ACTIVATE(M_CLAW_R);
  _wait_timeout(!(check_motor_flag(motors[M_CLAW_L], MOTOR_CLEAN) && check_motor_flag(motors[M_CLAW_R], MOTOR_CLEAN)), 30000, 1)
  {
    tick_motors();
  }
  _end
  M_SLEEP(M_CLAW_L);
  M_SLEEP(M_CLAW_R);


  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  M_TARGET(MOTOR_Z, 10);
  _wait_timeout(!M_ARRIVED(MOTOR_Z), 60000, 2)
  {
    tick_motors();
  }
  _end

  
  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_X);
  M_ACTIVATE(MOTOR_Y);
  M_TARGET(MOTOR_X, x);
  M_TARGET(MOTOR_Y, y);
  _wait_timeout(!M_ARRIVED(MOTOR_Y) || !M_ARRIVED(MOTOR_X), 60000, 3)
  {
    tick_motors();
  }
  _end

//  return;
  
  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  M_TARGET(MOTOR_Z, zref);
  _wait_timeout((!M_ARRIVED(MOTOR_Z)) && Z_SENSE_M.getValue(), 60000, 4)
  {
    tick_motors();
  }
  _end
  M_TARGET(MOTOR_Z, motors[MOTOR_Z].position);
  if(Z_SENSE_M.getValue()) {
    return DEAD(150);
  }

  if(abs(zref - motors[MOTOR_Z].position) > Z_ERROR) {}
  {
    return DEAD(200);
  }


  //reset claw
  M_LOCK_ALL();
  M_ACTIVATE(M_CLAW_L);
  M_ACTIVATE(M_CLAW_R);
  M_TARGET(M_CLAW_L, 9000);
  M_TARGET(M_CLAW_R, 9000);
  _wait_timeout(!M_ARRIVED(M_CLAW_L) || !M_ARRIVED(M_CLAW_R), 30000, 1)
  {
    tick_motors();
  }
  _end
  M_SLEEP(M_CLAW_L);
  M_SLEEP(M_CLAW_R);


  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  M_TARGET(MOTOR_Z, 10);
  _wait_timeout(!M_ARRIVED(MOTOR_Z), 60000, 2)
  {
    tick_motors();
  }
  _end

}


void act_forcereset(OSCMessage &msg, int addrOffset)
{
  wdt_enable(WDTO_2S);
}

void act_simulate_delay(OSCMessage &msg, int addrOffset)
{
  if (msg.isInt(0)) //check method parameter
  {
    _delay(msg.getInt(0)); //special delay, maintains osc activity during delay.
  }
  else //illegal param
  {
    DEAD(100); //machine is stalled for safety, now you need to reset
  }
}

//ROUTE (string Path, function Action, bool Condition, int FunctionId )
//  Path: method name in host OSCMessage
//  Action: actual logic performed
//  Condition: state check, your function won't be called when condition = false
//  FunctionId: false   | temporary function, state won't be touched when executing.
//              number  | stateful function, will automatically change 'FunctionId' in state
//                        and send to host. host will recognize the id and know which function is
//                        being actively executed.

#define ACTIONS                                                     \
  ROUTE("/report", act_report, ANYTIME, TEMP)                       \
  ROUTE("/force_reset", act_forcereset, STATE == STATE_ERROR, TEMP) \
  ROUTE("/init", act_init, STATE == STATE_UNINITIALIZED, 1)         \
  ROUTE("/act_simulate_delay", act_simulate_delay, STATE == STATE_IDLE, 2) \
  ROUTE("/grab", act_grab, STATE == STATE_IDLE, 3) \
  ROUTE("/moveTo", act_moveTo, STATE == STATE_IDLE, 10)
