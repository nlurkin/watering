/*
 * main.cpp
 *
 *  Created on: 2 Jul 2017
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "AutomaticWater.h"

//TODO add detection of sensor failure and permanently disable pump
//TODO add detection of empty tank and permanently disable pump

AutomaticWater waterSystem(22);

void setup(){
   Serial.begin(115200);

   // Set tick at 1s - used for pump and sensor, not for display and buttons
   waterSystem.setTickInterval(1000);
   waterSystem.addCircuit(10, 50, 40);
   waterSystem.addCircuit(11, 51, 41);
   waterSystem.initSystem();
}

void loop(){
	// But run at 0.1s
	// This ensures enough reactivity when buttons are pressed
	waterSystem.tick();
	delay(100);
}

