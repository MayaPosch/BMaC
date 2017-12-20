LDFLAGS := $(LDFLAGS) -lmosquittopp -lmosquitto -lPocoUtil -lPocoNet -lPocoNetSSL -lPocoFoundation -L/usr/local/opt/openssl/lib/
CFLAGS := $(CFLAGS) -g3 -I/usr/local/opt/openssl/include/

TARGET = influx_mqtt
SOURCES := $(wildcard *.cpp)

CC = g++

all: 
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LDFLAGS)

clean : 
	-rm -f *.o influx_mqtt

.PHONY: all clean
