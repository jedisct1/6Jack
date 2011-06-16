
lib_LTLIBRARIES = \
	lib6jack.la

lib6jack_la_SOURCES = \
	app.c \
	common.h \
	filter.c \
	filter.h \
	hook-bind.c \
	hook-bind.h \
	hook-close.c \
	hook-close.h \
	hook-connect.c \
	hook-connect.h \
	hook-read.c \
	hook-read.h \
	hook-socket.c \
	hook-socket.h \
	hooks.c \
	hooks.h \
	id-name.c \
	id-name.h \
	id-name_p.h \
	interpose.c \
	log.c \
	log.h \
	msgpack-extensions.c \
	msgpack-extensions.h \
	utils.c \
	utils.h

lib6jack_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(SIXJACK_LIBRARY_VERSION)

bin_PROGRAMS = \
	6jack-test

6jack_test_SOURCES = \
	6jack-test.c

6jack_test_LDADD = \
	$(AM_LDADD) \
	lib6jack.la

6jack_test_DEPENDENCIES = \
	lib6jack.la

bin_SCRIPTS = \
	6jack

6jack: 6jack.in Makefile
	$(do_subst) < $(srcdir)/6jack.in > 6jack
	
do_subst = \
	sed -e 's,[@]DESTDIR[@],$(DISTDIR),g' \
		-e 's,[@]datadir[@],$(datadir),g' \
		-e 's,[@]libdir[@],$(libdir),g' \
		-e 's,[@]lib_LTLIBRARIES[@],$(lib_LTLIBRARIES),g' \
		-e 's,[@]PACKAGE[@],$(PACKAGE),g' \
		-e 's,[@]VERSION[@],$(VERSION),g'

CLEANFILES = $(bin_SCRIPTS)

EXTRA_DIST = \
	example-filter.rb \
	6jack.in

