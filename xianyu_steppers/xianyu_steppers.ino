#include "motion.h"

void setup() {
  Serial.begin(9600);
  init_motors();
}

void tick() {
  tick_motors();
}

void loop() {
  tick();
  _log_motor(0, 500);
  while (Serial.available()) {
    unsigned char r = Serial.read();
    if (r == 'L') {
      set_motor_lock(0, 1);
    } else if (r == 'U') {
      set_motor_lock(0, 0);
    }
  }
}
