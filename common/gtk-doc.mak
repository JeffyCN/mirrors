###########################################################################
# Everything below here is generic and you shouldn't need to change it.
###########################################################################
# thomas: except of course that we did

# thomas: copied from glib-2
# We set GPATH here; this gives us semantics for GNU make
# which are more like other make's VPATH, when it comes to
# whether a source that is a target of one rule is then
# searched for in VPATH/GPATH.
#
GPATH = $(srcdir)

# thomas: make docs parallel installable
TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)-@GST_API_VERSION@

EXTRA_DIST = 				\
	$(content_files)		\
	$(extra_files)			\
	$(HTML_IMAGES)			\
	$(DOC_MAIN_SGML_FILE)		\
	$(DOC_MODULE).types		\
	$(DOC_OVERRIDES)		\
	$(DOC_MODULE)-sections.txt

DOC_STAMPS =				\
	setup-build.stamp		\
	scan-build.stamp		\
	sgml-build.stamp		\
	html-build.stamp		\
	sgml.stamp		\
	html.stamp

SCANOBJ_FILES =				\
	$(DOC_MODULE).args		\
	$(DOC_MODULE).hierarchy		\
	$(DOC_MODULE).interfaces		\
	$(DOC_MODULE).prerequisites	\
	$(DOC_MODULE).signals		\
	.libs/$(DOC_MODULE)-scan.o

REPORT_FILES = \
	$(DOC_MODULE)-undocumented.txt \
	$(DOC_MODULE)-undeclared.txt \
	$(DOC_MODULE)-unused.txt

CLEANFILES = $(SCANOBJ_FILES) $(REPORT_FILES) $(DOC_STAMPS) doc-registry.xml

if ENABLE_GTK_DOC
all-local: html-build.stamp

#### setup ####

setup-build.stamp: $(content_files)
	-@if test "$(abs_srcdir)" != "$(abs_builddir)" ; then \
	    echo '  DOC   Preparing build'; \
	    files=`echo $(DOC_MAIN_SGML_FILE) $(DOC_OVERRIDES) $(DOC_MODULE)-sections.txt $(DOC_MODULE).types $(content_files)`; \
	    if test "x$$files" != "x" ; then \
	        for file in $$files ; do \
	            test -f $(abs_srcdir)/$$file && \
	                cp -pu $(abs_srcdir)/$$file $(abs_builddir)/ || true; \
	        done; \
	    fi; \
	fi
	@touch setup-build.stamp

#### scan ####

# in the case of non-srcdir builds, the built gst directory gets added
# to gtk-doc scanning; but only then, to avoid duplicates
scan-build.stamp: $(HFILE_GLOB) $(CFILE_GLOB)
	@echo '  DOC   Scanning header files'
	@_source_dir='' ;						\
	for i in $(DOC_SOURCE_DIR) ; do					\
	    _source_dir="$${_source_dir} --source-dir=$$i" ;	        \
	done ;							        \
	gtkdoc-scan							\
		$(SCAN_OPTIONS) $(EXTRA_HFILES)				\
		--module=$(DOC_MODULE)					\
		$${_source_dir}         				\
		--ignore-headers="$(IGNORE_HFILES)"
	@if grep -l '^..*$$' $(DOC_MODULE).types > /dev/null; then	\
	    echo "  DOC   Introspecting gobjects"; \
	    GST_PLUGIN_SYSTEM_PATH_1_0=`cd $(top_builddir) && pwd`		\
	    GST_PLUGIN_PATH_1_0=						\
	    GST_REGISTRY_1_0=doc-registry.xml				\
	    $(GTKDOC_EXTRA_ENVIRONMENT)					\
	    CC="$(GTKDOC_CC)" LD="$(GTKDOC_LD)"				\
	    CFLAGS="$(GTKDOC_CFLAGS) $(CFLAGS)"				\
	    LDFLAGS="$(GTKDOC_LIBS) $(LDFLAGS)"				\
	    gtkdoc-scangobj --type-init-func="gst_init(NULL,NULL)"	\
	        --module=$(DOC_MODULE) ;				\
	else								\
	    for i in $(SCANOBJ_FILES) ; do				\
	       test -f $$i || touch $$i ;				\
	    done							\
	fi
	@touch scan-build.stamp

$(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_MODULE)-overrides.txt: scan-build.stamp
	@true

#### xml ####

sgml-build.stamp: setup-build.stamp $(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(expand_content_files)
	@echo '  DOC   Building XML'
	@gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)  --expand-content-files="$(expand_content_files)" --main-sgml-file=$(DOC_MAIN_SGML_FILE) --output-format=xml $(MKDB_OPTIONS)
	@cp ../version.entities xml
	@touch sgml-build.stamp

sgml.stamp: sgml-build.stamp
	@true

#### html ####

html-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	@echo '  DOC   Building HTML'
	@rm -rf html
	@mkdir html
	@cp -pr xml html
	@cp ../version.entities ./
	@mkhtml_options=""; \
	gtkdoc-mkhtml 2>&1 --help | grep  >/dev/null "\-\-verbose"; \
	if test "$(?)" = "0"; then \
	  if test "x$(V)" = "x1"; then \
	    mkhtml_options="$$mkhtml_options --verbose"; \
	  fi; \
	fi; \
	@gtkdoc-mkhtml 2>&1 --help | grep  >/dev/null "\-\-path"; \
	if test "$(?)" = "0"; then \
	  mkhtml_options=--path="$(abs_srcdir)"; \
	fi; \
	cd html && gtkdoc-mkhtml $$mkhtml_options $(MKHTML_OPTIONS) $(DOC_MODULE)-@GST_API_VERSION@ ../$(DOC_MAIN_SGML_FILE)
	@rm -rf html/xml
	@rm -f version.entities
	@test "x$(HTML_IMAGES)" = "x" ||  ( cd $(srcdir) && cp $(HTML_IMAGES) $(abs_builddir)/html )
	@echo '  DOC   Fixing cross-references'
	@gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=html --html-dir=$(HTML_DIR) $(FIXXREF_OPTIONS)
	@touch html-build.stamp

clean-local-gtkdoc:
	@rm -rf xml tmpl html
# clean files copied for nonsrcdir templates build
	@if test x"$(srcdir)" != x. ; then \
	        rm -rf $(DOC_MODULE).types; \
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
	    rm -f $(DOC_MAIN_SGML_FILE) ; \
	    rm -f $(DOC_OVERRIDES) ; \
	    rm -f $(DOC_MODULE).types ; \
	    rm -f $(DOC_MODULE).interfaces ; \
	    rm -f $(DOC_MODULE).prerequisites ; \
	    rm -f $(DOC_MODULE)-sections.txt ; \
	    rm -f $(content_files) ; \
	    rm -rf tmpl/*.sgml ; \
	fi
	@rm -rf *.o

maintainer-clean-local: clean
	@cd $(srcdir) && rm -rf html \
		xml $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

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
# Require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtkdoc:
else
dist-check-gtkdoc:
	@echo "*** gtk-doc must be installed and enabled in order to make dist"
	@false
endif

dist-hook: dist-check-gtkdoc dist-hook-local
	mkdir $(distdir)/html
	cp html/* $(distdir)/html
	-cp $(srcdir)/$(DOC_MODULE).types $(distdir)/
	-cp $(srcdir)/$(DOC_MODULE)-sections.txt $(distdir)/
	cd $(distdir) && rm -f $(DISTCLEANFILES)
	-gtkdoc-rebase --online --relative --html-dir=$(distdir)/html

.PHONY : dist-hook-local docs

# avoid spurious build errors when distchecking with -jN
.NOTPARALLEL:
