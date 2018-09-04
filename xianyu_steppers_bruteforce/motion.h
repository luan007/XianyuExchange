
#ifndef __MOTION_H__
#define __MOTION_H__

#define GND(p) pinMode(p, OUTPUT); digitalWrite(p, LOW);

#define MOTOR_Z 0
#define M_CLAW_L 1
#define M_CLAW_R 2
#define MOTOR_Y 3
#define MOTOR_X 4
#define MOTOR_BELT 5

#define MOTOR_CLEAN 0x01        //R
#define MOTOR_MOVING 0x02       //R
#define MOTOR_LOCK 0x04         //RW
#define MOTOR_ENABLE_SLEEP 0x08 //RW
#include <Pin.h>                // Include Pin Library

#define check_motor_flag(m, f) (m.flags & f)
#define enable_motor_flag(m, f) (m.flags |= f)
#define disable_motor_flag(m, f) (m.flags &= ~f)
#define check_motor_flag(m, f) (m.flags & f)

#define M_SLEEP(m) motors[m].PIN_EN.setHigh();
#define M_TARGET(m, to) motors[m].target = to; sanitize_target(&motors[m]);
#define M_ARRIVED(m) (motors[m].target == motors[m].position)

typedef struct motor_t
{
  Pin PIN_STEP;
  Pin PIN_DIR;
  Pin PIN_EN;
  Pin PIN_RESET_SENSOR;

  char clearDirection;
  unsigned char reset_hit;
  unsigned char flags;

  long range;
  long position;
  long target;

  float _speed;
  float _maxSpeed;
  float _acceleration;
  float _resetSpeed;
  bool invert_en;

  float _stepInterval;
  float _lastStepTime;
  float __delta;

  unsigned char __step_hl;

};

motor_t motors[] = {
  //Z
  { .PIN_STEP = Pin(41),
    .PIN_DIR = Pin(42),
    .PIN_EN = Pin(43),
    .PIN_RESET_SENSOR = Pin(34),
    .clearDirection = -1,
    .reset_hit = 1,
    .flags = 0,
    .range = 26000,
    .position = 0,
    .target = 10000,
    ._speed = 1.0,
    ._maxSpeed = 8000,
    ._acceleration = 3000.0,
    ._resetSpeed = 2500
  },

  //L
  { .PIN_STEP = Pin(2),
    .PIN_DIR = Pin(5),
    .PIN_EN = Pin(8),
    .PIN_RESET_SENSOR = Pin(22),
    .clearDirection = -1,
    .reset_hit = 1,
    .flags = 0,
    .range = 9000,
    .position = 0,
    .target = 0,
    ._speed = 1.0,
    ._maxSpeed = 20000,
    ._acceleration = 0,
    ._resetSpeed = 20000
  },

  //R
  { .PIN_STEP = Pin(3),
    .PIN_DIR = Pin(6),
    .PIN_EN = Pin(8),
    .PIN_RESET_SENSOR = Pin(26),
    .clearDirection = -1,
    .reset_hit = 1,
    .flags = 0,
    .range = 9000,
    .position = 0,
    .target = 0,
    ._speed = 1.0,
    ._maxSpeed = 20000,
    ._acceleration = 0,
    ._resetSpeed = 20000
  },


  //Y
  { .PIN_STEP = Pin(38),
    .PIN_DIR = Pin(39),
    .PIN_EN = Pin(40),
    .PIN_RESET_SENSOR = Pin(33),
    .clearDirection = -1,
    .reset_hit = 1,
    .flags = 0,
    .range = 11500,
    .position = 0,
    .target = 0,
    ._speed = 1.0,
    ._maxSpeed = 8000,
    ._acceleration = 1000.0,
    ._resetSpeed = 1500
  },


  //X
  { .PIN_STEP = Pin(35),
    .PIN_DIR = Pin(36),
    .PIN_EN = Pin(37),
    .PIN_RESET_SENSOR = Pin(32),
    .clearDirection = -1,
    .reset_hit = 1,
    .flags = 0,
    .range = 11000,
    .position = 0,
    .target = 0,
    ._speed = 1.0,
    ._maxSpeed = 8000,
    ._acceleration = 1000.0,
    ._resetSpeed = 1500
  },


  //BELT
  { .PIN_STEP = Pin(44),
    .PIN_DIR = Pin(45),
    .PIN_EN = Pin(46),
    .PIN_RESET_SENSOR = NULL,
    .clearDirection = -1,
    .reset_hit = 0,
    .flags = MOTOR_CLEAN,
    .range = 0,
    .position = 0,
    .target = 0,
    ._speed = 1.0,
    ._maxSpeed = 500,
    ._acceleration = 0,
    ._resetSpeed = 500,
    .invert_en = true
  }
};

