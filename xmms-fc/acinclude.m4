dnl -------------------------------------------------------------------------
dnl Try to find a file (or one of more files in a list of dirs).
dnl -------------------------------------------------------------------------

AC_DEFUN(FC_FIND_FILE,
[
    $3=""
    for i in $2;
    do
        for j in $1;
        do
	        if test -r "$i/$j"; then
		        $3=$i
                break 2
            fi
        done
    done
])

dnl -------------------------------------------------------------------------
dnl Check whether compiler has a working ``bool'' type.
dnl -------------------------------------------------------------------------

AC_DEFUN(FC_CHECK_BOOL,
[
    AC_MSG_CHECKING([for bool])
    AC_CACHE_VAL(fc_cv_have_bool,
    [
        AC_TRY_COMPILE(
            [],
            [bool aBool = true;],
            [fc_cv_have_bool=yes],
            [fc_cv_have_bool=no]
        )
    ])
    AC_MSG_RESULT($fc_cv_have_bool)
    if test "$fc_cv_have_bool" = yes; then
		AC_DEFINE(FC_HAVE_BOOL)
    fi
])

dnl -------------------------------------------------------------------------
dnl Check whether C++ library has member ios::bin instead of ios::binary.
dnl -------------------------------------------------------------------------

AC_DEFUN(FC_CHECK_IOS_BINARY,
[
    AC_MSG_CHECKING(whether standard member ios::binary is available)
    AC_CACHE_VAL(fc_cv_have_ios_binary,
    [
        AC_TRY_COMPILE(
            [
#include <fstream.h>
#include <iostream.h>
            ],
		    [ifstream myTest("testfile",ios::in|ios::binary);],
		    [fc_cv_have_ios_binary=yes],
		    [fc_cv_have_ios_binary=no]
	    )
    ])
    AC_MSG_RESULT($fc_cv_have_ios_binary)
    if test "$fc_cv_have_ios_binary" = yes; then
        AC_DEFINE(FC_HAVE_IOS_BINARY)
    fi
])

dnl -------------------------------------------------------------------------
dnl Check whether C++ compiler supports the "nothrow allocator".
dnl -------------------------------------------------------------------------

AC_DEFUN(FC_CHECK_NOTHROW,
[
    AC_MSG_CHECKING(whether nothrow allocator is available)
    AC_CACHE_VAL(fc_cv_have_nothrow,
    [
        AC_TRY_COMPILE(
            [#include <new>],
		    [char* buf = new(std::nothrow) char[1024];],
		    [fc_cv_have_nothrow=yes],
		    [fc_cv_have_nothrow=no]
	    )
    ])
    AC_MSG_RESULT($fc_cv_have_nothrow)
    if test "$fc_cv_have_nothrow" = yes; then
        AC_DEFINE(FC_HAVE_NOTHROW)
    fi
])

dnl -------------------------------------------------------------------------
dnl Pass C++ compiler options to libtool which supports C only.
dnl -------------------------------------------------------------------------

AC_DEFUN(FC_PROG_LIBTOOL,
[
    fc_save_cc="$CC"
    fc_save_cflags="$CFLAGS"
    CC="$CXX"
    CFLAGS="$CXXFLAGS"
    AM_PROG_LIBTOOL
    CFLAGS="$fc_save_cflags"
    CC="$fc_save_cc"
])

