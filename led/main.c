#include "led.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"

volatile int led_running=1;

int main(int argc, char *argv[]){
  if(initGpio()==-1){
    printf("\033[1;31mErreur: initiation du sdk échoué\033[0m\n");
    return 1;
  }
  //recuperation des options
  led_opt opts;
  getOpts(&opts, argc, argv);

  if(!opts.stop){
    //reboot
    stop_led(opts.start._pin,0);
    //passage du gpio en mode output
    set_mode_gpio(opts.start.pin,OUTPUT);
    //add ecoute de signal
    signal(SIGUSR1,handler_led);
    //copie le process en cours dans un autre thread
    //et les differencie sous pid
    pid_t pid=fork();
    if(pid==0){
      //lancement de led
      while(led_running){
        set_state_gpio(opts.start.pin,HIGH);
        wait_ms(opts.start.delay_open);
        set_state_gpio(opts.start.pin,LOW);
        wait_ms(opts.start.delay_close);
      }
    }else if(pid>0){
      //enregistre le thread sur lequel la led est lancee.
      //et informe son lancement puis fin de commande
      char *cacheD=get_path(CACHE_DIR);
      mkdir(cacheD,S_IRWXU);
      char *cache=get_path(CACHE);
      FILE *_led=fopen(cache,"a");
      fprintf(_led,"%d=>%d\n",opts.start._pin,pid);
      fclose(_led);
      const char *info="\033[1;32mLED allumé\033[0m\nPin: %d\nDelai d'allumage: %d\nDelai d'arrêt: %d\n";
      printf(info, opts.start._pin,opts.start.delay_open,opts.start.delay_close);
    }else{
      printf("\033[1;31mErreur: fork a échoué\033[0m\n");
      exit(1);
    }
  }else{
    stop_led(opts.start._pin,1);
  }

  return 0;
}

//gestionnaire de signal
void handler_led(int signum){
  if(signum==SIGUSR1){
    //stop l'execution de la led
    led_running=0;
  }
}

//stop la led (ne signal rien , s'il s'agit d'un reboot)
void stop_led(int pin,int stop){
  //recuperer le fichier de cache
  char *cache=get_path(CACHE);
  FILE *_led=fopen(cache,"r");
  if(_led==NULL){
    if(stop){
      printf("\033[0;31mAucune LED n'est allumée\033[0m\n");
    }
    return;
  }
  //recuperer le thread sur lequel la led s'execute
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
  //envoyer le signal personnalise `SIGUSR1` au thread
  if(kill(pid,SIGUSR1)==-1){
    perror("kill");
    exit(1);
  }
  //remettre le gpio utilise a l'etat input
  set_mode_gpio(pin,INPUT);
  if(stop){
    printf("\033[1;32mLED éteint\033[0m\n");
  }
}

//recuperation du chemin d'un fichier
char *get_path(char *_path){
  char *wf=getenv("wfm");
  char *path=malloc(strlen(wf)+strlen(_path)+1);
  strcpy(path,wf);
  strcat(path,_path);
  return path;
}