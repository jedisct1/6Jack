AC_PREREQ(2.61)
AC_INIT(6jack, 0.3, bugs at pureftpd dot org)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/app.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.9 dist-bzip2])
AM_MAINTAINER_MODE

AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

SIXJACK_LIBRARY_VERSION=0:0:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                |        |        or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed
AC_SUBST(SIXJACK_LIBRARY_VERSION)

# Checks for programs.
AC_PROG_CC_C99
AC_USE_SYSTEM_EXTENSIONS
CPPFLAGS="$CPPFLAGS -D_XPG4_2=1 -D_GNU_SOURCE=1"
AC_PROG_LIBTOOL
AC_PATH_PROG([RONN], [ronn])

CFLAGS="$CFLAGS -fvisibility=hidden"

if test -d /usr/local/include; then
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
fi

if test -d /usr/local/lib; then
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
fi

# Checks for libraries.
AC_CHECK_LIB([msgpack], [msgpack_zone_free])
AC_CHECK_LIB([dl], [dlsym])

# Checks for header files.
AC_SYS_LARGEFILE
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdint.h stdbool.h limits.h])
AC_CHECK_HEADERS([netinet/in.h sys/socket.h netdb.h])
AC_CHECK_HEADERS([syslog.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_RESTRICT
AC_C_FLEXIBLE_ARRAY_MEMBER
AC_C_VARARRAYS
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T
AC_TYPE_UINT32_T
AC_TYPE_SIGNAL
AC_CHECK_TYPES([ptrdiff_t])

AC_MSG_CHECKING([whether sin_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
]], [[
do {
 struct sockaddr_in a;
 (void) a.sin_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SIN_LEN,,[define if you have sin_len])
],[
  AC_MSG_RESULT(no)
])  

AC_MSG_CHECKING([whether __ss_family is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
]], [[
do {
 struct sockaddr_storage a;
 (void) a.__ss_family;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_FAMILY,,[define if you have __ss_family])
],[
  AC_MSG_RESULT(no)
])  

AC_MSG_CHECKING([whether ss_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
]], [[
do {
 struct sockaddr_storage a;
 (void) a.ss_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_SS_LEN,,[define if you have ss_len])
],[
  AC_MSG_RESULT(no)
])  

AC_MSG_CHECKING([whether __ss_len is defined])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
]], [[
do {
 struct sockaddr_storage a;
 (void) a.__ss_len;
} while(0)
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE___SS_LEN,,[define if you have __ss_len])
],[
  AC_MSG_RESULT(no)
])

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit socket strerror])

# Switches
AC_HEADER_ASSERT

AC_ARG_ENABLE(debug,
[AS_HELP_STRING(--enable-debug,For maintainers only - please do not use)],
[ if test "x$enableval" = "xyes" ; then
    CFLAGS="$CFLAGS -DDEBUG=1 -g -Werror -Wall -W -Wcast-align -Wbad-function-cast -Wstrict-prototypes -Wwrite-strings -Wreturn-type "
  fi ])

AC_SUBST([MAINT])

AC_CONFIG_FILES([Makefile
                 example-filters/Makefile
                 src/Makefile
                 man/Makefile])
AC_OUTPUT
