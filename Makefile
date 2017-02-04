CC=gcc

all:

lin:
	$(CC) -o "ld-wrap.so" -Wall -std=c99 -fPIC -shared -ldl -Wl,-init,init "ld-wrap.c"
	$(CC) -o "portacle" "portacle.c"

win:
	$(CC) -o "fontreg.exe" -Wall -std=c99 -mwindows "fontreg.c"
	windres "portacle.rc" -O coff -o "portacle.res"
	$(CC) -o "portacle.exe" "portacle.c" "portacle.res" -lshlwapi 

mac:
	$(CC) -o "portacle" "portacle.c"
