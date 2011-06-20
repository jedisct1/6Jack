6jack(8) -- A framework for analyzing/testing/fuzzing network applications
==========================================================================

## SYNOPSIS

`6jack` <filter> <command> [<args>]

## DESCRIPTION

**6jack** runs a command, intercepts calls to common network-related
functions and pass them through a filter as **MessagePack** serialized
objects.

**6jack** works at application level. It's a simple library that gets
preloaded before the actual application.

**Pre-filters** can inspect and alter the content prior to calling the
actual function. A **pre-filter** can also totally bypass the actual call,
in order to simulate a call without actually hitting the network.

**Post-filters** can inspect and alter the content after the actual call.
In particular, **post-filters** can change return codes and the `errno`
value in order to simulate failures.

## OPTIONS

<command> is the name of the command to run.

<filter> is the absolute path to an application that receives a stream of
**MessagePack** serialized objects on `stdin` and for every object, should
push a serialized reply to `stdout`.

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

## ENVIRONMENT

When a `SIXJACK_BYPASS` environment variable is defined, calls are not
diverted to the filter any more.

An application is free to set and unset `SIXJACK_BYPASS`, in order to
explicitly disable **6jack** in some sections.

## RETURN VALUES

**6jack** returns the exit value of <command> after completion.

## SECURITY CONSIDERATIONS

**6jack** has been designed as a tool for testing applications.

It is not suitable for running in a production environment or with
untrusted data / filters.

## WWW

**6jack** is hosted on Github: `https://github.com/jedisct1/6jack`

Releases can be downloaded from: http://download.pureftpd.org/6jack/

## SEE ALSO

**MessagePack** home page: `http://msgpack.org/`

