/*
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2021 Sensnology AB
   Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0 - Maximilian Wache

   DESCRIPTION
   The ESO8266 WiFi Gateway sends data received from sensors to the WiFi link.
   The gateway also accepts input on the WiFi interface, which is then sent out to the radio network.
   To connect all available LED and button features from MySensors a port extender IC 8574(A) is used.

   LED purposes:
   - RX (green) - blink fast on radio message received. In inclusion mode will blink fast only on presentation received
   - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
   - ERR (red) - fast blink on error during transmission error or receive CRC error
   - To use own indication and inclusion-indication handler define MY_INDICATION_HANDLER and MY_INCLUSION_INDICATION_HANDLER.
     Then provide own function handlers indication() and inclusionModeIndication().
     In this example these handlers are connected to the Wire (I2C) port expander 8574(A), see MyIoExpander.h.

   See https://www.mysensors.org/build/connect_radio for wiring instructions.

   If you are using a "barebone" ESP8266, see
   https://www.mysensors.org/build/esp8266_gateway#wiring-for-barebone-esp8266

   Inclusion mode button and LED:
   - Both, button and LED, are also connected to the port expander 8574(A) and are updated by MyIoExpander.h.

   To configure the pins of the port expander see private const values at top of class MyIoExpander.

   Hardware SHA204 signing is currently not supported!

   Make sure to fill in your SSID and WiFi password in the file arduino_secrets.h. 
   You can copy and rename arduino_secrets.h.template to get this.
*/

#include "arduino_secrets.h"

#define SKETCH "My Wemos Gateway"
#define VERSION "V1.0"

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable support for I_DEBUG messages.
#define MY_SPECIAL_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 74880

// Enables and select radio type (if attached)
#define MY_RADIO_RF24

// Set transmission channel
#define MY_RF24_CHANNEL (21)
// RF24 radio network identifier (default 0x00,0xFC,0xE1,0xA8,0xA8)
#define MY_RF24_BASE_RADIO_ID 0x01,0xFC,0xE1,0xA8,0xA9
// RF24_PA_HIGH/RF24_PA_LOW/RF24_PA_MIN/RF24_PA_MAX
#define MY_RF24_PA_LEVEL (RF24_PA_LOW)
// Keep D1 and D2 free for I2C
#define MY_RF24_CE_PIN (D3)

#define MY_GATEWAY_ESP8266

#define MY_WIFI_SSID SECRET_WIFI_SSID
#define MY_WIFI_PASSWORD SECRET_WIFI_PASSWORD
#define MY_HOSTNAME SECRET_HOSTNAME

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
#define MY_IP_ADDRESS SECRET_IP_ADDRESS
#define MY_IP_GATEWAY_ADDRESS SECRET_IP_GATEWAY_ADDRESS
#define MY_IP_SUBNET_ADDRESS SECRET_IP_SUBNET_ADDRESS

// Enable UDP communication
//#define MY_USE_UDP  // If using UDP you need to set MY_CONTROLLER_IP_ADDRESS below

// The port to keep open on node server mode
#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 2

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 20

// Set blinking period
#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Use own indication handler
#define MY_INDICATION_HANDLER

// Use own inclusion indication handler
#define MY_INCLUSION_INDICATION_HANDLER

#include <MySensors.h>
#include <Wire.h>

#include "MyEsp8266Wifi.h"
#include "MyIoExpander.h"

void presentation()
{
  // Present locally attached sensors here
  sendSketchInfo(SKETCH, VERSION);
}

void receive(const MyMessage& message)
{
}

void indication(const indication_t ind)
{
  expander.setLedIndication(ind);
}

void onExpanderInputChange(uint8_t pin, bool state)
{
  static bool auxLedState = false;
  if (pin == 7)
  {
    if (state)
    {
      auxLedState = !auxLedState;
      expander.setAuxLed(auxLedState);
    }
  }
}

void inclusionModeIndication(bool newMode)
{
  expander.setInclusionMode(newMode);
}

void setup()
{
  Wire.begin();
  
  myEsp8266Wifi.setup();
  expander.setup();
  expander.attachOnInputChange(onExpanderInputChange);
}

void loop()
{
  myEsp8266Wifi.loop();
  expander.loop();
}
