6Jack
=====

A framework for analyzing/testing/fuzzing network applications.

## DESCRIPTION

**6Jack** runs a command, intercepts calls to common network-related
functions and pass them through a filter as **MessagePack** serialized
objects.
  
         App                External process
         ---                ----------------         
          |                (started on-demand)
          V
    +----------+
    |   ....   |
    +----------+
          |
          V                    +-----+
    +----------+  pre-filter   |     |
    |  call()  | ------------> |  F  |
    +----------+               |  i  |
                               |  l  |
                               |  t  |
    +----------+  post-filter  |  e  |
    |   ....   | <-----------  |  r  |
    +----------+               |     |
          |                    +-----+
          V

**6Jack** is especially suitable for:

  * Writing tests for clients and servers.  
  Tests for networked applications are ofte limited to sending a bunch
  of requests and comparing the answers to the expected ones.
  However, in production, system calls can fail for a variety of reasons.
  Packets get lost or delayed. Weird queries are received because of bogus
  clients or attackers. Content can get altered by third parties.
  How do you test for that? For example, how often does your test suite
  actually include tests for cases like "quota exceeded" or "no space left
  on device"?
  6Jack makes it easy to simulate this kind of failure, without having
  to patch your existing software.
  
  * Debugging and reverse engineering protocols.
  **tcpdump** is a super powerful tool. However, it has been designed to
  log incoming and outgoing packets.
  **6Jack** can alter the data sent from and to system calls. It can
  help you understand what's going on over the wire by modifying stuff
  and observing the impact.
  
**6Jack** works at application level. It's a simple library that gets
preloaded before the actual application.

**Pre-filters** can inspect and alter the content prior to calling the
actual function. A **pre-filter** can also totally bypass the actual call,
in order to simulate a call without actually hitting the network.

**Post-filters** can inspect and alter the content after the actual call.
In particular, **post-filters** can change return codes and the `errno`
value in order to simulate failures.

## DIVERTED FUNCTIONS

  * bind(2)
  * close(2)
  * connect(2)
  * read(2)
  * recv(2)
  * recvfrom(2)
  * recvmsg(2)
  * send(2)
  * sendmsg(2)
  * sendto(2)
  * socket(2)
  * write(2)

## FILTERS

A filter is a standalone application that can be written in any
language currently supported by **MessagePack**: Ruby, C, C++, C#,
Scala, Lua, PHP, Python, Java, D, Go, Node.JS, Perl, Haskell, Erlang,
Javascript and OCaml.

A filter should read a stream of **MessagePack** objects from the
standard input (`stdin`) and for every object, should push a
serialized reply to the standard output (`stdout`).

Every diverted function sends exactly one PRE object and one POST
object, and synchronously waits for a reply to each object.

## ENVIRONMENT

When a `SIXJACK_BYPASS` environment variable is defined, calls are not
diverted to the filter any more.

An application is free to set and unset `SIXJACK_BYPASS`, in order to
explicitly disable **6Jack** in some sections.
