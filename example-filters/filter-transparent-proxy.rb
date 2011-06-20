#! /usr/bin/env ruby

# A sample filter that redirects every connection to port 80
# to 127.0.0.1 port 8080

require "msgpack"

pac = MessagePack::Unpacker.new
loop do
  begin
    data = STDIN.readpartial(65536)
  rescue EOFError
    break
  end
  pac.feed(data)
  pac.each do |obj|    
    if obj["function"] == "connect" && obj["remote_port"] == 80      
      obj["remote_port"] = 8080
      obj["remote_host"] = "127.0.0.1"
    else
      obj = { version: obj["version"] }
    end
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end
