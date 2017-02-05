CC=gcc -std=c99 -Wall -D_GNU_SOURCE

all:

lin:
	$(CC) -o "ld-wrap.so" -fPIC -shared -ldl -Wl,-init,init "ld-wrap.c"
	$(CC) -o "portacle" "portacle.c"

win:
	$(CC) -o "fontreg.exe" -mwindows "fontreg.c"
	windres "portacle.rc" -O coff -o "portacle.res"
	$(CC) -o "portacle.exe" "portacle.c" "portacle.res" -lshlwapi 

mac:
	$(CC) -o "portacle" "portacle.c"
