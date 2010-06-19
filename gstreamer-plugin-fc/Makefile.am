AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

dist-hook:
		@RM@ -f $(distdir)/config.h

EXTRA_DIST = m4 config.h.in

plugindir = @GST_PLUGIN_DIR@

plugin_LTLIBRARIES = libgstfcdec.la

libgstfcdec_la_LDFLAGS = @GST_PLUGIN_LDFLAGS@

libgstfcdec_la_SOURCES = \
gstfcdec.c \
gstfcdec.h
