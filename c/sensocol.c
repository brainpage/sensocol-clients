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
/ Sensocol:
/ Main functions for creating sensocol packets
*/




#include <stdlib.h> /* Included for generating rand number.  Should be made platform specific */
#include "sensocol.h"
#include "sensocol_encoding.h"
#include "aes.h"


void sensocol_packet_init(sensocol_packet_t *packet, sc_byte_t * sensor_id, sc_uint16_t sensor_id_length)
{
  packet->timestamp = 0;
  packet->feature_count = 0;
  packet->buffer_offset = 0;
  packet->sensor_id_length = sensor_id_length;
  sensocol_write_raw_bytes(packet->sensor_id, 0, sensor_id, sensor_id_length);
}

void sensocol_packet_add_feature(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, SENSOCOL_TYPE type)
{
  sc_uint16_t i;

  sensocol_feature_t *feature;
  // For convenience
  feature = &packet->features[packet->feature_count];

  //Set type
  feature->type = type;

  //Copy name and length
  sensocol_write_raw_bytes(feature->name, 0, name, name_length);
  feature->name_len = name_length;

  //Increase feature count
  packet->feature_count++;
} 


/* The following functions add various data types to the sensocol packet 
 * Notes:
 *   - Integers are all stored as variable zig-zag int.  Sensocol does not care about length.
 *   - Floating points numbers should be either by float (4 bytes) or double (8 bytes)
 *   - In C the "string" and "bytes" implementation are the same.  However, the server considers "string"
 *     to be a UTF-8 encoded string and "bytes" is simply an array of bytes.
 */


/*Add Long Feature*/
void sensocol_packet_add_long(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_int32_t value)
{
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_int);
  packet->buffer_offset = sensocol_write_long(packet->buffer, packet->buffer_offset, value);
}

/*Add 64bit Long Feature*/
void sensocol_packet_add_long_long(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_int64_t value)
{
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_int);
  packet->buffer_offset = sensocol_write_long_long(packet->buffer, packet->buffer_offset, value);
}

/*Add String Features */
void sensocol_packet_add_string(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_byte_t *value, sc_uint16_t length)
{
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_string);
  packet->buffer_offset = sensocol_write_bytes(packet->buffer, packet->buffer_offset, value, length);
}

/*Add Byte Array */
void sensocol_packet_add_bytes(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_byte_t *value, sc_uint16_t length)
{
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_bytes);
  packet->buffer_offset = sensocol_write_bytes(packet->buffer, packet->buffer_offset, value, length);
}

/* Add Float Feature */
void sensocol_packet_add_float(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_float_t value)
{
  union {
		sc_float_t f;
		sc_uint32_t i;
	} v;
  v.f = value;
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_float);
  packet->buffer_offset = sensocol_write_raw_little_endian32(v.i, packet->buffer, packet->buffer_offset);
}


/*Add Double Feature*/
void sensocol_packet_add_double(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_double_t value)
{
  union {
		sc_double_t f;
		sc_uint64_t i;
	} v;
  v.f = value;
  sensocol_packet_add_feature(packet, name_length, name, sensocol_type_double);
  packet->buffer_offset = sensocol_write_raw_little_endian64(v.i, packet->buffer, packet->buffer_offset);
}



sc_uint16_t sensocol_generate_post_packet(sensocol_packet_t *packet, sc_uint32_t access_key, sc_byte_t *access_token, sc_byte_t *schema_md5, sc_byte_t *buffer)
{
  sc_uint16_t offset = 0;
  sc_uint16_t size = 23; /* Header(1) + Length(2) + Access Key(4) + Access Token(16)  */

  /* Setup the size of the packet */
  size = size + sensocol_size_int32_encoded(packet->sensor_id_length) + packet->sensor_id_length + packet->buffer_offset;

  // Packet format: [Command:4bits][Encoding:4bits][Size:2bytes][UserKey:4bytes][SensorId:VarString][AccessToken:16bytes][SchemaMD5First4Bytes:4bytes][SensocolData]
  offset = sensocol_write_raw_byte(SENSOCOL_POST, buffer, offset);
  offset = sensocol_write_raw_little_endian16(size, buffer, offset);
  offset = sensocol_write_raw_little_endian32(access_key, buffer, offset);
  offset = sensocol_write_bytes(buffer, offset, (sc_byte_t *)packet->sensor_id, packet->sensor_id_length);
  offset = sensocol_write_raw_bytes(buffer, offset, access_token, 16);
  offset = sensocol_write_raw_bytes(buffer, offset, schema_md5, 4);
  offset = sensocol_write_raw_bytes(buffer, offset, (sc_byte_t *)packet->buffer, packet->buffer_offset);
  return offset;

}

sc_uint16_t sensocol_generate_aes128_post_packet(sensocol_packet_t *packet, sc_uint32_t access_key, sc_byte_t *access_token, sc_byte_t *schema_md5, sc_byte_t *buffer, sc_byte_t *aes_key)
{
  sc_uint16_t offset = 0;
  sc_uint16_t header = 11; /* HeaderByte + Size + Access Key + Nonce */
  sc_byte_t iv[16];
  sc_uint16_t encrypted_size;
  aes_context ctx;
  sc_uint32_t nonce;
  // Calculate size of the encrypted part of the packet
  encrypted_size = packet->buffer_offset + 16 + 4;

  // Make sure its 16byte aligned
  if(encrypted_size % 16 > 0)
    encrypted_size = encrypted_size + (16 - encrypted_size % 16);

  /* Setup the size of the packet */
  header = header + sensocol_size_int32_encoded(packet->sensor_id_length) + packet->sensor_id_length;


  offset = sensocol_write_raw_byte(SENSOCOL_POST_AES128, buffer, offset);
  offset = sensocol_write_raw_little_endian16(header + encrypted_size, buffer, offset);
  offset = sensocol_write_raw_little_endian32(access_key, buffer, offset);

  /* TODO: Replace this with a proper random number or time based nonce based on
   * your platform.  */
  nonce = rand();


  offset = sensocol_write_raw_little_endian32(nonce, buffer, offset);
  offset = sensocol_write_bytes(buffer, offset, (sc_byte_t *)packet->sensor_id, packet->sensor_id_length);
  offset = sensocol_write_raw_bytes(buffer, offset, access_token, 16);
  offset = sensocol_write_raw_bytes(buffer, offset, schema_md5, 4);
  offset = sensocol_write_raw_bytes(buffer, offset, (sc_byte_t *)packet->buffer, encrypted_size - 20);

  /* Setup the IV.  We use the access_key and nonce repeated as our 16 bit IV */
  sensocol_write_raw_bytes(iv, 0, &buffer[3], 8);
  sensocol_write_raw_bytes(iv, 8, &buffer[3], 8);

  /* Setup AES128 encryption */
  aes_setkey_enc(&ctx, aes_key, 128);

  
  /* We encrypt everything after the sensor_id. The sensor_id
   * is not encrypted because it may be tied to the key
   */
  
  // Find position to start encryption
  aes_crypt_cbc(&ctx, AES_ENCRYPT, encrypted_size, iv, &buffer[header], &buffer[header]);

  return offset;
 
}
