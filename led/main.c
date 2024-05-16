#include "led.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "wiringPi.h"

volatile int led_running=1;

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
  char *line=malloc(15);
  int trouve=0;
  pid_t pid;
  char *cache_tmp=get_path(CACHE_TMP);
  FILE *_tmp=fopen(cache_tmp,"a");
  while(fgets(line,15,_led)!=NULL){
    char *_line=malloc(strlen(line));
    strcpy(_line,line);
    char *_pin=strtok(line,"=>");
    if(atoi(_pin)==pin){
      trouve=1;
      pid=atoi(strtok(NULL,"=>"));
    }else{
      fprintf(_tmp,"%s",_line);
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