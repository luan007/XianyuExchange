//Notes: do not use delay - use _delay & _delayMicroseconds
#define ANYTIME true
#define TEMP false
#define SAFE_HEIGHT 10
#define MOVE_HEIGHT 8000
#define Z_SENSE_PIN 24
#define Z_SENSE_M Pin(Z_SENSE_PIN)
#define Z_ERROR 1000

#define BELT_SENSE_FRONT Pin(29)
#define BELT_SENSE_BACK Pin(28)

inline void _reset_claws() {
  //reset claw
  M_LOCK_ALL();
  M_ACTIVATE(M_CLAW_L);
  M_ACTIVATE(M_CLAW_R);
  disable_motor_flag(motors[M_CLAW_L], MOTOR_CLEAN);
  disable_motor_flag(motors[M_CLAW_R], MOTOR_CLEAN);
  _wait_timeout(!(check_motor_flag(motors[M_CLAW_L], MOTOR_CLEAN) && check_motor_flag(motors[M_CLAW_R], MOTOR_CLEAN)), 30000, 1)
  {
    tick_motors();
  }
  _end
  M_SLEEP(M_CLAW_L);
  M_SLEEP(M_CLAW_R);
}

inline void _close_claws() {
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
}

inline void _lift_z(int _to) {
  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  M_TARGET(MOTOR_Z, _to);
  _wait_timeout(!M_ARRIVED(MOTOR_Z), 60000, 2)
  {
    tick_motors();
  }
  _end
}

inline void _lower_z(int zref) {
  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_Z);
  M_TARGET(MOTOR_Z, zref);
  _wait_timeout((!M_ARRIVED(MOTOR_Z)) && digitalRead(Z_SENSE_PIN), 60000, 4)
  {
    tick_motors();
  }
  _end
  M_TARGET(MOTOR_Z, motors[MOTOR_Z].position);
  //debounce..
  _delay(50);
  if (digitalRead(Z_SENSE_PIN)) {
    return DEAD(150);
  }
  if (abs(zref - motors[MOTOR_Z].position) > Z_ERROR)
  {
    return DEAD(200);
  }
}

inline void _move_to(int x, int y) {
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
}

inline int _grab_at(int x, int y, int zref) {
  _reset_claws();
  _lift_z(MOVE_HEIGHT);
  _move_to(x, y);
  _lower_z(zref);
  _close_claws();
  _lift_z(SAFE_HEIGHT);
}

inline int _put_to(int x, int y, int zref) {
  _lift_z(SAFE_HEIGHT);
  _move_to(x, y);
  _lower_z(zref);
  _reset_claws();
  _lift_z(MOVE_HEIGHT);
}

void act_report(OSCMessage &msg, int addrOffset)
{
  report_status();
}

void act_readIO(OSCMessage &msg, int addrOffset)
{
  OSCMessage outgoing_msg("/io");
  int i = 0;
  while(msg.isInt(i)) {
    outgoing_msg.add((int32_t)digitalRead(msg.getInt(i++)));
  }
  SLIPSerial.beginPacket();
  outgoing_msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  outgoing_msg.empty();
}

void act_init(OSCMessage &msg, int addrOffset)
{

  Z_SENSE_M.setInput();
  Z_SENSE_M.setPullupOn();

  BELT_SENSE_BACK.setInput();
  BELT_SENSE_BACK.setPullupOn();
  
  BELT_SENSE_FRONT.setInput();
  BELT_SENSE_FRONT.setPullupOn();

  

  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_BELT);
  motors[MOTOR_BELT].position = 0;
  motors[MOTOR_BELT].target = 1000;
  _wait_timeout(!M_ARRIVED(MOTOR_BELT), 60000, 4)
  {
    tick_motors();
  }
  _end

  
  M_LOCK_ALL();
  M_ACTIVATE(MOTOR_BELT);
  motors[MOTOR_BELT].position = 0;
  motors[MOTOR_BELT].target = -1000;
  _wait_timeout(!M_ARRIVED(MOTOR_BELT), 60000, 4)
  {
    tick_motors();
  }
  _end


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
  //  _lift_z(MOVE_HEIGHT);
}

struct params {

  int layer1Z;
  int layer2Z;

  int fromX;
  int fromY;
  int fromZ;

  int beltX;
  int beltY;

  int bufferX;
  int bufferY;
  int bufferZ;

};


void act_grab(OSCMessage &msg, int addrOffset)
{
  params p;
  for (int i = 0; i < 10; i++) {
    if (!msg.isInt(i)) {
      return DEAD(100);
    }
  }
  int i = 0;
  p.layer1Z = msg.getInt(i++);
  p.layer2Z = msg.getInt(i++);
  p.fromX = msg.getInt(i++);
  p.fromY = msg.getInt(i++);
  p.fromZ = msg.getInt(i++);
  p.beltX = msg.getInt(i++);
  p.beltY = msg.getInt(i++);
  p.bufferX = msg.getInt(i++);
  p.bufferY = msg.getInt(i++);
  p.bufferZ = msg.getInt(i++);

  if (p.fromZ == 1) {
    _grab_at(p.fromX, p.fromY, p.layer1Z);
    _put_to(p.bufferX, p.bufferY, p.bufferZ);
  }
  _grab_at(p.fromX, p.fromY, p.fromZ == 0 ? p.layer1Z : p.layer2Z);
  _put_to(p.beltX, p.beltY, p.layer1Z);
}

void act_retract(OSCMessage &msg, int addrOffset)
{
  params p;
  for (int i = 0; i < 10; i++) {
    if (!msg.isInt(i)) {
      return DEAD(100);
    }
  }
  int i = 0;
  p.layer1Z = msg.getInt(i++);
  p.layer2Z = msg.getInt(i++);
  p.fromX = msg.getInt(i++);
  p.fromY = msg.getInt(i++);
  p.fromZ = msg.getInt(i++);
  p.beltX = msg.getInt(i++);
  p.beltY = msg.getInt(i++);
  p.bufferX = msg.getInt(i++);
  p.bufferY = msg.getInt(i++);
  p.bufferZ = msg.getInt(i++);

  _grab_at(p.beltX, p.beltY, p.layer1Z);
  _put_to(p.fromX, p.fromY, p.fromZ == 0 ? p.layer1Z : p.layer2Z);
  if (p.fromZ == 1) {
    _grab_at(p.bufferX, p.bufferY, p.bufferZ);
    _put_to(p.fromX, p.fromY, p.layer1Z);
  }
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
  ROUTE("/readIO", act_readIO, ANYTIME, TEMP)                       \
  ROUTE("/force_reset", act_forcereset, STATE == STATE_ERROR, TEMP) \
  ROUTE("/init", act_init, STATE == STATE_UNINITIALIZED, 1)         \
  ROUTE("/act_simulate_delay", act_simulate_delay, STATE == STATE_IDLE, 2) \
  ROUTE("/grab", act_grab, STATE == STATE_IDLE, 3) \
  ROUTE("/retract", act_retract, STATE == STATE_IDLE, 4)
