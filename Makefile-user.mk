# User-configurable options.

# Environment options:
## WiFi
# WiFi SSID for the network to connect to and its password.
WIFI_SSID = MyWiFiNetwork
WIFI_PWD = MyWiFiPassword

## MQTT
# MQTT host and port.
MQTT_HOST = localhost
# For SSL support, uncomment the following line or compile with this parameter.
#ENABLE_SSL=1
# MQTT SSL port (for example):
ifdef ENABLE_SSL
MQTT_PORT = 8883 
else
MQTT_PORT = 1883
endif

# Uncomment if password authentication is used.
# USE_MQTT_PASSWORD=1
# MQTT username & password (if needed):
# MQTT_USERNAME = esp8266
# MQTT_PWD = ESPassword

# MQTT topic prefix: added to all MQTT subscriptions and publications.
# Can be left empty, but must be defined.
# If not left empty, should end with a '/' to avoid merging with topic names.
MQTT_PREFIX = 

# MQTT URL. This uses the earlier defined MQTT details to compose a full URL.
# No editing required.
ifdef ENABLE_SSL
MQTT_URL := mqtts://
else
MQTT_URL := mqtt://
endif

ifdef USE_MQTT_PASSWORD
MQTT_URL := $(MQTT_URL)$(MQTT_USERNAME):$(MQTT_PWD)@
endif

MQTT_URL := $(MQTT_URL)$(MQTT_HOST):$(MQTT_PORT)

## OTA
# OTA (update) URL. Only change the host name (and port).
OTA_URL = http://ota.host.net/ota.php?uid=

# Pass flags to compiler
USER_CFLAGS := $(USER_CFLAGS) -DWIFI_SSID="\"$(WIFI_SSID)"\"
USER_CFLAGS := $(USER_CFLAGS) -DWIFI_PWD="\"$(WIFI_PWD)"\"
USER_CFLAGS := $(USER_CFLAGS) -DMQTT_URL="\"$(MQTT_URL)"\"
USER_CFLAGS := $(USER_CFLAGS) -DMQTT_HOST="\"$(MQTT_HOST)"\"
# USER_CFLAGS := $(USER_CFLAGS) -DMQTT_PORT="$(MQTT_PORT)"
# USER_CFLAGS := $(USER_CFLAGS) -DMQTT_USERNAME="\"$(MQTT_USERNAME)"\"
USER_CFLAGS := $(USER_CFLAGS) -DOTA_URL="\"$(OTA_URL)"\"
USER_CFLAGS := $(USER_CFLAGS) -DMQTT_PWD="\"$(MQTT_PWD)"\"
ifdef USE_MQTT_PASSWORD
USER_CFLAGS := $(USER_CFLAGS) -DUSE_MQTT_PASSWORD="\"$(USE_MQTT_PASSWORD)"\"
endif
USER_CFLAGS := $(USER_CFLAGS) -DMQTT_PREFIX="\"$(MQTT_PREFIX)"\"