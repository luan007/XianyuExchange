#define MOTOR_X 0
#define MOTOR_Y 1
#define MOTOR_Z 2
#define MOTOR_CLAW_L 3
#define MOTOR_CLAW_R 4
#define MOTOR_BELT 5
#define MOTOR_DOOR 6

#include "modules.h"

void setup() {
  Serial.begin(9600);
  init_sensors();
  init_motors();
}

void tick() {
  tick_sensors();
  tick_motors();
}

void loop() {
  tick();
  _log_motor(0, 500);
  while (Serial.available()) {
    unsigned char r = Serial.read();
    if (r == 'L') {
      enable_motor_flag(motors[0], MOTOR_LOCK)
    } else if (r == 'U') {
      disable_motor_flag(motors[0], MOTOR_LOCK)
    }
  }
}
