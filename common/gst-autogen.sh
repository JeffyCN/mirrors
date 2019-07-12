# a silly hack that generates autoregen.sh but it's handy
# Remove the old autoregen.sh first to create a new file,
# as the current one may be being read by the shell executing
# this script.
if [ -f "autoregen.sh" ]; then
  rm autoregen.sh
fi
echo "#!/bin/sh" > autoregen.sh
echo "./autogen.sh $@ \$@" >> autoregen.sh
chmod +x autoregen.sh

# helper functions for autogen.sh

debug ()
# print out a debug message if DEBUG is a defined variable
{
  if test ! -z "$DEBUG"
  then
    echo "DEBUG: $1"
  fi
}

version_get ()
# based on the command's version output, set variables
# _MAJOR, _MINOR, _MICRO, _VERSION, using the given prefix as variable prefix
#
# arg 1: command binary name
# arg 2: (uppercased) variable name prefix
{
  COMMAND=$1
  VARPREFIX=`echo $2 | tr .,- _`

  # strip everything that's not a digit, then use cut to get the first field
  pkg_version=`$COMMAND --version|head -n 1|sed 's/^.*)[^0-9]*//'|cut -d' ' -f1`
  debug "pkg_version $pkg_version"
  # remove any non-digit characters from the version numbers to permit numeric
  # comparison
  pkg_major=`echo $pkg_version | cut -d. -f1 | sed s/[a-zA-Z\-].*//g`
  pkg_minor=`echo $pkg_version | cut -d. -f2 | sed s/[a-zA-Z\-].*//g`
  pkg_micro=`echo $pkg_version | cut -d. -f3 | sed s/[a-zA-Z\-].*//g`
  test -z "$pkg_major" && pkg_major=0
  test -z "$pkg_minor" && pkg_minor=0
  test -z "$pkg_micro" && pkg_micro=0
  debug "found major $pkg_major minor $pkg_minor micro $pkg_micro"
  eval ${VARPREFIX}_MAJOR=$pkg_major
  eval ${VARPREFIX}_MINOR=$pkg_minor
  eval ${VARPREFIX}_MICRO=$pkg_micro
  eval ${VARPREFIX}_VERSION=$pkg_version
}

version_compare ()
# Checks whether the version of VARPREFIX is equal to or
# newer than the requested version
# arg1: VARPREFIX
# arg2: MAJOR
# arg3: MINOR
# arg4: MICRO
{
  VARPREFIX=`echo $1 | tr .,- _`
  MAJOR=$2
  MINOR=$3
  MICRO=$4

  eval pkg_major=\$${VARPREFIX}_MAJOR;
  eval pkg_minor=\$${VARPREFIX}_MINOR;
  eval pkg_micro=\$${VARPREFIX}_MICRO;

  #start checking the version
  debug "version_compare: $VARPREFIX against $MAJOR.$MINOR.$MICRO"

    # reset check
    WRONG=

    if [ ! "$pkg_major" -gt "$MAJOR" ]; then
      debug "major: $pkg_major <= $MAJOR"
      if [ "$pkg_major" -lt "$MAJOR" ]; then
        debug "major: $pkg_major < $MAJOR"
        WRONG=1
      elif [ ! "$pkg_minor" -gt "$MINOR" ]; then
        debug "minor: $pkg_minor <= $MINOR"
        if [ "$pkg_minor" -lt "$MINOR" ]; then
          debug "minor: $pkg_minor < $MINOR"
          WRONG=1
        elif [ "$pkg_micro" -lt "$MICRO" ]; then
          debug "micro: $pkg_micro < $MICRO"
	  WRONG=1
        fi
      fi
    fi
    if test ! -z "$WRONG"; then
      debug "version_compare: $VARPREFIX older than $MAJOR.$MINOR.$MICRO"
      return 1
    fi
    debug "version_compare: $VARPREFIX equal to/newer than $MAJOR.$MINOR.$MICRO"
    return 0
}


