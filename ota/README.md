# OTA update service

**Author:** Maya Posch

**Last update:** 2017/12/20

## Introduction

The OTA update service (**OTAU**) works together with the CMN firmware to allow a CMN to update itself with a new firmware image. After the CMN receives an update trigger via its maintenance interface, it contacts a hard-coded HTTP URI to obtain the new firmware image.

The OTAU currently is a simple browser-based script using which CMNs can be added using their UID (MAC address) along with the name of the firmware image they should be downloading. When the CMN contacts the OTAU running inside a webserver, the OTAU selects the appropriate firmware image and sends it to the CMN.

## Building

There is nothing to build with this project. Simply copy the two PHP files to any PHP-capable webserver (matching the OTA update URI in the CMN firmware) and ensure that the paths for the firmware and database folders in `ota.php` are accessible by the script.

To add, edit or remove nodes from the OTA system, simply navigate to the script with a web browser.

**Note:** At this point no security is provided by default. It's recommended to set up basic HTTP authentication or better to ensure that no unauthorised access can take place.