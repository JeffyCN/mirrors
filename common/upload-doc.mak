# this snippet is to be included by both our docbook manuals
# and gtk-doc API references

# it adds an upload target to each of these dir's Makefiles

# each Makefile.am should define the following variables:
# - DOC: the base name of the documentation
#        (faq, manual, pwg, gstreamer, gstreamer-libs)
# - FORMATS: the formats in which DOC is output
#            (html ps pdf)

# if you want to use it, make sure your $HOME/.ssh/config file contains the
# correct User entry for the Host entry for the DOC_SERVER

# these variables define the location of the online docs
DOC_SERVER = gstreamer.freedesktop.org
DOC_BASE = /srv/gstreamer.freedesktop.org/www/data/doc
DOC_URL = $(DOC_SERVER):$(DOC_BASE)

upload: $(FORMATS)
	@if echo $(FORMATS) | grep html > /dev/null; then \
	  echo "Preparing docs for upload (rebasing cross-references) ..." ; \
	  if test x$(builddir) != x$(srcdir); then \
	    echo "make upload can only be used if srcdir == builddir"; \
	    exit 1; \
	  fi; \
	  # gtkdoc-rebase sometimes gets confused, so reset everything to \
	  # local links before rebasing to online links                   \
	  gtkdoc-rebase --html-dir=$(builddir)/html 2>/dev/null 2>/dev/null ; \
	  rebase=`gtkdoc-rebase --verbose --online --html-dir=$(builddir)/html` ; \
	  echo "$$rebase" | grep -e "On-*line"; \
	  for req in glib gobject gstreamer gstreamer-libs gst-plugins-base-libs; do \
	    if ! ( echo "$$rebase" | grep -i -e "On-*line.*/$$req/" ); then \
	      echo "===============================================================================" ; \
	      echo " Could not determine online location for $$req docs. Cross-referencing will be " ; \
	      echo " broken, so not uploading. Make sure the library's gtk-doc documentation is    " ; \
	      echo " installed somewhere in /usr/share/gtk-doc.                                    " ; \
	      echo "===============================================================================" ; \
	      exit 1; \
	    fi; \
	  done; \
	  export SRC="$$SRC html"; \
	fi; \
	if echo $(FORMATS) | grep ps > /dev/null; then export SRC="$$SRC $(DOC).ps"; fi; \
	if echo $(FORMATS) | grep pdf > /dev/null; then export SRC="$$SRC $(DOC).pdf"; fi; \
	\
	# upload releases to both X.Y/ and head/ subdirectories \
	export DIR=$(DOC_BASE)/gstreamer/$(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR)/$(DOC); \
	echo Uploading $$SRC to $(DOC_SERVER):$$DIR; \
	ssh $(DOC_SERVER) mkdir -p $$DIR; \
	rsync -rv -e ssh --delete $$SRC $(DOC_SERVER):$$DIR; \
	ssh $(DOC_SERVER) chmod -R g+w $$DIR; \
	\
	export DIR=$(DOC_BASE)/gstreamer/head/$(DOC); \
	echo Uploading $$SRC to $(DOC_SERVER):$$DIR; \
	ssh $(DOC_SERVER) mkdir -p $$DIR; \
	rsync -rv -e ssh --delete $$SRC $(DOC_SERVER):$$DIR; \
	ssh $(DOC_SERVER) chmod -R g+w $$DIR; \
	\
	if echo $(FORMATS) | grep html > /dev/null; then \
	  echo "Un-preparing docs for upload (rebasing cross-references) ..." ; \
	  gtkdoc-rebase --html-dir=$(builddir)/html ; \
	fi; \
	echo Done
