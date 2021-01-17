/*
 * OregonDecoder.h
 *
 *  Created on: 17 Jan 2021
 *      Author: Nicolas Lurkin
 */

#ifndef OREGON_OREGONDECODER_H_
#define OREGON_OREGONDECODER_H_

#include <Arduino.h>

class OregonDecoder {
public:
  OregonDecoder();
  virtual ~OregonDecoder();

  void next_nibble(byte nibble, uint8_t nibble_pos);
  void reset();
  void print();

  bool  is_complete();
  bool  checksum();
  float get_temperature();
  uint16_t get_relative_humidity();

private:
  void decode_data(byte nibble, uint8_t nibble_pos);
  uint32_t read_bcd(byte nibble, size_t pos);

  enum DECODE_STAGE {
    SYNC, SID, CHANNEL, ROLLING, FLAGS, DATA, CS
  };
  DECODE_STAGE _stage;
  bool _got_sync;
  bool _data_complete;
  bool _postamble_complete;
  uint16_t _sensor_id;
  uint8_t _channel;
  uint8_t _rolling;
  uint8_t _flags;
  uint8_t _checksum;
  uint8_t _sum;
  uint16_t _temperature;
  uint16_t _rel_hum;
};

#endif /* OREGON_OREGONDECODER_H_ */
