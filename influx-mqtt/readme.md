# Influx-mqtt #

**Author:** Maya Posch

**Last update:** 2017/12/20

## Introduction

This application is an MQTT client which can subscribe to multiple topics, writing each message's payload into an InfluxDB instance using its HTTP-based line-protocol.

## Features ##

- Subscribes to all topics specified in the configuration file.
- Supports HTTP and HTTPS.
- Uses last part of topic name for Influx series name.
- Based on libmosquitto (MQTT) and POCO (HTTP(S)).
- Written in C++.
- Simple Make-based project.

## Payload format ##

The payload format format for an MQTT message is expected to have this format:

*[location];[value]*

## InfluxDB line protocol format ##

These MQTT payload values are then parsed and written to the InfluxDB in this format:

**URL:** 

*[InfluxDB URL]/write?db=[DB name]*

**POST data:**

*[series],location=[location] value=[value]*



> Please note that the Influx Line Protocol does **not** support spaces and similar. As this service currently does not escape these characters, adding spaces to a location or value will result in the InfluxDB write failing.

## Building the application ##

In order to build the Influx-mqtt service, one needs to have these dependencies installed:

- g++ (C++ compiler)
- Make
- libmosquittopp (C++ wrapper for libmosquitto)
- libmosquitto (C MQTT library)
- POCO libraries (HTTP and text parsing)
- OpenSSL (for HTTPS)

Simply execute the following command in the folder with the Makefile:

    make

This should build and link the project, creating a single 'influx_mqtt' executable.

Building the code has been tested on OS X and Linux (Ubuntu 14.04LTS, 16.04LTS, Debian (stable), Raspbian).

On Windows, use MinGW with the MSYS2 environment or similar.

## Configuration ##

The configuration file ('config.ini') is in the INI format and can be used to define the following options:

### MQTT ###

The MQTT section defines all MQTT-related options:

**host**

The MQTT broker to connect to.

**port**

The port of the MQTT broker (anonymous, non-TLS port only)

**topics**

The MQTT topics to subscribe to. The string after the last slash (if any) is used as the name for the Influx series. It should therefore be unique.

### Influx ###

The Influx section defines all InfluxDB-related options:

**host**

The InfluxDB's host name or IP.

**port**

The InfluxDB's port.

**secure**

Whether to use HTTPS to connect or not. ('true' or 'false').

**db**

The name of the InfluxDB database to use.

## Running Influx-MQTT

In order to run the application, simply execute the binary:

	$ influx_mqtt

This will assume that the configuration file is in the same folder. If it is missing, the service will attempt to use default settings ('localhost', port 1883).

Specifying the location of the configuration file is also possible:

	$ influx_mqtt /path/to/config.ini 

## Missing features ##

Features which might be useful but currently absent:

1. Password-based authentication for MQTT.
2. TLS-based encryption for MQTT.
3. Graceful shutdown (unsubscribe).
4. Escaping of spaces in location and value strings.

