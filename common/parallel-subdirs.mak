# include this at the end of $MODULE/ext/Makefile.am to force make to
# build subdirectories in parallel when make -jN is used. We will end up
# descending into all subdirectories a second time, but only after the first
# (parallel) run has finished, so it should go right through the second time.

.PHONY: independent-subdirs $(SUBDIRS)

independent-subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

all-recursive: independent-subdirs
