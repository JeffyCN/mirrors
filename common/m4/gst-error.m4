dnl handle various error-related things

dnl Thomas Vander Stichele <thomas@apestaart.org>
dnl Tim-Philipp Müller <tim centricular net>

dnl Last modification: 2008-02-18

dnl AG_GST_SET_ERROR_CFLAGS([ADD-WERROR], [MORE_FLAGS])
dnl AG_GST_SET_ERROR_CXXFLAGS([ADD-WERROR], [MORE_FLAGS])
dnl AG_GST_SET_LEVEL_DEFAULT([IS-GIT-VERSION])


dnl Sets WARNING_CFLAGS and ERROR_CFLAGS to something the compiler 
dnl will accept and AC_SUBST them so they are available in Makefile
dnl
dnl WARNING_CFLAGS will contain flags to make the compiler emit more
dnl   warnings.
dnl ERROR_CFLAGS will contain flags to make those warnings fatal,
dnl   unless ADD-WERROR is set to "no"
dnl 
dnl If MORE_FLAGS is set, tries to add each of the given flags
dnl to WARNING_CFLAGS if the compiler supports them. Each flag is 
dnl tested separately.
dnl
dnl These flags can be overridden at make time:
dnl make ERROR_CFLAGS=
AC_DEFUN([AG_GST_SET_ERROR_CFLAGS],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AS_COMPILER_FLAG])

  WARNING_CFLAGS=""
  ERROR_CFLAGS=""

  dnl if we support -Wall, set it unconditionally
  AS_COMPILER_FLAG(-Wall,
                   WARNING_CFLAGS="$WARNING_CFLAGS -Wall")

  dnl Warn if declarations after statements are used (C99 extension)
  AS_COMPILER_FLAG(-Wdeclaration-after-statement,
        WARNING_CFLAGS="$WARNING_CFLAGS -Wdeclaration-after-statement")

  dnl Warn if variable length arrays are used (C99 extension)
  AS_COMPILER_FLAG(-Wvla,
        WARNING_CFLAGS="$WARNING_CFLAGS -Wvla")

  dnl Warn for invalid pointer arithmetic
  AS_COMPILER_FLAG(-Wpointer-arith,
        WARNING_CFLAGS="$WARNING_CFLAGS -Wpointer-arith")

  dnl if asked for, add -Werror if supported
  if test "x$1" != "xno"
  then
    AS_COMPILER_FLAG(-Werror, ERROR_CFLAGS="$ERROR_CFLAGS -Werror")

    dnl if -Werror isn't suported, try -errwarn=%all (Sun Forte case)
    if test "x$ERROR_CFLAGS" = "x"
    then
      AS_COMPILER_FLAG([-errwarn=%all], [
          ERROR_CFLAGS="-errwarn=%all"
          dnl try -errwarn=%all,no%E_EMPTY_DECLARATION,
          dnl no%E_STATEMENT_NOT_REACHED,no%E_ARGUEMENT_MISMATCH,
          dnl no%E_MACRO_REDEFINED (Sun Forte case)
          dnl For Forte we need disable "empty declaration" warning produced by un-needed semicolon
          dnl "statement not reached" disabled because there is g_assert_not_reached () in some places
          dnl "macro redefined" because of gst/gettext.h
          dnl FIXME: is it really supposed to be 'ARGUEMENT' and not 'ARGUMENT'?
          for f in 'no%E_EMPTY_DECLARATION' \
                   'no%E_STATEMENT_NOT_REACHED' \
                   'no%E_ARGUEMENT_MISMATCH' \
                   'no%E_MACRO_REDEFINED' \
                   'no%E_LOOP_NOT_ENTERED_AT_TOP'
          do
            AS_COMPILER_FLAG([-errwarn=%all,$f], [
              ERROR_CFLAGS="$ERROR_CFLAGS,$f"
            ])
          done
      ])
    else
      dnl Add -fno-strict-aliasing for GLib versions before 2.19.8
      dnl as before G_LOCK and friends caused strict aliasing compiler
      dnl warnings.
      PKG_CHECK_EXISTS([glib-2.0 < 2.19.8], [
        AS_COMPILER_FLAG(-fno-strict-aliasing,
            ERROR_CFLAGS="$ERROR_CFLAGS -fno-strict-aliasing")
	])
    fi
  fi

  if test "x$2" != "x"
  then
    UNSUPPORTED=""
    list="$2"
    for each in $list
    do
      AS_COMPILER_FLAG($each,
          WARNING_CFLAGS="$WARNING_CFLAGS $each",
          UNSUPPORTED="$UNSUPPORTED $each")
    done
    if test "X$UNSUPPORTED" != X ; then
      AC_MSG_NOTICE([unsupported compiler flags: $UNSUPPORTED])
    fi
  fi

  AC_SUBST(WARNING_CFLAGS)
  AC_SUBST(ERROR_CFLAGS)
  AC_MSG_NOTICE([set WARNING_CFLAGS to $WARNING_CFLAGS])
  AC_MSG_NOTICE([set ERROR_CFLAGS to $ERROR_CFLAGS])
])

