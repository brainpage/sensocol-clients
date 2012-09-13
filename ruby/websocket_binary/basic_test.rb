###########################

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
#
## Sends the load averages via sensocol every 2 seconds.
# Uses Sensocol Websocket
#
##############################


$LOAD_PATH << File.dirname(__FILE__) + "/lib"

require "sensocol_web_socket"
if ARGV.size != 4
  #Example:  ruby basic_test.rb ws://127.0.0.1:7070/ test 2134507778 bd3bf04fe399fc9014fa0ddb03f2759b
  $stderr.puts("Usage: ruby samples/stdio_client.rb ws://HOST:PORT/ SENSOR_NAME USERNMAME PASSWORD")
  exit(1)
end

# Initiate client
@client = SensocolWebSocket.new(ARGV[0] + "websocket/#{ARGV[1]}", :username => ARGV[2], :password => ARGV[3])


# Cleaning exit when interrupt is sent
@continue_loop = true

trap("INT") { 
  puts "Exiting"
  @client.close
  @continue_loop = false
}

# Send load every 2 seconds
while @continue_loop do
  load_averages = `uptime`.split(" ")
  
  @client.timestamp = Time.now.utc.to_i
  @client.features = {"load1" => load_averages[-3].to_f, "load2"=>load_averages[-2].to_f, "load3" => load_averages[-3].to_f}
  @client.send
  puts "Sent Data"
  sleep(2)
end

