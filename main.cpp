/*
 * main.cpp
 *
 *  Created on: 2 Jul 2017
 *      Author: nicol
 */

#include <Arduino.h>
#include "AutomaticWater.h"

AutomaticWater waterSystem;

void setup(){
   Serial.begin(115200);

   waterSystem.setTickInterval(1000);
   waterSystem.initSystem();
}

void loop(){
	waterSystem.tick();
	delay(100);
}

