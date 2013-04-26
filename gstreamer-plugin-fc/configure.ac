AC_INIT([gstfcdec], [0.2])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([gstfcdec.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_PROG_CC
AC_LANG_C

AC_HEADER_STDC

AC_CHECK_HEADER(fc14audiodecoder.h,
    [],[AC_MSG_ERROR([fc14audiodecoder.h not found])])
AC_CHECK_LIB(fc14audiodecoder,fc14dec_new)
if ! test "$ac_cv_lib_fc14audiodecoder_fc14dec_new" = yes; then
    AC_MSG_ERROR([libfc14audiodecoder not found])
fi

dnl versions of gstreamer and plugins-base
GST_MAJORMINOR=0.10
GST_REQUIRED=0.10.0
GSTPB_REQUIRED=0.10.0

PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST=yes,HAVE_GST=no)

if test "x$GST_LIBS" != "x"; then
    CFLAGS="$CFLAGS $GST_CFLAGS"
    CXXFLAGS="$CXXFLAGS $GST_CFLAGS"
    LIBS="$LIBS $GST_LIBS"
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

AC_PATH_PROG(RM,rm,rm)

AM_DISABLE_STATIC
AM_PROG_LIBTOOL

dnl set the location where plugins should be installed
if test "x${prefix}" = "x$HOME"; then
  GST_PLUGIN_DIR="$HOME/.gstreamer-$GST_MAJORMINOR/plugins"
else
  GST_PLUGIN_DIR="\$(libdir)/gstreamer-$GST_MAJORMINOR"
fi
AC_SUBST(GST_PLUGIN_DIR)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

dnl make GST_MAJORMINOR available in Makefile.am
AC_SUBST(GST_MAJORMINOR)

AC_OUTPUT(Makefile)
