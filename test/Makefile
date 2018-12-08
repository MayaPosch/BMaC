# Makefile for the BMaC integration test server.
#
# Builds the BMaC integration test server.
#
# (c) Maya Posch

export TOP := $(CURDIR)

GPP = g++
GCC = gcc
MAKEDIR = mkdir -p
RM = rm
#AR = ar

OUTPUT = bmac_server
INCLUDE = -I .
#-DPOCO_WIN32_UTF8
FLAGS := $(INCLUDE) -g3 -std=c++17 -U__STRICT_ANSI__
LIB := -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON
CPPFLAGS := $(FLAGS)
CFLAGS := -g3 
CPP_SOURCES := $(wildcard *.cpp) $(wildcard devices/*.cpp)
CPP_OBJECTS := $(addprefix obj/,$(notdir) $(CPP_SOURCES:.cpp=.o))

all: makedir $(C_OBJECTS) $(CPP_OBJECTS) bin/$(OUTPUT)
	
obj/%.o: %.cpp
	$(GPP) -c -o $@ $< $(CPPFLAGS)
	
bin/$(OUTPUT):
	-rm -f $@
	$(GPP) -o $@ $(C_OBJECTS) $(CPP_OBJECTS) $(LIB)
	
makedir:
	$(MAKEDIR) bin
	$(MAKEDIR) obj/devices

clean:
	$(RM) $(CPP_OBJECTS)
	