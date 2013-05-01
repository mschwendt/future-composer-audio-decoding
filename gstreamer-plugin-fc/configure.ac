AC_INIT([gstfcdec], [0.2])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([gstfcdec.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_PROG_CC
AC_LANG_C

AC_HEADER_STDC

AM_DISABLE_STATIC
AM_PROG_LIBTOOL

AC_PATH_PROG(RM,rm,rm)

PKG_PROG_PKG_CONFIG()

m4_ifdef([PKG_CHECK_VAR], [],
[AC_DEFUN([PKG_CHECK_VAR],
    [AC_REQUIRE([PKG_PROG_PKG_CONFIG])dnl
     AC_ARG_VAR([$1], [value of $3 for $2, overriding pkg-config])dnl
     _PKG_CONFIG([$1], [variable="][$3]["], [$2])
     AS_VAR_COPY([$1], [pkg_cv_][$1])
     AS_VAR_IF([$1], [""], [$5], [$4])dnl
    ])# PKG_CHECK_VAR
])

AC_CHECK_HEADER(fc14audiodecoder.h,
    [],[AC_MSG_ERROR([fc14audiodecoder.h not found])])
AC_CHECK_LIB(fc14audiodecoder,fc14dec_new)
if ! test "$ac_cv_lib_fc14audiodecoder_fc14dec_new" = yes; then
    AC_MSG_ERROR([libfc14audiodecoder not found])
fi

dnl === GStreamer 0.10.x ===

GST_MAJORMINOR=0.10
GST_REQUIRED=0.10.0

AC_MSG_CHECKING([GStreamer $GST_MAJORMINOR])
PKG_CHECK_EXISTS([gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED], AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]) )

PKG_CHECK_MODULES([GST],[gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED],[HAVE_GST=yes],[HAVE_GST=no])
if test "$HAVE_GST" = "yes"; then

    PKG_CHECK_VAR([GST_PLUGIN_DIR], [gstreamer-$GST_MAJORMINOR], [pluginsdir], [], [AC_MSG_ERROR([Cannot retrieve GStreamer pluginsdir pkgconfig variable])])
fi

dnl === GStreamer 1.0.x ===

GST1_MAJORMINOR=1.0
GST1_REQUIRED=1.0.0

AC_MSG_CHECKING([GStreamer $GST1_MAJORMINOR])
PKG_CHECK_EXISTS([gstreamer-$GST1_MAJORMINOR >= $GST1_REQUIRED], AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]) )

PKG_CHECK_MODULES([GST1],[gstreamer-$GST1_MAJORMINOR >= $GST1_REQUIRED],[HAVE_GST1=yes],[HAVE_GST1=no])
if test "$HAVE_GST1" = "yes"; then
    PKG_CHECK_VAR([GST1_PLUGIN_DIR], [gstreamer-$GST1_MAJORMINOR], [pluginsdir], [], [AC_MSG_ERROR([Cannot retrieve GStreamer pluginsdir pkgconfig variable])])

    PKG_CHECK_MODULES([GST1_AUDIO],[gstreamer-audio-$GST1_MAJORMINOR >= $GST1_REQUIRED],[],AC_MSG_ERROR([gstreamer-audio-$GST1_MAJORMINOR.pc not found!]))
fi

dnl ===

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

AM_CONDITIONAL([GST_COND], [test "$HAVE_GST" = yes])
AM_CONDITIONAL([GST1_COND], [test "$HAVE_GST1" = yes])
AC_CONFIG_FILES([gst1/Makefile])

AC_OUTPUT([Makefile])