version_check ()
# check the version of a package
# first argument : package name (executable)
# second argument : optional path where to look for it instead
# third argument : source download url
# rest of arguments : major, minor, micro version
# all consecutive ones : suggestions for binaries to use
# (if not specified in second argument)
{
  PACKAGE=$1
  PKG_PATH=$2
  URL=$3
  MAJOR=$4
  MINOR=$5
  MICRO=$6

  # for backwards compatibility, we let PKG_PATH=PACKAGE when PKG_PATH null
  if test -z "$PKG_PATH"; then PKG_PATH=$PACKAGE; fi
  debug "major $MAJOR minor $MINOR micro $MICRO"
  VERSION=$MAJOR
  if test ! -z "$MINOR"; then VERSION=$VERSION.$MINOR; else MINOR=0; fi
  if test ! -z "$MICRO"; then VERSION=$VERSION.$MICRO; else MICRO=0; fi

  debug "major $MAJOR minor $MINOR micro $MICRO"

  for SUGGESTION in $PKG_PATH; do
    COMMAND="$SUGGESTION"

    # don't check if asked not to
    test -z "$NOCHECK" && {
      printf "  checking for $COMMAND >= $VERSION ... "
    } || {
      # we set a var with the same name as the package, but stripped of
      # unwanted chars
      VAR=`echo $PACKAGE | sed 's/-//g'`
      debug "setting $VAR"
      eval $VAR="$COMMAND"
      return 0
    }

    which $COMMAND > /dev/null 2>&1
    if test $? -eq 1;
    then 
      debug "$COMMAND not found"
      continue
    fi

    VARPREFIX=`echo $COMMAND | sed 's/-//g' | tr [:lower:] [:upper:]`
    version_get $COMMAND $VARPREFIX

    version_compare $VARPREFIX $MAJOR $MINOR $MICRO
    if test $? -ne 0; then
      echo "found $pkg_version, not ok !"
      continue
    else
      echo "found $pkg_version, ok."
      # we set a var with the same name as the package, but stripped of
      # unwanted chars
      VAR=`echo $PACKAGE | sed 's/-//g'`
      debug "setting $VAR"
      eval $VAR="$COMMAND"
      return 0
    fi
  done

  echo "$PACKAGE not found !"
  echo "You must have $PACKAGE installed to compile $package."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at $URL"
  return 1;
}

aclocal_check ()
{
  # normally aclocal is part of automake
  # so we expect it to be in the same place as automake
  # so if a different automake is supplied, we need to adapt as well
  # so how's about replacing automake with aclocal in the set var,
  # and saving that in $aclocal ?
  # note, this will fail if the actual automake isn't called automake*
  # or if part of the path before it contains it
  if [ -z "$automake" ]; then
    echo "Error: no automake variable set !"
    return 1
  else
    aclocal=`echo $automake | sed s/automake/aclocal/`
    debug "aclocal: $aclocal"
    if [ "$aclocal" != "aclocal" ];
    then
      CONFIGURE_DEF_OPT="$CONFIGURE_DEF_OPT --with-aclocal=$aclocal"
    fi
    if [ ! -x `which $aclocal` ]; then
      echo "Error: cannot execute $aclocal !"
      return 1
    fi
  fi
}

autoheader_check ()
{
  # same here - autoheader is part of autoconf
  # use the same voodoo
  if [ -z "$autoconf" ]; then
    echo "Error: no autoconf variable set !"
    return 1
  else
    autoheader=`echo $autoconf | sed s/autoconf/autoheader/`
    debug "autoheader: $autoheader"
    if [ "$autoheader" != "autoheader" ];
    then
      CONFIGURE_DEF_OPT="$CONFIGURE_DEF_OPT --with-autoheader=$autoheader"
    fi
    if [ ! -x `which $autoheader` ]; then
      echo "Error: cannot execute $autoheader !"
      return 1
    fi
  fi

}

