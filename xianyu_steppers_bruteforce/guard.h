#include <avr/wdt.h>
#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>


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
  outgoing_msg.add((char)STATE);
  outgoing_msg.add((char)FUNCTION);
  outgoing_msg.add((char)ERROR_CODE);
  SLIPSerial.beginPacket();
  outgoing_msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  outgoing_msg.empty();
}

void update_state(char state, char function, char error_code) {
  bool change = state != STATE || function != FUNCTION || error_code != ERROR_CODE;
  if (!change) return;
  STATE = state;
  FUNCTION = function;
  ERROR_CODE = error_code;
  report_status();
}


#define _wait_if(cond) while(cond) {
#define _wait_timeout(cond, timeout, code) set_timeout(timeout, code); \
  while(cond) { check_timeout();
#define _end keep_alive(); }


void keep_alive() {
  if (!SLIPSerial.endofPacket() && SLIPSerial.available()) {
    incoming_msg.fill(SLIPSerial.read());
    if (SLIPSerial.endofPacket()) {
      if (!incoming_msg.hasError()) {
        parse_msg();
      }
      incoming_msg.empty();
    }
  }
}

inline void set_timeout(int t, unsigned char error_code) {
  err_code = error_code;
  _timeout = t > 0 ? (millis() + t) : -1;
}


inline bool check_timeout() {
  if (_timeout == -1) return true;
  if (millis() > _timeout) {
    //dead
    update_state(STATE_ERROR, FUNCTION, err_code);
    _wait_if(true)
    _end
    return false;
  }
}



#define ROUTE(r, m, rs, id) if(rs && incoming_msg.fullMatch(r)) { \
  if(id >= 0) { update_state(STATE_BUSY, id, 0); } \
  incoming_msg.dispatch(r, m); \
  update_state(STATE_IDLE, 0, 0); \
  return; \ 
}


#include "actions.h"
void parse_msg() {
  ACTIONS
}


inline void preinit() {
  wdt_disable();
  SLIPSerial.begin(9600);
  delay(100);
}



