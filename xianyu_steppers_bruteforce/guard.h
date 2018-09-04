#include <avr/wdt.h>
#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

#include "motion.h"

SLIPEncodedSerial SLIPSerial(Serial);
void parse_msg();
OSCMessage incoming_msg;

long _timeout = -1;
unsigned char err_code;

#define STATE_IDLE 0
#define STATE_BUSY 1
#define STATE_ERROR -2
#define STATE_UNINITIALIZED -1

char STATE = STATE_UNINITIALIZED; //not initialized
char FUNCTION = 0; //current function (none)
char ERROR_CODE = 0;
//other stuff


void report_status() {
  OSCMessage outgoing_msg("/state");
  outgoing_msg.add((int32_t)STATE);
  outgoing_msg.add((int32_t)FUNCTION);
  outgoing_msg.add((int32_t)ERROR_CODE);
  SLIPSerial.beginPacket();
//  SLIPSerial.println("REPORT_STATE");
  outgoing_msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  outgoing_msg.empty();
}

void update_state(char state, char function, char error_code) {
  bool change = (state != STATE) || (function != FUNCTION) || (error_code != ERROR_CODE);
  if (!change) return;
  STATE = state;
  FUNCTION = function;
  ERROR_CODE = error_code;
  report_status();
}


bool has_data = false;
bool data_clean = true;
void keep_alive() {
  if (!has_data && !data_clean) {
    data_clean = true;
    incoming_msg.empty();
  }
  if (!SLIPSerial.endofPacket() && SLIPSerial.available()) {
    incoming_msg.fill(SLIPSerial.read());
    has_data = true;
    data_clean = false;
  }
  if (SLIPSerial.endofPacket() && has_data) {
    has_data = false;
    if (!incoming_msg.hasError()) {
//      SLIPSerial.beginPackp90-=et();
//      SLIPSerial.println("REC");
//      char addr[100];
//      incoming_msg.getAddress(addr);
//      SLIPSerial.println(addr);
//      SLIPSerial.endPacket();
      return parse_msg(); //tail recur in case :)
    }
  }
}

#define _wait_if(cond) while(cond) {
#define _wait_timeout(cond, timeout, code) set_timeout(timeout, code); \
  while(cond) { check_timeout();
#define _end keep_alive(); }

void _delay(long ms) {
  long _ms = millis() + ms;
  _wait_if(millis() < _ms)
  _end
}

void _delayMicroseconds(long us) {
  long _ns = micros() + us;
  _wait_if(micros() < _ns)
  _end
}

inline void set_timeout(int t, unsigned char error_code) {
  err_code = error_code;
  _timeout = t > 0 ? (millis() + t) : -1;
}

inline void DEAD(int err_code) {
  update_state(STATE_ERROR, FUNCTION, err_code);
  _wait_if(true)
  _end
}

inline bool check_timeout() {
  if (_timeout == -1) return true;
  if (millis() > _timeout) {
    //dead
    DEAD(err_code);
    //never goes here
    return false;
  }
}


#define ROUTE(r, m, rs, id) if(rs && incoming_msg.fullMatch(r)) { \
    if(id > 0) { update_state(STATE_BUSY, id, 0); } \
    incoming_msg.dispatch(r, m); \
    if(id > 0) { update_state(STATE_IDLE, 0, 0); } \
    return; \
  }

#include "actions.h"
void parse_msg() {
  ACTIONS
}

inline void preinit() {
  wdt_disable();
  SLIPSerial.begin(9600);
  init_motors();
  delay(100);
}



