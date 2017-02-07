#include <stdio.h>

#if defined(_WIN32) || defined(WIN32)
#include "fontreg_win.c"
#elif defined(__APPLE__)
#include "fontreg_mac.c"
#elif defined(__linux__)
#include "fontreg_lin.c"
#endif

int main(int argc, char **argv){
  int exit = 0;
  for(int i=1; i<argc; ++i){
    if(!add_font(argv[i])){
      ++exit;
      fprintf(stderr, "Failed to add font %s\n", argv[i]);
    }
  }
  return exit;
}
