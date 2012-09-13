/*--------------------------------------------------------------------------+
| Portions of this code are:
|
| Copyright 2008-2012 Technische Universitaet Muenchen                     |
|                                                                          |
| Licensed under the Apache License, Version 2.0 (the "License");          |
| you may not use this file except in compliance with the License.         |
| You may obtain a copy of the License at                                  |
|                                                                          |
|    http://www.apache.org/licenses/LICENSE-2.0                            |
|                                                                          |
| Unless required by applicable law or agreed to in writing, software      |
| distributed under the License is distributed on an "AS IS" BASIS,        |
| WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
| See the License for the specific language governing permissions and      |
| limitations under the License.                                           |
| Source: http://code.google.com/p/protobuf-embedded-c/source/browse/trunk/edu.tum.cs.ccts.protobuf.embedded/src/edu/tum/cs/ccts/protobuf/embedded/embedded-c-file.stg
| 
| Everything else:
| Copyright 2012 Brainpage Technology Ltd.
+--------------------------------------------------------------------------*/

#include "sensocol_encoding.h"

sc_uint32_t encode_zig_zag32(sc_int32_t n) {
    // Note:  the right-shift must be arithmetic
    return (n << 1) ^ (n >> 31);
}
sc_uint64_t encode_zig_zag64(sc_int64_t n) {
    // Note:  the right-shift must be arithmetic
    return (n << 1) ^ (n >> 63);
}


sc_uint16_t sensocol_size_int32_encoded(sc_int32_t value)
{
  sc_uint32_t value2;
  sc_uint16_t offset = 0;
  value2 = encode_zig_zag32(value);
  while (1) {
        if ((value2 & ~0x7F) == 0) {
            offset++;
            return offset;
        } else {
            offset++;
            value2 = value2 >> 7;
        }
    }
    return offset;
}

sc_uint16_t sensocol_write_raw_byte(sc_byte_t value, void *_buffer, sc_uint16_t offset) {
    *((sc_byte_t *)_buffer + offset) = value;
    return ++offset;
}

sc_uint16_t write_raw_varint32(sc_uint32_t value, void *_buffer, sc_uint16_t offset) {
    while (1) {
        if ((value & ~0x7F) == 0) {
            offset = sensocol_write_raw_byte((char)value, _buffer, offset);
            return offset;
        } else {
            offset = sensocol_write_raw_byte((char)((value & 0x7F) | 0x80), _buffer, offset);
            value = value >> 7;
        }
    }
    return offset;
}
sc_uint16_t write_raw_varint64(sc_uint64_t value, void *_buffer, sc_uint16_t offset) {
    while (1) {
        if ((value & ~0x7F) == 0) {
            offset = sensocol_write_raw_byte((char)value, _buffer, offset);
            return offset;
        } else {
            offset = sensocol_write_raw_byte((char)((value & 0x7F) | 0x80), _buffer, offset);
            value = value >> 7;
        }
    }
    return offset;
}

/** Write a little endian 16-bit integer */
sc_uint16_t sensocol_write_raw_little_endian16(sc_uint16_t value, void *_buffer, sc_uint16_t offset){
  offset = sensocol_write_raw_byte((char)(value & 0xFF), _buffer, offset);
  offset = sensocol_write_raw_byte((char)((value >> 8) & 0xFF), _buffer, offset);
  return offset;
}

/** Write a little-endian 32-bit integer. */
sc_uint16_t sensocol_write_raw_little_endian32(sc_uint32_t value, void *_buffer, sc_uint16_t offset) {
    offset = sensocol_write_raw_byte((char)((value      ) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >>  8) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 16) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 24) & 0xFF), _buffer, offset);
    
    return offset;
}

/** Write a little-endian 64-bit integer. */
sc_uint16_t sensocol_write_raw_little_endian64(sc_uint64_t value, void *_buffer, sc_uint16_t offset) {
    offset = sensocol_write_raw_byte((char)((value      ) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >>  8) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 16) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 24) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 32) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 40) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 48) & 0xFF), _buffer, offset);
    offset = sensocol_write_raw_byte((char)((value >> 56) & 0xFF), _buffer, offset);
    
    return offset;
}

sc_uint16_t sensocol_write_raw_bytes(void *_buffer, sc_uint16_t offset, sc_byte_t *bytes, sc_uint16_t length)
{
  sc_uint16_t i;
  for(i=0;i<length;i++){
    offset = sensocol_write_raw_byte(bytes[i], _buffer, offset);
  }
  return offset;
}

sc_uint16_t sensocol_write_long(void *_buffer, sc_uint16_t offset, sc_int32_t value)
{
  return write_raw_varint32(encode_zig_zag32(value), _buffer, offset);
}
sc_uint16_t sensocol_write_long_long(void *_buffer, sc_uint16_t offset, sc_int64_t value)
{
  return write_raw_varint64(encode_zig_zag64(value), _buffer, offset);
}


sc_uint16_t sensocol_write_bytes(void *_buffer, sc_uint16_t offset, sc_byte_t *bytes, sc_uint16_t length)
{
  sc_uint16_t i;
  
  // We first write the length of these bytes using the varsc_uint16_t format.  Then the actual bytes
  offset = write_raw_varint32(encode_zig_zag32(length), _buffer, offset);
  for(i=0;i<length;i++){
    offset = sensocol_write_raw_byte(bytes[i], _buffer, offset);
  }
  return offset;
}


