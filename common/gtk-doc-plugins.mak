# This is an include file specifically tuned for building documentation
# for GStreamer plug-ins

help:
	@echo
	@echo "If you are a doc maintainer, run 'make update' to update"
	@echo "the documentation files maintained in git"
	@echo
	@echo Other useful make targets:
	@echo
	@echo  check-inspected-versions: make sure the inspected plugin info
	@echo                            is up to date before a release
	@echo

# update the stuff maintained by doc maintainers
update: scanobj-update
	$(MAKE) check-outdated-docs

# We set GPATH here; this gives us semantics for GNU make
# which are more like other make's VPATH, when it comes to
# whether a source that is a target of one rule is then
# searched for in VPATH/GPATH.
#
GPATH = $(srcdir)

# thomas: make docs parallel installable
TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)-@GST_API_VERSION@

MAINTAINER_DOC_STAMPS =			\
	scanobj-build.stamp

EXTRA_DIST = 				\
	$(MAINTAINER_DOC_STAMPS)		\
	$(srcdir)/inspect/*.xml		\
	$(SCANOBJ_FILES)		\
	$(content_files)		\
	$(extra_files)			\
	$(HTML_IMAGES)			\
	$(DOC_MAIN_SGML_FILE)	\
	$(DOC_OVERRIDES)		\
	$(DOC_MODULE)-sections.txt

# we don't add scanobj-build.stamp here since they are built manually by docs
# maintainers and result is commited to git
DOC_STAMPS =				\
	scan-build.stamp		\
	tmpl-build.stamp		\
	sgml-build.stamp		\
	html-build.stamp		\
	scan.stamp			\
	tmpl.stamp			\
	sgml.stamp			\
	html.stamp

# files generated/updated by gtkdoc-scangobj
SCANOBJ_FILES =				\
	$(DOC_MODULE).args              \
	$(DOC_MODULE).hierarchy         \
	$(DOC_MODULE).interfaces        \
	$(DOC_MODULE).prerequisites     \
	$(DOC_MODULE).signals           \
	$(DOC_MODULE).types

SCANOBJ_FILES_O =			\
	.libs/$(DOC_MODULE)-scan.o

# files generated/updated by gtkdoc-scan
SCAN_FILES =				\
	$(DOC_MODULE)-sections.txt	\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE)-decl.txt		\
	$(DOC_MODULE)-decl-list.txt


REPORT_FILES = \
	$(DOC_MODULE)-undocumented.txt \
	$(DOC_MODULE)-undeclared.txt \
	$(DOC_MODULE)-unused.txt

CLEANFILES = \
	$(SCANOBJ_FILES_O) \
	$(REPORT_FILES) \
	$(DOC_STAMPS) \
	inspect-registry.xml

INSPECT_DIR = inspect

if ENABLE_GTK_DOC
all-local: html-build.stamp

### inspect GStreamer plug-ins; done by documentation maintainer ###

# only look at the plugins in this module when building inspect .xml stuff
INSPECT_REGISTRY=$(top_builddir)/docs/plugins/inspect-registry.xml
INSPECT_ENVIRONMENT=\
	LC_ALL=C \
	GST_PLUGIN_SYSTEM_PATH_1_0= \
	GST_PLUGIN_PATH_1_0=$(top_builddir)/gst:$(top_builddir)/sys:$(top_builddir)/ext:$(top_builddir)/plugins:$(top_builddir)/src:$(top_builddir)/gnl \
	GST_REGISTRY_1_0=$(INSPECT_REGISTRY) \
	PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" \
	$(INSPECT_EXTRA_ENVIRONMENT)

#### scan gobjects; done by documentation maintainer ####
scanobj-update:
	-rm scanobj-build.stamp
	$(MAKE) scanobj-build.stamp

# gstdoc-scanobj produces 5 output files (.new)
# scangobj-merge.py merges them into the file which we commit later
# TODO: also merge the hierarchy
scanobj-build.stamp: $(SCANOBJ_DEPS) $(basefiles)
	@echo "  DOC   Introspecting gobjects"
	@if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES) $(SCAN_FILES);			\
	    do								\
	        if test -e $(srcdir)/$$f; then cp -u $(srcdir)/$$f . ; fi;	\
	    done;							\
	fi;								\
	mkdir -p $(INSPECT_DIR); \
	scanobj_options=""; \
	if test "x$(V)" = "x1"; then \
	    scanobj_options="--verbose"; \
	fi; \
	$(INSPECT_ENVIRONMENT) 					\
	CC="$(GTKDOC_CC)" LD="$(GTKDOC_LD)"				\
	CFLAGS="$(GTKDOC_CFLAGS) $(CFLAGS) $(WARNING_CFLAGS)"	\
	LDFLAGS="$(GTKDOC_LIBS) $(LDFLAGS)"				\
	$(GST_DOC_SCANOBJ) $$scanobj_options --type-init-func="gst_init(NULL,NULL)"	\
	    --module=$(DOC_MODULE) --source=$(PACKAGE) --inspect-dir=$(INSPECT_DIR) &&		\
	    echo "  DOC   Merging introspection data" && \
	    $(PYTHON)						\
	    $(top_srcdir)/common/scangobj-merge.py $(DOC_MODULE) || exit 1;	\
	if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES);					\
	    do								\
	        cmp -s ./$$f $(srcdir)/$$f || cp ./$$f $(srcdir)/ ;		\
	    done;							\
	fi;								\
	touch scanobj-build.stamp

$(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(SCANOBJ_FILES_O): scan-build.stamp
	@true

### scan headers; done on every build ###
scan-build.stamp: $(HFILE_GLOB) $(EXTRA_HFILES) $(basefiles) scanobj-build.stamp
	@echo '  DOC   Scanning header files'
	@if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES) $(SCAN_FILES);			\
	    do								\
	        if test -e $(srcdir)/$$f; then cp -u $(srcdir)/$$f . ; fi;	\
	    done;							\
	fi
	@_source_dir='' ;						\
	for i in $(DOC_SOURCE_DIR) ; do					\
	    _source_dir="$${_source_dir} --source-dir=$$i" ;	        \
	done ;							        \
	gtkdoc-scan							\
	    $(SCAN_OPTIONS) $(EXTRA_HFILES)				\
	    --module=$(DOC_MODULE)					\
	    $${_source_dir}             				\
	    --ignore-headers="$(IGNORE_HFILES)";			\
	touch scan-build.stamp

#### update templates; done on every build ####

# in a non-srcdir build, we need to copy files from the previous step
# and the files from previous runs of this step
tmpl-build.stamp: $(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_OVERRIDES)
	@echo '  DOC   Rebuilding template files'
	@if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES) $(SCAN_FILES);			\
	    do								\
	        if test -e $(srcdir)/$$f; then cp -u $(srcdir)/$$f . ; fi;	\
	    done;							\
	fi
	@gtkdoc-mktmpl --module=$(DOC_MODULE)
	@$(PYTHON) \
		$(top_srcdir)/common/mangle-tmpl.py $(srcdir)/$(INSPECT_DIR) tmpl
	@touch tmpl-build.stamp

tmpl.stamp: tmpl-build.stamp
	@true

#### xml ####

sgml-build.stamp: tmpl.stamp scan-build.stamp $(CFILE_GLOB) $(top_srcdir)/common/plugins.xsl $(expand_content_files)
	@echo '  DOC   Building XML'
	@-mkdir -p xml
	@for a in $(srcdir)/$(INSPECT_DIR)/*.xml; do \
	    xsltproc --stringparam module $(MODULE) \
		$(top_srcdir)/common/plugins.xsl $$a > xml/`basename $$a`; done
	@for f in $(EXAMPLE_CFILES); do \
		$(PYTHON) $(top_srcdir)/common/c-to-xml.py $$f > xml/element-`basename $$f .c`.xml; done
	@gtkdoc-mkdb \
		--module=$(DOC_MODULE) \
		--source-dir=$(DOC_SOURCE_DIR) \
		 --expand-content-files="$(expand_content_files)" \
		--main-sgml-file=$(srcdir)/$(DOC_MAIN_SGML_FILE) \
		--output-format=xml \
		--ignore-files="$(IGNORE_HFILES) $(IGNORE_CFILES)" \
		$(MKDB_OPTIONS)
	@cp ../version.entities xml
	@touch sgml-build.stamp

sgml.stamp: sgml-build.stamp
	@true

#### html ####

html-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	@echo '  DOC   Building HTML'
	@rm -rf html
	@mkdir html
	@cp $(srcdir)/$(DOC_MAIN_SGML_FILE) html
	@for f in $(content_files); do cp $(srcdir)/$$f html; done
	@cp -pr xml html
	@cp ../version.entities html
	@mkhtml_options=""; \
	gtkdoc-mkhtml 2>&1 --help | grep  >/dev/null "\-\-verbose"; \
	if test "$(?)" = "0"; then \
	  if test "x$(V)" = "x1"; then \
	    mkhtml_options="$$mkhtml_options --verbose"; \
	  fi; \
	fi; \
	cd html && gtkdoc-mkhtml $$mkhtml_options $(DOC_MODULE)-@GST_API_VERSION@ $(DOC_MAIN_SGML_FILE)
	@rm -f html/$(DOC_MAIN_SGML_FILE)
	@rm -rf html/xml
	@rm -f html/version.entities
	@test "x$(HTML_IMAGES)" = "x" || for i in "" $(HTML_IMAGES) ; do \
	    if test "$$i" != ""; then cp $(srcdir)/$$i html ; fi; done
	@echo '  DOC   Fixing cross-references'
	@gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=html --html-dir=$(HTML_DIR) $(FIXXREF_OPTIONS)
	@touch html-build.stamp

clean-local-gtkdoc:
	@rm -rf xml tmpl html
# clean files copied for nonsrcdir templates build
	@if test x"$(srcdir)" != x. ; then \
	    rm -rf $(SCANOBJ_FILES) $(SCAN_FILES) $(REPORT_FILES) \
	        $(MAINTAINER_DOC_STAMPS); \
	fi
else
all-local:
clean-local-gtkdoc:
endif

clean-local: clean-local-gtkdoc
	@rm -f *~ *.bak
	@rm -rf .libs

distclean-local:
	@rm -f $(REPORT_FILES) \
	        $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt
	@rm -rf tmpl/*.sgml.bak
	@rm -f $(DOC_MODULE).hierarchy
	@rm -f *.stamp || true
	@if test "$(abs_srcdir)" != "$(abs_builddir)" ; then \
	    rm -f $(DOC_MODULE)-docs.sgml ; \
	    rm -f $(DOC_MODULE).types ; \
	    rm -f $(DOC_MODULE).interfaces ; \
	    rm -f $(DOC_MODULE)-overrides.txt ; \
	    rm -f $(DOC_MODULE).prerequisites ; \
	    rm -f $(DOC_MODULE)-sections.txt ; \
	    rm -rf tmpl/*.sgml ; \
	    rm -rf $(INSPECT_DIR); \
	fi
	@rm -rf *.o

MAINTAINERCLEANFILES = $(MAINTAINER_DOC_STAMPS)

# thomas: make docs parallel installable; devhelp requires majorminor too
install-data-local:
	(installfiles=`echo $(builddir)/html/*.sgml $(builddir)/html/*.html $(builddir)/html/*.png $(builddir)/html/*.css`; \
	if test "$$installfiles" = '$(builddir)/html/*.sgml $(builddir)/html/*.html $(builddir)/html/*.png $(builddir)/html/*.css'; \
	then echo '-- Nothing to install' ; \
	else \
	  $(mkinstalldirs) $(DESTDIR)$(TARGET_DIR); \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  pngfiles=`echo ./html/*.png`; \
	  if test "$$pngfiles" != './html/*.png'; then \
	    for i in $$pngfiles; do \
	      echo '-- Installing '$$i ; \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	    done; \
	  fi; \
	  echo '-- Installing $(builddir)/html/$(DOC_MODULE)-@GST_API_VERSION@.devhelp2' ; \
	  if test -e $(builddir)/html/$(DOC_MODULE)-@GST_API_VERSION@.devhelp2; then \
	            $(INSTALL_DATA) $(builddir)/html/$(DOC_MODULE)-@GST_API_VERSION@.devhelp2 \
	            $(DESTDIR)$(TARGET_DIR)/$(DOC_MODULE)-@GST_API_VERSION@.devhelp2; \
	  fi; \
	  $(GTKDOC_REBASE) --relative --dest-dir=$(DESTDIR) --html-dir=$(DESTDIR)$(TARGET_DIR) || true ; \
	fi)
uninstall-local:
	if test -d $(DESTDIR)$(TARGET_DIR); then \
	  rm -rf $(DESTDIR)$(TARGET_DIR)/*; \
	  rmdir -p $(DESTDIR)$(TARGET_DIR) 2>/dev/null || true; \
	else \
	  echo '-- Nothing to uninstall' ; \
	fi;

#
# Checks
#
if ENABLE_GTK_DOC
check-hierarchy: $(DOC_MODULE).hierarchy
	@if grep '	' $(DOC_MODULE).hierarchy; then \
	    echo "$(DOC_MODULE).hierarchy contains tabs, please fix"; \
	    /bin/false; \
	fi

check: check-hierarchy
endif

# wildcard is apparently not portable to other makes, hence the use of find
inspect_files = $(shell find $(srcdir)/$(INSPECT_DIR) -name '*.xml')

check-inspected-versions:
	@echo Checking plugin versions of inspected plugin data ...; \
	fail=0 ; \
	for each in $(inspect_files) ; do \
	  if (grep -H '<version>' $$each | grep -v '<version>$(VERSION)'); then \
	    echo $$each should be fixed to say version $(VERSION) or be removed ; \
	    echo "sed -i -e 's/<version.*version>/<version>$(VERSION)<\/version>/'" $$each; \
	    echo ; \
	    fail=1; \
	  fi ; \
	done ; \
	exit $$fail

check-outdated-docs:
	$(AM_V_GEN)echo Checking for outdated plugin inspect data ...; \
	fail=0 ; \
	if [ -d $(top_srcdir)/.git/ ]; then \
	  files=`find $(srcdir)/inspect/ -name '*xml'`; \
	  for f in $$files; do \
	    ver=`grep '<version>$(PACKAGE_VERSION)</version>' $$f`; \
	    if test "x$$ver" = "x"; then \
	      plugin=`echo $$f | sed -e 's/^.*plugin-//' -e 's/.xml//'`; \
	      # echo "Checking $$plugin $$f"; \
	      pushd "$(top_srcdir)" >/dev/null; \
	      pinit=`git grep -A3 GST_PLUGIN_DEFINE -- ext/ gst/ sys/ | grep "\"$$plugin\""`; \
	      popd >/dev/null; \
	      # echo "[$$pinit]"; \
	      if test "x$$pinit" = "x"; then \
	        printf " **** outdated docs for plugin %-15s: %s\n" $$plugin $$f; \
	        fail=1; \
	      fi; \
	    fi; \
	  done; \
	fi ; \
	exit $$fail

#
# Require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtkdoc:
else
dist-check-gtkdoc:
	@echo "*** gtk-doc must be installed and enabled in order to make dist"
	@false
endif

# FIXME: decide whether we want to dist generated html or not
# also this only works, if the project has been build before
# we could dist html only if its there, but that might lead to missing html in
# tarballs
dist-hook: dist-check-gtkdoc dist-hook-local
	mkdir $(distdir)/html
	cp html/* $(distdir)/html
	-cp $(srcdir)/$(DOC_MODULE).types $(distdir)/
	-cp $(srcdir)/$(DOC_MODULE)-sections.txt $(distdir)/
	cd $(distdir) && rm -f $(DISTCLEANFILES)
	-gtkdoc-rebase --online --relative --html-dir=$(distdir)/html

.PHONY : dist-hook-local docs check-outdated-docs inspect

# avoid spurious build errors when distchecking with -jN
.NOTPARALLEL:
