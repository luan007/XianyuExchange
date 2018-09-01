#ifndef __MOTION_H__
#define __MOTION_H__

#define MOTOR_CLEAN 0x01        //R
#define MOTOR_MOVING 0x02       //R
#define MOTOR_LOCK 0x04         //RW
#define MOTOR_ENABLE_SLEEP 0x08 //RW
#include <Pin.h>                // Include Pin Library
#define set_motor_lock(i, l) l ? (motors[i].flags |= MOTOR_LOCK) : (motors[i].flags &= ~MOTOR_LOCK)

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

  float _stepInterval;
  float _lastStepTime;
  float __delta;

  unsigned char __step_hl;
};

motor_t motors[] = {
    {.PIN_STEP = Pin(2),
     .PIN_DIR = Pin(5),
     .PIN_EN = Pin(8),
     .PIN_RESET_SENSOR = Pin(14),
     .clearDirection = -1,
     .reset_hit = 1,
     .flags = MOTOR_ENABLE_SLEEP,
     .range = 300000,
     .position = 0,
     .target = 300000,

     ._speed = 1.0,
     ._maxSpeed = 20000,
     ._acceleration = 2000.0,
     .resetSpeed = 30000}};

void _compute_speed(motor_t *m)
{
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
  m->PIN_RESET_SENSOR.setInput();
  m->PIN_RESET_SENSOR.setPullupOn();
  m->PIN_STEP.setLow();
  m->PIN_DIR.setLow();
  m->PIN_EN.setLow();
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

  m->reset_hit = !m->PIN_RESET_SENSOR.getValue();

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
    m->target = m->target < 0 ? 0 : (m->target > m->range ? m->range : m->target);

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
      m->_stepInterval = abs(1000000.0 / m->resetSpeed);
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
        m->PIN_EN = 0; //enable the motor
        m->PIN_DIR = step > 0 ? 1 : 0;
        m->PIN_STEP = m->__step_hl; //tick
        if (!m->__step_hl)          //add one when we have falling edge
        {
          m->position += step;
        }
      }
      else
      {                //dirty, reset requested
        m->PIN_EN = 0; //enable the motor
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
      m->PIN_EN = HIGH;
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

int tick_motors()
{
  for (int i = 0; i < __motor_len; i++)
  {
    _tick_motor(&motors[i]);
  }
  return 0;
}

#endif