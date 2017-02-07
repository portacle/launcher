CC=gcc -std=c99 -Wall -D_GNU_SOURCE

all:

lin: portacle_lin.c portacle.c ld-wrap.c
	$(CC) -o "ld-wrap.so" -fPIC -shared -ldl -Wl,-init,init "ld-wrap.c"
	$(CC) -o "portacle" "portacle.c"

win: portacle_win.c portacle.c fontreg.c
	$(CC) -o "fontreg.exe" -mwindows "fontreg.c"
	windres "portacle.rc" -O coff -o "portacle.res"
	$(CC) -o "portacle.exe" "portacle.c" "portacle.res" -lshlwapi -mwindows -mconsole

mac: portacle_mac.c portacle.c
	$(CC) -o "portacle" "portacle.c"
