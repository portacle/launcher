CC ?= gcc
CFLAGS ?= -std=c99 -Wall -D_GNU_SOURCE -g
OUTPUT ?= build
IGNORE := $(shell mkdir -p $(OUTPUT))
SOURCES := $(wildcard src/*)
TARGET := unknown

ifeq ($(OS),Windows_NT)
	TARGET := win
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		TARGET := lin
	endif
	ifeq ($(UNAME_S),Darwin)
		TARGET := mac
	endif
endif

.PHONY = all lin win mac
all:
	make $(TARGET)

lin: $(SOURCES)
	$(CC) -o "$(OUTPUT)/ld-wrap.so" $(CFLAGS) -fPIC -shared -Wl,-init,init "src/ld-wrap.c" -ldl
	$(CC) -o "$(OUTPUT)/libnss_mymachines.so.2" $(CFLAGS) -D MODULE=mymachines -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_myhostname.so.2" $(CFLAGS) -D MODULE=myhostname -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_resolve.so.2" $(CFLAGS) -D MODULE=resolve -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) -static "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -lGL -lm -lGLU -lgcrypt

win: $(SOURCES)
	windres -o "$(OUTPUT)/portacle.res" "src/portacle.rc" -O coff
	$(CC) -o "$(OUTPUT)/portacle.exe" $(CFLAGS) "src/portacle.c" "$(OUTPUT)/portacle.res" -lshlwapi -mwindows -mconsole
	$(CC) -o "$(OUTPUT)/credentials.exe" $(CFLAGS) "src/portacle_credentials.c" -lshlwapi -mwindows -lglfw3 -lopengl32 -lm -lGLU32 -lgcrypt

mac: $(SOURCES)
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lgcrypt -L/usr/local/lib

