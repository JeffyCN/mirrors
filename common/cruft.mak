# checks for left-over files in the (usually uninstalled) tree, ie. for
# stuff that best be deleted to avoid problems like having old plugin binaries
# lying around.
#
# set CRUFT_FILES and/or CRUFT_DIRS in your Makefile.am when you include this

check-cruft:
	@cruft_files=""; cruft_dirs=""; \
	for f in $(CRUFT_FILES); do \
	  if test -e $$f; then \
	    cruft_files="$$cruft_files $$f"; \
	  fi \
	done; \
	for d in $(CRUFT_DIRS); do \
	  if test -e $$d; then \
	    cruft_dirs="$$cruft_dirs $$d"; \
	  fi \
	done; \
	if test "x$$cruft_files$$cruft_dirs" != x; then \
	  echo; \
	  echo "**** CRUFT ALERT *****"; \
	  echo; \
	  echo "The following files and directories may not be needed any "; \
	  echo "longer (usually because a plugin has been merged into     "; \
	  echo "another plugin, moved to a different module, or been      "; \
	  echo "renamed), and you probably want to clean them up if you   "; \
	  echo "don't have local changes:                                 "; \
	  echo; \
	  for f in $$cruft_files; do echo "file $$f"; done; \
	  echo; \
	  for d in $$cruft_dirs; do echo "directory $$d"; done; \
	  echo; \
	  echo "'make clean-cruft' will remove these for you."; \
	  echo; \
	fi

clean-cruft-dirs:
	@for d in $(CRUFT_DIRS); do \
	  if test -e $$d; then \
	    rm -r "$$d" && echo "Removed directory $$d"; \
	  fi \
	done

clean-cruft-files:
	@for f in $(CRUFT_FILES); do \
	  if test -e $$f; then \
	    rm "$$f" && echo "Removed file $$f"; \
	  fi \
	done

clean-cruft: clean-cruft-dirs clean-cruft-files

# also might want to add this to your Makefile.am:
#
# all-local: check-cruft

