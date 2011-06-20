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

  * __Writing tests for clients and servers__
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
  
  * __Debugging and reverse engineering protocols__
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

## FILTERS

A filter is a standalone application that can be written in any
language currently supported by **MessagePack**: Ruby, C, C++, C#,
Scala, Lua, PHP, Python, Java, D, Go, Node.JS, Perl, Haskell, Erlang,
Javascript and OCaml.

A filter should read a stream of **MessagePack** objects from the
standard input (`stdin`) and for every object, should push a
serialized reply to the standard output (`stdout`).

Every diverted function sends exactly one **PRE** object and one
**POST** object, and synchronously waits for a reply to each object.

## COMMON PROPERTIES

Objects sent by all diverted functions include the following properties:

  * `version`:
  Always `1` for now.
  
  * `filter_type`:
  Either `PRE` or `POST`.
  
  * `pid`:
  The process ID.
  
  * `function`:
  The function name, for instance `recvfrom`.
  
  * `fd`:
  The file descriptor. The only pre-filter that doesn't include `fd`
  is the `socket()` pre-filter.

  * `return_value`:
  The return value of the system call.
  
  * `errno`:
  The value of `errno`, only meaningful in POST-filters.
  
  * `local_host`:
  The IP address of the local side of the socket. IPv6 is fully
  supported.
  
  * `local_port`:
  The port of the local side of the socket.

  * `remote_host`:
  The IP address of the remote side of the socket. IPv6 is fully
  supported.
  
  * `remote_port`:
  The port of the remote side of the socket.  

The reply from a filter should at least include:

  * `version`:
  Should be identical to the previous value of `version`: `1`.
  
  The reply from a filter can contain only this property. In this case,
  the filter will act as a pass-through: nothing will be altered, as if
  there was no filter at all.

Additional properties that can be added to replies for all functions
(everything is optional):

  * `return_value`:
  Change the return value. For example, even if a call to `socket()`
  returns a valid descriptor, a filter can override the return value and
  return -1 as if the call had failed.

  * `errno`:
  Override the value of `errno`. This way, you can test how your
  application recovers from different types of errors.
  
  * `force_close`:
  This value is a boolean. If TRUE, the descriptor is closed.
  
  * `bypass`:
  This value is a boolean. If TRUE, the actual system call is
  bypassed. It means that you can test how your application behaves
  without actually receiving or sending data from/to the network.  

## FUNCTION-SPECIFIC PROPERTIES

### `write()`

#### **PRE** filter

__In__:

  * `data`:
  The data to be written, as a **MessagePack** __raw__ object.

#### **POST** filter

__In__:

  * `data`:
  The data that has been written, as a **MessagePack** __raw__ object.

__Out__:

  * `data`:
  Overrides the data that was supposed to be written. The new data can
  be of any size, and can even be larger than the initial data.
  The return value will be automatically adjusted.
  
### `socket()`

#### **PRE** filter

__In__:

  * `domain`:
  One of `PF_LOCAL`, `PF_UNIX`, `PF_INET`, `PF_ROUTE`, `PF_KEY`, `PF_INET6`,
  `PF_SYSTEM`, `PF_NDRV`, `PF_NETLINK` and `PF_FILE`.

  * `type`:
  One of `SOCK_STREAM`, `SOCK_DGRAM`, `SOCK_RAW`, `SOCK_SEQPACKET` and
  `SOCK_RDM`.

  * `protocol`:
  One of `IPPROTO_IP`, `IPPROTO_ICMP`, `IPPROTO_IGMP`, `IPPROTO_IPV4`,
  `IPPROTO_TCP`, `IPPROTO_UDP`, `IPPROTO_IPV6`, `IPPROTO_ROUTING`,
  `IPPROTO_FRAGMENT`, `IPPROTO_GRE`, `IPPROTO_ESP`, `IPPROTO_AH`,
  `IPPROTO_ICMPV6`, `IPPROTO_NONE`, `IPPROTO_DSTOPTS`, `IPPROTO_IPCOMP`,
  `IPPROTO_PIM` and `IPPROTO_PGM`.

__Out__:

  * `domain`:
  Override the domain.

  * `type`:
  Override the type.

  * `protocol`:
  Override the type.

#### **POST** filter

  **POST** filters get the same data as **PRE** filters.  

## ENVIRONMENT

When a `SIXJACK_BYPASS` environment variable is defined, calls are not
diverted to the filter any more.

An application is free to set and unset `SIXJACK_BYPASS`, in order to
explicitly disable **6Jack** in some sections.

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
