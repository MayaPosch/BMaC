# Command & Control node

Also called **CMN**. In its current iteration these are ESP8266-based systems. Generally they are fitted into a wall- or ceiling-mounted enclosure using a custom board with the ability to mount temperature, CO2 and other sensors. They can also be in the form of a simple switch, in which case they are fitted into an electrical box, usually to replace a manual switch.

Each node runs the same firmware, yet can be configured via the CCS (Command & Control Server) to only enable specific modules.

Current modules are:

- **Temperature/Humidity** - Uses the Bosch BME280 sensor to read temperature, humidity and air pressure.
- **CO2** - Uses the MH-Z19 and compatible (MH-Z14) CO2 sensor's serial interface to obtain PPM levels.
- **Jura** - Reads out current statistics on total number of coffee consumed for compatible Toptronic-based automatic coffee machines.
- **JuraTerm** - Allows for direct control of compatible Toptronic-based coffee machines using the serial protocol over MQTT, terminal-style.
- **Motion** - Uses compatible PIR motion sensors (e.g.  the HC-SR501) with 3.3V TTL output.
- **PWM** - Generates a PWM signal of variable duty, set to 1 KHz.
- **IO** - Uses the MCP23008 GPIO expander to increase the number of digital GPIO pins.
- **Switch** - Uses an external 2-coil latching relay to switch an input between two possible outputs and read the current setting back.

## Usage ##

The firmware is designed to require minimal configuration. Other than the necessity of adding a file with the WiFi credentials (`files/wifi_creds.txt`), the configuration of the MQTT broker and OTA server is handled automatically, assuming the following dependencies are met:

- BMaC Controller server active on the network.
- MQTT broker active on the network.
- [NyanSD](https://github.com/MayaPosch/NyanSD "NyanSD") server running on port 11310 (UDP) for MQTT broker discovery.

With these services in place, the firmware will:

- Read the WiFi credentials from the `wifi_creds.txt` file.
- Connect to WiFi and broadcast a NyanSD message for the MQTT broker.
- Connect to the first MQTT broker.
- Send MQTT message to `cc/config` with its MAC as payload to obtain its configuration.

At this point the node should be active, and configurable via the BMaC Controller server.

## Building

This project is based on the Sming ESP8266 framework (https://github.com/SmingHub/Sming). As of writing the current 4.6 release is recommended.

After following the installation instructions for Sming in its documentation, to customise the BMaC firmware for the target WiFi network the credentials have to be added to the `files/` folder. This file has this format, with a question mark separating the SSID and password:

```
<WiFi SSID>?<WiFi password>
```

After this, there are a few more configuration options that can be set in the `component.mk` file of the project:


* Whether MQTT is run in secure (TLS) or insecure mode.

Note that when using TLS, one has to add the client certificate and key to the `files/` folder in binary format (not Base64):

* files/**esp8266.client.crt.binary**
* files/**esp8266.client.key.binary**

These files will then be added to the filesystem on the firmware image.

**NOTE:** Due to the limited RAM on the ESP8266, it is required to set the SSL fragment size option on the TLS termination point (MQTT broker or proxy, e.g. HAProxy). A fragment size of about 2 kB is recommended. The default 16 kB size (for TX & RX) will cause the ESP8266 to run out of memory during handshake and reset.

## PCB KiCad project ##

The KiCad project for the controller's PCB is found in the `kicad/` folder. From these the Gerber files can be generated for the PCBs.

![](kicad/pwm_analogue_controller_v11_00.jpg)

The PCB is designed for the CamdenBoss CBRS01VWH enclosure (wall/ceiling-mount).