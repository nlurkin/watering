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
  if(_postamble_complete || nibble_pos>_data_offset[PAEND]) // Got all data. Rest are repetitions.
    return;

  if(nibble_pos>0 && (nibble_pos<_data_end_nibble))
    _sum += nibble;

  size_t pos;
  switch(_nibble_dict[nibble_pos]){
  case SYNC:
    _got_sync = nibble == 0xA;
    break;
  case ID:
    pos = nibble_pos - _data_offset[ID];
    _sensor_id |= nibble << pos*4;
    break;
  case CHANNEL:
    determine_sensor(); // Information available now
    _channel = nibble;
    break;
  case ROLLING:
    pos = nibble_pos - _data_offset[ROLLING];
    _rolling |= nibble << pos*4;
    break;
  case FLAG:
    _flags = nibble;
    break;
  default:
    decode_data(nibble, nibble_pos);
  }
}

void OregonDecoder::reset() {
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
  _nibble_dict[0] = SYNC;
  _nibble_dict[1] = ID;
  _nibble_dict[2] = ID;
  _nibble_dict[3] = ID;
  _nibble_dict[4] = ID;
  _nibble_dict[5] = CHANNEL;
  _nibble_dict[6] = ROLLING;
  _nibble_dict[7] = ROLLING;
  _nibble_dict[8] = FLAG;
  _data_offset[SYNC] = 0;
  _data_offset[ID] = 1;
  _data_offset[CHANNEL] = 5;
  _data_offset[ROLLING] = 6;
  _data_offset[FLAG] = 8;
  // We should know by 8th nibble. Else there is a problem and we can stop in any case
  _data_offset[PAEND] = 8;
  _data_end_nibble = 8;
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

void OregonDecoder::decode_data(byte nibble, uint8_t nibble_pos) {
  size_t pos;
  switch(_nibble_dict[nibble_pos]){
  case TEMP:
    pos = nibble_pos - _data_offset[TEMP];
    _temperature += read_bcd(nibble, pos);
    break;
  case TEMP_SIGN:
    if(nibble!=0)
    _temperature *= -1;
    break;
  case HUMIDITY:
    pos = nibble_pos - _data_offset[HUMIDITY];
    _rel_hum += read_bcd(nibble, pos);
    break;
  case EMPTY:
    //Nothing to do
    break;
  case CHECKSUM:
    pos = nibble_pos - _data_offset[CHECKSUM];
    _checksum |= nibble << pos*4;
    break;
  case PAEND:
    _postamble_complete = true;
    break;
  default:
    break;
  }
}

uint32_t OregonDecoder::read_bcd(byte nibble, size_t pos) {
  uint32_t mult = 1;
  for(size_t i=1; i<=pos; ++i)
    mult *= 10;
  return nibble * mult;
}

/*
 * For THGR221
 * 0    : A     (sync)
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
 *
 * For THN132N
 * 0    : A     (sync)
 * 1-4  : EC40  (ID)
 * 5    : 1     (channel)
 * 6-7  : 49    (Rolling)
 * 8    : 0     (Flag)
 * 9-11 : 332   (Temp)
 * 12   : 0     (Temp sign)
 * 13-14: 24    (Checksum)
 * 15   : FC    (post-amble)
 */
void OregonDecoder::determine_sensor() {
  switch(_sensor_id) {
  case 0x02D1:
  case 0x428F:
  case 0x4B8F:
    _nibble_dict[9]  = TEMP;
    _nibble_dict[10] = TEMP;
    _nibble_dict[11] = TEMP;
    _nibble_dict[12] = TEMP_SIGN;
    _nibble_dict[13] = HUMIDITY;
    _nibble_dict[14] = HUMIDITY;
    _nibble_dict[15] = EMPTY;
    _nibble_dict[16] = CHECKSUM;
    _nibble_dict[17] = CHECKSUM;
    _nibble_dict[18] = PAEND;
    _data_offset[TEMP] = 9;
    _data_offset[TEMP_SIGN] = 12;
    _data_offset[HUMIDITY] = 13;
    _data_offset[EMPTY] = 15;
    _data_offset[CHECKSUM] = 16;
    _data_offset[PAEND] = 18;
    _data_end_nibble = 16;
    break;
  case 0x04CE:
  case 0x448C:
    _nibble_dict[9]  = TEMP;
    _nibble_dict[10] = TEMP;
    _nibble_dict[11] = TEMP;
    _nibble_dict[12] = TEMP_SIGN;
    _nibble_dict[13] = CHECKSUM;
    _nibble_dict[14] = CHECKSUM;
    _nibble_dict[15] = PAEND;
    _data_offset[TEMP] = 9;
    _data_offset[TEMP_SIGN] = 12;
    _data_offset[CHECKSUM] = 13;
    _data_offset[PAEND] = 15;
    _data_end_nibble = 13;
  }
}

uint16_t OregonDecoder::get_sensor_id() {
  return _sensor_id;
}

uint8_t OregonDecoder::get_rolling_code() {
  return _rolling;
}

uint8_t OregonDecoder::get_flags() {
  return _flags;
}

bool OregonDecoder::has_temperature() {
  switch(_sensor_id) {
  case 0x02D1:
  case 0x428F:
  case 0x4B8F:
  case 0x04CE:
  case 0x448C:
    return true;
    break;
  }
  return false;
}

bool OregonDecoder::has_humidity() {
  switch(_sensor_id) {
  case 0x02D1:
  case 0x428F:
  case 0x4B8F:
    return true;
    break;
  case 0x04CE:
  case 0x448C:
    return false;
    break;
  }
  return false;
}
