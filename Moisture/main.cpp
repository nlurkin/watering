/*
 * main.cpp
 *
 *  Created on: 2 Jul 2017
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "AutomaticWater.h"
#include "ESP8266Wifi.h"
#include "MQTTStream.h"
#include "MQTTControl.h"

//TODO add detection of sensor failure and permanently disable pump
//TODO add detection of empty tank and permanently disable pump

AutomaticWater waterSystem(22);
ESP8266Wifi wifi;
MQTTClient mqtt(wifi, "arduino");
MQTTStream mySerial(mqtt);
MQTTControl pubServer(mqtt, "arduino");

const char ssid[] = {""};
const char pwd[]  = {""};
const char serverIP[] = {""};

unsigned long last_millis;
unsigned long heartbeat_millis;

void setup(){
  Serial.begin(115200);
  Serial1.begin(115200);

  Serial.println(F("----- Arduino WIFI -----"));
  wifi.init(ssid, pwd, true, true);
  mySerial.setDestination(serverIP, 8000);
  mySerial.begin();
  pubServer.setDestination(serverIP, 1883);
  pubServer.begin();

  mySerial.println("Arduino running");

  // Set tick at 1s - used for pump and sensor, not for display and buttons
  waterSystem.setTickInterval(1000);
  waterSystem.addCircuit(10, 50, 40);
  waterSystem.addCircuit(11, 51, 41);
  waterSystem.initSystem();
  waterSystem.setPublicationServer(&pubServer);

  uint8_t trials = 0;
  while(!pubServer.advertise() && trials++<10) {}
  last_millis = millis();
  heartbeat_millis = millis();
}

void loop(){
  if(!wifi.isConnected()){
    //Seems we lost the wifi... try to reconnect
    wifi.connectWifi(ssid, pwd);
    wifi.checkWifiConnection();
  }

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    wifi.sendSomething(command.c_str());
  }
  else if (mySerial.available() > 0) {
    String command = mySerial.readStringUntil('\n');
    wifi.sendSomething(command.c_str());
  }

  pubServer.listen();

  // But run at 0.1s
  // This ensures enough reactivity when buttons are pressed
  if(millis()-last_millis>100){
    waterSystem.tick();
    last_millis = millis();
    //delay(100);
  }
  bool forceUpdate = false;
  if(millis()-heartbeat_millis > 60000) {
    // Force an update every minute as a heartbeat
    forceUpdate = true;
    heartbeat_millis = millis();
  }
  pubServer.serve(forceUpdate);
}

