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
#define DEBUG_PLN(ser, str)                \
   ser.print(millis());            \
   ser.print(": ");                \
   ser.print(__PRETTY_FUNCTION__); \
   ser.print(' ');                 \
   ser.print(__FILE__);            \
   ser.print(':');                 \
   ser.print(__LINE__);            \
   ser.print(' ');                 \
   ser.println(str);
#define DEBUG_PRAW(ser, str)\
   ser.print(str);
#define DEBUG_P(ser, str)                  \
   ser.print(millis());            \
   ser.print(": ");                \
   ser.print(__PRETTY_FUNCTION__); \
   ser.print(' ');                 \
   ser.print(__FILE__);            \
   ser.print(':');                 \
   ser.print(__LINE__);            \
   ser.print(' ');                 \
   ser.print(str);
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRAW(str)
#define DEBUG_P(str)
#endif

#endif

#endif /* DEBUGGING_DEBUGDEF_HH_ */
