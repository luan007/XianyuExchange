#include <AccelStepper.h>
#define Z_GET_EN            8

#define Z_GET_L_PUL         2
#define Z_GET_L_DIR         5

#define Z_GET_R_PUL         3
#define Z_GET_R_DIR         6

#define Z_BUTTON_L_HIGH     22
#define Z_BUTTON_L_LOW      23
#define Z_BUTTON_M_HIGH     24
#define Z_BUTTON_M_LOW      25
#define Z_BUTTON_R_HIGH     26
#define Z_BUTTON_R_LOW      27

#define OUT_SENSER_BACK     28
#define OUT_SENSER_FRONT    29
#define PUT_SENSER_2        30
#define PUT_SENSER_1        31

#define X_SENSER            32
#define Y_SENSER            33
#define Z_SENSER            34

#define X_PUL               35
#define X_DIR               36
#define X_EN                37

#define Y_PUL               38
#define Y_DIR               39
#define Y_EN                40

#define Z_PUL               41
#define Z_DIR               42
#define Z_EN                43

#define FRONT_PUL           44
#define FRONT_DIR           45
#define FRONT_EN            46
#define MAX_X               11000
#define MAX_Y               11650

#define MAX_L               14200
#define MAX_R               12900

AccelStepper stepperX(1, X_PUL, X_DIR);
AccelStepper stepperY(1, Y_PUL, Y_DIR);
AccelStepper stepperZ(1, Z_PUL, Z_DIR);

AccelStepper stepperLG(1, Z_GET_L_PUL, Z_GET_L_DIR);
AccelStepper stepperRG(1, Z_GET_R_PUL, Z_GET_R_DIR);

AccelStepper stepperF(1, FRONT_PUL, FRONT_DIR);

void setup() {
  pinMode(Z_BUTTON_L_HIGH, INPUT_PULLUP);
  pinMode(Z_BUTTON_L_LOW, OUTPUT);
  pinMode(Z_BUTTON_M_HIGH, INPUT_PULLUP);
  pinMode(Z_BUTTON_M_LOW, OUTPUT);
  pinMode(Z_BUTTON_R_HIGH, INPUT_PULLUP);
  pinMode(Z_BUTTON_R_LOW, OUTPUT);
  digitalWrite(Z_BUTTON_L_LOW, LOW);
  digitalWrite(Z_BUTTON_M_LOW, LOW);
  digitalWrite(Z_BUTTON_R_LOW, LOW);

  pinMode(OUT_SENSER_BACK, INPUT_PULLUP);
  pinMode(OUT_SENSER_FRONT, INPUT_PULLUP);
  pinMode(PUT_SENSER_2, INPUT_PULLUP);
  pinMode(PUT_SENSER_1, INPUT_PULLUP);

  pinMode(X_PUL, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(X_EN, OUTPUT);
  pinMode(Y_PUL, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(Y_EN, OUTPUT);
  pinMode(Z_PUL, OUTPUT);
  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_EN, OUTPUT);

  pinMode(Z_GET_EN, OUTPUT);

  pinMode(FRONT_PUL, OUTPUT);
  pinMode(FRONT_DIR, OUTPUT);
  pinMode(FRONT_EN, OUTPUT);

  digitalWrite(FRONT_EN, HIGH);
  digitalWrite(Z_GET_EN, LOW);
  digitalWrite(X_EN, LOW);
  digitalWrite(Y_EN, LOW);
  digitalWrite(Z_EN, LOW);

  Serial.begin(9600);
  stepperX.setMaxSpeed(2000);
  stepperX.setAcceleration(700);

  stepperY.setMaxSpeed(2000);
  stepperY.setAcceleration(700);

  stepperZ.setMaxSpeed(2000);
  stepperZ.setAcceleration(700);

  stepperF.setMaxSpeed(2000);
  stepperF.setAcceleration(700);

  stepperLG.setMaxSpeed(2000);
  stepperLG.setAcceleration(700);

  stepperRG.setMaxSpeed(2000);
  stepperRG.setAcceleration(700);
}

int targetX = 0;
int targetY = 0;
int targetZ = 0;

int targetLG = 0;
int targetRG = 0;

boolean z_m_button_flag = true;

void loop() {
  
  if (digitalRead(OUT_SENSER_BACK) == LOW) {
    //Serial.println("back");
  }
  if (digitalRead(OUT_SENSER_FRONT) == LOW) {
    //Serial.println("front");
  }
  if (digitalRead(PUT_SENSER_2) == LOW) {
    //Serial.println("senser_2");
  }
  if (digitalRead(PUT_SENSER_1) == LOW) {
    //Serial.println("senser_1");
  }

  if (Serial.available() > 0) {
    String head;
    String s = Serial.readString();
    int n = s.indexOf(":");
    head = s.substring(0, n);
    s = s.substring(n + 1);
    int pos = s.toInt();
    if (head == "X") {
      Serial.println(pos);
      if (pos > MAX_X) {
        targetX = MAX_X;
      } else {
        targetX = pos;
      }
    } else if (head == "Y") {
      Serial.println(pos);
      if (pos > MAX_Y) {
        targetY = MAX_Y;
      } else {
        targetY = pos;
      }
    } else if (head == "Z") {
      Serial.println(pos);
      z_m_button_flag = true;
      targetZ  = pos;
    } else if (head == "L") {
      Serial.println(pos);
      if (pos > MAX_L) {
        targetLG = MAX_L;
      } else {
        targetLG = pos;
      }
    } else if (head == "R") {
      Serial.println(pos);
      if (pos > MAX_R) {
        targetRG = MAX_R;
      } else {
        targetRG = pos;
      }
    }
  }

  if (digitalRead(X_SENSER) == LOW) {
    Serial.println("x_senser");
    stepperX.stop();
    stepperX.setCurrentPosition(0);
    stepperX.moveTo(10);
    targetX = 10;
    stepperX.run();
  } else {
    stepperX.moveTo(targetX);
    stepperX.run();
  }
  
  if (digitalRead(Y_SENSER) == LOW) {
    Serial.println("y_senser");
    stepperY.stop();
    stepperY.setCurrentPosition(0);
    stepperY.moveTo(10);
    targetY = 10;
    stepperY.run();
  } else {
    stepperY.moveTo(targetY);
    stepperY.run();
  }
  
  if (digitalRead(Z_SENSER) == LOW) {
    stepperZ.stop();
    stepperZ.setCurrentPosition(0);
    stepperZ.moveTo(10);
    targetZ = 10;
    stepperZ.run();
  } else {
    stepperZ.moveTo(targetZ);
    stepperZ.run();
  }

  if (digitalRead(Z_BUTTON_L_HIGH) == LOW) {
    stepperLG.stop();
    stepperLG.setCurrentPosition(0);
    stepperLG.moveTo(200);
    targetLG = 200;
    stepperLG.run();
  } else {
    stepperLG.moveTo(targetLG);
    stepperLG.run();
  }

  if (digitalRead(Z_BUTTON_M_HIGH) == LOW) {
    if (z_m_button_flag) {
      stepperZ.stop();
      targetZ = stepperZ.currentPosition() - 30;
      Serial.println(targetZ);
      z_m_button_flag = false;
    }
  }

  if (digitalRead(Z_BUTTON_R_HIGH) == LOW) {
    stepperRG.stop();
    stepperRG.setCurrentPosition(0);
    stepperRG.moveTo(200);
    targetRG = 200;
    stepperRG.run();
  } else {
    stepperRG.moveTo(targetRG);
    stepperRG.run();
  }
}






