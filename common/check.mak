# keep target around, since it's referenced in the modules' Makefiles
clean-local-check:
	@echo

if HAVE_VALGRIND
# hangs spectacularly on some machines, so let's not do this by default yet
check-valgrind:
	$(MAKE) valgrind
else
check-valgrind:
	@true
endif

LOOPS ?= 10

# run any given test by running make test.check
# if the test fails, run it again at at least debug level 2
%.check: %
	@$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=20					\
	$* ||							\
	$(TESTS_ENVIRONMENT)					\
	GST_DEBUG=$$GST_DEBUG,*:2				\
	CK_DEFAULT_TIMEOUT=20					\
	$*

# just like 'check', but don't run it again if it fails (useful for debugging)
%.check-norepeat: %
	@$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=20					\
	$*

# run any given test in a loop
%.torture: %
	@for i in `seq 1 $(LOOPS)`; do				\
	$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=20					\
	$*; done

# run any given test in an infinite loop
%.forever: %
	@while true; do						\
	$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=20					\
	$* || break; done

# valgrind any given test by running make test.valgrind
%.valgrind: %
	@$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=360					\
	G_SLICE=always-malloc					\
	$(LIBTOOL) --mode=execute				\
	$(VALGRIND_PATH) -q					\
	$(foreach s,$(SUPPRESSIONS),--suppressions=$(s))	\
	--tool=memcheck --leak-check=full --trace-children=yes	\
	--leak-resolution=high --num-callers=20			\
	./$* 2>&1 | tee valgrind.log
	@if grep "==" valgrind.log > /dev/null 2>&1; then	\
	    rm valgrind.log;					\
	    exit 1;						\
	fi
	@rm valgrind.log
	
# valgrind any given test and generate suppressions for it
%.valgrind.gen-suppressions: %
	@$(TESTS_ENVIRONMENT)					\
	CK_DEFAULT_TIMEOUT=360					\
	G_SLICE=always-malloc					\
	$(LIBTOOL) --mode=execute				\
	$(VALGRIND_PATH) -q 					\
	$(foreach s,$(SUPPRESSIONS),--suppressions=$(s))	\
	--tool=memcheck --leak-check=full --trace-children=yes	\
	--leak-resolution=high --num-callers=20			\
	--gen-suppressions=all					\
	./$* 2>&1 | tee suppressions.log
	
# valgrind torture any given test
%.valgrind-torture: %
	@for i in `seq 1 $(LOOPS)`; do				\
		$(MAKE) $*.valgrind ||				\
		(echo "Failure after $$i runs"; exit 1) ||	\
		exit 1;						\
	done
	@banner="All $(LOOPS) loops passed";			\
	dashes=`echo "$$banner" | sed s/./=/g`;			\
	echo $$dashes; echo $$banner; echo $$dashes

# valgrind any given test until failure by running make test.valgrind-forever
%.valgrind-forever: %
	@while $(MAKE) $*.valgrind; do				\
	  true; done

# gdb any given test by running make test.gdb
%.gdb: %
	@$(TESTS_ENVIRONMENT)					\
	CK_FORK=no						\
	$(LIBTOOL) --mode=execute				\
	gdb $*

%.lcov-reset:
	$(MAKE) $*.lcov-run
	$(MAKE) $*.lcov-report

%.lcov: %
	$(MAKE) $*.lcov-reset

if GST_GCOV_ENABLED
%.lcov-clean:
	$(MAKE) -C $(top_builddir) lcov-clean

%.lcov-run:
	$(MAKE) $*.lcov-clean
	$(MAKE) $*.check

%.lcov-report:
	$(MAKE) -C $(top_builddir) lcov-report
else
%.lcov-run:
	echo "Need to reconfigure with --enable-gcov"

%.lcov-report:
	echo "Need to reconfigure with --enable-gcov"
endif

# torture tests
torture: $(TESTS)
	-rm test-registry.*
	@echo "Torturing tests ..."
	@for i in `seq 1 $(LOOPS)`; do				\
		$(MAKE) check ||				\
		(echo "Failure after $$i runs"; exit 1) ||	\
		exit 1;						\
	done
	@banner="All $(LOOPS) loops passed";			\
	dashes=`echo "$$banner" | sed s/./=/g`;			\
	echo $$dashes; echo $$banner; echo $$dashes

# forever tests
forever: $(TESTS)
	-rm test-registry.*
	@echo "Forever tests ..."
	@while true; do						\
		$(MAKE) check ||				\
		(echo "Failure"; exit 1) ||			\
		exit 1;						\
	done

