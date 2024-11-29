#define pico 0

#if pico == 0
  #include "wiringPi.h"
#endif

#include "./gpio/gpio.h"
#include "./sdk/sdk.h"