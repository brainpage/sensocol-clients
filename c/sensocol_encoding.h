/*
/
/ Copyright 2012 Brainpage Ltd.
/ Licensed under the Apache License, Version 2.0 (the "License");
/ you may not use this file except in compliance with the License.
/ You may obtain a copy of the License at
/
/ http://www.apache.org/licenses/LICENSE-2.0
/
/ Unless required by applicable law or agreed to in writing, software
/ distributed under the License is distributed on an "AS IS" BASIS,
/ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/ See the License for the specific language governing permissions and
/   limitations under the License.
/ 
/ Sensocol Encoding:
/ Functions for encoding sensocol data
*/

#ifndef _SENSOCOL_ENCODING_H
#define _SENSOCOL_ENCODING_H

#include "sc_platform.h"
#include "sensocol.h"
/**
 * \brief       Writes an array of bytes to the output buffer
 * 
 * \param _buffer   The output buffer to write to
 * \param offset    The starting offset to begin writing at
 * \param bytes     The bytes to actually Write
 * \param length    The length of the bytes to write.
 *
 * \return          The new offset position (input offset + amount written)
 */
sc_uint16_t sensocol_write_bytes(void *_buffer, sc_uint16_t offset, sc_byte_t *bytes, sc_uint16_t length);

/**
 * \brief       Writes a long (32bit signed integer) to the output buffer.  This uses the Varsc_uint16_t Zig-Zag encoding
 * 
 * \param _buffer   The output buffer to write to
 * \param offset    The starting offset to begin writing at
 * \param value     The long value to write
 *
 * \return          The new offset position (input offset + amount written)
 */
sc_uint16_t sensocol_write_long(void *_buffer, sc_uint16_t offset, sc_int32_t value);

/**
 * \brief       Writes a 64bit signed integer to the output buffer
 * 
 * \param _buffer   The output buffer to write to
 * \param offset    The starting offset to begin writing at
 * \param value     The value to write
 *
 * \return          The new offset position (input offset + amount written)
 */
sc_uint16_t sensocol_write_long_long(void *_buffer, sc_uint16_t offset,sc_int64_t value);

/**
 * \brief       Determine the encoded length of a 32bit signed integer when converted to a Varsc_uint16_t Zig-Zag
 * 
 * \param value     The value to determine the length of
 *
 * \return          The byte length when the value is converted to the Varsc_uint16_t Zig Zag int.
 */
sc_uint16_t sensocol_size_int32_encoded(sc_int32_t value);

/**
 * \brief           Writes a raw byte to a buffer.  Helper function
 * 
 * \param value     Raw byte to write
 * \param _buffer   Buffer to write to
 * \param offset    Offset to write the new byte at
 *
 * \return          The new offset position (input offset + amount written).  Practically offset++
 */
sc_uint16_t sensocol_write_raw_byte(sc_byte_t value, void *_buffer, sc_uint16_t offset);

/**
 * \brief           Writes a 32bit value in little endian format to a buffer
 * 
 * \param value     32bit value to write
 * \param _buffer   Buffer to write to
 * \param offset    Offset to write the new bytes at
 *
 * \return          The new offset position (input offset + amount written).  Practically offset+4
 */
sc_uint16_t sensocol_write_raw_little_endian32(sc_uint32_t value, void *_buffer, sc_uint16_t offset); 

/**
 * \brief           Writes a 64bit value in little endian format to a buffer
 * 
 * \param value     64bit value to write
 * \param _buffer   Buffer to write to
 * \param offset    Offset to write the new bytes at
 *
 * \return          The new offset position (input offset + amount written).  Practically offset+8
 */
sc_uint16_t sensocol_write_raw_little_endian64(sc_uint64_t value, void *_buffer, sc_uint16_t offset);

/**
 * \brief           Writes a 16bit value in little endian format to a buffer
 * 
 * \param value     16bit value to write
 * \param _buffer   Buffer to write to
 * \param offset    Offset to write the new bytes at
 *
 * \return          The new offset position (input offset + amount written).  Practically offset+2
 */
sc_uint16_t sensocol_write_raw_little_endian16(sc_uint16_t value, void *_buffer, sc_uint16_t offset);

/**
 * \brief           Writes an array of bytes to a buffer
 * 
 * \param _buffer   Buffer to write to
 * \param offset    Offset to write the new bytes at
 * \param bytes     Bytes to Write
 * \param length    Length of bytes to write
 * \return          The new offset position (input offset + amount written).  Practically offset+length
 */
sc_uint16_t sensocol_write_raw_bytes(void *_buffer, sc_uint16_t offset, sc_byte_t *bytes, sc_uint16_t length);
#endif


