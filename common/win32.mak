# various tests to make sure we dist the win32 stuff (for MSVC builds) right

# the MANIFEST contains all win32 related files that should be disted
win32 = $(shell cat $(top_srcdir)/win32/MANIFEST)

# wildcard is apparently not portable to other makes, hence the use of find
# these are library .def files with the symbols to export
win32defs = $(shell find $(top_srcdir)/win32/common -name '*.def')

# wildcard is apparently not portable to other makes, hence the use of find
# these are files that need to be disted with CRLF line endings:
win32crlf = $(shell find $(top_srcdir)/win32 -name '*.dsw' -o -name '*.dsp')

win32-debug:
	@echo; \
	echo win32     = $(win32); \
	echo; \
	echo win32defs = $(win32defs); \
	echo; \
	echo win32crlf = $(win32crlf); \
	echo

win32-check-crlf:
	@echo Checking win32 files for CR LF line endings ...; \
	fail=0 ; \
	for each in $(win32crlf) ; do \
	  result=`perl -e 'print grep(/\r\n/,<>)' "$$each" | wc -l`; \
	  if test "$$result" = 0 ; then \
	    echo $$each must be fixed to have CRLF line endings ; \
	    fail=1; \
	  fi ; \
	done ; \
	exit $$fail

# make sure all symbols we export on linux are defined in the win32 .def too
# (don't care about other unixes for now, it's enough if it works on one of
# the linux build bots; we assume .so )
check-exports:
	@fail=0 ; \
	for l in $(win32defs); do \
	  libbase=`basename "$$l" ".def"`; \
	  libso=`find "$(top_builddir)" -name "$$libbase-@GST_API_VERSION@.so" | grep -v /_build/ | head -n1`; \
	  libdef="$(top_srcdir)/win32/common/$$libbase.def"; \
	  if test "x$$libso" != "x"; then \
	    echo Checking symbols in $$libso; \
	    if ! ($(top_srcdir)/common/check-exports $$libdef $$libso) ; then \
	      fail=1; \
	    fi; \
	  fi; \
	done ; \
	if test $$fail != 0; then \
	  echo '-----------------------------------------------------------'; \
	  echo 'Run this to update the .def files:'; \
	  echo 'make update-exports'; \
	  echo '-----------------------------------------------------------'; \
	fi; \
	exit $$fail

update-exports:
	make check-exports 2>&1 | patch -p1
	git add win32/common/libgst*.def
	git diff --cached -- win32/common/
	echo '^^^--- updated and staged changes above'

# complain about nonportable printf format strings (%lld, %llu, %zu etc.)
check-nonportable-print-format:
	@fail=0 ; \
	loc=`find "$(top_srcdir)" -name '*.c' | xargs grep -n -e '%[0-9]*ll[udx]' -e '%[0-9]*z[udx]'`; \
	if test "x$$loc" != "x"; then \
	  echo "Please fix the following print format strings:" ; \
	  find "$(top_srcdir)" -name '*.c' | xargs grep -n -e '%[0-9]*ll[udx]' -e '%[0-9]*z[udx]'; \
	  fail=1; \
	fi; \
	exit $$fail

dist-hook: check-exports win32-check-crlf


