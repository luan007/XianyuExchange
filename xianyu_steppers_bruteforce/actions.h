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

void act_demo(OSCMessage &msg, int addrOffset) {
  
}


#define ACTIONS \
ROUTE("/report", act_report, true, false) \
ROUTE("/init", act_init, STATE == STATE_UNINITIALIZED, 1) \
ROUTE("/demo", act_demo, STATE == STATE_IDLE, 2)

