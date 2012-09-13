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
/ Sensocol Schema:
/ Functions for creating sensocol schemas
*/


#include "sensocol_schema.h"
sc_uint16_t sensocol_schema_from_packet(sensocol_packet_t *packet, sc_byte_t * schema_buffer)
{
  sc_uint16_t i;
  sc_uint16_t offset = 0;
  offset = sensocol_write_raw_bytes(schema_buffer, offset, "{\"headers\": {",13);
  if(packet->timestamp > 0)
    offset = sensocol_write_raw_bytes(schema_buffer, offset, "\"timestamp\":true", 16);

  offset = sensocol_write_raw_bytes(schema_buffer, offset, "}, \"features\": [", 16);

  for(i=0;i<packet->feature_count;i++){
    if(i > 0)
      offset = sensocol_write_raw_bytes(schema_buffer, offset, ", ", 2);

    offset = sensocol_write_raw_bytes(schema_buffer, offset, "{\"name\":\"", 9);
    offset = sensocol_write_raw_bytes(schema_buffer, offset, packet->features[i].name, packet->features[i].name_len);
    switch(packet->features[i].type){
      case sensocol_type_int:
        offset = sensocol_write_raw_bytes(schema_buffer, offset, "\", \"type\":\"int\"}", 16);

        break;
      case sensocol_type_string:
        offset = sensocol_write_raw_bytes(schema_buffer, offset, "\", \"type\":\"string\"}", 19);
        break;
       case sensocol_type_bytes:
        offset = sensocol_write_raw_bytes(schema_buffer, offset, "\", \"type\":\"bytes\"}", 18);
        break;
      case sensocol_type_float:
        offset = sensocol_write_raw_bytes(schema_buffer, offset, "\", \"type\":\"float\"}", 18);
        break;
      case sensocol_type_double:
        offset = sensocol_write_raw_bytes(schema_buffer, offset, "\", \"type\":\"double\"}", 19);
        break;


      
    }
  }
  offset = sensocol_write_raw_bytes(schema_buffer, offset, "]}", 2);
  return offset;

}

sc_uint16_t sensocol_schema_generate_response(sensocol_packet_t *packet, sc_byte_t *schema_buffer, sc_uint16_t schema_len, sc_byte_t *buffer)
{
  sc_uint16_t offset = 0;
  sc_uint16_t size = 3; /* Header(1) + Length(2) + Access Key(4) + Access Token(16)  */
  size += schema_len;
  /* Setup the size of the packet */

  offset = sensocol_write_raw_byte(SENSOCOL_POST_SCHEMA, buffer, offset);
  offset = sensocol_write_raw_little_endian16(size, buffer, offset);
  offset = sensocol_write_raw_bytes(buffer, offset, schema_buffer, schema_len);
  return offset;

}