inline void sanitize_target(motor_t *m) {
  if(m->range != 0) {
    m->target = m->target < 0 ? 0 : (m->target > m->range ? m->range : m->target);
  }
}

void _compute_speed(motor_t *m)
{
  if (m->_acceleration == 0)  {
    m->_speed = m->_maxSpeed;
    m->_stepInterval = abs(1000000.0 / m->_speed);
    return;
  }
  long distanceTo = m->__delta;
  float requiredSpeed;
  if (distanceTo == 0)
    return 0.0;            // Were there
  else if (distanceTo > 0) // Clockwise
    requiredSpeed = sqrt(2.0 * distanceTo * m->_acceleration);
  else // Anticlockwise
    requiredSpeed = -sqrt(2.0 * -distanceTo * m->_acceleration);

  if (requiredSpeed > m->_speed)
  {
    // Need to accelerate in clockwise direction
    if (m->_speed == 0)
      requiredSpeed = sqrt(2.0 * m->_acceleration);
    else
      requiredSpeed = m->_speed + abs(m->_acceleration / m->_speed);
    if (requiredSpeed > m->_maxSpeed)
      requiredSpeed = m->_maxSpeed;
  }
  else if (requiredSpeed < m->_speed)
  {
    // Need to accelerate in anticlockwise direction
    if (m->_speed == 0)
      requiredSpeed = -sqrt(2.0 * m->_acceleration);
    else
      requiredSpeed = m->_speed - abs(m->_acceleration / m->_speed);
    if (requiredSpeed < -m->_maxSpeed)
      requiredSpeed = -m->_maxSpeed;
  }

  m->_speed = requiredSpeed;
  m->_stepInterval = abs(1000000.0 / m->_speed);
  //  Serial.println(requiredSpeed);
  //  return requiredSpeed;
}

int _initialize_motor(motor_t *m)
{
  m->_lastStepTime = 0;
  m->_stepInterval = 1;
  m->__delta = 0; //safe to init
  m->PIN_STEP.setOutput();
  m->PIN_DIR.setOutput();
  m->PIN_EN.setOutput();
  if (m->PIN_RESET_SENSOR != NULL) {
    m->PIN_RESET_SENSOR.setInput();
    m->PIN_RESET_SENSOR.setPullupOn();
  }
  m->PIN_STEP.setLow();
  m->PIN_DIR.setLow();
  if(!m->invert_en) {
    m->PIN_EN.setHigh();
  } else {
    m->PIN_EN.setLow();
  }
  m->__step_hl = 0;
  //  pinMode(ms->PIN_STEP, OUTPUT);
  //  pinMode(ms->PIN_DIR, OUTPUT);
  //  pinMode(ms->PIN_EN, OUTPUT);
  //  pinMode(ms->PIN_RESET_SENSOR, INPUT_PULLUP);
  //  digitalWrite(ms->PIN_EN, LOW);
  //  digitalWrite(ms->PIN_STEP, LOW);
  //  digitalWrite(ms->PIN_DIR, LOW);
  return 0;
}

void _log_motor(int index, int milli)
{
  static long lastCalled = 0;
  if (millis() - lastCalled < milli)
  {
    return;
  }
  lastCalled = millis();
  motor_t *m = &motors[index];
  Serial.print("M");
  Serial.print(index);
  Serial.print(" (");
  Serial.print((m->flags & MOTOR_CLEAN) ? "OK" : "?");
  Serial.print((m->flags & MOTOR_MOVING) ? " ACT" : "");
  Serial.print((m->flags & MOTOR_LOCK) ? " LOCK" : "");
  Serial.print((m->flags & MOTOR_ENABLE_SLEEP) ? " SLP" : "");
  Serial.print(") ");
  Serial.print(m->position);
  Serial.print("/");
  Serial.print(m->range);
  Serial.print(" ->");
  Serial.print(m->target);
  Serial.print(" @");
  Serial.println(m->_speed);
  return;
}

