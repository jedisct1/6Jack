AC_PREREQ(2.61)
AC_INIT(6jack, 0.1, bugs at pureftpd dot org)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/6jack.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.9 dist-bzip2])
AM_MAINTAINER_MODE

AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

# Checks for programs.
AC_PROG_CC_C99
AC_USE_SYSTEM_EXTENSIONS
CPPFLAGS="$CPPFLAGS -D_XPG4_2=1"
AC_PROG_LIBTOOL

if test -d /usr/local/include; then
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
fi

if test -d /usr/local/lib; then
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
fi

# Checks for libraries.
AC_CHECK_LIB([msgpack], [msgpack_pack_raw_body])

# Checks for header files.
AC_SYS_LARGEFILE
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdint.h stdbool.h])
AC_CHECK_HEADERS([sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_RESTRICT
AC_C_FLEXIBLE_ARRAY_MEMBER
AC_C_VARARRAYS
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_INTPTR_T
AC_TYPE_UINTPTR_T
AC_TYPE_UINT32_T
AC_TYPE_SIGNAL
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([atexit socket])

# Switches
AC_HEADER_ASSERT

AC_ARG_WITH(debug,
[AS_HELP_STRING(--with-debug,For maintainers only - please do not use)],
[ if test "x$withval" = "xyes" ; then
    CFLAGS="$CFLAGS -DDEBUG=1 -g -Wall -W -Wcast-align -Wbad-function-cast -Wstrict-prototypes -Wwrite-strings -Wreturn-type "
  fi ])

AC_SUBST([MAINT])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
