/*
 * main.cpp
 *
 *  Created on: 2 Jul 2017
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "AutomaticWater.h"

AutomaticWater waterSystem;

void setup(){
   Serial.begin(115200);

   // Set tick at 1s - used for pump and sensor, not for display and buttons
   waterSystem.setTickInterval(1000);
   waterSystem.initSystem();
}

void loop(){
	// But run at 0.1s
	// This ensures enough reactivity when buttons are pressed
	waterSystem.tick();
	delay(100);
}

