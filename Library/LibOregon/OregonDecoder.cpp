/*
 * OregonDecoder.cpp
 *
 *  Created on: 17 Jan 2021
 *      Author: Nicolas Lurkin
 */

#include "OregonDecoder.h"

OregonDecoder::OregonDecoder() {
  reset();
}

OregonDecoder::~OregonDecoder() {
}

void OregonDecoder::next_nibble(byte nibble, uint8_t nibble_pos) {
  if(_postamble_complete) // Got all data. Rest are repetitions.
    return;

  if(nibble_pos>0 && !_data_complete)
    _sum += nibble;
  if(nibble_pos==0) {
    _got_sync = nibble == 0xA;
  }
  else if(nibble_pos<=4) {
    size_t pos = nibble_pos-1;
    _sensor_id |= nibble << pos*4;
  }
  else if(nibble_pos==5) {
    _channel = nibble;
  }
  else if(nibble_pos<=7) {
    size_t pos = nibble_pos - 6;
    _rolling |= nibble << pos*4;
  }
  else if(nibble_pos==8) {
    _flags = nibble;
  }
  else {
    decode_data(nibble, nibble_pos);
  }
}

void OregonDecoder::reset() {
  _stage = SYNC;
  _got_sync = false;
  _sensor_id = 0;
  _channel = 0;
  _rolling = 0;
  _flags = 0;
  _checksum = 0;
  _sum = 0;
  _temperature = 0;
  _rel_hum = 0;
  _postamble_complete = false;
  _data_complete = false;
}

void OregonDecoder::print() {
  Serial.print("Sensor: ");
  Serial.print(_sensor_id, HEX);
  Serial.print("  Channel: ");
  Serial.println(_channel, HEX);
  Serial.print("RCode : ");
  Serial.print(_rolling, HEX);
  Serial.print("    Flag   : ");
  Serial.println(_flags, BIN);
  Serial.print("Temp  : ");
  Serial.print(get_temperature());
  Serial.print(" RH     : ");
  Serial.print(get_relative_humidity());
  Serial.println("%");
  Serial.print("CS    : ");
  Serial.print(_checksum, HEX);
  Serial.print("    Sum    : ");
  Serial.print(_sum, HEX);
  Serial.print(" => ");
  Serial.println(checksum() ? "Valid" : "Invalid");
}

float OregonDecoder::get_temperature() {
  return _temperature * 0.1;
}

uint16_t OregonDecoder::get_relative_humidity() {
  return _rel_hum;
}

bool OregonDecoder::checksum() {
  return _checksum==_sum;
}

bool OregonDecoder::is_complete() {
  return _postamble_complete;
}

/*
 * For THGR221
 * 0   : A     (sync)
 * 1-4  : F824  (ID)
 * 5    : 1     (channel)
 * 6-7  : 49    (Rolling)
 * 8    : 0     (Flag)
 * 9-11 : 332   (Temp)
 * 12   : 0     (Temp sign)
 * 13-14: 43    (Humidity)
 * 15   : 8     (Unknown)
 * 16-17: 24    (Checksum)
 * 18-19: FC    (post-amble)
 */
void OregonDecoder::decode_data(byte nibble, uint8_t nibble_pos) {
  if(nibble_pos<=11) {
    size_t pos = nibble_pos - 9;
    _temperature += read_bcd(nibble, pos);
  }
  else if(nibble_pos==12) {
    if(nibble!=0)
    _temperature *= -1;
  }
  else if(nibble_pos<=14) {
    size_t pos = nibble_pos - 13;
    _rel_hum += read_bcd(nibble, pos);
  }
  else if(nibble_pos==15) {
    //Nothing to do
    _data_complete = true;
  }
  else if(nibble_pos<=17) {
    size_t pos = nibble_pos - 16;
    _checksum |= nibble << pos*4;
  }
  else
    _postamble_complete = true;
}

uint32_t OregonDecoder::read_bcd(byte nibble, size_t pos) {
  uint32_t mult = 1;
  for(size_t i=1; i<=pos; ++i)
    mult *= 10;
  return nibble * mult;
}
