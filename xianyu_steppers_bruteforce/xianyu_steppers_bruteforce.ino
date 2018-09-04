#include "guard.h"


void setup()
{
  GND(23)
  GND(25)
  GND(27)
  preinit();
}

void loop()
{
  keep_alive();
}

