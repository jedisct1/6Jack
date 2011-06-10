#! /usr/bin/env ruby

require "msgpack"
require "awesome_print"

pac = MessagePack::Unpacker.new
loop do
  begin
    data = STDIN.readpartial(65536)
  rescue EOFError
    break
  end
  pac.feed(data)
  pac.each do |obj|
    warn obj.awesome_inspect
    STDOUT.write obj.to_msgpack
    STDOUT.flush
  end
end

