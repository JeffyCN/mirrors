AC_PREREQ(2.62)

dnl please read gstreamer/docs/random/autotools before changing this file

dnl initialize autoconf
dnl releases only do -Wall, git and prerelease does -Werror too
dnl use a three digit version number for releases, and four for git/pre
AC_INIT([GStreamer Plug-ins], [1.0.0])

AG_GST_INIT

dnl initialize automake
AM_INIT_AUTOMAKE([-Wno-portability 1.11 no-dist-gzip dist-xz tar-ustar])

dnl define PACKAGE_VERSION_* variables
AS_VERSION

dnl define the output header for config
AM_CONFIG_HEADER([config.h])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE([enable])

dnl sets host_* variables
AC_CANONICAL_HOST

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])

dnl our libraries and install dirs use GST_API_VERSION in the filename
dnl to allow side-by-side installation of different API versions
GST_API_VERSION=1.0
AC_SUBST(GST_API_VERSION)
AC_DEFINE_UNQUOTED(GST_API_VERSION, "$GST_API_VERSION",
  [GStreamer API Version])

AG_GST_LIBTOOL_PREPARE
AS_LIBTOOL(GST, 5, 0, 5)

dnl *** required versions of GStreamer stuff ***
GST_REQ=1.0.0
GSTPB_REQ=1.0.0

dnl *** autotools stuff ****

dnl allow for different autotools
AS_AUTOTOOLS_ALTERNATE

dnl Add parameters for aclocal
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4 -I common/m4")

dnl set up gettext
dnl the version check needs to stay here because autopoint greps for it
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])
AG_GST_GETTEXT([gst-plugins-$GST_API_VERSION])

dnl *** check for arguments to configure ***

AG_GST_ARG_DISABLE_FATAL_WARNINGS

AG_GST_ARG_DEBUG
AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV

AG_GST_ARG_EXAMPLES

AG_GST_ARG_WITH_PKG_CONFIG_PATH
AG_GST_ARG_WITH_PACKAGE_NAME
AG_GST_ARG_WITH_PACKAGE_ORIGIN

AG_GST_ARG_WITH_PLUGINS

AG_GST_ARG_ENABLE_EXTERNAL

AG_GST_ARG_ENABLE_EXPERIMENTAL

dnl *** checks for platform ***

dnl * hardware/architecture *

dnl common/m4/gst-arch.m4
dnl check CPU type
AG_GST_ARCH

dnl Determine endianness
AC_C_BIGENDIAN

dnl *** checks for programs ***

dnl find a compiler
AC_PROG_CC
AC_PROG_CC_STDC

dnl determine c++ compiler
AC_PROG_CXX
dnl determine if c++ is available on this system
AC_CHECK_PROG(HAVE_CXX, $CXX, yes, no)

AC_PROG_OBJC

dnl check if the compiler supports '-c' and '-o' options
AM_PROG_CC_C_O

dnl find an assembler
AM_PROG_AS

dnl check if the compiler supports do while(0) macros
AG_GST_CHECK_DOWHILE_MACROS

AC_PATH_PROG(VALGRIND_PATH, valgrind, no)
AM_CONDITIONAL(HAVE_VALGRIND, test ! "x$VALGRIND_PATH" = "xno")

dnl *** checks for libraries ***

dnl check for libm, for sin()
LT_LIB_M
AC_SUBST(LIBM)

dnl *** checks for header files ***

dnl check if we have ANSI C header files
AC_HEADER_STDC

dnl *** checks for types/defines ***

dnl *** checks for structures ***

dnl *** checks for compiler characteristics ***

dnl check if we have GCC inline-asm
AS_GCC_INLINE_ASSEMBLY([HAVE_GCC_ASM=yes], [HAVE_GCC_ASM=no])
if test x$HAVE_GCC_ASM = xyes ; then
  AC_DEFINE(HAVE_GCC_ASM, 1,
    [Define if compiler supports gcc inline assembly])
fi
AM_CONDITIONAL(HAVE_GCC_ASM, test "x$HAVE_GCC_ASM" = "xyes")

dnl *** checks for library functions ***

LIBS_SAVE=$LIBS
LIBS="$LIBS $LIBM"
AC_CHECK_FUNCS(rint sinh cosh asinh fpclass)
LIBS=$LIBS_SAVE

