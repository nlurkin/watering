/*
 * OregonReader.cpp
 *
 * http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf
 * https://www.osengr.org/WxShield/Downloads/Weather-Sensor-RF-Protocols.pdf
 *
 *  Created on: 16 Jan 2021
 *      Author: Nicolas Lurkin
 */

#include "OregonReader.h"

uint8_t OregonReader::reader_pin = 0;

volatile word rwidth;
volatile OregonReader::RF_STATE pin_state;

void interrupt_width(void) {
  pin_state = digitalRead(OregonReader::reader_pin) == 0 ? OregonReader::RF_FALLING : OregonReader::RF_RISING;
  static word last;
  rwidth = micros() - last;
  last += rwidth;
}

void OregonReader::setup(uint8_t pin) {
  reader_pin = pin;
  pinMode(reader_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(reader_pin), interrupt_width, CHANGE);
}

OregonReader::OregonReader() :
  _available(false)
{
  reset();
}

OregonReader::~OregonReader() {
}

bool OregonReader::loop() {
  cli();
  word width = rwidth;
  rwidth = 0;
  sei();
  if (width != 0)
    return next_width(width, get_state());

  return false;
}

OregonReader::RF_STATE OregonReader::get_state() {
  return pin_state;
}

bool OregonReader::next_width(word width, RF_STATE state) {
  if (width > 40000) { // End of message ~ About 9-10 clock cycles without signal
    reset();
    return false;
  }

  if (width < 200 || width > 1400) { // Definitely outside allowed range
    reset();
    return false;
  }

  byte is_long = 0;
  if (state == RF_FALLING)
    is_long = width >= 615;
  else if (state == RF_RISING)
    is_long = width >= 850;

  return decode(is_long, state);
}

int8_t OregonReader::read_bit(byte is_long, RF_STATE state) {
  _half_time += is_long ? 2 : 1;

  if (_half_time % 2) { // Even, we are on a bit edge
    return add_bit(state);
  } else { // Odd, we are on a clock edge
    // No data, check for error
    if (is_long) { // Invalid, cannot have two longs in a row on a clock edge, this would mean we didn't get a transition on the bit edge, which is against the protocol.
      reset();
    }
  }
  return -1;
}

bool OregonReader::decode(byte rf_long, RF_STATE state) {
  if (_state == LOCKING) {
    //Waiting for a series of repeated bits (shorts or long)
    if (!rf_long){
      if(_prev_long){
        // We got a short during long Locking, this is not supposed to happen so this is
        // probably not a preamble sequence.
        reset();
      }
      else
        ++_nshorts; // We might be in a short Locking
    }
    else {
      if(!_prev_long){
        // We got a long during short Locking, this is not supposed to happen so this is
        // probably not a preamble sequence.
        reset();
      }
      else
        ++_nlongs; // We might be in a long Locking
    }

    // Check whether we reached a correct number to decide this was a preamble.
    if (_nshorts > 16 && state == RF_FALLING) { // Corresponds to 8 1s in shorts sequence
      // Can reasonably be guessed we got a preamble sequence
      _state = SYNCINC;
      _protocol = V3;
      ++_half_time;
      ++_ht_offset;
      _double_bit = true;
    }
    if (_nlongs > 7 && state == RF_FALLING) { // Corresponds to 7 "01"s
      // Can reasonably be guessed we got a preamble sequence
      _state = SYNCINC;
      _protocol = V2;
      ++_half_time;
      ++_ht_offset;
      _double_bit = true;
    }

		// Record which kind of series we have
    _prev_long = rf_long;
  } else if (_state == SYNCINC) {
    // Waiting for the first 0 (RISING when _half_time is even and double_bit = false)

    _ht_offset += rf_long ? 2 : 1;
    int8_t val = read_bit(rf_long, state);
    if (val == 0 && _double_bit) {
      // First 0 bit of SYNC
      _state = DATA;
    } else if(val == 0 && !_double_bit) { // Happens only for V2
      // Checking bit
    } else if (val == 1) {// Else still a 1. Do not record it in the nibble
      _nibble = 0;
    }
  } else if (_state == DATA) // Keep reading
    read_bit(rf_long, state);

  // True if we are definitely inside a communication
  return _state != LOCKING;
}

byte OregonReader::add_bit(RF_STATE state) {
  int divider = 2;
  if(_protocol == V2){
    _double_bit = !_double_bit;
    divider = 4;
  }

  uint8_t nibble_pos = ((_half_time - _ht_offset) / divider) % 4;
  byte val = state == RF_FALLING;

  if(_protocol == V2 && !_double_bit) // This is the first bit of the pair. It is inverted
    return val;

  _nibble |= val << nibble_pos;
  if (nibble_pos == 3) {
    // Nibble is complete -> send it to the decoder
    _decoder.next_nibble(_nibble, _nibble_num++);
    _nibble = 0;
  }
  return val;
}

void OregonReader::reset() {
  end_of_message();
  if (_nibble_num > 0 && !_available) // Do not bother resetting the decoder if we did not put anything inside yet. Also do not reset if not read.
    _decoder.reset();
  _half_time = 0;
  _state = LOCKING;
  _nshorts = 0;
  _nlongs = 0;
  _ht_offset = 0;
  _nibble_num = 0;
  _nibble = 0;
  _double_bit = false;
  _protocol = VUNKNOWN;
}

void OregonReader::end_of_message() {
  if (_decoder.is_complete())
    _available = true;
}

bool OregonReader::available() {
  return _available;
}

OregonDecoder OregonReader::get_data() {
  OregonDecoder temp(_decoder);
  _available = false;
  _decoder.reset();
  return temp;
}
