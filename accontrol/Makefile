LDFLAGS := $(LDFLAGS) -lmosquittopp -lmosquitto -lPocoNet -lPocoNetSSL -lPocoUtil -lPocoData -lPocoDataSQLite -lPocoJSON -lPocoFoundation
CFLAGS := $(CFLAGS) -g3 -std=c++11 -lpthread

TARGET = accontrol
SOURCES := $(wildcard *.cpp)

CC = g++

all: 
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS)

clean : 
	-rm -f *.o accontrol

.PHONY: all clean
