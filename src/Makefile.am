
lib_LTLIBRARIES = \
        lib6jack.la
        
lib6jack_la_SOURCES = \
        common.h \
        6jack.c \
        utils.h \
        utils.c \
        msgpack-extensions.h \
        msgpack-extensions.c \
        id-name.h \
        id-name.c \
        filter.h \
        filter.c \
        hook-socket.h \
        hook-socket.c

lib6jack_la_LDFLAGS = \
        $(AM_LDFLAGS) \
        -version-info $(SIXJACK_LIBRARY_VERSION)

bin_PROGRAMS = \
        6jack-test

6jack_test_SOURCES = \
        6jack-test.c

6jack_test_LDADD = \
        $(AM_LDDD) \
        lib6jack.la
        
6jack_test_DEPENDENCIES = \
        lib6jack.la

EXTRA_DIST = \
        a.rb
