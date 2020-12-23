/*
 * AutomaticWaterConfig.h
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef AUTOMATICWATERCONFIG_H_
#define AUTOMATICWATERCONFIG_H_

#include <Arduino.h>

namespace AW{
  enum MainMode {MAIN_MODE_MONITOR, MAIN_MODE_CALIB, MAIN_MODE_SHOW};     /** enum to identify the main modes */
  static constexpr uint8_t MAX_SENSORS = 5;
}; /* namespace AW */



#endif /* AUTOMATICWATERCONFIG_H_ */
