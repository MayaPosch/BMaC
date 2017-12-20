# AC control service

**Author:** Maya Posch

**Last update:** 2017/12/20

## Introduction

The AC control service (accontrol, or **ACCS**) builds upon the CCS, CMNs as well as the data stored in the Influx database in order to control the room temperature using installed Fan Coil Units (**FCUs**), along with the valves in the system (both 2- and 4-pipe FCUs).

It provides a HTTP REST API along with a simple browser-based interface using which one can set the target temperature for a particular node and read out the current temperature.

## Building

ACCS can in principle be built on any platform that can provide the POCO (https://pocoproject.org/) and libmosquittopp dependencies, along with a C++ compiler. This is easiest on a UNIX-based OS like Linux or BSD.

On Ubuntu one would install the *libmosquittopp-dev* and *libpoco-dev* packages.

With all dependencies installed, simply call `make` in the CCS folder and it should build the binary.

## Running

Before starting the binary, one would edit the **config.ini** file, to set the address and port of the MQTT broker. At this point encrypted MQTT connections are not supported, meaning that it is recommended to install the service on the same system as the broker to keep unencrypted traffic restricted to the loopback interface. 

Further configuration options are the HTTP port on which the ACCS' webserver listens, the InfluxDB settings, and options for a Eureka discovery and authentication server (both currently disabled). 

**Note:** for proper functioning, the HTTP, MQTT and InfluxDB settings have to be correct.

After updating the configuration file, one can either run the binary directly, or install it as a system service. Consult the operating manual for your operating system on how to do this.

## Controls

To use the web interface, simply navigate to the configured port of the system ACCS is running on with a web browser and the interface should load.

## Important

ACCS as a project is still very much a prototype, only having been used in one location at this point. It is limited to the following configuration as a result:

Single room, with 2-pipe FCUs and a central switch for changing the system from heating to cooling and vice versa.

It is currently planned to refactor ACCS to properly model a large number of rooms, supporting both types of FCUs and various configurations.