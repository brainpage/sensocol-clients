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

We can functionally consider sensocol in three aspects: encoding, schema and transport.

Encoding
--------

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


Schema
=====

The schema is a JSON document.  Here is an example:
{"headers" => {"timestamp":true}, "features" => [{"name":"feature1", "type":"int"}, {"name":"feature2", "type":"float"}]}

There are two parts:

"headers"
Indicates header data in the packet.  Currently only "timestamp" is supported. 
If "timestamp" is set to true then the data includes a timestamp.  If not, the server will assign a timestamp to the 
data based on the time it arrives.

"features"
An array of features in the data.  Each feature item includes "name" and "type".

"name"
The name of the feature.  Should be composed of characters (A-Za-z), numbers (0-9) and underscores (_).  All characters
are technically support, but not recommended.

"type"
The type of the data.  See Encoding section above for types.


Transport
========

There are two transport methods:

*Binary Request* - The data is posted via a TCP connection that is shutdown after the transaction is completed.  Should be used with low-data volumes and situations where it is expensive to keep the TCP connection alive.
*WebSocket* - A persistant TCP connection is established with the server.  Should be used when there is a high volume of data and keeping the connection alive has no cost.



At the top levels it has two sectio


Public Client Libraries for Sensocol