dnl Sets WARNING_CXXFLAGS and ERROR_CXXFLAGS to something the compiler 
dnl will accept and AC_SUBST them so they are available in Makefile
dnl
dnl WARNING_CXXFLAGS will contain flags to make the compiler emit more
dnl   warnings.
dnl ERROR_CXXFLAGS will contain flags to make those warnings fatal,
dnl   unless ADD-WERROR is set to "no"
dnl 
dnl If MORE_FLAGS is set, tries to add each of the given flags
dnl to WARNING_CFLAGS if the compiler supports them. Each flag is 
dnl tested separately.
dnl
dnl These flags can be overridden at make time:
dnl make ERROR_CXXFLAGS=
AC_DEFUN([AG_GST_SET_ERROR_CXXFLAGS],
[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AS_CXX_COMPILER_FLAG])

  ERROR_CXXFLAGS=""
  WARNING_CXXFLAGS=""

  dnl if we support -Wall, set it unconditionally
  AS_CXX_COMPILER_FLAG(-Wall, WARNING_CXXFLAGS="$WARNING_CXXFLAGS -Wall")

  dnl if asked for, add -Werror if supported
  if test "x$1" != "xno"
  then
    AS_CXX_COMPILER_FLAG(-Werror, ERROR_CXXFLAGS="$ERROR_CXXFLAGS -Werror")

    if test "x$ERROR_CXXFLAGS" != "x"
    then
        dnl add exceptions
        AS_CXX_COMPILER_FLAG([-Wno-non-virtual-dtor], ERROR_CXXFLAGS="$ERROR_CXXFLAGS -Wno-non-virtual-dtor")

	dnl Add -fno-strict-aliasing for GLib versions before 2.19.8
	dnl as before G_LOCK and friends caused strict aliasing compiler
	dnl warnings.
	PKG_CHECK_EXISTS([glib-2.0 < 2.19.8], [
	  AS_CXX_COMPILER_FLAG([-fno-strict-aliasing],
	    ERROR_CXXFLAGS="$ERROR_CXXFLAGS -fno-strict-aliasing")
	  ])
    else
      dnl if -Werror isn't suported, try -errwarn=%all
      AS_CXX_COMPILER_FLAG([-errwarn=%all], ERROR_CXXFLAGS="$ERROR_CXXFLAGS -errwarn=%all")
      if test "x$ERROR_CXXFLAGS" != "x"; then
        dnl try -errwarn=%all,no%E_EMPTY_DECLARATION,
        dnl no%E_STATEMENT_NOT_REACHED,no%E_ARGUEMENT_MISMATCH,
        dnl no%E_MACRO_REDEFINED (Sun Forte case)
        dnl For Forte we need disable "empty declaration" warning produced by un-needed semicolon
        dnl "statement not reached" disabled because there is g_assert_not_reached () in some places
        dnl "macro redefined" because of gst/gettext.h
        dnl FIXME: is it really supposed to be 'ARGUEMENT' and not 'ARGUMENT'?
        dnl FIXME: do any of these work with the c++ compiler? if not, why
        dnl do we check at all?
        for f in 'no%E_EMPTY_DECLARATION' \
                 'no%E_STATEMENT_NOT_REACHED' \
                 'no%E_ARGUEMENT_MISMATCH' \
                 'no%E_MACRO_REDEFINED' \
                 'no%E_LOOP_NOT_ENTERED_AT_TOP'
        do
          AS_CXX_COMPILER_FLAG([-errwarn=%all,$f], ERROR_CXXFLAGS="$ERROR_CXXFLAGS,$f")
        done
      fi
    fi
  fi

  if test "x$2" != "x"
  then
    UNSUPPORTED=""
    list="$2"
    for each in $list
    do
      AS_CXX_COMPILER_FLAG($each,
          WARNING_CXXFLAGS="$WARNING_CXXFLAGS $each",
          UNSUPPORTED="$UNSUPPORTED $each")
    done
    if test "X$UNSUPPORTED" != X ; then
      AC_MSG_NOTICE([unsupported compiler flags: $UNSUPPORTED])
    fi
  fi

  AC_SUBST(WARNING_CXXFLAGS)
  AC_SUBST(ERROR_CXXFLAGS)
  AC_MSG_NOTICE([set WARNING_CXXFLAGS to $WARNING_CXXFLAGS])
  AC_MSG_NOTICE([set ERROR_CXXFLAGS to $ERROR_CXXFLAGS])
])

