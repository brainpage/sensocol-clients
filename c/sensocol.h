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

#ifndef _SENSOCOL_H
#define _SENSOCOL_H

#include "sc_platform.h"
/////////////////////////////
// NOTE: As this library is primarily designed to be used
// in low-power embedded systems, we have opted to NOT
// use dynamic memory allocation as this various widely
// by embeeded systems.  The below settings
// determine buffer sizes of pre-allocated arrays
////////////////////////////////

// TODO: Adjust these values to fit your actual use case
#define MAX_FEATURES 10
#define MAX_BUFFER_LENGTH 1024
#define MAX_FEATURE_NAME_LENGTH 25
#define MAX_SENSOR_ID_LENGTH 25

/////////////////////
// Sensocol Server Command Bytes
// These are the first byte of a packet.
// The first four bits are the command. 
// The second four bits specify the encryption
////////////////////
#define SENSOCOL_POST 0x10
#define SENSOCOL_POST_AES128 0x11
#define SENSOCOL_RESPONSE_OK 0x20
#define SENSOCOL_RESPONSE_AUTH_ERROR 0x80
#define SENSOCOL_REQUEST_SCHEMA 0x30
#define SENSOCOL_POST_SCHEMA 0x40
#define SENSOCOL_POST_SCHEMA_WITH_AES 0x41
#define SENSOCOL_RESPONSE_SCHEMA_ERROR 0x90
#define SENSOCOL_RESPONSE_PARSE_ERROR 0xA0

// The different data types sensocol supports
typedef enum SENSOCOL_TYPE{
  sensocol_type_int = 0,
  sensocol_type_float,
  sensocol_type_double,
  sensocol_type_string,
  sensocol_type_bytes
} SENSOCOL_TYPE;


// Each feature of this sensor reading
typedef struct sensocol_feature {
  SENSOCOL_TYPE type;
  sc_byte_t name[25];
  sc_uint16_t name_len;
} sensocol_feature_t;

// Overall packet info
typedef struct sensocol_packet {
  sc_uint32_t timestamp;
  sc_uint16_t sensor_id_length;
  sc_byte_t * sensor_id[MAX_SENSOR_ID_LENGTH];
  sc_uint16_t feature_count;
  sensocol_feature_t features[MAX_FEATURES];
  sc_uint16_t buffer_offset;
  sc_byte_t * buffer[MAX_BUFFER_LENGTH];

} sensocol_packet_t;

/**
 * \brief                   Initiate sensocol packet
 * \param packet            Allocated packet structure
 * \param sensor_id         The ID of this sensor
 * \param sensor_id_length  The length of the sensor_id
 *
 */
void sensocol_packet_init(sensocol_packet_t *packet, sc_byte_t * sensor_id, sc_uint16_t sensor_id_length);

/**
 * \brief                   Add a long valued feature to this sensor packet
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The actual long value to add
 */
void sensocol_packet_add_long(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_int32_t value);

/**
 * \brief                   Add a string valued feature to this sensor packet. For data storage
 *                          and processing purposes this is considered to be UTF-8 formated text.
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The array of bytes that make up the string
 */
void sensocol_packet_add_string(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_byte_t *value, sc_uint16_t length);

/**
 * \brief                   Adds a feature composed of an array of bytes to this sensor packet.
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The array of bytes
 */
void sensocol_packet_add_bytes(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_byte_t *value, sc_uint16_t length);

/**
 *
 * \brief                   Add a long long (64bit signed integer) valued feature to this sensor packet
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The actual long long value to add
 */
void sensocol_packet_add_long_long(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_int64_t value);

/**
 * \brief                   Add a float valued feature to this sensor packet
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The actual float value to add
 */
void sensocol_packet_add_float(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_float_t value);

/**
 * \brief                   Add a double valued feature to this sensor packet
 * \param packet            The initialized packet
 * \param name_length       The length of the name field
 * \param name              The name of the feature
 * \param value             The actual double value to add
 */
void sensocol_packet_add_double(sensocol_packet_t *packet, sc_uint16_t name_length, sc_byte_t *name, sc_double_t value);

/**
 * \brief                   Generates the actual packet to send across the TCP socket
 * \param packet            The initialized packet with features already added
 * \param access_key        The access key to the server
 * \param access_token      The access token to the server
 * \param md5               The MD5 of the Schema
 * \param buffer            The byte buffer to store the final packet in.
 *
 * \return                  The size of the generated packet
 */
sc_uint16_t sensocol_generate_post_packet(sensocol_packet_t *packet, sc_uint32_t access_key, sc_byte_t *access_token, sc_byte_t *md5, sc_byte_t *buffer);

/**
 * \brief                   Generates the actual packet to send across the TCP socket
 * \param packet            The initialized packet with features already added
 * \param access_key        The access key to the server
 * \param access_token      The access token to the server
 * \param md5               The MD5 of the Schema
 * \param buffer            The byte buffer to store the final packet in.
 *
 * \return                  The size of the generated packet
 */
sc_uint16_t sensocol_generate_aes128_post_packet(sensocol_packet_t *packet, sc_uint32_t access_key, sc_byte_t *access_token, sc_byte_t *schema_md5, sc_byte_t *buffer, sc_byte_t *aes_key);
#endif


