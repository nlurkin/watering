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
#include "ControlServer.h"
#include "BME280Reader.h"

ESP8266Wifi wifi;
ControlServer pubServer(wifi);

const char ssid[] = {""};
const char pwd[]  = {""};
const char serverIP[] = {""};

LCDDisplay lcd(100);
MenuWelcome _m_welcome(lcd.get_lcd_handle());
MenuBME     _m_bme(lcd.get_lcd_handle());

BME280Reader bme1;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  wifi.init(ssid, pwd, false, true);

  lcd.add_menu(&_m_welcome);
  lcd.add_menu(&_m_bme);

  bme1.init(&_m_bme);

  Serial.println("Stating publication server");
  pubServer.setDestination(serverIP, 8000);
  pubServer.begin(80);

  Serial.println("Advertising services");
  pubServer.advertise();
}

int interval = 0;

void loop() {

  lcd.tick();
  if(interval % 20 == 0){
    bme1.updateAll();
    interval = 0;
  }
  ++interval;
  delay(100);
}
