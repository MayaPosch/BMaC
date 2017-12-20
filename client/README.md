# BMaC - C&C Client

**Author:** Maya Posch

**Last Updated:** 2017/12/19

## Overview

This project contains the Command & Control client for the Building Monitoring and Control (BMaC) system. It allows one to connect to a BMaC Command & Control server instance, after which one can add, modify and remove nodes within the BMaC network using a graphical user interface.

## Building

The BMaC C&C client is written in C++/Qt 5.x. It also uses the Mosquitto client library (libmosquitto & libmosquittopp) for MQTT functionality.

For building on Windows (with 64-bit MSVC) the required dynamic library to link with is provided. On other OSes, use the preferred package manager to download these dependencies.

With Qt 5 installed, the project can be built within its root folder with:

- qmake
- make

## General use

The resulting binary from building the project can be launched, after which the graphical UI is opened. Using the menu options one can connect to a remote C&C server using the MQTT broker's IP address or host name, along with the desired port.

The use of TLS certificates for an encrypted connection with is also an option, though it should be noted that this feature is still experimental and may not work (yet). The path to the CA, Cert & Key files will be asked after selected 'Yes' on the question whether to use TLS or not.

The MQTT broker (e.g. Mosquitto) forms hereby the communication fabric between the client, server and other components of the BMaC system.



After connecting to the MQTT broker, one can create new BMaC nodes via the menu option in the client. For the UID use the node's MAC address (obtained separately). This is the same UID that the BMaC nodes will identify themselves with to the C&C server.

With the UID added, one is asked for a custom location string. This string can be anything, but should be something descriptive. This location string is later used to address individual modules.

Finally, one can configure the settings for the node, enabling certain modules and saving the configuration to just the node (assuming it is online),  or to the C&C server. The node will automatically obtain its configuration from the C&C server the next time it starts.