dnl Check whether isinf() is defined by math.h
AC_CACHE_CHECK([for isinf], ac_cv_have_isinf,
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <math.h>]], [[float f = 0.0; int i=isinf(f)]])],[ac_cv_have_isinf="yes"],[ac_cv_have_isinf="no"]))
if test "$ac_cv_have_isinf" = "yes"
then
    AC_DEFINE(HAVE_ISINF, 1, [Define to 1 if you have the `isinf' function.])
fi

dnl Check for mmap (needed by electricfence plugin)
AC_FUNC_MMAP
AM_CONDITIONAL(GST_HAVE_MMAP, test "x$ac_cv_func_mmap_fixed_mapped" = "xyes")

dnl Check for mmap (needed by electricfence plugin)

AC_CHECK_TYPE([struct ip_mreqn], [
              AC_DEFINE(HAVE_IP_MREQN,, [Define if we have struct ip_mreqn])],,
              [#include <netinet/in.h>])

dnl *** checks for dependency libraries ***


dnl GLib
GLIB_REQ=2.32.0
AG_GST_GLIB_CHECK([$GLIB_REQ])

dnl Orc
ORC_CHECK([0.4.16])

dnl checks for gstreamer
dnl uninstalled is selected preferentially -- see pkg-config(1)
AG_GST_CHECK_GST($GST_API_VERSION, [$GST_REQ], yes)
AG_GST_CHECK_GST_BASE($GST_API_VERSION, [$GST_REQ], yes)
AG_GST_CHECK_GST_PLUGINS_BASE($GST_API_VERSION, [$GSTPB_REQ], yes)

GST_TOOLS_DIR=`$PKG_CONFIG --variable=toolsdir gstreamer-$GST_API_VERSION`
if test -z $GST_TOOLS_DIR; then
  AC_MSG_ERROR([no tools dir defined in GStreamer pkg-config file; core upgrade needed.])
fi
AC_SUBST(GST_TOOLS_DIR)

AC_MSG_NOTICE(Using GStreamer Core Plugins in $GST_PLUGINS_DIR)
AC_MSG_NOTICE(Using GStreamer Base Plugins in $GSTPB_PLUGINS_DIR)

dnl Check for documentation xrefs
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
GST_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-$GST_API_VERSION`"
GSTPB_PREFIX="`$PKG_CONFIG --variable=prefix gstreamer-plugins-base-$GST_API_VERSION`"
AC_SUBST(GLIB_PREFIX)
AC_SUBST(GST_PREFIX)
AC_SUBST(GSTPB_PREFIX)

dnl GTK is optional and used in examples
HAVE_GTK=no
GTK_REQ=3.0.0

dnl *** set variables based on configure arguments ***

dnl set license and copyright notice
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [GStreamer license])
AC_SUBST(GST_LICENSE)

dnl set location of plugin directory
AG_GST_SET_PLUGINDIR

dnl define an ERROR_CFLAGS Makefile variable
dnl -Wundef: too many broken headers
AG_GST_SET_ERROR_CFLAGS($FATAL_WARNINGS, [
      -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls
      -Wwrite-strings -Wold-style-definition -Waggregate-return
      -Winit-self -Wmissing-include-dirs -Waddress -Wno-multichar
      -Wnested-externs $NO_WARNINGS])

dnl define an ERROR_CXXFLAGS Makefile variable
AG_GST_SET_ERROR_CXXFLAGS($FATAL_WARNINGS, [
        -Wmissing-declarations -Wredundant-decls
        -Wwrite-strings
        -Winit-self -Wmissing-include-dirs -Waddress -Wno-multichar
        $NO_WARNINGS])

dnl define an ERROR_OBJCFLAGS Makefile variable
AG_GST_SET_ERROR_OBJCFLAGS($FATAL_WARNINGS, [
      -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls
      -Wwrite-strings -Wold-style-definition
      -Winit-self -Wmissing-include-dirs -Wno-multichar
      -Wnested-externs $NO_WARNINGS])

dnl define correct level for debugging messages
AG_GST_SET_LEVEL_DEFAULT($GST_GIT)

dnl used in examples
AG_GST_DEFAULT_ELEMENTS

dnl *** plug-ins to include ***

dnl Non ported plugins (non-dependant, then dependant)
dnl Make sure you have a space before and after all plugins

dnl these are all the gst plug-ins, compilable without additional libs
AG_GST_CHECK_PLUGIN(vpudec)
AG_GST_CHECK_PLUGIN(vpuenc)
AG_GST_CHECK_PLUGIN(videorga)

dnl of EXT plugins

dnl *** finalize CFLAGS, LDFLAGS, LIBS

dnl Overview:
dnl GST_OPTION_CFLAGS:  common flags for profiling, debugging, errors, ...
dnl GST_*:              flags shared by all built objects
dnl GST_ALL_LDFLAGS:    linker flags shared by all
dnl GST_LIB_LDFLAGS:    not needed, we don't install libraries
dnl GST_LT_LDFLAGS:     library versioning of our libraries
dnl GST_PLUGIN_LDFLAGS: flags to be used for all plugins

dnl GST_OPTION_CFLAGS
if test "x$USE_DEBUG" = xyes; then
   PROFILE_CFLAGS="-g"
fi
AC_SUBST(PROFILE_CFLAGS)

if test "x$PACKAGE_VERSION_NANO" = "x1"; then
  dnl Define _only_ for git (not pre-releases or releases)
  DEPRECATED_CFLAGS="-DGST_DISABLE_DEPRECATED"
else
  DEPRECATED_CFLAGS=""
fi
AC_SUBST(DEPRECATED_CFLAGS)

dnl every flag in GST_OPTION_CFLAGS, GST_OPTION_CXXFLAGS and GST_OPTION_OBJCFLAGS can be overridden
dnl at make time with e.g. make ERROR_CFLAGS=""
GST_OPTION_CFLAGS="\$(WARNING_CFLAGS) \$(ERROR_CFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_OPTION_CXXFLAGS="\$(WARNING_CXXFLAGS) \$(ERROR_CXXFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_OPTION_OBJCFLAGS="\$(WARNING_OBJCFLAGS) \$(ERROR_OBJCFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
AC_SUBST(GST_OPTION_CFLAGS)
AC_SUBST(GST_OPTION_CXXFLAGS)
AC_SUBST(GST_OPTION_OBJCFLAGS)

dnl our libraries need to be versioned correctly
AC_SUBST(GST_LT_LDFLAGS)

dnl FIXME: do we want to rename to GST_ALL_* ?
dnl prefer internal headers to already installed ones
dnl also add builddir include for enumtypes and marshal
dnl add ERROR_CFLAGS, but overridable
GST_CFLAGS="$GST_CFLAGS -DGST_USE_UNSTABLE_API"
GST_CXXFLAGS="$GST_CFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_CXXFLAGS)"
GST_OBJCFLAGS="$GST_CFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_OBJCFLAGS)"
GST_CFLAGS="$GST_CFLAGS $GLIB_EXTRA_CFLAGS \$(GST_OPTION_CFLAGS)"
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_CXXFLAGS)
AC_SUBST(GST_OBJCFLAGS)
AC_SUBST(GST_LIBS)

dnl LDFLAGS really should only contain flags, not libs - they get added before
dnl whatevertarget_LIBS and -L flags here affect the rest of the linking
GST_ALL_LDFLAGS="-no-undefined"
AC_SUBST(GST_ALL_LDFLAGS)

dnl this really should only contain flags, not libs - they get added before
dnl whatevertarget_LIBS and -L flags here affect the rest of the linking
GST_PLUGIN_LDFLAGS="-module -avoid-version -export-symbols-regex '^[_]*gst_plugin_desc.*' $GST_ALL_LDFLAGS"
AC_SUBST(GST_PLUGIN_LDFLAGS)

dnl *** output files ***

PKG_CHECK_MODULES([ROCKCHIP_MPP], [rockchip_mpp])
PKG_CHECK_MODULES([ROCKCHIP_VPU], [rockchip_vpu])

dnl keep this alphabetic per directory, please
AC_CONFIG_FILES(
Makefile
gst/Makefile
gst/vpudec/Makefile
gst/vpuenc/Makefile
gst/videorga/Makefile
po/Makefile.in
common/Makefile
common/m4/Makefile
m4/Makefile
)

dnl Create the config.h file for Visual Studio builds
dnl Beware of spaces and /'s in some of the shell variable contents.
sed \
    -e 's/.*config.h.in.*autoheader.*/\/* Autogenerated config.h created for win32 Visual Studio builds *\/\n\n\/* PREFIX -- specifically added for Windows for easier moving *\/\n#define PREFIX "C:\\\\gstreamer"\n\n#define GST_INSTALL_PLUGINS_HELPER PREFIX "\\\\libexec\\\\gst-install-plugins-helper.exe"/' \
    -e 's/.* GETTEXT_PACKAGE$/#define GETTEXT_PACKAGE "'$GETTEXT_PACKAGE'"/' \
    -e 's/.* GST_DATADIR$/#define GST_DATADIR PREFIX "\\\\share"/' \
    -e 's/.* GST_LEVEL_DEFAULT$/#define GST_LEVEL_DEFAULT GST_LEVEL_ERROR/' \
    -e 's/.* GST_LICENSE$/#define GST_LICENSE "'$GST_LICENSE'"/' \
    -e 's/.* GST_API_VERSION$/#define GST_API_VERSION "'$GST_API_VERSION'"/' \
    -e "s,.* GST_PACKAGE_NAME$,#define GST_PACKAGE_NAME \"${GST_PACKAGE_NAME}\"," \
    -e 's/.* GST_PACKAGE_ORIGIN$/#define GST_PACKAGE_ORIGIN "Unknown package origin"/' \
    -e "s,.* GST_PACKAGE_RELEASE_DATETIME$,#define GST_PACKAGE_RELEASE_DATETIME \"${GST_PACKAGE_RELEASE_DATETIME}\"," \
    -e 's/.* HAVE_CPU_I386$/#define HAVE_CPU_I386 1/' \
    -e 's/.* HAVE_FGETPOS$/#define HAVE_FGETPOS 1/' \
    -e 's/.* HAVE_FSETPOS$/#define HAVE_FSETPOS 1/' \
    -e 's/.* HAVE_LIBXML2$/#define HAVE_LIBXML2 1/' \
    -e 's/.* HAVE_PROCESS_H$/#define HAVE_PROCESS_H 1/' \
    -e 's/.* HAVE_STDLIB_H$/#define HAVE_STDLIB_H 1/' \
    -e 's/.* HAVE_STRING_H$/#define HAVE_STRING_H 1/' \
    -e 's/.* HAVE_SYS_STAT_H$/#define HAVE_SYS_STAT_H 1/' \
    -e 's/.* HAVE_SYS_TYPES_H$/#define HAVE_SYS_TYPES_H 1/' \
    -e 's/.* HAVE_WIN32$/#define HAVE_WIN32 1/' \
    -e 's/.* HOST_CPU$/#define HOST_CPU "i686"/' \
    -e 's/.* LIBDIR$/#ifdef _DEBUG\n#  define LIBDIR PREFIX "\\\\debug\\\\lib"\n#else\n#  define LIBDIR PREFIX "\\\\lib"\n#endif/' \
    -e 's/.* LOCALEDIR$/#define LOCALEDIR PREFIX "\\\\share\\\\locale"/' \
    -e "s/.* PACKAGE$/#define PACKAGE \"$PACKAGE\"/" \
    -e 's/.* PACKAGE_BUGREPORT$/#define PACKAGE_BUGREPORT "http:\/\/bugzilla.gnome.org\/enter_bug.cgi?product=GStreamer"/' \
    -e "s/.* PACKAGE_NAME$/#define PACKAGE_NAME \"$PACKAGE_NAME\"/" \
    -e "s/.* PACKAGE_STRING$/#define PACKAGE_STRING \"$PACKAGE_STRING\"/" \
    -e 's/.* PACKAGE_TARNAME$/#define PACKAGE_TARNAME "'$PACKAGE_TARNAME'"/' \
    -e 's/.* PACKAGE_VERSION$/#define PACKAGE_VERSION "'$PACKAGE_VERSION'"/' \
    -e 's/.* PLUGINDIR$/#ifdef _DEBUG\n#  define PLUGINDIR PREFIX "\\\\debug\\\\lib\\\\gstreamer-0.11"\n#else\n#  define PLUGINDIR PREFIX "\\\\lib\\\\gstreamer-0.11"\n#endif/' \
    -e 's/.* USE_BINARY_REGISTRY$/#define USE_BINARY_REGISTRY/' \
    -e 's/.* VERSION$/#define VERSION "'$VERSION'"/' \
    -e "s/.* DEFAULT_AUDIOSINK$/#define DEFAULT_AUDIOSINK \"directsoundsink\"/" \
    -e "s/.* DEFAULT_AUDIOSRC$/#define DEFAULT_AUDIOSRC \"audiotestsrc\"/" \
    -e "s/.* DEFAULT_VIDEOSRC$/#define DEFAULT_VIDEOSRC \"videotestsrc\"/" \
    -e "s/.* DEFAULT_VISUALIZER$/#define DEFAULT_VISUALIZER \"goom\"/" \
    config.h.in >win32/common/config.h-new

AC_OUTPUT
AG_GST_OUTPUT_PLUGINS
ORC_OUTPUT
