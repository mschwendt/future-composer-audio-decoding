EXTRA_DIST = Config.h.in FC.README

pkglibdir = `${XMMSCFG} --plugin-dir`/Input

pkglib_LTLIBRARIES = libfc.la

libfc_la_LDFLAGS = -module -avoid-version

XMMS_CFLAGS = `${XMMSCFG} --cflags`
# XMMS_LIBS = `${XMMSCFG} --libs`
INCLUDES = $(XMMS_CFLAGS)
LIBS = @LIBS@ @GTK_LIBS@
# LIBS = @LIBS@ @GTK_LIBS@ $(XMMS_LIBS)
GTK_CFLAGS = @GTK_CFLAGS@
AM_CFLAGS = @CFLAGS@ $(GTK_CFLAGS)
AM_CXXFLAGS = @CXXFLAGS@ $(GTK_CFLAGS)

libfc_la_SOURCES = \
about.c \
about.h \
configure.c \
configure.h \
FC.cpp \
FC_Defs.h \
FC.h \
LamePaula.cpp \
LamePaula.h \
Main.cpp \
MyEndian.h \
MyTypes.h \
SmartPtr.h
