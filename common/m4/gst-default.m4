dnl default elements used for tests and such

dnl AG_GST_DEFAULT_ELEMENTS

AC_DEFUN([AG_GST_DEFAULT_ELEMENTS],
[
  dnl decide on default elements
  dnl FIXME: describe where exactly this gets used
  dnl FIXME: decide if it's a problem that this could point to sinks from
  dnl        depending plugin modules
  dnl FIXME: when can we just use autoaudiosrc and autovideosrc?
  DEFAULT_AUDIOSINK="autoaudiosink"
  DEFAULT_VIDEOSINK="autovideosink"
  DEFAULT_AUDIOSRC="alsasrc"
  DEFAULT_VIDEOSRC="v4l2src"
  DEFAULT_VISUALIZER="goom"
  case "$host" in
    *-sun-* | *pc-solaris* )
      DEFAULT_AUDIOSRC="sunaudiosrc"
      ;;
    *-darwin* )
      DEFAULT_AUDIOSRC="osxaudiosrc"
      ;;
  esac

  dnl Default audio sink
  AC_ARG_WITH(default-audiosink,
    AC_HELP_STRING([--with-default-audiosink], [specify default audio sink]),
    [
      case "${withval}" in
        yes) AC_MSG_ERROR(bad value ${withval} for --with-default-audiosink) ;;
        no)  AC_MSG_ERROR(bad value ${withval} for --with-default-audiosink) ;;
        *)   DEFAULT_AUDIOSINK="${withval}" ;;
      esac
    ],
    [
      DEFAULT_AUDIOSINK="$DEFAULT_AUDIOSINK"
    ] dnl Default value as determined above
  )
  AC_MSG_NOTICE(Using $DEFAULT_AUDIOSINK as default audio sink)
  AC_SUBST(DEFAULT_AUDIOSINK)
  AC_DEFINE_UNQUOTED(DEFAULT_AUDIOSINK, "$DEFAULT_AUDIOSINK",
    [Default audio sink])

  dnl Default audio source
  AC_ARG_WITH(default-audiosrc,
    AC_HELP_STRING([--with-default-audiosrc], [specify default audio source]),
    [
      case "${withval}" in
        yes) AC_MSG_ERROR(bad value ${withval} for --with-default-audiosrc) ;;
        no)  AC_MSG_ERROR(bad value ${withval} for --with-default-audiosrc) ;;
        *)   DEFAULT_AUDIOSRC="${withval}" ;;
      esac
    ],
    [
      DEFAULT_AUDIOSRC="$DEFAULT_AUDIOSRC"
    ] dnl Default value as determined above
  )
  AC_MSG_NOTICE(Using $DEFAULT_AUDIOSRC as default audio source)
  AC_SUBST(DEFAULT_AUDIOSRC)
  AC_DEFINE_UNQUOTED(DEFAULT_AUDIOSRC, "$DEFAULT_AUDIOSRC",
    [Default audio source])

  dnl Default video sink
  AC_ARG_WITH(default-videosink,
    AC_HELP_STRING([--with-default-videosink], [specify default video sink]),
    [
      case "${withval}" in
        yes) AC_MSG_ERROR(bad value ${withval} for --with-default-videosink) ;;
        no)  AC_MSG_ERROR(bad value ${withval} for --with-default-videosink) ;;
        *)   DEFAULT_VIDEOSINK="${withval}" ;;
      esac
    ],
    [
      DEFAULT_VIDEOSINK="$DEFAULT_VIDEOSINK"
    ] dnl Default value as determined above
  )
  AC_MSG_NOTICE(Using $DEFAULT_VIDEOSINK as default video sink)
  AC_SUBST(DEFAULT_VIDEOSINK)
  AC_DEFINE_UNQUOTED(DEFAULT_VIDEOSINK, "$DEFAULT_VIDEOSINK",
    [Default video sink])

  dnl Default video source
  AC_ARG_WITH(default-videosrc,
    AC_HELP_STRING([--with-default-videosrc], [specify default video source]),
    [
      case "${withval}" in
        yes) AC_MSG_ERROR(bad value ${withval} for --with-default-videosrc) ;;
        no)  AC_MSG_ERROR(bad value ${withval} for --with-default-videosrc) ;;
        *)   DEFAULT_VIDEOSRC="${withval}" ;;
      esac
    ],
    [
      DEFAULT_VIDEOSRC="$DEFAULT_VIDEOSRC"
    ] dnl Default value as determined above
  )
  AC_MSG_NOTICE(Using $DEFAULT_VIDEOSRC as default video source)
  AC_SUBST(DEFAULT_VIDEOSRC)
  AC_DEFINE_UNQUOTED(DEFAULT_VIDEOSRC, "$DEFAULT_VIDEOSRC",
    [Default video source])

    dnl Default visualizer
  AC_ARG_WITH(default-visualizer,
    AC_HELP_STRING([--with-default-visualizer], [specify default visualizer]),
    [
      case "${withval}" in
        yes) AC_MSG_ERROR(bad value ${withval} for --with-default-visualizer) ;;
        no)  AC_MSG_ERROR(bad value ${withval} for --with-default-visualizer) ;;
        *)   DEFAULT_VISUALIZER="${withval}" ;;
      esac
    ],
    [
      DEFAULT_VISUALIZER="$DEFAULT_VISUALIZER"
    ] dnl Default value as determined above
  )
  AC_MSG_NOTICE(Using $DEFAULT_VISUALIZER as default visualizer)
  AC_SUBST(DEFAULT_VISUALIZER)
  AC_DEFINE_UNQUOTED(DEFAULT_VISUALIZER, "$DEFAULT_VISUALIZER",
    [Default visualizer])
])
