
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
        id-name.c

lib6jack_la_LDFLAGS = -version-info 0:1

bin_PROGRAMS = \
        6jack-test

6jack_test_SOURCES = \
        6jack-test.c

6jack_test_LDADD = \
        lib6jack.la

EXTRA_DIST = \
        a.rb
