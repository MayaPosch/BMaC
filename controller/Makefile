#

export TOP := $(CURDIR)

ifndef ARCH
ARCH := $(shell g++ -dumpmachine)/
endif

# Include the file with the versioning information ('VERSION' variable).
include version.mk
VERSIONINFO = -D__VERSION="\"$(VERSION)\""

LDFLAGS := $(LDFLAGS) 
LIBS 	:= -lnymphmqtt -lbytebauble -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoData \
			-lPocoDataSQLite -lPocoFoundation -lPocoJSON 
CFLAGS := $(CFLAGS) -g3 -std=c++11 $(VERSIONINFO)

TARGET = bmaccontrol
SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix obj/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))

GCC = g++
MAKEDIR = mkdir -p
RM = rm
AR = ar

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
ifndef ANDROID
ifndef ANDROID64
ifndef ANDROIDX86
ifndef ANDROIDX64
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
endif
endif
endif
endif
else
	LIBS += -pthread
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        MAKE := gmake
    endif
endif

all: server

makedir:
	$(MAKEDIR) bin/$(ARCH)
	$(MAKEDIR) obj/$(ARCH)

obj/$(ARCH)%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)

server: makedir $(OBJECTS)
	$(GCC) -o bin/$(ARCH)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)
	#$(GCC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS)

clean : 
	#-rm -f *.o accontrol
	$(RM) $(OBJECTS)

.PHONY: all clean

# --- INSTALLATION ---

PREFIX ?= /usr/local
	
# ifeq ($(PREFIX),/usr/local)
	# CONFDIR := $(PREFIX)/etc
# else
	# CONFDIR := /etc
# endif

CONFDIR := /etc
	
.PHONY: install
install:
	install -d $(DESTDIR)$(PREFIX)/bin/ \
		-d $(DESTDIR)$(CONFDIR)/bmac/
	install -m 755 bin/$(OUTPUT) $(DESTDIR)$(PREFIX)/bin/
	install -m 644 config.ini $(DESTDIR)$(CONFDIR)/bmac/

.PHONY: install-systemd
install-systemd:
	install -d $(DESTDIR)$(CONFDIR)/systemd/system/ 
	cp systemd/bmaccontroller.service $(DESTDIR)$(CONFDIR)/systemd/system/bmaccontroller.service

.PHONY: install-openrc
install-openrc:
	install -d $(DESTDIR)$(CONFDIR)/init.d/	
	cp openrc/bmaccontroller $(DESTDIR)$(CONFDIR)/init.d/bmaccontroller
