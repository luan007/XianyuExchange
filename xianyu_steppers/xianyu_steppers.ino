#define STEPPER_X 0
#define STEPPER_Y 1
#define STEPPER_Z 2
#define STEPPER_CLAW_L 3
#define STEPPER_CLAW_R 4
#define STEPPER_BELT 5
#define STEPPER_DOOR 6

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
