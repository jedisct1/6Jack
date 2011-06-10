#ifndef __COMMON_H__
#define __COMMON_H__ 1

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <dlfcn.h>
#include <spawn.h>

#define VERSION_MAJOR 1

#ifndef __GNUC__
# ifdef __attribute__
#  undef __attribute__
# endif
# define __attribute__(a)
#endif

#ifndef environ
# ifdef __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
# else
extern char **environ;
# endif
#endif

#if defined(__APPLE__) && !defined(__clang__)
# define USE_INTERPOSERS 1
#endif

#ifdef USE_INTERPOSERS
# define INTERPOSE(F) sixjack_interposed_ ## F
#else
# define INTERPOSE(F) F
#endif

#endif
