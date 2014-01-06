
EXTRA_DIST = Golden.c setup.py

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
	@if test "$(srcdir)" = "."; then \
	  $(PYTHON) $(srcdir)/setup.py install; fi
