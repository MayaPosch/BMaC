# Command & Control server

**Author:** Maya Posch

**Last Update:** 2017/12/20

## Introduction

The BMaC's Command & Control server (C&C server, or **CCS**) is used to register the configuration for individual nodes with. Each node can feature a different range of active sensors and actuators, which are enabled in the firmware at start-up of the node after they obtain their configuration from this server.

The C&C client is used to read out the current node configurations, update configurations and add new nodes.

## Building

CCS can in principle be built on any platform that can provide the POCO (https://pocoproject.org/) and libmosquittopp dependencies, along with a C++ compiler. This is easiest on a UNIX-based OS like Linux or BSD.

On Ubuntu one would install the *libmosquittopp-dev* and *libpoco-dev* packages.

With all dependencies installed, simply call `make` in the CCS folder and it should build the binary.

## Running

Before starting the binary, one would edit the **config.ini** file, to set the address and port of the MQTT broker. At this point encrypted MQTT connections are not supported, meaning that it is recommended to install the service on the same system as the broker to keep unencrypted traffic restricted to the loopback interface. 

After updating the configuration file, one can either run the binary directly, or install it as a system service. Consult the operating manual for your operating system on how to do this.