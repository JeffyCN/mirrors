## .PHONY so it always rebuilds it
.PHONY: lcov-reset lcov lcov-run lcov-report lcov-upload lcov-clean

# run lcov from scratch, always
lcov-reset:
	$(MAKE) lcov-run
	$(MAKE) lcov-report

# run lcov from scratch if the dir is not there
lcov:
	$(MAKE) lcov-reset

if GST_GCOV_ENABLED
# reset lcov stats
lcov-clean:
	@-rm -rf lcov
	lcov --directory . --zerocounters

# reset run coverage tests
lcov-run:
	-$(MAKE) lcov-clean
	-if test -d tests/check; then $(MAKE) -C tests/check inspect; fi
	-$(MAKE) check

# generate report based on current coverage data
lcov-report:
	mkdir lcov
	lcov --compat-libtool --directory . --capture --output-file lcov/lcov.info
	lcov --list-full-path -l lcov/lcov.info | grep -v "`cd $(top_srcdir) && pwd`" | cut -d\| -f1 > lcov/remove
	lcov --list-full-path -l lcov/lcov.info | grep "tests/check/" | cut -d\| -f1 >> lcov/remove
	lcov --list-full-path -l lcov/lcov.info | grep "docs/plugins/" | cut -d\| -f1 >> lcov/remove
	lcov -r lcov/lcov.info `cat lcov/remove` > lcov/lcov.cleaned.info
	rm lcov/remove
	mv lcov/lcov.cleaned.info lcov/lcov.info
	genhtml -t "$(PACKAGE_STRING)" -o lcov --num-spaces 2 lcov/lcov.info

lcov-upload: lcov
	rsync -rvz -e ssh --delete lcov/* gstreamer.freedesktop.org:/srv/gstreamer.freedesktop.org/www/data/coverage/lcov/$(PACKAGE)

else
lcov-run:
	echo "Need to reconfigure with --enable-gcov"

lcov-report:
	echo "Need to reconfigure with --enable-gcov"
endif