dnl Sets WARNING_OBJCFLAGS and ERROR_OBJCFLAGS to something the compiler 
dnl will accept and AC_SUBST them so they are available in Makefile
dnl
dnl WARNING_OBJCFLAGS will contain flags to make the compiler emit more
dnl   warnings.
dnl ERROR_OBJCFLAGS will contain flags to make those warnings fatal,
dnl   unless ADD-WERROR is set to "no"
dnl 
dnl If MORE_FLAGS is set, tries to add each of the given flags
dnl to WARNING_CFLAGS if the compiler supports them. Each flag is 
dnl tested separately.
dnl
dnl These flags can be overridden at make time:
dnl make ERROR_OBJCFLAGS=
AC_DEFUN([AG_GST_SET_ERROR_OBJCFLAGS],
[
  AC_REQUIRE([AC_PROG_OBJC])
  AC_REQUIRE([AS_OBJC_COMPILER_FLAG])

  ERROR_OBJCFLAGS=""
  WARNING_OBJCFLAGS=""

  dnl if we support -Wall, set it unconditionally
  AS_OBJC_COMPILER_FLAG(-Wall, WARNING_OBJCFLAGS="$WARNING_OBJCFLAGS -Wall")

  dnl if asked for, add -Werror if supported
  if test "x$1" != "xno"
  then
    AS_OBJC_COMPILER_FLAG(-Werror, ERROR_OBJCFLAGS="$ERROR_OBJCFLAGS -Werror")

    if test "x$ERROR_OBJCFLAGS" != "x"
    then
	dnl Add -fno-strict-aliasing for GLib versions before 2.19.8
	dnl as before G_LOCK and friends caused strict aliasing compiler
	dnl warnings.
	PKG_CHECK_EXISTS([glib-2.0 < 2.19.8], [
	  AS_OBJC_COMPILER_FLAG([-fno-strict-aliasing],
	    ERROR_OBJCFLAGS="$ERROR_OBJCFLAGS -fno-strict-aliasing")
	  ])
    else
      dnl if -Werror isn't suported, try -errwarn=%all
      AS_OBJC_COMPILER_FLAG([-errwarn=%all], ERROR_OBJCFLAGS="$ERROR_OBJCFLAGS -errwarn=%all")
      if test "x$ERROR_OBJCFLAGS" != "x"; then
        dnl try -errwarn=%all,no%E_EMPTY_DECLARATION,
        dnl no%E_STATEMENT_NOT_REACHED,no%E_ARGUEMENT_MISMATCH,
        dnl no%E_MACRO_REDEFINED (Sun Forte case)
        dnl For Forte we need disable "empty declaration" warning produced by un-needed semicolon
        dnl "statement not reached" disabled because there is g_assert_not_reached () in some places
        dnl "macro redefined" because of gst/gettext.h
        dnl FIXME: is it really supposed to be 'ARGUEMENT' and not 'ARGUMENT'?
        dnl FIXME: do any of these work with the c++ compiler? if not, why
        dnl do we check at all?
        for f in 'no%E_EMPTY_DECLARATION' \
                 'no%E_STATEMENT_NOT_REACHED' \
                 'no%E_ARGUEMENT_MISMATCH' \
                 'no%E_MACRO_REDEFINED' \
                 'no%E_LOOP_NOT_ENTERED_AT_TOP'
        do
          AS_OBJC_COMPILER_FLAG([-errwarn=%all,$f], ERROR_OBJCFLAGS="$ERROR_OBJCFLAGS,$f")
        done
      fi
    fi
  fi

  if test "x$2" != "x"
  then
    UNSUPPORTED=""
    list="$2"
    for each in $list
    do
      AS_OBJC_COMPILER_FLAG($each,
          WARNING_OBJCFLAGS="$WARNING_OBJCFLAGS $each",
          UNSUPPORTED="$UNSUPPORTED $each")
    done
    if test "X$UNSUPPORTED" != X ; then
      AC_MSG_NOTICE([unsupported compiler flags: $UNSUPPORTED])
    fi
  fi

  AC_SUBST(WARNING_OBJCFLAGS)
  AC_SUBST(ERROR_OBJCFLAGS)
  AC_MSG_NOTICE([set WARNING_OBJCFLAGS to $WARNING_OBJCFLAGS])
  AC_MSG_NOTICE([set ERROR_OBJCFLAGS to $ERROR_OBJCFLAGS])
])

dnl Sets the default error level for debugging messages
AC_DEFUN([AG_GST_SET_LEVEL_DEFAULT],
[
  dnl define correct errorlevel for debugging messages. We want to have
  dnl GST_ERROR messages printed when running cvs builds
  if test "x[$1]" = "xyes"; then
    GST_LEVEL_DEFAULT=GST_LEVEL_ERROR
  else
    GST_LEVEL_DEFAULT=GST_LEVEL_NONE
  fi
  AC_DEFINE_UNQUOTED(GST_LEVEL_DEFAULT, $GST_LEVEL_DEFAULT,
    [Default errorlevel to use])
  dnl AC_SUBST so we can use it for win32/common/config.h
  AC_SUBST(GST_LEVEL_DEFAULT)
])
