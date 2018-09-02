//Notes: do not use delay - use _delay & _delayMicroseconds
#define ANYTIME true
#define TEMP false

void act_report(OSCMessage &msg, int addrOffset)
{
  report_status();
}

void act_init(OSCMessage &msg, int addrOffset)
{
  _delay(1000);
  //when init is finished, the ROUTE macro will set state to STATE_IDLE
  //please refer to [guard.h - LINE: 101]
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
  ROUTE("/act_simulate_delay", act_simulate_delay, STATE == STATE_IDLE, 2)
