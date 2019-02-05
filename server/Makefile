LDFLAGS := $(LDFLAGS) -lmosquittopp -lmosquitto -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoData -lPocoDataSQLite -lPocoFoundation
CFLAGS := $(CFLAGS) -g3 -std=c++11

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	LIBS := -lws2_32
endif

TARGET = cnc
SOURCES := $(wildcard *.cpp)

CC = g++

all: 
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS) $(LIBS)

clean : 
	-rm -f *.o cnc

.PHONY: all clean
