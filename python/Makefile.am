
EXTRA_DIST = Golden.c setup.py entryIterator.py

all-local:
	@if test "$(srcdir)" = "."; then \
	  $(PYTHON) $(srcdir)/setup.py build; fi
check-local:
	@if test "$(srcdir)" = "."; then \
	  $(PYTHON) $(srcdir)/setup.py check; fi
clean-local:
	@if test "$(srcdir)" = "."; then \
	  $(PYTHON) $(srcdir)/setup.py clean -a; fi
install-exec-local:
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
   $(PYTHON) $(srcdir)/setup.py install; \
 elif [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
   $(PYTHON) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi
