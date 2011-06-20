#! /usr/bin/env ruby

# A sample filter that shuts down any connection with ECONNRESET
# if a string is found.

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
    obj =
      if obj["filter_type"] == "POST" &&
         ["recv", "read"].include?(obj["function"]) &&
         obj["remote_port"] == 80 && obj["data"].include?("skyrock")
        { version: obj["version"], force_close: true, return_value: -1,
          errno: Errno::ECONNRESET::Errno }
      else
        { version: obj["version"] }
      end
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end
