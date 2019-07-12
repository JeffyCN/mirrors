AC_DEFUN([AG_GST_BISON_CHECK],
[
  dnl FIXME: check if AC_PROG_YACC is suitable here
  dnl FIXME: make precious
  AC_PATH_PROG(BISON_PATH, bison, no)
  if test x$BISON_PATH = xno; then
    AC_MSG_ERROR(Could not find bison)
  fi

  dnl check bison version
  dnl we need version >= 1.875 for the reentrancy support
  dnl in the parser.
  dnl First lines observed: 'bison (GNU Bison) 2.3' or 'GNU Bison version 1.28'
  bison_min_version=1.875
  bison_version=`$BISON_PATH --version | head -n 1 |  sed 's/^[[^0-9]]*//' | sed 's/[[^0-9]]*$//' | cut -d' ' -f1`
  AC_MSG_CHECKING([bison version $bison_version >= $bison_min_version])

  if perl -we "exit ((v$bison_version ge v$bison_min_version) ? 0 : 1)"; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_ERROR([no])
  fi
])

AC_DEFUN([AG_GST_FLEX_CHECK],
[
  dnl we require flex for building the parser
  AC_PATH_PROG(FLEX_PATH, flex, no)
  if test x$FLEX_PATH = xno; then
    AC_MSG_ERROR(Could not find flex)
  fi

  dnl check flex version
  dnl we need version >= 2.5.31 for the reentrancy support
  dnl in the parser.
  flex_min_version=2.5.31
  flex_version=`$FLEX_PATH --version | head -n 1 | awk '{print $2}'`
  AC_MSG_CHECKING([flex version $flex_version >= $flex_min_version])
  if perl -w <<EOF
    (\$min_version_major, \$min_version_minor, \$min_version_micro ) = "$flex_min_version" =~ /(\d+)\.(\d+)\.(\d+)/;
    (\$flex_version_major, \$flex_version_minor, \$flex_version_micro ) = "$flex_version" =~ /(\d+)\.(\d+)\.(\d+)/;
    exit (((\$flex_version_major > \$min_version_major) ||
     ((\$flex_version_major == \$min_version_major) &&
      (\$flex_version_minor > \$min_version_minor)) ||
     ((\$flex_version_major == \$min_version_major) &&
      (\$flex_version_minor == \$min_version_minor) &&
      (\$flex_version_micro >= \$min_version_micro)))
     ? 0 : 1);
EOF
  then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_ERROR([no])
  fi
])
