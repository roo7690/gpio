#include "joystick.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include "signal.h"
#include "sys/shm.h"
#include "sys/ipc.h"

int running=1;
int shared_memory=0;

int main(int argc,char *argv[]){
  joystick_opt opts;
  getOpts(&opts,argc,argv);
  int id=opts.start._pin*100+opts.start._cs;
  //creation d'une memoire partage pour stocker data out
  shared_memory=shmget(opts.start._pin,sizeof(joystick_do),IPC_CREAT|0600);
  if(shared_memory<0){
    perror("Erreur a la creation de la memoire partage");
    exit(1);
  }

  if(wiringPiSetup()!=0){
    perror("Wiringpi setup echec");
    exit(1);
  }

  if(!opts.stop){
    task_stop(1,"joystick",id);
    task_stop(1,"joystick",id*100);
    task_run("joystick",id,getJoystick,&opts);
    task_run("joystick",id*100,viewJoystick,&id);
  }else{
    task_stop(0,"joystick",id);
    task_stop(0,"joystick",id*100);
  }
}

//task recuperer les data joystick
void getJoystick(void *data){
  joystick_opt *opts=(joystick_opt *)data;

  //listen signal
  if(signal(SIGTERM,handler_signal)==SIG_ERR){
    perror("erreur lors de la config d'un SIGTERM");
    exit(1);
  }

  struct joy *currjoy=opts->start.joy;
  joystick_do *data_out=(joystick_do *)shmat(shared_memory,NULL,0);
  if(data_out==(joystick_do *)-1){
    perror("Erreur lors de l'attachement de la memoire partage");
    exit(1);
  }

  pinMode(opts->start.cs,OUTPUT);
  digitalWrite(opts->start.cs,HIGH);
  delay(0);
  while(running){
    digitalWrite(opts->start.cs,LOW);
    //selection de l'entree
    int input[4];
    switch(currjoy->id){
      case 0://ch0
        *input=*(int []){HIGH,HIGH,LOW,LOW};
        break;
      case 1://ch1
        *input=*(int []){HIGH,HIGH,HIGH,LOW};
        break;
      case 2://ch2
        *input=*(int []){HIGH,HIGH,LOW,HIGH};
        break;
      case 3://ch3
        *input=*(int []){HIGH,HIGH,HIGH,HIGH};
        break;
    }
    pinMode(opts->start.pin,OUTPUT);
    digitalWrite(opts->start.pin,input[0]);
    delay(opts->start.period);
    digitalWrite(opts->start.pin,input[1]);
    delay(opts->start.period);
    digitalWrite(opts->start.pin,input[2]);
    delay(opts->start.period);        
    digitalWrite(opts->start.pin,input[3]);

    //wait conversion
    pinMode(opts->start.pin,INPUT);
    delay(1.5*opts->start.period);
    //get
    __uint8_t bits=0;
    for(int i=0;i<=7;i++){
      int bit=digitalRead(opts->start.pin);
      bits=bits<<1|bit;
      delay(opts->start.period);
    }
    //stock
    switch(currjoy->id){
      case 0://ch0
        data_out->x=bits;
        break;
      case 1://ch1
        data_out->y=bits;
        break;
      case 2://ch2
        data_out->z=bits;
        break;
      case 3://ch3
        data_out->r=bits;
        break;
    }

    currjoy=currjoy->next;

    digitalWrite(opts->start.cs,HIGH);
    delay(0);
  }

  pinMode(opts->start.pin,INPUT);
  pinMode(opts->start.cs,INPUT);
  exit(0);
}

//task view les data joystick
void viewJoystick(void *data){
  int *id=(int *)data;
  //listen signal
  if(signal(SIGTERM,handler_signal)==SIG_ERR){
    perror("erreur lors de la config d'un SIGTERM");
    exit(1);
  }

  //recup memoire shared
  joystick_do *data_out=(joystick_do *)shmat(shared_memory,NULL,0);
  if(data_out==(joystick_do *)-1){
    perror("Erreur lors de l'attachement de la memoire partage");
    exit(1);
  }

  //view
  char *cache_dir=get_path(CACHE_DIR);
  char *view_file=malloc(strlen(cache_dir)+19);
  char *_id=malloc(5);
  if(view_file==NULL||id==NULL){
    perror("Erreur d'allocation de memoire");
    exit(1);
  }
  snprintf(_id,5,"%d",*id);
  strcpy(view_file,cache_dir);
  strcat(view_file,"joystick.");
  strcat(view_file,_id);
  strcat(view_file,".view");
  FILE *view_f=fopen(view_file,"w");
  while(running){
    fseek(view_f,0,0);
    fprintf(view_f,
      "x=> %d\ny=> %d\nz=> %d\nr=> %d",
      data_out->x,data_out->y,data_out->z,data_out->r);
    delay(DEFAULT_PERIOD_VIEW);
  }

  fclose(view_f);
  remove(view_file);
  exit(0);
}

