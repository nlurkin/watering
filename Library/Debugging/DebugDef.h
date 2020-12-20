/*
 * DebugDef.hh
 *
 *  Created on: 20 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef DEBUGGING_DEBUGDEF_HH_
#define DEBUGGING_DEBUGDEF_HH_

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#ifdef DEBUG
#include <Arduino.h>
#define DEBUG_PLN(str)                \
   Serial.print(millis());            \
   Serial.print(": ");                \
   Serial.print(__PRETTY_FUNCTION__); \
   Serial.print(' ');                 \
   Serial.print(__FILE__);            \
   Serial.print(':');                 \
   Serial.print(__LINE__);            \
   Serial.print(' ');                 \
   Serial.println(str);
#define DEBUG_PRAW(str)\
   Serial.print(str);
#define DEBUG_P(str)                  \
   Serial.print(millis());            \
   Serial.print(": ");                \
   Serial.print(__PRETTY_FUNCTION__); \
   Serial.print(' ');                 \
   Serial.print(__FILE__);            \
   Serial.print(':');                 \
   Serial.print(__LINE__);            \
   Serial.print(' ');                 \
   Serial.print(str);
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRAW(str)
#define DEBUG_P(str)
#endif

#endif

#endif /* DEBUGGING_DEBUGDEF_HH_ */
