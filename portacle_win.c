#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#define WIN
#define UNICODE 1
#define PLATFORM "win"
#define LIBRARY_VAR "PATH"
#define PATHSEP "\\"
#define PATHLEN MAX_PATH
#define VARLEN 32767
#define VARSEP ";"

int path_up(char *path){
  PathRemoveBackslash(path);
  PathRemoveFileSpec(path);
  return 1;
}

int exe_dir(char *path){
  if(!GetModuleFileName(NULL, path, PATHLEN))
    return 0;

  PathRemoveFileSpec(path);
  return 1;
}

int app_name(char *argv0, char *name){
  if(!GetModuleFileName(NULL, path, PATHLEN))
    return 0;

  PathStripPath(path);
  PathRemoveExtension(path);
  return 1;
}

int set_env(char *name, char *value){
  return SetEnvironmentVariable(name, value);
}

int get_env(char *name, char *value){
  return GetEnvironmentVariable(name, value, VARLEN);
}

// Unsafe, but w/e
int qcat(char *target, int offset, char *arg){
  target[offset] = '"';
  offset++;
  for(int j=0; arg[j]!=0; ++j){
    if(arg[j] == '"'){
      target[offset] = '\\';
      ++offset;
    }
    target[offset] = arg[j];
  }
  target[i] = '"';
  return i+1;
}

int launch(char *path, int argc, char **argv){
  PROCESS_INFORMATION process_info = {0};
  STARTUPINFO startup_info = {0};

  char* command[VARLEN] = {0};
  int offset = qcat(command, 0, path);
  for(int i=0; i<argc; ++i){
    command[offset] = ' ';
    offset = qcat(command, offset+1, argv[i]);
  }
  
  return CreateProcess(NULL,
                       command,
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NO_WINDOW,
                       NULL, 
                       NULL, 
                       &startup_info,
                       &process_info);
}
