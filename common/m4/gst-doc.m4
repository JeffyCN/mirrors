AC_DEFUN([AG_GST_DOCBOOK_CHECK],
[
  dnl choose a location to install docbook docs in
  if test "x$PACKAGE_TARNAME" = "x"
  then
    AC_MSG_ERROR([Internal error - PACKAGE_TARNAME not set])
  fi
  docdir="\$(datadir)/doc/$PACKAGE_TARNAME-$GST_API_VERSION"

  dnl enable/disable docbook documentation building
  AC_ARG_ENABLE(docbook,
  AC_HELP_STRING([--enable-docbook],
                 [use docbook to build documentation [default=no]]),,
                 enable_docbook=no)

  have_docbook=no

  if test x$enable_docbook = xyes; then
    dnl check if we actually have everything we need

    dnl check for docbook tools
    AC_CHECK_PROG(HAVE_DOCBOOK2PS, docbook2ps, yes, no)
    AC_CHECK_PROG(HAVE_XSLTPROC, xsltproc, yes, no)
    AC_CHECK_PROG(HAVE_JADETEX, jadetex, yes, no)
    AC_CHECK_PROG(HAVE_PS2PDF, ps2pdf, yes, no)

    dnl check if we can process docbook stuff
    AS_DOCBOOK(have_docbook=yes, have_docbook=no)

    dnl check for extra tools
    AC_CHECK_PROG(HAVE_DVIPS, dvips, yes, no)
    AC_CHECK_PROG(HAVE_XMLLINT, xmllint, yes, no)

    AC_CHECK_PROG(HAVE_PNGTOPNM, pngtopnm, yes, no)
    AC_CHECK_PROG(HAVE_PNMTOPS,  pnmtops,  yes, no)
    AC_CHECK_PROG(HAVE_EPSTOPDF, epstopdf, yes, no)

    dnl check if we can generate HTML
    if test "x$HAVE_XSLTPROC" = "xyes" && \
       test "x$enable_docbook" = "xyes" && \
       test "x$HAVE_XMLLINT" = "xyes"; then
      DOC_HTML=yes
      AC_MSG_NOTICE(Will output HTML documentation)
     else
      DOC_HTML=no
      AC_MSG_NOTICE(Will not output HTML documentation)
    fi

    dnl check if we can generate PS
    if test "x$HAVE_DOCBOOK2PS" = "xyes" && \
       test "x$enable_docbook" = "xyes" && \
       test "x$HAVE_XMLLINT" = "xyes" && \
       test "x$HAVE_JADETEX" = "xyes" && \
       test "x$HAVE_DVIPS" = "xyes" && \
       test "x$HAVE_PNGTOPNM" = "xyes" && \
       test "x$HAVE_PNMTOPS" = "xyes"; then
      DOC_PS=yes
      AC_MSG_NOTICE(Will output PS documentation)
    else
      DOC_PS=no
      AC_MSG_NOTICE(Will not output PS documentation)
    fi

    dnl check if we can generate PDF - using only ps2pdf
    if test "x$DOC_PS" = "xyes" && \
       test "x$enable_docbook" = "xyes" && \
       test "x$HAVE_XMLLINT" = "xyes" && \
       test "x$HAVE_PS2PDF" = "xyes"; then
      DOC_PDF=yes
      AC_MSG_NOTICE(Will output PDF documentation)
    else
      DOC_PDF=no
      AC_MSG_NOTICE(Will not output PDF documentation)
    fi

    dnl if we don't have everything, we should disable
    if test "x$have_docbook" != "xyes"; then
      enable_docbook=no
    fi
  fi

  dnl if we're going to install documentation, tell us where
  if test "x$have_docbook" = "xyes"; then
    AC_MSG_NOTICE(Installing documentation in $docdir)
    AC_SUBST(docdir)
  fi

  AM_CONDITIONAL(ENABLE_DOCBOOK,      test x$enable_docbook = xyes)
  AM_CONDITIONAL(DOC_HTML,            test x$DOC_HTML = xyes)
  AM_CONDITIONAL(DOC_PDF,             test x$DOC_PDF = xyes)
  AM_CONDITIONAL(DOC_PS,              test x$DOC_PS = xyes)
])
