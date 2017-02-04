#if defined(_WIN32) || defined(WIN32)
#include "portacle_win.c"
#elif defined(__APPLE__)
#include "portacle_mac.c"
#elif defined(__linux__)
#include "portacle_lin.c"
#endif

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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
  char var[VARLEN];
  if(!get_env(name, var)) return 0;
  strcat(var, VARSEP);
  strcat(var, value);
  if(!set_env(name, var)) return 0;
  return 1;
}

int add_args(char **rargv, int argc, char **argv, int c, ...){
  va_list argp;
  va_start(argp, c);
  for(int i=0; i<c; ++i){
    char *part = va_arg(argp, char *);
    rargv[i] = part;
  }
  va_end(argp);

  for(int i=0; i<argc; i++){
    rargv[i+c] = argv[i];
  }
  return 1;
}

int emacs_version(char *root, char *version){
  char path[PATHLEN];
  pathcat(path, root, 4, "emacs", PLATFORM, "libexec", "emacs", "");
  DIR *dir = opendir(path);
  if(!dir) return 0;
  struct dirent *entry = readdir(dir);
  strcpy(version, entry->d_name);
  closedir(dir);
  return 1;
}

int launch_emacs(char *root, int argc, char **argv){
  char path[PATHLEN], data[PATHLEN], share[PATHLEN], version[PATHLEN];
  if(!emacs_version(root, version)) return 0;
  pathcat(share, root, 4, "emacs", "share", "emacs", version);
  
  if(!set_env("EMACSLOADPATH", "")) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "site-lisp", ""))) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "lisp", ""))) return 0;
  DIR *dir = opendir(path);
  if(!dir) return 0;
  struct dirent *entry;
  while((entry = readdir(dir)) != 0){
    if(!add_env("EMACSLOADPATH", pathcat(path, share, 3, "lisp", entry->d_name, ""))) return 0;
  }
  closedir(dir);
  
  if(!add_env("PATH", pathcat(path, root, 6, "emacs", PLATFORM, "libexec", "emacs", version, ""))) return 0;
  if(!set_env("EMACSDATA", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("EMACSDOC", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("GTK_MODULES", "")) return 0;
  if(!set_env("GTK2_MODULES", "")) return 0;
  if(!set_env("GTK3_MODULES", "")) return 0;

  char *rargv[argc+7];
  add_args(rargv, argc, argv, 7, "-q",
           "--name", "Portacle",
           "-T", "portacle",
           "-l", pathcat(path, root, 2, "config", "emacs-init.el"));
  return launch(pathcat(path, root, 4, "emacs", PLATFORM, "bin", "emacs"), argc+7, rargv);
}

int launch_git(char *root, int argc, char **argv){
  char path[PATHLEN];
  pathcat(path, root, 4, "git", PLATFORM, "bin", "git");
  return launch(path, argc, argv);
}

int launch_sbcl(char *root, int argc, char **argv){
  char path[PATHLEN], home[PATHLEN];
  pathcat(home, root, 5, "sbcl", PLATFORM, "lib", "sbcl", "");
  pathcat(path, root, 4, "sbcl", PLATFORM, "bin", "sbcl");
  if(!set_env("SBCL_HOME", home)) return 0;
  return launch(path, argc, argv);
}

int configure_env(char *root){
  char xdg[PATHLEN], bin[PATHLEN], lib[PATHLEN];
  pathcat(xdg, root, 2, "config", "");
  pathcat(bin, root, 4, "usr", PLATFORM, "bin", "");
  pathcat(lib, root, 4, "usr", PLATFORM, "lib", "");
  
  if(!set_env("ROOT", root)) return 1;
  if(!set_env("XDG_CONFIG_HOME", xdg)) return 1;
  if(!add_env("PATH", bin)) return 1;
  if(!add_env(LIBRARY_VAR, lib)) return 1;

  return 0;
}

int main(int argc, char **argv){
  char root[PATHLEN];
  char app[PATHLEN];
  
  if(!find_root(root)) return 1;
  if(!app_name(argv[0], app)) return 1;
  if(!configure_env(root)) return 1;

  if(strcmp(app, "portacle") == 0){
    if(argc > 1){
      argc--;
      argv++;
      strcpy(app, argv[0]);
    }else{
      strcpy(app, "emacs");
    }
  }

  if (strcmp(app, "emacs") == 0){
    if(!launch_emacs(root, argc, argv))
      return 2;
  }else if (strcmp(app, "git") == 0){
    if(!launch_git(root, argc, argv))
      return 2;
  }else if (strcmp(app, "sbcl") == 0){
    if(!launch_sbcl(root, argc, argv))
      return 2;
  }else{
    printf("Unknown application: %s\n", app);
    return 3;
  }
  
  return 0;
}
