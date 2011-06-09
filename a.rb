#! /usr/bin/env ruby

require "msgpack"
require "awesome_print"

pac = MessagePack::Unpacker.new
xxx = 0
loop do
  data = STDIN.readpartial(65536)
  warn "Ruby len=#{data.length}"
  pac.feed(data)
  obj = { version: 1, return_code: 45, yop: 45 }  
  pac.each do |obj|
#    warn obj.awesome_inspect
    obj["extra"] = 42 + xxx
    xxx += 1
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end

