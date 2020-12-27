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
#define DEBUGS_PLN(ser, str)                \
   ser.print(millis());            \
   ser.print(": ");                \
   ser.print(__PRETTY_FUNCTION__); \
   ser.print(' ');                 \
   ser.print(__FILE__);            \
   ser.print(':');                 \
   ser.print(__LINE__);            \
   ser.print(' ');                 \
   ser.println(str);
#define DEBUGS_PRAW(ser, str)\
   ser.print(str);
#define DEBUGS_PRAWLN(ser, str)\
   ser.println(str);
#define DEBUGS_P(ser, str)                  \
   ser.print(millis());            \
   ser.print(": ");                \
   ser.print(__PRETTY_FUNCTION__); \
   ser.print(' ');                 \
   ser.print(__FILE__);            \
   ser.print(':');                 \
   ser.print(__LINE__);            \
   ser.print(' ');                 \
   ser.print(str);
#define DEBUG_PLN(str) DEBUGS_PLN(Serial, str)
#define DEBUG_PRAW(str) DEBUGS_PRAW(Serial, str)
#define DEBUG_PRAWLN(str) DEBUGS_PRAWLN(Serial, str)
#define DEBUG_P(str) DEBUGS_P(Serial, str)
#else
#define DEBUG_PLN(str)
#define DEBUG_PRAW(str)
#define DEBUG_PRAWLN(str)
#define DEBUG_P(str)
#define DEBUGS_PLN(ser, str)
#define DEBUGS_PRAW(ser, str)
#define DEBUGS_PRAWLN(ser, str)
#define DEBUGS_P(ser, str)
#endif

#endif

#endif /* DEBUGGING_DEBUGDEF_HH_ */
