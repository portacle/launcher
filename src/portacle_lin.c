#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LIN 1
#define PLATFORM "lin"
#define LIBRARY_VAR "LD_LIBRARY_PATH"
#define PATHSEP "/"
#define PATHLEN PATH_MAX // Not actually accurate, but w/e
#define VARLEN 32767     // Not actually accurate, but w/e
#define VARSEP ":"

int path_up(char *path){
  char *rpath = dirname(path);
  if(rpath == 0)
    return 0;
  strcpy(path, rpath);
  return 1;
}

int exe_dir(char *path){
  if(readlink("/proc/self/exe", path, PATHLEN) < 0)
    return 0;
  if(!path_up(path)) return 0;
  return 1;
}

int app_name(char *argv0, char *name){
  char *rname = basename(argv0);
  if(rname == 0)
    return 0;
  strcpy(name, rname);
  return 1;
}

int set_env(char *name, char *value){
  if(setenv(name, value, 1) == 0)
    return 1;
  return 0;
}

int get_env(char *name, char *value){
  char *rvalue = getenv(name);
  if(rvalue == NULL) return 0;
  strcpy(value, rvalue);
  return 1;
}

int execpath(char *root, char *nameish, char *target){
  strcpy(target, root);
  strcat(target, PATHSEP);
  strcat(target, nameish);
  return 1;
}

int launch(char *path, int argc, char **argv){  
  extern char **environ;
  char *rargv[argc+1];
  for(int i=0; i<argc; ++i){
    rargv[i] = argv[i];
  }
  rargv[argc] = 0;

  char debug[VARLEN]={0};
  get_env("PORTACLE_DEBUG", debug);
  if(debug[0] != 0){
    fprintf(stderr, "\n  Launching executable: %s\n", path);
    for(int i=0; rargv[i]; ++i)
      fprintf(stderr, "argv%i %s\n", i, rargv[i]);
    fprintf(stderr, "\n");
  }
  
  if(execve(path, rargv, environ) < 0)
    return 0;
  return 1;
}

int launch_ld(char *path, int argc, char **argv){
  char libpath[PATHLEN], loader[PATHLEN];
  get_env("LW_LIBRARY_PATH", libpath);
  get_env("LW_LOADER_PATH", loader);
  
  char *rargv[argc+3];
  rargv[0] = argv[0];
  rargv[1] = "--library-path";
  rargv[2] = libpath;
  rargv[3] = path;
  for(int i=1; i<argc; ++i){
    rargv[i+3] = argv[i];
  }
  
  return launch(loader, argc+3, rargv);
}

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
    execvp(argv[0], argv);
    fprintf(stderr, "Exec for %s failed: %s\n", argv[0], strerror(errno));
    exit(0);
  }else{
    int status;
    waitpid(pid, &status, 0);
    return !status;
  }
}

int add_font(char *file){
  char *name = basename(file);
  char *home = getenv("HOME");
  char target[PATHLEN] = {0}, fonts[PATHLEN] = {0};

  strcpy(fonts, home);
  strcat(fonts, "/.local/share/fonts/");
  strcpy(target, fonts);
  strcat(target, name);
  
  if(access(target, F_OK) == -1){
    if(!run(3, "mkdir", "-p", fonts))
      return 0;
    if(!run(4, "cp", "-p", file, target))
      return 0;
  }
  return 1;
}

int reg_fonts(){
  char *home = getenv("HOME");
  char fonts[PATHLEN];

  strcpy(fonts, home);
  strcat(fonts, "/.local/share/fonts/");
  if(!run(2, "fc-cache", fonts))
    return 0;
  return 1;
}
