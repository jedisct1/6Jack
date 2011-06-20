#! /usr/bin/env ruby

# A trivial 6jack filter that receives messages and echoes them.
# All messages are printed to stderr by the way.

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
