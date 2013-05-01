AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

dist-hook:
		@RM@ -f $(distdir)/config.h

EXTRA_DIST = m4 config.h.in

if GST1_COND
    GST1_MAYBE = gst1
endif
SUBDIRS = $(GST1_MAYBE)

plugindir = @GST_PLUGIN_DIR@

plugin_LTLIBRARIES = 
if GST_COND
plugin_LTLIBRARIES += libgstfcdec.la
endif

libgstfcdec_la_LDFLAGS = @GST_PLUGIN_LDFLAGS@

libgstfcdec_la_SOURCES = \
gstfcdec.c \
gstfcdec.h
