#include "stdbool.h"

typedef int A_gpio;

A_gpio a_gpio(int pin);
int initGpio();
void set_mode_gpio(A_gpio pin,bool mode);
void set_state_gpio(A_gpio pin,bool state);