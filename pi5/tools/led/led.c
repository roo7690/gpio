#include "led.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include "regex.h"
#include "signal.h"

int running=1;

int main(int argc, char *argv[]){
  //recuperation des options
  led_opt opts;
  getOpts(&opts, argc, argv);
  if(wiringPiSetup()!=0){
    perror("Wiringpi setup echec");
    exit(1);
  }

  if(!opts.stop){
    task_stop(1,"led",opts.start._pin);
    task_run("led",opts.start._pin,led,&opts);
  }else{
    task_stop(0,"led",opts.start._pin);
  }
}

//task led
void led(void *_opts){
  led_opt *opts=(led_opt *)_opts;
  
  //listen signal
  if(signal(SIGTERM,handler_signal)==SIG_ERR){
    perror("erreur lors de la config d'un SIGTERM");
    exit(1);
  }
  
  //passage du gpio en mode output
  pinMode(opts->start.pin,OUTPUT);
  while(running){
    digitalWrite(opts->start.pin,HIGH);
    delay(opts->start.delay_open);
    digitalWrite(opts->start.pin,LOW);
    delay(opts->start.delay_close);
  }
  pinMode(opts->start.pin,INPUT);
  exit(0);
}

//handler de gestionnaire de signal
void handler_signal(int signal){
  running=0;
}

//Option de la task led
void getOpts(led_opt *opts, int argc, char *argv[]){
  char *cmd=argv[1];
  if(cmd==NULL){
    help(1);
    exit(1);
  }
  opts->start.pin = a_gpio(DEFAULT_PIN);
  //conserve le pin choisi
  opts->start._pin=DEFAULT_PIN;

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

    while ((opt = getopt_long(argc,argv,"p:d:",_opts,&opt_index)) != -1){
      switch (opt){
        case 'p':
          opts->start.pin = a_gpio(atoi(optarg));
          //conserve le pin choisi
          opts->start._pin=atoi(optarg);
          break;
        case 'd':
          regex_t regex;
          int ret=regcomp(&regex,"^[0-9]+\\+[0-9]+$",REG_EXTENDED);
          ret=regexec(&regex,optarg,0,NULL,0);
          regfree(&regex);
          if(ret){
            help(1);
            exit(1);
          }
          char *del_op=strtok(optarg,"+");
          char *del_cl=strtok(NULL,"+");
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
          opts->start._pin = atoi(optarg);
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

//help pour la task led
void help(int error){
  if(error){
    printf("\033[1;31mInvalid command\033[0m\n");
  }
  printf("Usage: led [start] [-p|--pin] <pin> [-d|--delay] <delay_open+delay_close> en ms\n   or: led [stop] [-p|--pin] <pin>\n");
}