void act_report(OSCMessage &msg, int addrOffset) {
  report_status();
}


void act_init(OSCMessage &msg, int addrOffset) {
  _wait_timeout(digitalRead(1) == LOW, 10000, 30) 
   {
      //no op
   }
   _end
}

void act_forcereset(OSCMessage &msg, int addrOffset) {
  wdt_enable(WDT2S);
}


void act_demo(OSCMessage &msg, int addrOffset) {
  
}


#define ACTIONS \
ROUTE("/report", act_report, true, false) \
ROUTE("/force_reset", act_report, STATE == STATE_ERROR, false) \
ROUTE("/init", act_init, STATE == STATE_UNINITIALIZED, 1) \
ROUTE("/demo", act_demo, STATE == STATE_IDLE, 2)

