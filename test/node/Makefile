# Makefile for the BMaC firmware integration test.
#
# Builds the BMaC ESP8266 node firmware 
#
# (c) Maya Posch

export TOP := $(CURDIR)

GPP = g++
GCC = gcc
MAKEDIR = mkdir -p
RM = rm
AR = ar

OUTPUT = bmac_esp8266
OUTLIB = lib$(OUTPUT).a
INCLUDE = -I $(TOP)/ \
			-I $(TOP)/SmingCore/ \
			-I $(TOP)/SmingCore/network \
			-I $(TOP)/SmingCore/network/Http \
			-I $(TOP)/SmingCore/network/Http/Websocket \
			-I $(TOP)/SmingCore/network/libmosquitto \
			-I $(TOP)/SmingCore/network/libmosquitto\cpp \
			-I $(TOP)/SmingCore/wiring \
			-I $(TOP)/esp8266/app
#-DPOCO_WIN32_UTF8
FLAGS := $(INCLUDE) -g3 -U__STRICT_ANSI__
LIB := -Llib -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON -lmosquitto -lmosquittopp 
# -l$(OUTPUT)

include ./../../esp8266/version
include ./Makefile-user.mk

CPPFLAGS := $(FLAGS) -DVERSION="\"$(VERSION)\"" $(USER_CFLAGS) -std=c++17 
CFLAGS := -g3 
CPP_SOURCES := $(wildcard SmingCore/*.cpp) \
			$(wildcard SmingCore/network/*.cpp) \
			$(wildcard SmingCore/network/Http/*.cpp) \
			$(wildcard SmingCore/wiring/*.cpp)
FW_SOURCES := $(wildcard $(TOP)/esp8266/app/*.cpp)
CPP_OBJECTS := $(addprefix obj/,$(notdir) $(CPP_SOURCES:.cpp=.o))
FW_OBJECTS := $(addprefix obj/,$(notdir) $(FW_SOURCES:.cpp=.o))

all: makedir $(CPP_OBJECTS) lib/$(OUTLIB) $(FW_OBJECTS) bin/$(OUTPUT)
	
obj/%.o: %.cpp
	$(GPP) -c -o $@ $< $(CPPFLAGS)
	
obj/%.o: %.c
	$(GCC) -c -o $@ $< $(CFLAGS)
	
lib/$(OUTLIB): $(CPP_OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
bin/$(OUTPUT):
	-rm -f $@
	$(GPP) -o $@ $(CPPFLAGS) $(LIB) $(FW_SOURCES) $(CPP_SOURCES)
	
makedir:
	$(MAKEDIR) bin
	$(MAKEDIR) lib
	$(MAKEDIR) obj
	$(MAKEDIR) obj/SmingCore/network
	$(MAKEDIR) obj/SmingCore/wiring
	$(MAKEDIR) obj/esp8266/app

clean:
	$(RM) $(C_OBJECTS) $(CPP_OBJECTS)
	