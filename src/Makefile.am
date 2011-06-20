
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
	hook-recv.c \
	hook-recv.h \
	hook-recvfrom.c \
	hook-recvfrom.h \
	hook-recvmsg.c \
	hook-recvmsg.h \
	hook-send.c \
	hook-send.h \
	hook-sendto.c \
	hook-sendto.h \
	hook-socket.c \
	hook-socket.h \
	hook-write.c \
	hook-write.h \
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
lautoma	utils.h

lib6jack_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(SIXJACK_LIBRARY_VERSION)

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

