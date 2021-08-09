SUBDIRS = gst-libs gst common m4

ACLOCAL_AMFLAGS = -I m4 -I common/m4

include $(top_srcdir)/common/release.mak

include $(top_srcdir)/common/coverage/lcov.mak

include $(top_srcdir)/common/cruft.mak

check: check-exports

all-local: check-cruft
