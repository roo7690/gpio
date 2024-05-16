#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include "regex.h"
#include "signal.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "wiringPi.h"

#define DEFAULT_PIN 0
#define DEFAULT_DELAY 1000
#define CACHE_DIR "/.cache"
#define CACHE "/.cache/led.pid"
#define CACHE_TMP "/.cache/led.tmp"

volatile int led_running=1;

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

int main(int argc, char *argv[]){
  if(wiringPiSetup()==-1){
    printf("\033[1;31mErreur: wiringPiSetup a échoué\033[0m\n");
    return 1;
  }

  led_opt opts;
  getOpts(&opts, argc, argv);

  if(!opts.stop){
    stop_led(opts.start.pin,0);
    pinMode(opts.start.pin,OUTPUT);
    signal(SIGUSR1,handler_led);
    pid_t pid=fork();
    if(pid==0){
      while(led_running){
        digitalWrite(opts.start.pin,HIGH);
        delay(opts.start.delay_open);
        digitalWrite(opts.start.pin,LOW);
        delay(opts.start.delay_close);
      }
    }else if(pid>0){
      char *cacheD=get_path(CACHE_DIR);
      mkdir(cacheD,S_IRWXU);
      char *cache=get_path(CACHE);
      FILE *_led=fopen(cache,"a");
      fprintf(_led,"%d=>%d\n",opts.start.pin,pid);
      fclose(_led);
      const char *info="\033[1;32mLED allumé\033[0m\nPin: %d\nDelai d'allumage: %d\nDelai d'arrêt: %d\n";
      printf(info, opts.start.pin,opts.start.delay_open,opts.start.delay_close);
    }else{
      printf("\033[1;31mErreur: fork a échoué\033[0m\n");
      exit(1);
    }
  }else{
    stop_led(opts.start.pin,1);
  }

  return 0;
}

void getOpts(led_opt *opts, int argc, char *argv[]){
  char *cmd=argv[1];
  if(cmd==NULL){
    help(1);
    exit(1);
  }
  opts->start.pin = DEFAULT_PIN;

  if(strcmp(cmd,"start")==0){
    opts->start.delay_open = DEFAULT_DELAY;
    opts->start.delay_close = DEFAULT_DELAY;
    opts->stop = 0;

    int opt;
    int opt_index=0;
    static struct option _opts[]={
      {"pin", required_argument, 0, 'p'},
      {"delay", required_argument, 0, 'd'},
      {0,0,0,0}
    };                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   

    while ((opt = getopt_long(argc,argv,"pd:",_opts,&opt_index)) != -1){
      switch (opt){
        case 'p':
          opts->start.pin = atoi(optarg);
          break;
        case 'd':
          regex_t regex;
          int ret=regcomp(&regex,"[0-9]+-[0-9]+",REG_EXTENDED);
          ret=regexec(&regex,optarg,0,NULL,0);
          regfree(&regex);
          if(ret){
            help(1);
            exit(1);
          }
          char *del_op=strtok(optarg,"-");
          char *del_cl=strtok(NULL,"-");
          opts->start.delay_open = atoi(del_op);
          opts->start.delay_close = atoi(del_cl);
          break;
        default:
          help(1);
          exit(1);
      }
    }
  }else if(strcmp(cmd,"stop")==0){
    opts->stop = 1;
    int opt;
    int opt_index=0;
    static struct option _opts[]={
      {"pin",required_argument,0,'p'},
      {0,0,0,0}
    };

    while((opt = getopt_long(argc,argv,"p:",_opts,&opt_index)) != -1){
      switch (opt){
        case 'p':
          opts->start.pin = atoi(optarg);
          break;
        default:
          help(1);
          exit(1);
      }
    }
  }else if(strcmp(cmd,"-h")==0 || strcmp(cmd,"--help")==0){
    help(0);
    exit(0);
  }else{
    help(1);
    exit(1);
  }
}

void help(int error){
  if(error){
    printf("\033[1;31mInvalid command\033[0m\n");
  }
  printf("Usage: led [start] [-p|--pin] <pin> [-d|--delay] <delay_open-delay_close> en ms\n   or: led [stop] [-p|--pin] <pin>\n");
}

void handler_led(int signum){
  if(signum==SIGUSR1){
    led_running=0;
  }
}

void stop_led(int pin,int stop){
  char *cache=get_path(CACHE);
  FILE *_led=fopen(cache,"r");
  if(_led==NULL){
    if(stop){
      printf("\033[0;31mAucune LED n'est allumée\033[0m\n");
    }
    return;
  }
  char line[15];
  int trouve=0;
  pid_t pid;
  char *cache_tmp=get_path(CACHE_TMP);
  FILE *_tmp=fopen(cache_tmp,"a");
  while(fgets(line,15,_led)!=NULL){
    char *_pin=strtok(line,"=>");
    if(atoi(_pin)==pin){
      trouve=1;
      pid=atoi(strtok(NULL,"=>"));
    }else{
      fprintf(_tmp,"%s",line);
    }
  }
  fclose(_led);
  fclose(_tmp);
  remove(cache);
  rename(cache_tmp,cache);
  if(!trouve){
    if(stop){
      printf("\033[0;31mAucune LED n'est allumée au pin %d\033[0m\n",pin);
    }
    return;
  }
  if(kill(pid,SIGUSR1)==-1){
    perror("kill");
    exit(1);
  }
  pinMode(pin,INPUT);
  if(stop){
    printf("\033[1;32mLED éteint\033[0m\n");
  }
}

char *get_path(char *_path){
  char *wf=getenv("wf");
  char *path=malloc(strlen(wf)+strlen(_path)+1);
  strcpy(path,wf);
  strcat(path,_path);
  return path;
}