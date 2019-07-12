dnl
dnl Check for working do while(0) macros. This is used by G_STMT_START
dnl and G_STMT_END in glib/gmacros.h. Without having this defined we
dnl get "ambigious if-else" compiler warnings when compling C++ code.
dnl
dnl Copied from GLib's configure.in
dnl
AC_DEFUN([AG_GST_CHECK_DOWHILE_MACROS],[

dnl *** check for working do while(0) macros ***
AC_CACHE_CHECK([for working do while(0) macros], _cv_g_support_dowhile_macros, [
	AC_TRY_COMPILE([],[
	#define STMT_START do
	#define STMT_END while(0)
	#define STMT_TEST STMT_START { i = 0; } STMT_END
	int main(void) { int i = 1; STMT_TEST; return i; }],
	[_cv_g_support_dowhile_macros=yes],
	[_cv_g_support_dowhile_macros=no],
	[_cv_g_support_dowhile_macros=yes])
])
if test x$_cv_g_support_dowhile_macros = xyes; then
  AC_DEFINE(HAVE_DOWHILE_MACROS, 1, [define for working do while(0) macros])
fi
])
