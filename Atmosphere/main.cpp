/*
 * main.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>

#include "LCDDisplay.h"
#include "MenuWelcome.h"
#include "MenuBME.h"
#include "ESP8266Wifi.h"
#include "MQTTControl.h"
#include "BME280Reader.h"
#include "OregonSetup.h"

ESP8266Wifi wifi(&Serial2);
MQTTClient mqtt(wifi, "arduino");
MQTTControl pubServer(mqtt, "arduino");

OregonSetup oregon;

const char ssid[] = {""};
const char pwd[]  = {""};
const char serverHost[] = {""};

LCDDisplay lcd(100);
MenuWelcome _m_welcome(lcd.get_lcd_handle());
MenuBME     _m_bme(lcd.get_lcd_handle());

BME280Reader bme1;

unsigned long last_millis;
unsigned long heartbeat_millis;
unsigned long reset_failure;
bool connected;

void setup() {
  connected = false;
  Serial.begin(115200);
  Serial2.begin(115200);

  lcd.add_menu(&_m_welcome);
  lcd.add_menu(&_m_bme);

  mqtt.setUserPass("ardhome", "huY!rd89%");

  wifi.init(ssid, pwd, true, false);

  bme1.init(&_m_bme);
  oregon.init(19);

  Serial.println("Stating publication server");
  pubServer.setDestination(serverHost, 1883);
  pubServer.begin();

  bme1.setPublicationServer(&pubServer);
  oregon.setPublicationServer(&pubServer);

  Serial.println("Advertising services");
  pubServer.advertise();

  last_millis = millis();
  heartbeat_millis = millis();

  bme1.updateAll();
  pubServer.advertise();
  pubServer.serve(true);

  connected = true;
  _m_welcome.connected(true);

  reset_failure = 0;
}

bool watchdog() {
  bool watchdog_ok = true;
  if(!wifi.isConnected()){ //Indicate connection lost
    if(!wifi.checkWifiConnection()){ //Double check, maybe only IP lost and recovered afterward
      _m_welcome.connected(false);
      connected = false;
      //Seems we really lost the wifi...
      watchdog_ok = false;
      if(reset_failure>10) {// Wifi reconnect failed many times. Try a full RESET of the ESP
        watchdog_ok = wifi.init(ssid, pwd, true, false);
        reset_failure = 0;
        if(watchdog_ok)
          pubServer.begin();
      }
      else { // No RESET at this point, just try to reconnect
        wifi.connectWifi(ssid, pwd);
        delay(1000);
        watchdog_ok = wifi.checkWifiConnection();
        if(!watchdog_ok)
          ++reset_failure;
      }
    }
  }
  // watchdog_ok is true if we have connection, or if we managed to recover a connection
  if(watchdog_ok && !connected) {
    _m_welcome.connected(true);
    connected = true;
  }
  return watchdog_ok;
}

void loop() {
  if((millis()-last_millis) % 30000)
    if(!watchdog()) // Something wrong, we cannot proceed
      return;

  oregon.updateAll(); //Needs constant update. Does nothing if does not read 433MHz signal, but locks once receiving.

  lcd.tick();
  if(millis()-last_millis>60000){
    // Update every minute
    bme1.updateAll();
    last_millis = millis();
  }

  bool forceUpdate = false;
  if(millis()-heartbeat_millis > 600000) {
    // Force an update every 10 minute as a heartbeat
    forceUpdate = true;
    heartbeat_millis = millis();
  }
  pubServer.serve(forceUpdate);
}
