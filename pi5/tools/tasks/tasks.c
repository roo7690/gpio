#include "tasks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "unistd.h"
#include "sys/stat.h"
#include "signal.h"

//recuperation du chemin d'un fichier
char *get_path(char *_path){
  char *wf=getenv(WORKSPACES);
  if(wf==NULL){
    fprintf(stderr,"La variable d'environnement %s est introuvable.\nCela specifie le workspaces, le dossier .cache s'y instalera.",WORKSPACES);
    exit(1);
  }
  char *path=malloc(strlen(wf)+strlen(_path)+1);
  if(path==NULL){
    perror("Erreur d'allocation de memoire\n");
    exit(1);
  }
  strcpy(path,wf);
  strcat(path,_path);
  return path;
}

//lancement d'une task
void task_run(char *task_name,int id,void (*task)(void *_param),void *param){
  pid_t pid=fork();
  if(pid==0){
    //exec de la task
    task(param);
  }else if(pid>0){
    //enregistre le task en cache
    char *cache_dir=get_path(CACHE_DIR);
    mkdir(cache_dir,S_IRWXU);
    char *cache_file=malloc(strlen(cache_dir)+strlen(task_name)+strlen(PID_EXT)+1);
    if(cache_file==NULL){
      perror("Erreur d'allocation de memoire\n");
      exit(1);
    }
    strcpy(cache_file,cache_dir);
    strcat(cache_file,task_name);
    strcat(cache_file,PID_EXT);
    FILE *cache=fopen(cache_file,"a");
    fprintf(cache,"%d=>%d\n",id,pid);
    fclose(cache);
  }else{
    fprintf(stderr,"fork a echoue\n");
    exit(1);
  }
}

//arret d'une task
void task_stop(int reboot,char *task_name,int id){
  char *cache_dir=get_path(CACHE_DIR);
  char *cache_file=malloc(strlen(cache_dir)+strlen(task_name)+strlen(PID_EXT)+1);
  if(cache_file==NULL){
    perror("Erreur d'allocation de memoire\n");
    exit(1);
  }
  strcpy(cache_file,cache_dir);
  strcat(cache_file,task_name);
  strcat(cache_file,PID_EXT);
  FILE *cache=fopen(cache_file,"r");
  if(cache==NULL){
    if(!reboot){
      fprintf(stderr,"Aucune task %s trouve",task_name);
    }
  }else{
    //recuperer le thread sur lequel la task s'execute
    char *line=malloc(CACHE_SIZE_LINE);
    int trouve=0;
    pid_t pid;
    char *cache_tmp=malloc(strlen(cache_file)+1);
    if(cache_tmp==NULL){
      perror("Erreur d'allocation de memoire\n");
      exit(1);
    }
    strcpy(cache_tmp,cache_dir);
    strcat(cache_tmp,".");
    strcat(cache_tmp,task_name);
    strcat(cache_tmp,PID_EXT);
    FILE *_tmp=fopen(cache_tmp,"a");
    while(fgets(line,CACHE_SIZE_LINE,cache)!=NULL){
      char *_line=malloc(strlen(line));
      if(_line==NULL){
        perror("Erreur d'allocation de memoire\n");
        exit(1);
      }
      strcpy(_line,line);
      char *_id=strtok(line,"=>");
      if(atoi(_id)==id){
        trouve=1;
        pid=atoi(strtok(NULL,"=>"));
      }else{
        fprintf(_tmp,"%s",_line);
      }
    }
    fclose(cache);
    fclose(_tmp);
    remove(cache_file);
    rename(cache_tmp,cache_file);
    if(trouve){
      if(kill(pid,SIGTERM)!=0){
        fprintf(stderr,"La task %s id: %d n'a pas ete arrete\n",task_name,id);
      }
    }
  }
}