dnl macros to set GST_PACKAGE_RELEASE_DATETIME

dnl ===========================================================================
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME
dnl
dnl Usage:
dnl
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME()
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([no]...)
dnl sets the release datetime to the current date
dnl (no = this is not a release, but git or prerelease)
dnl
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([YYYY-MM-DD])
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([yes], [YYYY-MM-DD])
dnl sets the release datetime to the specified date (and time, if given)
dnl (yes = this is a release, not git or prerelease)
dnl
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([yes], [DOAP-FILE], [RELEASE-VERSION])
dnl sets the release date to the release date associated with version
dnl RELEASE-VERSION in the .doap file DOAP-FILE
dnl (yes = this is a release, not git or prerelease)
dnl
dnl We need to treat pre-releases like git because there won't be an entry
dnl in the .doap file for pre-releases yet, and we don't want to use the
dnl date of the last release either.
dnl ===========================================================================
AC_DEFUN([AG_GST_SET_PACKAGE_RELEASE_DATETIME],
[
  dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME()
  dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([yes]...)
  if test "x$1" = "xno" -o "x$1" = "x"; then
    GST_PACKAGE_RELEASE_DATETIME=`date -u "+%Y-%m-%dT%H:%MZ"`
  elif test "x$1" = "xyes"; then
    dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([no], ["YYYY-MM-DD"])
    dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([no], [DOAP-FILE], [RELEASE-VERSION])
    if ( echo $1 | grep -e '^20[1-9][0-9]-[0-1][0-9]-[0-3][0-9]' >/dev/null ) ; then
      GST_PACKAGE_RELEASE_DATETIME=$1
    else
      dnl we assume the .doap file contains the date as YYYY-MM-DD
      YYYY_MM_DD=`sh "${srcdir}/common/extract-release-date-from-doap-file" $3 $2`;
      if test "x$YYYY_MM_DD" != "x"; then
        GST_PACKAGE_RELEASE_DATETIME=$YYYY_MM_DD
      else
        AC_MSG_ERROR([SET_PACKAGE_RELEASE_DATETIME: could not extract
            release date for release version $3 from $2])
        GST_PACKAGE_RELEASE_DATETIME=""
      fi
    fi
  dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([YYYY-MM-DD])
  elif ( echo $1 | grep -e '^20[1-9][0-9]-[0-1][0-9]-[0-3][0-9]' >/dev/null ) ; then
    GST_PACKAGE_RELEASE_DATETIME=$1
  else
    AC_MSG_WARN([SET_PACKAGE_RELEASE_DATETIME: invalid first argument])
    GST_PACKAGE_RELEASE_DATETIME=""
  fi

  if test "x$GST_PACKAGE_RELEASE_DATETIME" = "x"; then
    AC_MSG_WARN([Invalid package release date time: $GST_PACKAGE_RELEASE_DATETIME])
  else
    AC_MSG_NOTICE([Setting GST_PACKAGE_RELEASE_DATETIME to $GST_PACKAGE_RELEASE_DATETIME])

    AC_DEFINE_UNQUOTED([GST_PACKAGE_RELEASE_DATETIME],
        ["$GST_PACKAGE_RELEASE_DATETIME"],
        [GStreamer package release date/time for plugins as YYYY-MM-DD])
  fi
])

dnl ===========================================================================
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME_WITH_NANO
dnl
dnl Usage:
dnl
dnl AG_GST_SET_PACKAGE_RELEASE_DATETIME([NANO-VERSION], [DOAP-FILE], [RELEASE-VERSION])
dnl if NANO-VERSION is 0, sets the release date to the release date associated
dnl with version RELEASE-VERSION in the .doap file DOAP-FILE, otherwise sets
dnl the release date and time to the current date/time.
dnl
dnl We need to treat pre-releases like git because there won't be an entry
dnl in the .doap file for pre-releases yet, and we don't want to use the
dnl date of the last release either.
dnl ===========================================================================
AC_DEFUN([AG_GST_SET_PACKAGE_RELEASE_DATETIME_WITH_NANO],
[
  if test "x$1" = "x0"; then
    AG_GST_SET_PACKAGE_RELEASE_DATETIME([yes], [ $2 ], [ $3 ])
  else
    AG_GST_SET_PACKAGE_RELEASE_DATETIME([no])
  fi
])
