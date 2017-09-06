#include "toolkit.c"

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
  DIR *dir; struct dirent *entry;
  
  if(!emacs_version(root, version)) return 0;
  pathcat(share, root, 5, PLATFORM, "emacs", "share", "emacs", version);
  if(!set_env("EMACSLOADPATH", "")) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, root, 2, "config", ""))) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "site-lisp", ""))) return 0;
  if(!add_env("EMACSLOADPATH", pathcat(path, share, 2, "lisp", ""))) return 0;
  
  dir = opendir(path);
  if(dir){
    while((entry = readdir(dir)) != 0){
      if(is_directory_entry(path, entry->d_name)){
        char load[PATHLEN]={0};
        if(!add_env("EMACSLOADPATH", pathcat(load, share, 3, "lisp", entry->d_name, ""))) return 0;
      }
    }
    closedir(dir);
  }
  
  if(!add_env("PATH", pathcat(path, root, 6, PLATFORM, "emacs", "libexec", "emacs", version, ""))) return 0;
  if(!set_env("EMACSDATA", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("EMACSDOC", pathcat(path, share, 2, "etc", ""))) return 0;
  if(!set_env("GTK_MODULES", "")) return 0;
  if(!set_env("GTK2_MODULES", "")) return 0;
  if(!set_env("GTK3_MODULES", "")) return 0;
  
#ifdef LIN
  pathcat(path, root, 2, PLATFORM, "lib", "");
  dir = opendir(path);
  if(dir){
    while((entry = readdir(dir)) != 0){
      if(strstr(entry->d_name, "gdk_pixbuf")){
        if(!set_env("GDK_PIXBUF_MODULE_FILE", pathcat(path, root, 3, PLATFORM, "lib", entry->d_name))) return 0;
        break;
      }
    }
    closedir(dir);
  }
#endif

  pathcat(path, root, 2, "all", "fonts");
  dir = opendir(path);
  if(dir){
    while((entry = readdir(dir)) != 0){
      char *dot = strrchr(entry->d_name, '.');
      if(dot && streq(dot, ".ttf")){
        char font[PATHLEN]={0};
        if(!add_font(pathcat(font, path, 1, entry->d_name))){
          fprintf(stderr, "Warning: Failed to register font %s\n", font);
        }
      }
    }
    closedir(dir);
  }

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
  return launch_maybe_ld(path, argc+7, rargv);
}

int launch_git(char *root, int argc, char **argv){
  char path[PATHLEN]={0};
#ifdef LIN
  if(!set_env("LD_PRELOAD", pathcat(path, root, 3, PLATFORM, "launcher", "ld-wrap.so"))) return 0;
#endif
  if(!add_env("PATH", pathcat(path, root, 5, PLATFORM, "git", "libexec", "git-core", ""))) return 0;
  if(!set_env("GIT_SSL_CAINFO", pathcat(path, root, 3, "all", "ssl", "ca-bundle.crt"))) return 0;
  
  pathcat(path, root, 4, PLATFORM, "git", "bin", "git");
  return launch_maybe_ld(path, argc, argv);
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

int launch_hunspell(char *root, int argc, char **argv){
  char path[PATHLEN]={0};
  if(!set_env("WORDLIST", pathcat(path, root, 2, "config", ".personal-dictionary"))) return 0;
  if(!set_env("DICPATH", pathcat(path, root, 3, "all", "dictionaries", ""))) return 0;
  
  pathcat(path, root, 4, PLATFORM, "hunspell", "bin", "hunspell");
  return launch_maybe_ld(path, argc, argv);
}

int launch_credentials(char *root, int argc, char **argv){
  char path[PATHLEN]={0};
  if(!set_env("CREDENTIALS", pathcat(path, root, 2, "config", ".credentials"))) return 0;

  pathcat(path, root, 4, PLATFORM, "launcher", "credentials");
  return launch_maybe_ld(path, argc, argv);
}

int launch_fontreg(char *root, int argc, char **argv){
  int exit = 0;
  for(int i=1; i<argc; ++i){
    if(!add_font(argv[i])){
      ++exit;
      fprintf(stderr, "Failed to add font %s\n", argv[i]);
    }
  }
  return exit;
}

int launch_query(char *root, int argc, char **argv){
  if(argc <= 1){
    fprintf(stderr, "Possible queries:\n");
    fprintf(stderr, "apps platform root\n");
  }else{
    char *query_result = "unknown";
    /**/ if(streq(argv[1], "apps")) query_result = "query credentials emacs git sbcl ash hunspell fontreg";
    else if(streq(argv[1], "platform")) query_result = PLATFORM;
    else if(streq(argv[1], "root")) query_result = root;
    fprintf(stdout, "%s\n", query_result);
  }
  return 1;
}

int launch_unknown(char *root, int argc, char **argv){
  return 0;
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
  //if(!set_env("LW_SHELL", pathcat(path, root, 3, PLATFORM, "bin", "ash"))) return 0;

  char lang[PATHLEN]={0};
  if(!get_env("PORTACLE_LANG", lang) || streq(lang, "")){
    set_env("LANG", "en_US");
  }else{
    set_env("LANG", lang);
  }
  
  return 1;
}

int main(int argc, char **argv){
  char root[PATHLEN]={0}, app[PATHLEN]={0};
  setbuf(stderr, NULL);
  
  if(!find_root(root)){
    fprintf(stderr, "Fatal: could not determine the Portacle root directory.\n");
    return 100;
  }
  
  if(!app_name(argv[0], app)){
    fprintf(stderr, "Fatal: could not determine the application name.\n");
    return 101;
  }
  
  if(!configure_env(root)){
    fprintf(stderr, "Fatal: could not configure environment properly.\n");
    return 102;
  }

  if(streq(app, "portacle")){
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

  int (*app_launcher)(char *root, int argc, char **argv) = launch_unknown;
  if(streq(app, "query")) app_launcher = launch_query;
  else if(streq(app, "emacs")) app_launcher = launch_emacs;
  else if(streq(app, "git")) app_launcher = launch_git;
  else if(streq(app, "sbcl")) app_launcher = launch_sbcl;
  else if(streq(app, "ash")) app_launcher = launch_ash;
  else if(streq(app, "hunspell")) app_launcher = launch_hunspell;
  else if(streq(app, "fontreg")) app_launcher = launch_fontreg;
  else if(streq(app, "credentials")) app_launcher = launch_credentials;

  if(!app_launcher(root, argc, argv)){
    fprintf(stderr, "Fatal: failed to launch %s.\n", app);
    exitCode = 103;
  }
  
  return exitCode;
}
