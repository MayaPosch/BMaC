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
INCLUDE = -I SmingCore/ \
			-I SmingCore/network \
			-I SmingCore/network/Http \
			-I SmingCore/network/Http/Websocket \
			-I SmingCore/network/libmosquitto \
			-I SmingCore/network/libmosquitto\cpp \
			-I SmingCore/wiring
#-DPOCO_WIN32_UTF8
FLAGS := $(INCLUDE) -g3 -std=c++17 -U__STRICT_ANSI__
LIB := -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON
CPPFLAGS := $(FLAGS)
CFLAGS := -g3 
CPP_SOURCES := $(wildcard SmingCore/*.cpp) \
			$(wildcard SmingCore/network/*.cpp) \
			$(wildcard SmingCore/network/Http/*.cpp) \
			$(wildcard SmingCore/network/Http/Websocket/*.cpp) \
			$(wildcard SmingCore/network/libmosquitto/cpp*.cpp)
C_SOURCES := $(wildcard SmingCore/network/libmosquitto/*.cpp)
FW_SOURCES := $(wildcard ../../esp8266/app/*.cpp)
CPP_OBJECTS := $(addprefix obj/,$(notdir) $(CPP_SOURCES:.cpp=.o))
C_OBJECTS := $(addprefix obj/,$(notdir) $(C_SOURCES:.c=.o))

all: makedir $(C_OBJECTS) $(CPP_OBJECTS) bin/$(OUTPUT)
	
obj/%.o: %.cpp
	$(GPP) -c -o $@ $< $(CPPFLAGS)
	
obj/%.o: %.c
	$(GCC) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT):
	-rm -f $@
	$(GPP) -o $@ $(CPPFLAGS) $(FW_SOURCES) $(C_OBJECTS) $(CPP_OBJECTS) $(LIB)
	
makedir:
	$(MAKEDIR) bin
	$(MAKEDIR) obj
	$(MAKEDIR) obj/SmingCore/network/Http/Websocket
	$(MAKEDIR) obj/SmingCore/network/libmosquitto\cpp
	$(MAKEDIR) obj/SmingCore/wiring

clean:
	$(RM) $(C_OBJECTS) $(CPP_OBJECTS)
	