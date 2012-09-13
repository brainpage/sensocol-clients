###########################
#
# Copyright 2012 Brainpage Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
#   limitations under the License.
# 
# SensocolCodec:
# Encodes data and creates schema for sensocol binary protocol
##############################


require 'rubygems'
require 'json'
require 'digest/md5'
class SensocolCodec
    attr_reader :timestamp, :data, :feature_schema
    def initialize()
        @feature_schema = []
        @data = ""
        @timestamp = nil
    end 

    def add(name, val)
        type = case  
                when val.is_a?(Float)
                    @data << create_double(val)
                    "double"
                when val.is_a?(Fixnum) || val.is_a?(Bignum)
                    @data << create_long(val)
                    "int"
                when val.is_a?(String)
                    @data << create_string(val)
                    "string"
                else
                    @data << create_bytes(val)
                    "bytes"
                end 
        @feature_schema << {"name" => name, "type" => type}
        return self 
    end 

    def create_double(val)
        [val].pack('E')
    end

   def create_long(n)
       output = ""
        foo = n
        n = (n << 1) ^ (n >> 63)
        while (n & ~0x7F) != 0
          output << (((n & 0x7f) | 0x80).chr)
          n >>= 7
        end
        output << n.chr
      end
    
    def create_bytes(datum)
        output = create_long(datum.size)
        output << datum
        output 
      end
   
    def create_string(datum)
        str = datum.force_encoding("BINARY")
        output = create_long(str.size)
        output << str
        output
    end

    def features=(features)
        raise "Invalid Feature.  Must be Hash" unless features.is_a?(Hash)
        @data = ""
        @feature_schema = []
        features.each{|k,v| add(k, v)}
    end 

    def timestamp=(time)
        @timestamp = time
    end 

    def schema
      raw_schema.to_json
    end

    def raw_schema
      headers = {}
      headers["timestamp"] = true  unless @timestamp.nil?
      {"features" => @feature_schema, "headers" => headers} 
    end 

    def data_with_headers
      response = ""
      response << create_long(@timestamp) unless @timestamp.nil?
      response << @data
      return response
    end

    def packet
      schema_hash = Digest::MD5.digest(self.schema)[0..3]
      return schema_hash + data_with_headers
    end
end
