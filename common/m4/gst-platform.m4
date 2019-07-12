dnl AG_GST_PLATFORM
dnl Check for platform specific features and define some variables
dnl
dnl GST_EXTRA_MODULE_SUFFIX: contains a platform specific
dnl   extra module suffix additional to G_MODULE_SUFFIX
dnl
dnl HAVE_OSX: Defined if compiling for OS X
dnl
dnl GST_HAVE_UNSAFE_FORK: Defined if fork is unsafe (Windows)
dnl
dnl HAVE_WIN32: Defined if compiling on Win32
dnl

AC_DEFUN([AG_GST_PLATFORM],
[
  AC_REQUIRE([AC_CANONICAL_HOST])

  case $host_os in
    rhapsody*)
      AC_DEFINE_UNQUOTED(GST_EXTRA_MODULE_SUFFIX, [".dylib"], [Extra platform specific plugin suffix])
      ;;
    darwin*)
      AC_DEFINE_UNQUOTED(GST_EXTRA_MODULE_SUFFIX, [".dylib"], [Extra platform specific plugin suffix])
      AC_DEFINE_UNQUOTED(HAVE_OSX, 1, [Defined if compiling for OSX])
      ;;
    cygwin*)
      AC_DEFINE_UNQUOTED(GST_HAVE_UNSAFE_FORK, 1, [Defined when registry scanning through fork is unsafe])
      ;;
    mingw* | msvc* | mks*)
      dnl HAVE_WIN32 currently means "disable POSIXisms".
      AC_DEFINE_UNQUOTED(HAVE_WIN32, 1, [Defined if compiling for Windows])

      dnl define __MSVCRT_VERSION__ version if not set already by the
      dnl compiler (ie. mostly for mingw). This is needed for things like
      dnl __stat64 to be available. If set by the compiler, ensure it's
      dnl new enough - we need at least WinXP SP2.
      AC_TRY_COMPILE([ ], [ return __MSVCRT_VERSION__; ], [
          AC_TRY_COMPILE([ ], [
            #if __MSVCRT_VERSION__ < 0x0601
            #error "MSVCRT too old"
            #endif
          ], [
            AC_MSG_NOTICE([MSVCRT version looks ok])
          ], [
            AC_MSG_ERROR([MSVCRT version too old, need at least WinXP SP2])
          ])
      ], [
        AC_MSG_NOTICE([Setting MSVCRT version to 0x0601])
        AC_DEFINE_UNQUOTED(__MSVCRT_VERSION__, 0x0601, [We need at least WinXP SP2 for __stat64])
      ])
      ;;
     *)
      ;;
  esac
])

AC_DEFUN([AG_GST_LIBTOOL_PREPARE],
[
  dnl Persuade libtool to also link (-l) a 'pure' (DirectX) static lib,
  dnl i.e. as opposed to only import lib with dll counterpart.
  dnl Needs to be tweaked before libtool's checks.
  case $host_os in
  cygwin* | mingw*)
    lt_cv_deplibs_check_method=pass_all
    ;;
  esac
])