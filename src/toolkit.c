#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int exitCode = 0;

#if defined(_WIN32) || defined(WIN32)
#include "portacle_win.c"
#elif defined(__APPLE__)
#include "portacle_mac.c"
#elif defined(__linux__)
#include "portacle_lin.c"
#endif

int streq(char *a, char *b){
  return (strcmp(a, b) == 0);
}

char *pathcat(char *path, char *root, int c, ...){
  strcpy(path, root);
  
  va_list argp;
  va_start(argp, c);
  for(int i=0; i<c; ++i){
    char *part = va_arg(argp, char *);
    strcat(path, PATHSEP);
    strcat(path, part);
  }
  va_end(argp);
  return path;
}

int add_env(char *name, char *value){
  char ovalue[VARLEN]={0}, nvalue[VARLEN]={0};
  if(!get_env(name, ovalue))
    ovalue[0] = 0;
  strcat(nvalue, value);
  strcat(nvalue, VARSEP);
  strcat(nvalue, ovalue);
  if(!set_env(name, nvalue)) return 0;
  return 1;
}

int add_args(char **rargv, int argc, char **argv, int c, ...){  
  va_list argp;
  va_start(argp, c);
  rargv[0] = argv[0];
  for(int i=0; i<c; ++i){
    char *part = va_arg(argp, char *);
    rargv[i+1] = part;
  }
  va_end(argp);

  for(int i=1; i<argc; i++){
    rargv[i+c] = argv[i];
  }
  return 1;
}

int is_directory(char *path){
  struct stat s;
  if(stat(path, &s) < 0) return 0;
  return S_ISDIR(s.st_mode);
}

int is_directory_entry(char *root, char *name){
  if(streq(name, "..") || streq(name, "."))
    return 0;
  char path[PATHLEN]={0};
  pathcat(path, root, 1, name);
  return is_directory(path);
}

int is_root(char *root){
  char anchor[PATHLEN]={0};
  if(streq(root, "") || streq(root, "/") || streq(root, "."))
    return -1;
  if(access(pathcat(anchor, root, 1, ".portacle_root"), F_OK) < 0)
    return 0;
  return 1;
}

int find_root(char *root){
  if(!exe_dir(root)) return 0;
  int rootp;
  while((rootp = is_root(root)) != 1){
    if(rootp == -1) return 0;
    path_up(root);
  }
  strcat(root, PATHSEP);
  return 1;
}

int launch_maybe_ld(char *path, int argc, char **argv){
#ifdef LIN
  return launch_ld(path, argc, argv);
#else
  return launch(path, argc, argv);
#endif
}
