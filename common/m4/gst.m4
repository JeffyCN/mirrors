dnl AG_GST_INIT
dnl sets up use of GStreamer configure.ac macros
dnl all GStreamer autoconf macros are prefixed
dnl with AG_GST_ for public macros
dnl with _AG_GST_ for private macros
dnl
dnl We call AC_CANONICAL_TARGET and AC_CANONICAL_HOST so that
dnl it is valid before AC_ARG_PROGRAM is called

AC_DEFUN([AG_GST_INIT],
[
  m4_pattern_forbid(^_?AG_GST_)
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl we use host_ variables
  AC_REQUIRE([AC_CANONICAL_TARGET]) dnl we use target_ variables
])

dnl AG_GST_PKG_CONFIG_PATH
dnl
dnl sets up a GST_PKG_CONFIG_PATH variable for use in Makefile.am
dnl which contains the path of the in-tree pkgconfig directory first
dnl and then any paths specified in PKG_CONFIG_PATH.
dnl
dnl We do this mostly so we don't have to use unportable shell constructs
dnl such as ${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH} in Makefile.am to handle
dnl the case where the environment variable is not set, but also in order
dnl to avoid a trailing ':' in the PKG_CONFIG_PATH which apparently causes
dnl problems with pkg-config on windows with msys/mingw.
AC_DEFUN([AG_GST_PKG_CONFIG_PATH],
[
  GST_PKG_CONFIG_PATH="\$(top_builddir)/pkgconfig"
  if test "x$PKG_CONFIG_PATH" != "x"; then
    GST_PKG_CONFIG_PATH="$GST_PKG_CONFIG_PATH:$PKG_CONFIG_PATH"
  fi
  AC_SUBST([GST_PKG_CONFIG_PATH])
  AC_MSG_NOTICE([Using GST_PKG_CONFIG_PATH = $GST_PKG_CONFIG_PATH])
])
