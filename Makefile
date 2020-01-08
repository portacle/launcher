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
	$(CC) -o "$(OUTPUT)/libnss_cache.so.2" $(CFLAGS) -D MODULE=cache -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_db.so.2" $(CFLAGS) -D MODULE=db -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_docker.so.2" $(CFLAGS) -D MODULE=docker -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_extrausers.so.2" $(CFLAGS) -D MODULE=extrausers -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_gw_name.so.2" $(CFLAGS) -D MODULE=gw_name -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_hesiod.so.2" $(CFLAGS) -D MODULE=hesiod -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_ldap.so.2" $(CFLAGS) -D MODULE=ldap -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_ldapd.so.2" $(CFLAGS) -D MODULE=ldapd -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_libvirt.so.2" $(CFLAGS) -D MODULE=libvirt -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_lwres.so.2" $(CFLAGS) -D MODULE=lwres -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns.so.2" $(CFLAGS) -D MODULE=mdns -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns_minimal.so.2" $(CFLAGS) -D MODULE=mdns_minimal -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns4.so.2" $(CFLAGS) -D MODULE=mdns -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns4_minimal.so.2" $(CFLAGS) -D MODULE=mdns_minimal -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns6.so.2" $(CFLAGS) -D MODULE=mdns -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mdns6_minimal.so.2" $(CFLAGS) -D MODULE=mdns_minimal -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_myhostname.so.2" $(CFLAGS) -D MODULE=myhostname -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mymachines.so.2" $(CFLAGS) -D MODULE=mymachines -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_mysql.so.2" $(CFLAGS) -D MODULE=mysql -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_nis.so.2" $(CFLAGS) -D MODULE=nis -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_nisplus.so.2" $(CFLAGS) -D MODULE=nisplus -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_pgsql2.so.2" $(CFLAGS) -D MODULE=pgsql2 -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_rainbow2.so.2" $(CFLAGS) -D MODULE=rainbow2 -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_resolve.so.2" $(CFLAGS) -D MODULE=resolve -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_securepass.so.2" $(CFLAGS) -D MODULE=securepass -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_sss.so.2" $(CFLAGS) -D MODULE=sss -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_systemd.so.2" $(CFLAGS) -D MODULE=systemd -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/libnss_winbind.so.2" $(CFLAGS) -D MODULE=winbind -fPIC -shared "src/nss_stub.c" -lnss_dns
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) -static "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -lGL -lm -lGLU -lgcrypt

win: $(SOURCES)
	windres -o "$(OUTPUT)/portacle.res" "src/portacle.rc" -O coff
	$(CC) -o "$(OUTPUT)/portacle.exe" $(CFLAGS) "src/portacle.c" "$(OUTPUT)/portacle.res" -lshlwapi -mwindows -mconsole
	$(CC) -o "$(OUTPUT)/credentials.exe" $(CFLAGS) "src/portacle_credentials.c" -lshlwapi -mwindows -lglfw3 -lopengl32 -lm -lGLU32 -lgcrypt

mac: $(SOURCES)
	$(CC) -o "$(OUTPUT)/portacle" $(CFLAGS) "src/portacle.c"
	$(CC) -o "$(OUTPUT)/credentials" $(CFLAGS) "src/portacle_credentials.c" -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lgcrypt -L/usr/local/lib

