/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2021 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Maximilian Wache
 *
 */


#pragma once

#include <ArduinoOTA.h>

class MyEsp8266Wifi
{
  public:
    MyEsp8266Wifi() {}

    void setup()
    {
      initWiFi();
    }

    void loop()
    {
      // check for WiFi OTA updates
      ArduinoOTA.handle();
    }

    void yield()
    {
      // check for WiFi OTA updates
      ArduinoOTA.handle();
    }

  private:
    void printWiFiStatus()
    {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.print("WiFi connection failed. Status value: ");
        Serial.println(WiFi.status());
      } else {

        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());

        Serial.print("Local IP Address: ");
        IPAddress ip = WiFi.localIP();
        Serial.println(ip);

        Serial.print("Signal strength (RSSI): ");
        long rssi = WiFi.RSSI();
        Serial.print(rssi);
        Serial.println(" dBm");
      }
    }

    void initOTA()
    {
      // Port defaults to 8266
      // ArduinoOTA.setPort(8266);

      // Hostname defaults to esp8266-[ChipID]
      ArduinoOTA.setHostname(SECRET_OTA_HOSTNAME);

      // No authentication by default
      ArduinoOTA.setPassword(SECRET_OTA_PASSWORD);

      // Password can be set with it's md5 value as well
      // ArduinoOTA.setPasswordHash(SECRET_OTA_PASSWORD_HASH);

      ArduinoOTA.onStart([]() {
        Serial.println("Start updating sketch");
      });
      ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.print("OTA Progress: ");
        Serial.print(progress * 100 / total);
        Serial.println('%');
      });
      ArduinoOTA.onError([](ota_error_t error) {
        Serial.print("Error[");
        Serial.print(error);
        Serial.print("]: ");
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
      });
      ArduinoOTA.begin();
    }

    void initWiFi()
    {
#ifdef MY_IP_ADDRESS
      WiFi.setAutoConnect(false);
#else
      WiFi.setAutoConnect(true);
#endif
      printWiFiStatus();
      initOTA();
    }
};

MyEsp8266Wifi myEsp8266Wifi;
