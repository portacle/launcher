#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdio.h>

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
  char libpath[PATHLEN], loader[PATHLEN];
  get_env("LW_LIBRARY_PATH", libpath);
  get_env("LW_LOADER_PATH", loader);
  
  char *rargv[argc+4];
  rargv[0] = argv[0];
  rargv[1] = "--library-path";
  rargv[2] = libpath;
  rargv[3] = path;
  for(int i=1; i<argc; ++i){
    rargv[i+3] = argv[i];
  }
  rargv[argc+3] = 0;

  extern char **environ;
  if(execve(loader, rargv, environ) < 0)
    return 0;
  return 1;
}
