#define DEFAULT_PIN 0
#define DEFAULT_DELAY 1000
#define CACHE_DIR "/.cache"
#define CACHE "/.cache/led.pid"
#define CACHE_TMP "/.cache/led.tmp"

typedef struct led_opt{
  struct start{
    int pin;
    int delay_open;
    int delay_close;
  } start;
  int stop;
} led_opt;

void getOpts(led_opt *opts, int argc, char *argv[]);
void help(int error);
void handler_led(int signum);
void stop_led(int pin,int stop);
char *get_path(char *_path);