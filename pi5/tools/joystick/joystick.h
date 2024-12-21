#include "../../abstract/abstract.h"
#include "../tasks/tasks.h"
#include "stdint.h"

#define DEFAULT_PERIOD_VIEW 2

typedef struct joystick_opt{
  struct start{
    int pin;
    int _pin;
    int cs;
    int _cs;
    int period;
    struct joy{
      int id;
      struct joy *next;
    } *joy;
  } start;
  int stop;
} joystick_opt;

typedef struct joystick_do{
  __uint8_t x;__uint8_t y;__uint8_t z;__uint8_t r;
} joystick_do;

void getJoystick(void *data);
void viewJoystick(void *data);
void handler_signal(int signal);
void getOpts(joystick_opt *opts,int argc,char *argv[]);
void help(int error);