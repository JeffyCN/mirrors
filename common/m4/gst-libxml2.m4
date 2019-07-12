dnl call this macro with the minimum required version as an argument
dnl this macro sets and AC_SUBSTs XML_CFLAGS and XML_LIBS
dnl it also sets LIBXML_PKG, used for the pkg-config file

AC_DEFUN([AG_GST_LIBXML2_CHECK],
[
  dnl Minimum required version of libxml2
  dnl default to 2.4.9 if not specified
  LIBXML2_REQ=ifelse([$1],,2.4.9,[$1])
  AC_SUBST(LIBXML2_REQ)

  dnl check for libxml2
  PKG_CHECK_MODULES(XML, libxml-2.0 >= $LIBXML2_REQ,
                    HAVE_LIBXML2=yes, [
                      AC_MSG_RESULT(no)
                      HAVE_LIBXML2=no
                    ])
  if test "x$HAVE_LIBXML2" = "xyes"; then
    AC_DEFINE(HAVE_LIBXML2, 1, [Define if libxml2 is available])
  else
    AC_MSG_ERROR([
        Need libxml2 and development headers/files to build GStreamer.

        You can do without libxml2 if you pass --disable-loadsave to
        configure, but that breaks ABI, so don't do that unless you
        are building for an embedded setup and know what you are doing.
    ])
  fi
  dnl this is for the .pc file
  LIBXML_PKG=', libxml-2.0'
  AC_SUBST(LIBXML_PKG)
  AC_SUBST(XML_LIBS)
  AC_SUBST(XML_CFLAGS)

  dnl XML_LIBS might pull in -lz without zlib actually being on the system, so
  dnl try linking with these LIBS and CFLAGS
  ac_save_CFLAGS=$CFLAGS
  ac_save_LIBS=$LIBS
  CFLAGS="$CFLAGS $XML_CFLAGS"
  LIBS="$LIBS $XML_LIBS"
  AC_TRY_LINK([
#include <libxml/tree.h>
#include <stdio.h>
],[
/* function body */
],
    AC_MSG_NOTICE([Test xml2 program linked]),
    AC_MSG_ERROR([Could not link libxml2 test program.  Check if you have the necessary dependencies.])
  )
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
])
