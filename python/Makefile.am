
EXTRA_DIST = Golden.c setup.py entryIterator.py

all-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3)" != "/bin/false" ]; then \
	  $(PYTHON3) $(srcdir)/setup.py build; fi
check-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3)" != "/bin/false" ]; then \
	  $(PYTHON3) $(srcdir)/setup.py check; fi
clean-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3)" != "/bin/false" ]; then \
	  $(PYTHON3) $(srcdir)/setup.py clean -a; fi
install-exec-local:
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON2_7)" != "/bin/false" ]; then $(PYTHON2_7) $(srcdir)/setup.py install; fi; fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON3)" != "/bin/false" ]; then $(PYTHON3) $(srcdir)/setup.py install; fi; fi
	        
