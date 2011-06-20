#! /usr/bin/env ruby

# A sample filter that changes HTTP replies to uppercase.

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
    if obj["filter_type"] == "POST" &&
      ["recv", "read"].include?(obj["function"]) && obj["remote_port"] == 80
      obj["data"].upcase!
    else
      obj = { version: obj["version"] }
    end
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end
