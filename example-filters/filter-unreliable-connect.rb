#! /usr/bin/env ruby

# Make connect() randomly fail (50%) with "No route to host"

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
         obj["function"] == "connect" && rand < 0.5
        { version: obj["version"], return_value: -1, errno: 65 } # EHOSTUNREACH
      else
        { version: obj["version"] }
      end
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end
