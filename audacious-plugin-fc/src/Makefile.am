EXTRA_DIST = config.h.in

pkglibdir = $(AUDACIOUS_IPLUGIN_DIR)

pkglib_LTLIBRARIES = fcdecoder.la

fcdecoder_la_LDFLAGS = -module -avoid-version
fcdecoder_la_CFLAGS = $(AUDACIOUS_CFLAGS)
fcdecoder_la_LIBADD = $(AUDACIOUS_LIBS)

fcdecoder_la_SOURCES = \
audfc.cpp \
audfc.h \
configure.cpp \
configure.h
