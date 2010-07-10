EXTRA_DIST = config.h.in

pkglibdir = `${XMMSCFG} --plugin-dir`/Input

pkglib_LTLIBRARIES = fcdecoder.la

fcdecoder_la_LDFLAGS = -module -avoid-version

XMMS_CFLAGS = `${XMMSCFG} --cflags`
# XMMS_LIBS = `${XMMSCFG} --libs`
INCLUDES = $(XMMS_CFLAGS)
LIBS = @LIBS@ @GTK_LIBS@
# LIBS = @LIBS@ @GTK_LIBS@ $(XMMS_LIBS)
GTK_CFLAGS = @GTK_CFLAGS@
AM_CFLAGS = @CFLAGS@ $(GTK_CFLAGS)
AM_CXXFLAGS = @CXXFLAGS@ $(GTK_CFLAGS)

fcdecoder_la_SOURCES = \
about.c \
about.h \
configure.c \
configure.h \
main.c
