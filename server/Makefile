LDFLAGS := $(LDFLAGS) -lmosquittopp -lmosquitto -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoData -lPocoDataSQLite -lPocoFoundation
CFLAGS := $(CFLAGS) -g3 -std=c++11

TARGET = cnc
SOURCES := $(wildcard *.cpp)

CC = g++

all: 
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS)

clean : 
	-rm -f *.o cnc

.PHONY: all clean
