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
  uint16_t get_sensor_id();
  uint8_t get_rolling_code();
  uint8_t get_flags();

private:
  void decode_data(byte nibble, uint8_t nibble_pos);
  uint32_t read_bcd(byte nibble, size_t pos);
  void determine_sensor();

  enum DATA_BLOCK {
    SYNC, ID, CHANNEL, ROLLING, FLAG, TEMP, TEMP_SIGN, HUMIDITY, EMPTY, CHECKSUM, PAEND
  };
  bool _got_sync;
  bool _postamble_complete;
  uint8_t _channel;
  uint8_t _rolling;
  uint8_t _flags;
  uint8_t _checksum;
  uint8_t _sum;
  uint8_t _data_end_nibble;
  uint16_t _sensor_id;
  uint16_t _temperature;
  uint16_t _rel_hum;
  uint8_t _data_offset[PAEND];
  DATA_BLOCK _nibble_dict[20];
};

#endif /* OREGON_OREGONDECODER_H_ */
