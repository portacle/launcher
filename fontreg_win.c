#include<windows.h>

int resolve_path(char *path, char *resolved){
  int len = GetFullPathName(path, MAX_PATH, resolved, 0);
  if(MAX_PATH < len) return 0;
  if(len <= 0) return 0;
  return 1;
}

int add_font(char *file){
  char font[MAX_PATH] = "";

  if(!resolve_path(file, font)) return 0;
  if(!AddFontResource(font)) return 0;

  SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
  return 1;
}
