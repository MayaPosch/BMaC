# Command & Control server

**Author:** Maya Posch

**Last Update:** 2017/12/20

## Introduction

The BMaC's Command & Control server (C&C server, or **CCS**) is used for registering the configuration for individual nodes. Each node can feature a different range of active sensors and actuators, which are enabled in the firmware at start-up of the node after they obtain their configuration from this server.

The server also maintains a list of firmware images for the nodes. The C&C client can be used to maintain this list, and assign specific firmware images to nodes.

The C&C client is also used to read out the current node configurations, update configurations and add new nodes.

## Building

CCS can be built on any platform that provides the POCO (https://pocoproject.org/) and libmosquittopp dependencies, along with a C++ compiler. This is easiest on a UNIX-based OS like Linux or BSD.

On Ubuntu one would install the *libmosquittopp-dev* and *libpoco-dev* packages.

With all dependencies installed, simply call `make` in the CCS folder and it should build the binary.

## Running

Before starting the binary, one would edit the **config.ini** file, to set the address and port of the MQTT broker. At this point encrypted MQTT connections are not supported, meaning that it is recommended to install the service on the same system as the broker to keep unencrypted traffic restricted to the loopback interface. 

The HTTP port is for the OTA firmware update functionality for BMaC nodes. The host address and this port have to be encoded into the firmware image so that a simple trigger to the node suffices to have them fetch the new image over HTTP.

The default firmware name can also be specified here, under the 'Firmware' category. The hardcoded default for this is 'ota_unified.bin'.

Any connecting client will request an image file from the server which shows the layout of the building which is being managed. Users will place nodes on this map, meaning that it should be of sufficient resolution to be practical. This image file should be placed in the same folder as the binary, and called `map.png`. PNG is the preferred format, but other popular formats should work, too.

After updating the configuration file, one can either run the binary directly, or install it as a system service. Consult the operating manual for your operating system on how to do this.