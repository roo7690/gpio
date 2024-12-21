#define WORKSPACES "WFM"
#define CACHE_DIR "/.cache/"
#define PID_EXT ".pid"
#define CACHE_SIZE_LINE 25

char *get_path(char *_path);
void task_run(char *task_name,int id,void (*task)(void *_param),void *param);
void task_stop(int reboot,char *task_name,int id);