# valgrind all tests
valgrind: $(TESTS)
	@echo "Valgrinding tests ..."
	@failed=0;							\
	for t in $(filter-out $(VALGRIND_TESTS_DISABLE),$(TESTS)); do	\
		$(MAKE) $$t.valgrind;					\
		if test "$$?" -ne 0; then                               \
			echo "Valgrind error for test $$t";		\
			failed=`expr $$failed + 1`;			\
			whicht="$$whicht $$t";				\
		fi;							\
	done;								\
	if test "$$failed" -ne 0; then					\
		echo "$$failed tests had leaks or errors under valgrind:";	\
		echo "$$whicht";					\
		false;							\
	fi

# valgrind all tests until failure
valgrind-forever: $(TESTS)
	-rm test-registry.*
	@echo "Forever valgrinding tests ..."
	@while true; do						\
		$(MAKE) valgrind ||				\
		(echo "Failure"; exit 1) ||			\
		exit 1;						\
	done

# valgrind torture all tests
valgrind-torture: $(TESTS)
	-rm test-registry.*
	@echo "Torturing and valgrinding tests ..."
	@for i in `seq 1 $(LOOPS)`; do				\
		$(MAKE) valgrind ||				\
		(echo "Failure after $$i runs"; exit 1) ||	\
		exit 1;						\
	done
	@banner="All $(LOOPS) loops passed";			\
	dashes=`echo "$$banner" | sed s/./=/g`;			\
	echo $$dashes; echo $$banner; echo $$dashes

# valgrind all tests and generate suppressions
valgrind.gen-suppressions: $(TESTS)
	@echo "Valgrinding tests ..."
	@failed=0;							\
	for t in $(filter-out $(VALGRIND_TESTS_DISABLE),$(TESTS)); do	\
		$(MAKE) $$t.valgrind.gen-suppressions;			\
		if test "$$?" -ne 0; then                               \
			echo "Valgrind error for test $$t";		\
			failed=`expr $$failed + 1`;			\
			whicht="$$whicht $$t";				\
		fi;							\
	done;								\
	if test "$$failed" -ne 0; then					\
		echo "$$failed tests had leaks or errors under valgrind:";	\
		echo "$$whicht";					\
		false;							\
	fi

# inspect every plugin feature
GST_INSPECT = $(GST_TOOLS_DIR)/gst-inspect-$(GST_API_VERSION)
inspect:
	@echo "Inspecting features ..."
	@for e in `$(TESTS_ENVIRONMENT) $(GST_INSPECT) | head -n -2 	\
	  | cut -d: -f2`;						\
	  do echo Inspecting $$e;					\
	     $(GST_INSPECT) $$e > /dev/null 2>&1; done

help:
	@echo
	@echo "make check                         -- run all checks"
	@echo "make torture                       -- run all checks $(LOOPS) times"
	@echo "make (dir)/(test).check            -- run the given check once, repeat with GST_DEBUG=*:2 if it fails"
	@echo "make (dir)/(test).check-norepeat   -- run the given check once, but don't run it again if it fails"
	@echo "make (dir)/(test).forever          -- run the given check forever"
	@echo "make (dir)/(test).torture          -- run the given check $(LOOPS) times"
	@echo
	@echo "make (dir)/(test).gdb              -- start up gdb for the given test"
	@echo
	@echo "make valgrind                      -- valgrind all tests"
	@echo "make valgrind-forever              -- valgrind all tests forever"
	@echo "make valgrind-torture              -- valgrind all tests $(LOOPS) times"
	@echo "make valgrind.gen-suppressions     -- generate suppressions for all tests"
	@echo "                                      and save to suppressions.log"
	@echo "make (dir)/(test).valgrind         -- valgrind the given test"
	@echo "make (dir)/(test).valgrind-forever -- valgrind the given test forever"
	@echo "make (dir)/(test).valgrind-torture -- valgrind the given test $(LOOPS) times"
	@echo "make (dir)/(test).valgrind.gen-suppressions -- generate suppressions"
	@echo "                                               and save to suppressions.log"
	@echo "make inspect                       -- inspect all plugin features"
	@echo
	@echo
	@echo "Additionally, you can use the GST_CHECKS environment variable to"
	@echo "specify which test(s) should be run. This is useful if you are"
	@echo "debugging a failure in one particular test, or want to reproduce"
	@echo "a race condition in a single test."
	@echo
	@echo "Examples:"
	@echo
	@echo "  GST_CHECKS=test_this,test_that  make element/foobar.check"
	@echo "  GST_CHECKS=test_many_threads    make element/foobar.forever"
	@echo

