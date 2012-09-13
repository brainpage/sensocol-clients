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
# SensocolWebSocket:
# Handles talking to the server via the websocket binary codec
# Primary function is to handle schema requests and provider wrapper over websocket
##############################


require "web_socket"
require "sensocol_codec"
require "digest/md5"
class SensocolWebSocket < SensocolCodec
  def initialize(url,  options = {})
    super()
    @ws = WebSocket.new(url, options)
    @schema_list = {}
    @mutex = Mutex.new
    setup_listen
  end
  
  # Listen for responses from server in another thread.
  def setup_listen
    
    Thread.new() do
      while data = @ws.receive()
        request = JSON.parse(data)
        if request["action"] == "schema_request"
          if schema = @schema_list[request["schema_hash"]]
            response = {"action" => "schema_response", "schema_json" => schema, "schema_hash" => request["schema_hash"]}
            @ws.send(response.to_json)
          end 
        end 
        printf("Received: %p\n", data)
      end
      exit()
    end
  end

  # Send packet to server
  def send 

    # Remember - we just take the first 4 MD5 hash bytes for the schema.
    @mutex.synchronize do
      @schema_list[Digest::MD5.hexdigest(self.schema)[0..7]] = self.raw_schema
    end
    @ws.send_binary(self.packet)
    @features = {}
    @timestamp = nil
  end 
  
  # Close socket
  def close
    @ws.close
  end 
end
