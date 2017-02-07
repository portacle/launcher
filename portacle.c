#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32)
#include "portacle_win.c"
#elif defined(__APPLE__)
#include "portacle_mac.c"
#elif defined(__linux__)
#include "portacle_lin.c"
#endif

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
  if(strcmp(name, "..") == 0 ||
     strcmp(name, ".") == 0)
    return 0;
  char path[PATHLEN]={0};
  pathcat(path, root, 1, name);
  return is_directory(path);
}

int emacs_version(char *root, char *version){
  char path[PATHLEN]={0};
  pathcat(path, root, 4, PLATFORM, "emacs", "libexec", "emacs", "");
  DIR *dir = opendir(path);
  if(!dir) return 0;
  struct dirent *entry;
  while((entry = readdir(dir)) != 0){
    if(is_directory_entry(path, entry->d_name)){
      strcpy(version, entry->d_name);
      break;
    }
  }  
  closedir(dir);
  return 1;
}

int launch_emacs(char *root, int argc, char **argv){
  char path[PATHLEN]={0}, start[PATHLEN]={0}, share[PATHLEN]={0}, version[PATHLEN]={0};
  if(!emacs_version(root, version)) return 0;
  pathcat(share, root, 5, PLATFORM, "emacs", "share", "emacs", version);
  if(!set_env("EMACSLOADPATH", "")) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, root, 2, "config", ""))) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "site-lisp", ""))) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "lisp", ""))) return 0;
  DIR *dir = opendir(path);
  if(!dir) return 0;
  struct dirent *entry;
  while((entry = readdir(dir)) != 0){
    if(is_directory_entry(path, entry->d_name)){
      char load[PATHLEN]={0};
      if(!add_env("EMACSLOADPATH", pathcat(load, share, 3, "lisp", entry->d_name, ""))) return 0;
    }
  }
  closedir(dir);
  
  if(!add_env("PATH", pathcat(path, root, 6, PLATFORM, "emacs", "libexec", "emacs", version, ""))) return 0;
  if(!set_env("EMACSDATA", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("EMACSDOC", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("GTK_MODULES", "")) return 0;
  if(!set_env("GTK2_MODULES", "")) return 0;
  if(!set_env("GTK3_MODULES", "")) return 0;

  pathcat(start, root, 2, "config", "emacs-init.el");
  char *rargv[argc+7];
  add_args(rargv, argc, argv, 7, "--no-init-file",
           "--name", "Portacle",
           "--title", "Portacle",
           "--load", start);

  // Ensure the console disappears on Windows.
#ifdef WIN
  FreeConsole();
  win_create_flags = CREATE_NO_WINDOW;
#endif

  pathcat(path, root, 4, PLATFORM, "emacs", "bin", "emacs");
#ifdef LIN
  return launch_ld(path, argc+7, rargv);
#else
  return launch(path, argc+7, rargv);
#endif
}

int launch_git(char *root, int argc, char **argv){
  char path[PATHLEN]={0};
  if(!set_env("LD_PRELOAD", pathcat(path, root, 3, PLATFORM, "launcher", "ld-wrap.so"))) return 0;
  
  pathcat(path, root, 4, PLATFORM, "git", "bin", "git");
#ifdef LIN
  return launch_ld(path, argc, argv);
#else
  return launch(path, argc, argv);
#endif
}

int launch_sbcl(char *root, int argc, char **argv){
  char path[PATHLEN]={0}, start[PATHLEN]={0};

  pathcat(start, root, 2, "config", "sbcl-init.lisp");  
  char *rargv[argc+3];
  add_args(rargv, argc, argv, 3, "--no-sysinit",
           "--userinit", start);
  
  pathcat(path, root, 4, PLATFORM, "sbcl", "bin", "sbcl");
  return launch(path, argc+3, rargv);
}

int launch_ash(char *root, int argc, char **argv){
  char path[PATHLEN]={0};  
  pathcat(path, root, 3, PLATFORM, "bin", "ash");
  return launch(path, argc, argv);
}

int configure_env(char *root){
  char path[PATHLEN]={0};  

  if(!set_env("ROOT", root)) return 0;
  if(!set_env("XDG_CONFIG_HOME", pathcat(path, root, 2, "config", ""))) return 0;
  if(!set_env("SBCL_HOME", pathcat(path, root, 5, PLATFORM, "sbcl", "lib", "sbcl", ""))) return 0;
  if(!add_env("PATH", pathcat(path, root, 3, PLATFORM, "bin", ""))) return 0;
#ifdef WIN
  if(!add_env("PATH", pathcat(path, root, 3, PLATFORM, "lib", ""))) return 0;
#endif
  if(!set_env("LW_LIBRARY_PATH", pathcat(path, root, 3, PLATFORM, "lib", ""))) return 0;
  if(!set_env("LW_LOADER_PATH", pathcat(path, root, 3, PLATFORM, "lib", "ld-linux-x86-64.so.2"))) return 0;
  if(!set_env("LW_SHELL", pathcat(path, root, 3, PLATFORM, "bin", "ash"))) return 0;
  
  return 1;
}

int is_root(char *root){
  char anchor[PATHLEN]={0};
  if(strcmp(root, "") == 0
     || strcmp(root, "/") == 0
     || strcmp(root, ".") == 0)
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

int main(int argc, char **argv){
  char root[PATHLEN]={0}, app[PATHLEN]={0};
  setbuf(stderr, NULL);
  
  if(!find_root(root)){
    fprintf(stderr, "Fatal: could not determine the Portacle root directory.\n");
    return 1;
  }
  if(!app_name(argv[0], app)){
    fprintf(stderr, "Fatal: could not determine the application name.\n");
    return 1;
  }
  if(!configure_env(root)){
    fprintf(stderr, "Fatal: could not configure environment properly.\n");
    return 1;
  }

  if(strcmp(app, "portacle") == 0){
    if(argc > 1){
      argc--;
      argv++;
      strcpy(app, argv[0]);
    }else{
      strcpy(app, "emacs");
    }
  }

  char debug[VARLEN]={0};
  get_env("PORTACLE_DEBUG", debug);
  if(debug[0] != 0){
    char path[VARLEN]={0}, lib[VARLEN]={0};
    get_env("PATH", path);
    get_env(LIBRARY_VAR, lib);
    fprintf(stderr, "  Portacle Launcher System Info:\n");
    fprintf(stderr, "App:           %s\n", app);
    fprintf(stderr, "Platform:      %s\n", PLATFORM);
    fprintf(stderr, "Root:          %s\n", root);
    fprintf(stderr, "Binary Path:   %s\n", path);
    fprintf(stderr, "Library Path:  %s\n", lib);
  }

  if(strcmp(app, "emacs") == 0){
    if(!launch_emacs(root, argc, argv)){
      fprintf(stderr, "Fatal: failed to launch Emacs.\n");
      return 2;
    }
  }else if(strcmp(app, "git") == 0){
    if(!launch_git(root, argc, argv)){
      fprintf(stderr, "Fatal: failed to launch GIT.\n");
      return 2;
    }
  }else if(strcmp(app, "sbcl") == 0){
    if(!launch_sbcl(root, argc, argv)){
      fprintf(stderr, "Fatal: failed to launch SBCL.\n");
      return 2;
    }
  }else if(strcmp(app, "ash") == 0){
    if(!launch_ash(root, argc, argv)){
      fprintf(stderr, "Fatal: failed to launch ASH.\n");
      return 2;
    }
  }else{
    fprintf(stderr, "Fatal: Unknown application: %s\n", app);
    return 3;
  }
  
  return 0;
}