//handler de gestionnaire de signal
void handler_signal(int signal){
  running=0;
}

//Options de la task joystick
void getOpts(joystick_opt *opts, int argc, char *argv[]){
  char *cmd=argv[1];
  if(cmd==NULL){
    help(1);
    exit(1);
  }
  if(strcmp(cmd,"start")==0){
    opts->stop=0;

    int opt;
    int *opt_index=0;
    int p=0;int c=0;int f=0;int ch=0;
    static struct option _opts[]={
      {"pin",required_argument,0,'p'},
      {"cs",required_argument,0,'c'},
      {"frequence",required_argument,0,'f'},
      {"ch0",no_argument,0,'x'},
      {"ch1",no_argument,0,'y'},
      {"ch2",no_argument,0,'z'},
      {"ch3",no_argument,0,'r'},
      {0,0,0,0}
    };

    struct joy *currentjoy=malloc(sizeof(struct joy));
    if(currentjoy==NULL){
      perror("Erreur d'allocation");
      exit(1);
    }
    currentjoy->id=0;
    opts->start.joy=currentjoy;

    while((opt=getopt_long(argc,argv,"p:c:f:xyzr",_opts,opt_index))!=-1){
      switch(opt){
        case 'p':
          p=1;
          opts->start._pin=atoi(optarg);
          opts->start.pin=a_gpio(atoi(optarg));
          break;
        case 'c':
          c=1;
          opts->start._cs=atoi(optarg);
          opts->start.cs=a_gpio(atoi(optarg));
          break;
        case 'f':
          f=1;
          opts->start.period=atoi(optarg);
          break;
        case 'x':
          ch=1;
          currentjoy->id=0;
          struct joy *nextjoy0=malloc(sizeof(struct joy));
          if(nextjoy0==NULL){
            perror("Erreur d'allocation");
            exit(1);
          }
          currentjoy->next=nextjoy0;
          currentjoy=nextjoy0;
          break;
        case 'y':
          ch=1;
          currentjoy->id=1;
          struct joy *nextjoy1=malloc(sizeof(struct joy));
          if(nextjoy1==NULL){
            perror("Erreur d'allocation");
            exit(1);
          }
          currentjoy->next=nextjoy1;
          currentjoy=nextjoy1;
          break;
        case 'z':
          ch=1;
          currentjoy->id=2;
          struct joy *nextjoy2=malloc(sizeof(struct joy));
          if(nextjoy2==NULL){
            perror("Erreur d'allocation");
            exit(1);
          }
          currentjoy->next=nextjoy2;
          currentjoy=nextjoy2;
          break;
        case 'r':
          ch=1;
          currentjoy->id=3;
          struct joy *nextjoy3=malloc(sizeof(struct joy));
          if(nextjoy3==NULL){
            perror("Erreur d'allocation");
            exit(1);
          }
          currentjoy->next=nextjoy3;
          currentjoy=nextjoy3;
          break;
        default:
          help(1);
          exit(1);
      }
    }

    if(!ch||!p||!c||!f){
      help(1);
      exit(1);
    }

    *currentjoy=*opts->start.joy;
    free(opts->start.joy);
    opts->start.joy=currentjoy;
  }else if(strcmp(cmd,"stop")==0){
    opts->stop = 1;
    int opt;
    int opt_index=0;
    int p=0;int c=0;
    static struct option _opts[]={
      {"pin",required_argument,0,'p'},
      {"cs",required_argument,0,'c'},
      {0,0,0,0}
    };

    while((opt = getopt_long(argc,argv,"p:c:",_opts,&opt_index)) != -1){
      switch (opt){
        case 'p':
          p=1;
          opts->start._pin=atoi(optarg);
          break;
        case 'c':
          c=1;
          opts->start._cs=atoi(optarg);
          break;
        default:
          help(1);
          exit(1);
      }
    }
    if(p==0||c==0){
      help(1);
      exit(1);
    }
  }else if(strcmp(cmd,"-h")==0||strcmp(cmd,"--help")==0){
    help(0);
    exit(0);
  }else{
    help(1);
    exit(1);
  }
}

//help pour la task joystick
void help(int error){
  if(error){
    printf("\033[1;31mInvalid command\033[0m\n");
  }
  printf("Usage: joystick [start] [-p|--pin] <pin> [-c|--cs] <pin du chip select> [-f|--frequence] <period du clock> en ms\n      [-x|--ch0] [-y|--ch1] [-z|--ch2] [-r|--ch3], indique le(s) entree(s) dans l'ordre mentionne (! au moins 1 requis)\n   or: joystick [stop] [-p|--pin] <pin>\n");
}