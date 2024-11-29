#include "../abstract/abstract.h"

#define DEFAULT_PIN 11
#define DEFAULT_DELAY 1000
#define CACHE_DIR "/.cache"
#define CACHE "/.cache/led.pid"
#define CACHE_TMP "/.cache/led.tmp"

typedef struct led_opt{
  struct start{
    A_gpio pin;
    int _pin;
    int delay_open;
    int delay_close;
  } start;
  int stop;
} led_opt;

void getOpts(led_opt *opts, int argc, char *argv[]);
void help(int error);
char *get_path(char *_path);

void stop_led(int pin,int stop);
void handler_led(int signum);