//danger - this should not be called directly
int _tick_motor(motor_t *m)
{
  int ret = 0;
  char step = 0; //default: no motion
  
  //tick down anyways
  if (m->PIN_RESET_SENSOR != NULL) {
    m->reset_hit = !m->PIN_RESET_SENSOR.getValue();
  }
  if (m->reset_hit)
  {
    //lets reset everything
    m->__step_hl = 0;
    //ensure we have low here
    m->PIN_STEP = m->__step_hl; //tick

    //good we're at reset position
    m->flags |= MOTOR_CLEAN; //clean yay
    m->position = 0;         //current pos = 0
  }

  //motor clean (reset)?
  if (m->flags & MOTOR_CLEAN)
  {
    //sanitize target
    sanitize_target(m);
    //calculate delta
    m->__delta = m->target - m->position;

    //TODO: insert speed calculation here maybe?
    if (m->__delta > 0)
    {
      step = 1; //move OUT
    }
    else if (m->__delta < 0)
    {
      step = -1; //move IN
    }
  }
  else
  {
    step = m->clearDirection; //move back till we hit clear
  }

  //before we actually move, last check machine lock - *important safety measures
  if (m->flags & MOTOR_LOCK)
  {
    ret = -2;
    m->_speed = 1.0;
    m->_stepInterval = 0;
    step = 0;
  }

  if (step != 0)
  {
    unsigned long time = micros();
    //tick duty
    m->flags |= MOTOR_MOVING;
    //    m->_tick_speed = abs(20 * sin((float)millis() / 1000.0f)) * 30000 + 30000;
    //check reset
    int _need_reset = !(m->flags & MOTOR_CLEAN);
    //adapt
    if (_need_reset)
    {
      m->_stepInterval = abs(1000000.0 / m->_resetSpeed);
    }
    //    _need_reset ? (m->_reset_tick_interval = _tick_interval) : (m->_tick_interval = _tick_interval) ;
    if (time > m->_lastStepTime + m->_stepInterval)
    {
      if (!_need_reset)
      {
        _compute_speed(m);
      }
      m->_lastStepTime = time;
      m->__step_hl = !m->__step_hl; //quicker as IO operations're slow

      if (!_need_reset)
      {
        m->PIN_EN = m->invert_en ? 1 : 0; //enable the motor
        m->PIN_DIR = step > 0 ? 1 : 0;
        m->PIN_STEP = m->__step_hl; //tick
        if (!m->__step_hl)          //add one when we have falling edge
        {
          m->position += step;
        }
      }
      else
      { //dirty, reset requested
        m->PIN_EN = m->invert_en ? 1 : 0; //enable the motor
        m->PIN_DIR = step > 0 ? 1 : 0;
        m->PIN_STEP = m->__step_hl; //tick
        m->position = -1;           //invalid position
      }
    }
  }
  //if no move already
  else
  {
    //clear move bit
    m->flags &= ~MOTOR_MOVING;
    //check if we do sleep
    if (m->flags & MOTOR_ENABLE_SLEEP && (!(m->flags & MOTOR_LOCK)))
    {
      m->PIN_EN = m->invert_en ? LOW : HIGH;
    }
  }
  return ret;
}

//exports

int __motor_len = 0;
int init_motors()
{
  __motor_len = sizeof(motors) / sizeof(motors[0]);
  for (int i = 0; i < __motor_len; i++)
  {
    _initialize_motor(&motors[i]);
  }
  return 0;
}

int set_flags_to_all_motors(int en, int flag)
{
  __motor_len = sizeof(motors) / sizeof(motors[0]);
  for (int i = 0; i < __motor_len; i++)
  {
    if (en) {
      enable_motor_flag(motors[i], flag);
    } else {
      disable_motor_flag(motors[i], flag);
    }
  }
  return 0;
}

int tick_motors()
{
  for (int i = 0; i < __motor_len; i++)
  {
    _tick_motor(&motors[i]);
  }
  return 0;
}

#define M_LOCK_ALL() set_flags_to_all_motors(1, MOTOR_LOCK);
#define M_ACTIVATE(x) disable_motor_flag(motors[x], MOTOR_LOCK);
#define M_LOCK(x) enable_motor_flag(motors[x], MOTOR_LOCK);

#endif
