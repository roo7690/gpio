#include "../../abstract/abstract.h"
#include "../tasks/tasks.h"

#define DEFAULT_PIN 11
#define DEFAULT_DELAY 1000

void led(void *_opts);

typedef struct led_opt{
  struct start{
    A_gpio pin;
    int _pin;
    int delay_open;
    int delay_close;
  } start;
  int stop;
} led_opt;

void handler_signal(int signal);
void getOpts(led_opt *opts, int argc, char *argv[]);
void help(int error);