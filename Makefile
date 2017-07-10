CC ?= gcc
CFLAGS ?= -std=c99 -Wall -D_GNU_SOURCE
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

lin: $(OUTPUT)/lin
$(OUTPUT)/lin: $(SOURCES)
	$(CC) -o "$(OUTPUT)/ld-wrap.so" $(CFLAGS) -fPIC -shared -Wl,-init,init "src/ld-wrap.c" -ldl
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) -static "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -lGL -lm -lGLU -lgcrypt
	touch "$(OUTPUT)/lin"

win: $(OUTPUT)/win
$(OUTPUT)/win: $(SOURCES)
	windres -o "$(OUTPUT)/portacle.res" "src/portacle.rc" -O coff
	$(CC) -o "$(OUTPUT)/portacle.exe" $(CFLAGS) "src/portacle.c" "$(OUTPUT)/portacle.res" -lshlwapi -mwindows -mconsole
	$(CC) -o "$(OUTPUT)/credentials.exe" $(CFLAGS) "src/portacle_credentials.c" -lshlwapi -mwindows -lglfw3 -lopengl32 -lm -lGLU32 -lgcrypt
	touch "$(OUTPUT)/win"

mac: $(OUTPUT)/mac
$(OUTPUT)/mac: $(SOURCES)
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lgcrypt -L/usr/local/lib 
	touch "$(OUTPUT)/mac"

