#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strings.h>
#include <stdio.h>

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
  strcpy(name, argv0);
  PathStripPath(name);
  PathRemoveExtension(name);
  return 1;
}

int set_env(char *name, char *value){
  return SetEnvironmentVariable(name, value);
}

int get_env(char *name, char *value){
  SetLastError(0);
  GetEnvironmentVariable(name, value, VARLEN);
  if(GetLastError() == ERROR_ENVVAR_NOT_FOUND)
    return 0;
  return 1;
}

// Unsafe, but w/e
int qcat(char *target, int offset, char *arg){
  target[offset] = '"';
  ++offset;
  for(int j=0; arg[j]!=0; ++j){
    if(arg[j] == '"'){
      target[offset] = '\\';
      ++offset;
    }
    target[offset] = arg[j];
    ++offset;
  }
  target[offset] = '"';
  return offset+1;
}

int win_create_flags = 0;
int launch(char *path, int argc, char **argv){
  PROCESS_INFORMATION process_info = {0};
  STARTUPINFO startup_info = {0};

  char command[VARLEN] = {0};
  int offset = 0;
  for(int i=1; i<argc; ++i){
    command[offset] = ' ';
    offset = qcat(command, offset+1, argv[i]);
  }
  strcat(path, ".exe");

  char debug[VARLEN]={0};
  get_env("PORTACLE_DEBUG", debug);
  if(debug[0] != 0){
    fprintf(stderr, "\n  Launching executable: %s\n", path);
    for(int i=0; i<argc; ++i)
      fprintf(stderr, "argv%i %s\n", i, argv[i]);
    fprintf(stderr, "\n");
  }
  
  if(!CreateProcess(path,
                    command,
                    NULL,
                    NULL,
                    FALSE,
                    win_create_flags,
                    NULL, 
                    NULL, 
                    &startup_info,
                    &process_info))
    return 0;
  // Wait for process to exit.
  WaitForSingleObject(process_info.hProcess, INFINITE);
  GetExitCodeProcess(process_info.hProcess, &exitCode);
  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
  return 1;
}
