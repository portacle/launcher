#include <mach-o/dyld.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

#define MAC
#define PLATFORM "mac"
#define LIBRARY_VAR "DYLD_LIBRARY_PATH"
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
  uint32_t size = PATH_MAX;
  _NSGetExecutablePath(path, &size);
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

int add_font(char *file){
  return 0;
}