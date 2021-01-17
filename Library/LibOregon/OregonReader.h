/*
 * OregonReader.h
 *
 *  Created on: 16 Jan 2021
 *      Author: Nicolas Lurkin
 */

#ifndef OREGON_OREGONREADER_H_
#define OREGON_OREGONREADER_H_

#include <Arduino.h>

#include "OregonDecoder.h"

void interrupt_width(void);

class OregonReader {
public:
  enum RF_STATE {
    RF_RISING = 0, RF_FALLING = 1
  };
  static void setup(uint8_t pin);

  OregonReader();
  virtual ~OregonReader();

  bool loop();
  bool available();
  OregonDecoder get_data();

  static uint8_t reader_pin;
private:
  enum REC_STATE {
    WAITING, LOCKING, SYNCINC, DATA
  };
  RF_STATE get_state();
  void reset();
  void end_of_message();
  bool next_width(word width, RF_STATE state);
  bool decode(byte rf_long, RF_STATE state);

  byte add_bit(RF_STATE state);
  int8_t read_bit(byte rf_long, RF_STATE state);

  REC_STATE _state;
  bool _available;
  byte _nibble;
  uint8_t _nibble_num;
  int _nshorts;
  unsigned long int _half_time;
  unsigned long int _ht_offset;
  OregonDecoder _decoder;
};

#endif /* OREGON_OREGONREADER_H_ */
