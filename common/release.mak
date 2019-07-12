# include this snippet to add a common release: target by using
# include $(top_srcdir)/common/release.mak

release: dist
	@$(MAKE) $(PACKAGE)-$(VERSION).tar.xz.sha256sum
	@echo
	@echo "================================================================================================="
	@echo "http://gstreamer.freedesktop.org/src/$(PACKAGE)/$(PACKAGE)-$(VERSION).tar.xz"
	@cat $(PACKAGE)-$(VERSION).tar.xz.sha256sum
	@echo "================================================================================================="
	@if [ -d ~/releases/ ]; then \
	  cp -v $(PACKAGE)-$(VERSION).tar.xz ~/releases/; \
	fi
	@if [ -d ../www/data/src ]; then \
	  mv -v $(PACKAGE)-$(VERSION).tar.xz ../www/data/src/$(PACKAGE)/ ; \
	  mv -v $(PACKAGE)-$(VERSION).tar.xz.sha256sum ../www/data/src/$(PACKAGE)/ ; \
	fi
	@echo "================================================================================================="

# generate sha256 sum files
%.sha256sum: %
	@sha256sum $< > $@

# check that no marshal or enumtypes files are included
# this in turn ensures that distcheck fails for missing .list files which is currently
# shadowed when the corresponding .c and .h files are included.
distcheck-hook:
	@test "x" = "x`find $(distdir) -name \*-enumtypes.[ch] | grep -v win32`" && \
	test "x" = "x`find $(distdir) -name \*-marshal.[ch]`" || \
	( echo "*** Leftover enumtypes or marshal files in the tarball." && \
	  echo "*** Make sure the following files are not disted:" && \
	  find $(distdir) -name \*-enumtypes.[ch] | grep -v win32 && \
	  find $(distdir) -name \*-marshal.[ch] && \
	  false )