die_check ()
{
  # call with $DIE
  # if set to 1, we need to print something helpful then die
  DIE=$1
  if test "x$DIE" = "x1";
  then
    echo
    echo "- Please get the right tools before proceeding."
    echo "- Alternatively, if you're sure we're wrong, run with --nocheck."
    exit 1
  fi
}

autogen_options ()
{
  if test "x$1" = "x"; then
    return 0
  fi

  while test "x$1" != "x" ; do
    optarg=`expr "x$1" : 'x[^=]*=\(.*\)'`
    case "$1" in
      --noconfigure)
          NOCONFIGURE=defined
	  AUTOGEN_EXT_OPT="$AUTOGEN_EXT_OPT --noconfigure"
          echo "+ configure run disabled"
          shift
          ;;
      --nocheck)
	  AUTOGEN_EXT_OPT="$AUTOGEN_EXT_OPT --nocheck"
          NOCHECK=defined
          echo "+ autotools version check disabled"
          shift
          ;;
      -d|--debug)
          DEBUG=defined
	  AUTOGEN_EXT_OPT="$AUTOGEN_EXT_OPT --debug"
          echo "+ debug output enabled"
          shift
          ;;
      -h|--help)
          echo "autogen.sh (autogen options) -- (configure options)"
          echo "autogen.sh help options: "
          echo " --noconfigure            don't run the configure script"
          echo " --nocheck                don't do version checks"
          echo " --debug                  debug the autogen process"
          echo
          echo " --with-autoconf PATH     use autoconf in PATH"
          echo " --with-automake PATH     use automake in PATH"
          echo
          echo "Any argument either not in the above list or after a '--' will be "
          echo "passed to ./configure."
	  exit 1
          ;;
      --with-automake=*)
          AUTOMAKE=$optarg
          echo "+ using alternate automake in $optarg"
	  CONFIGURE_DEF_OPT="$CONFIGURE_DEF_OPT --with-automake=$AUTOMAKE"
          shift
          ;;
      --with-autoconf=*)
          AUTOCONF=$optarg
          echo "+ using alternate autoconf in $optarg"
	  CONFIGURE_DEF_OPT="$CONFIGURE_DEF_OPT --with-autoconf=$AUTOCONF"
          shift
          ;;
      --) shift ; break ;;
      *)
          echo "+ passing argument $1 to configure"
	  CONFIGURE_EXT_OPT="$CONFIGURE_EXT_OPT $1"
          shift
          ;;
    esac
  done

  for arg do CONFIGURE_EXT_OPT="$CONFIGURE_EXT_OPT $arg"; done
  if test ! -z "$CONFIGURE_EXT_OPT"
  then
    echo "+ options passed to configure: $CONFIGURE_EXT_OPT"
  fi
}

toplevel_check ()
{
  srcfile=$1
  test -f $srcfile || {
        echo "You must run this script in the top-level $package directory"
        exit 1
  }
}

tool_run ()
{
  tool=$1
  options=$2
  run_if_fail=$3
  echo "+ running $tool $options..."
  $tool $options || {
    echo
    echo $tool failed
    eval $run_if_fail
    exit 1
  }
}

install_git_hooks ()
{
  if test -d .git; then
    # install pre-commit hook for doing clean commits
    for hook in pre-commit; do
      if test ! \( -x .git/hooks/$hook -a -L .git/hooks/$hook \); then
        echo "+ Installing git $hook hook"
        rm -f .git/hooks/$hook
        ln -s ../../common/hooks/$hook.hook .git/hooks/$hook || {
          # if we couldn't create a symbolic link, try doing a plain cp
          if cp common/hooks/pre-commit.hook .git/hooks/pre-commit; then
            chmod +x .git/hooks/pre-commit;
          else
            echo "********** Couldn't install git $hook hook **********";
          fi
        }
      fi
    done
  fi
}
