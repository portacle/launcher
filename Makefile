CC ?= gcc
CFLAGS ?= -std=c99 -Wall
OUTPUT ?= build
IGNORE := $(shell mkdir -p $(OUTPUT))

.PHONY = all lin win mac
all:

lin: $(OUTPUT)/lin
$(OUTPUT)/lin: portacle_lin.c portacle.c ld-wrap.c Makefile
	$(CC) -o "$(OUTPUT)/ld-wrap.so" $(CFLAGS) -D_GNU_SOURCE -fPIC -shared -Wl,-init,init "ld-wrap.c" -ldl
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) -static -D_GNU_SOURCE "portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) -D_GNU_SOURCE "credentials/portacle_credentials.c" -lglfw -lGL -lm -lGLU -lgcrypt
	touch "$(OUTPUT)/lin"

win: $(OUTPUT)/win
$(OUTPUT)/win: portacle_win.c portacle.c portacle.rc portacle.ico Makefile
	windres -o "$(OUTPUT)/portacle.res" "portacle.rc" -O coff
	$(CC) -o "$(OUTPUT)/portacle.exe" $(CFLAGS) "portacle.c" "$(OUTPUT)/portacle.res" -lshlwapi -mwindows -mconsole
	$(CC) -o "$(OUTPUT)/credentials.exe" $(CFLAGS) "credentials/portacle_credentials.c" -lglfw3 -lopengl32 -lm -lGLU32  -lgcrypt
	touch "$(OUTPUT)/win"

mac: $(OUTPUT)/mac
$(OUTPUT)/mac: portacle_mac.c portacle.c Makefile
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) "portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "credentials/portacle_credentials.c" -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lGLEW -lgcrypt -L/usr/local/lib 
	touch "$(OUTPUT)/mac"

