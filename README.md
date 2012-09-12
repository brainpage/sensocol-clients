Sensocol Binary Protocol
================

Sensocol is a protocol and data serialization method designed for transmitting time-stamped data across the internet. 

#### Design Goals

* Schema-on-write (aka dynamic schema).  The client and server need not have a pre-existing understanding of the schema. This allows client devices to flexibly upgrade and change their schema depending upon their situation.
* Efficient.  Data is stored in the most binary efficient method realistically possible.
* Simple.  Where-ever possible we have choosen simplicity over complexity within the above two design constraints.  The protocol runs fine in embedded systems and has low over-head on servers.

#### Influences
  Sensocol is heavily influenced by Avro (and thrift/protocol buffers).  However we have simplified the transport machanism and encoding options and
  added encryption support.
  
#### Big Picture Design
Sensocol has to components: the schema and the encoded data.  

The schema is a JSON document that describes the encoded data. At the head of the encoded data is a hash of the schema.
If the server already knows the schema, it can decode the data.  If the server does not know this schema, it will request
the JSON document from the client for the schema.  

We can functionally consider sensocol in three aspects: schema, data encoding and transport.

Schema
=====

The schema is a JSON document.  Here is an example:

    {"headers" => {"timestamp":true}, "features" => [{"name":"feature1", "type":"int"}, {"name":"feature2", "type":"float"}]}

There are two parts:

### headers
Indicates header data in the packet.  Currently only "timestamp" is supported. 
If "timestamp" is set to true then the data includes a timestamp.  If not, the server will assign a timestamp to the 
data based on the time it arrives.

### features
An array of features in the data.  Each feature item includes "name" and "type".

##### name
The name of the feature.  Should be composed of characters (A-Za-z), numbers (0-9) and underscores (_).  All characters
are technically support, but not recommended.

##### type
The type of the data.  See Encoding section above for types.


Encoding
=======

Sensocol currently supports five data types.  They are: Integer, Float, Double, UTF-8 String, Byte Array.

Everything is encoded little-endian.

#### Integer (in schema: "int"):
Integers are encoded using a VarInt Zig-Zag encoding.  VarInt uses 7bits to encoded the integer and 1bit to tell whether the encoding continues in the next byte. 
Zig-Zag encoding turns a signed number into an unsigned number.  To understand how this works in detail, please see the description
at https://developers.google.com/protocol-buffers/docs/encoding or check out the source code.

#### Float (in schema: "float"):
Floats are a 32bit floating point number.  It is encoded using the IEEE float standard 32bits.

#### Double (in schema: "double"):
Doubles are a 64bit float point number.  It is encoded using the IEEE double standard using 64bits.

#### UTF-8 String (in schema: "string"):
A UTF-8 string is encoded in two parts.  The first is an integer (using the integer encoding above) of the length of the string.
THe second is simply the bytes composing the UTF-8 string.

#### Byte (in schema: "bytes"):
Encoded exactly the same as a string.  The difference is to allow server side processing to understand what the data is.

### Encoding process
Data is simply encoded using the above methods in the order that the features appears in the schema.  


Transport
========

There are two transport methods:

* **Binary Request** - The data is posted via a TCP connection that is shutdown after the transaction is completed.  Should be used with low-data volumes and situations where it is expensive to keep the TCP connection alive.
* **WebSocket** - A persistant TCP connection is established with the server.  Should be used when there is a high volume of data and keeping the connection alive has no cost.

Binary Request
--------------
(Example C Code Provided).

This is a simple request/response transport like HTTP, but using far less bandwidth.

### Dialogue
1. The client establishes a TCP connection to the server. 
2. Client posts sensor data to server.
3a. If the server has already seen data with this schema, the server responds with success and closes the TCP connction.
3b. If the server has NOT seen this schema before, the server requests the schema.
(4). The client sends the schema.
(5). The server responds with success and closes the TCP connection.

### Post data to the server (unencrypted)

The unencrypted format for a packet is:
    
    [Command:4bits][Encoding:4bits][Size:2bytes][AccessKey:4bytes][SensorId:String][AccessToken:16bytes][SchemaMD5First4Bytes:4bytes][[UserToken:16bytes][SensocolData]

* *Command:* For post, should be 0x1
* *Encoding:* For default encodoing should be 0x0
* *Size:* The size of the entire packet (not just the following bytes).
* *AccessKey:* The key (an integer) that is used to authenticate the device to the server.
* *SensorId:* The sensor id, stored as a sensocol string.  sensor id can be anything.
* *AccessToken:* The 16 byte access token used to authenticate the user to the server.
* *SchemaMD5First4Bytes:* The first 4 bytes of the md5 of the schema JSON.
* *Data:* The data encoded using sensocol encoding (above)

### Post data to the server (encrypted)

The encrypted format for a packet is:

    [Command:4bits][Encoding:4bits][Size:2bytes][AccessKey:4bytes][Nonce:4bytes][[SensorId:String][Encrypted Data]

The format for the encrypted data portion is

    [AccessToken:16bytes][MD5:4bytes][Data]

Differences from unencrypted sending:

* *Encoding:* For AES128 CBC, the value should be 0x1
* *Nonce:* A random number or the timestamp of when the packet is created.  Should be unique between packets.

AES128 CBC uses a 128bit Initialition Vector (IV).   The IV should be the 3rd - 11th bytes of the packet, repeated twice.  
i.e.:
 
    [AccessKey:4bytes][Nonce:4bytes][AccessKey:4bytes][Nonce:4bytes]

### Post data response.

The server will reply with a one byte response.  Possible response values are:

* OK: 0x20 (Close connection.  Everything is ok)
* Request Schema: 0x30 (The server requests that the client sends the schema)
* Auth Error: 0x80 (Invalid AccessKey and/or Token)
* Schema Error: 0x90 (Problem with the schema)
* Data Error: 0xA0 (Problem with the encoding of the actual data).

### Posting the schema.

If the server responds to a post request with 0x30, the client must send the schema to the server.

The schema response packet is simple:

    [Command:4bits][Encoding:4bits][Size:2bytes][Schema]
    
The command must be 0x4 and encoding is 0x0.  The schema must be a UTF-8 encoded JSON schema has described above.

The server will either using the response codes above.



