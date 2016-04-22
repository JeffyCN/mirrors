ALWAYS_SUBDIRS =		\
	gst			\
	common			\
	m4


SUBDIRS = $(ALWAYS_SUBDIRS)

DIST_SUBDIRS = $(ALWAYS_SUBDIRS)

ACLOCAL_AMFLAGS = -I m4 -I common/m4

include $(top_srcdir)/common/release.mak
include $(top_srcdir)/common/po.mak

check-valgrind:
	cd tests/check && make check-valgrind

include $(top_srcdir)/common/coverage/lcov.mak

include $(top_srcdir)/common/cruft.mak

check: check-exports

all-local: check-cruft
