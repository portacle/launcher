#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>

int run(int argc, ...){
  char *argv[argc+1];
  
  va_list argp;
  va_start(argp, argc);
  for(int i=0; i<argc; ++i){
    argv[i] = va_arg(argp, char *);
  }
  va_end(argp);
  argv[argc] = 0;

  pid_t pid = vfork();
  if(pid < 0){
    return 0;
  }else if(pid == 0){
    return execv(argv[0], argv);
  }else{
    int status;
    waitpid(pid, &status, 0);
    return status;
  }
}

int add_font(char *file){
  char *name = basename(file);
  char *cache = "~/.fonts/";
  char target[3000];

  strcpy(target, cache);
  strcat(target, name);

  if(access(target, F_OK) == -1){
    run(3, "/usr/bin/mkdir", "-p", cache);
    run(4, "/usr/bin/cp", "-p", file, target);
    run(2, "/usr/bin/fc-cache", cache);
  }
  return 0;
}
