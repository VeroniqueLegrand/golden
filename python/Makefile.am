
EXTRA_DIST = Golden.c setup.py entryIterator.py

all-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_4)" != "/bin/false" ]; then \
	  $(PYTHON3_4) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_5)" != "/bin/false" ]; then \
	  $(PYTHON3_5) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_7)" != "/bin/false" ]; then \
	  $(PYTHON3_7) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_8)" != "/bin/false" ]; then \
	  $(PYTHON3_8) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_9)" != "/bin/false" ]; then \
		$(PYTHON3_9) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_10)" != "/bin/false" ]; then \
		$(PYTHON3_10) $(srcdir)/setup.py build; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_11)" != "/bin/false" ]; then \
		$(PYTHON3_11) $(srcdir)/setup.py build; fi
check-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_4)" != "/bin/false" ]; then \
	  $(PYTHON3_4) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_5)" != "/bin/false" ]; then \
	  $(PYTHON3_5) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_7)" != "/bin/false" ]; then \
	  $(PYTHON3_7) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_8)" != "/bin/false" ]; then \
	  $(PYTHON3_8) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_9)" != "/bin/false" ]; then \
	  $(PYTHON3_9) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_10)" != "/bin/false" ]; then \
	  $(PYTHON3_10) $(srcdir)/setup.py check; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_11)" != "/bin/false" ]; then \
	  $(PYTHON3_11) $(srcdir)/setup.py check; fi

clean-local:
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON2_7)" != "/bin/false" ]; then \
	  $(PYTHON2_7) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_4)" != "/bin/false" ]; then \
	  $(PYTHON3_4) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_5)" != "/bin/false" ]; then \
	  $(PYTHON3_5) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_7)" != "/bin/false" ]; then \
	  $(PYTHON3_7) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_8)" != "/bin/false" ]; then \
	  $(PYTHON3_8) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_9)" != "/bin/false" ]; then \
	  $(PYTHON3_9) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_10)" != "/bin/false" ]; then \
	  $(PYTHON3_10) $(srcdir)/setup.py clean -a; fi
	@if [ "$(srcdir)" = "." ] && [ "$(PYTHON3_11)" != "/bin/false" ]; then \
	  $(PYTHON3_11) $(srcdir)/setup.py clean -a; fi

install-exec-local:
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON2_7)" != "/bin/false" ]; then $(PYTHON2_7) $(srcdir)/setup.py install; fi; fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON3_4)" != "/bin/false" ]; then $(PYTHON3_4) $(srcdir)/setup.py install; fi; fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON3_5)" != "/bin/false" ]; then $(PYTHON3_5) $(srcdir)/setup.py install; fi; fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" = "" ]; then \
		if [ "$(PYTHON3_6)" != "/bin/false" ]; then $(PYTHON3_6) $(srcdir)/setup.py install; fi; fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON2_7)" != "/bin/false" ]; then $(PYTHON2_7) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_4)" != "/bin/false" ]; then $(PYTHON3_4) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_5)" != "/bin/false" ]; then $(PYTHON3_5) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_7)" != "/bin/false" ]; then $(PYTHON3_7) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_8)" != "/bin/false" ]; then $(PYTHON3_8) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_9)" != "/bin/false" ]; then $(PYTHON3_9) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_10)" != "/bin/false" ]; then $(PYTHON3_10) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
	if [ "$(srcdir)" = "." ] && [ "$(dir_for_binding_install)" != "" ]; then \
		if [ "$(PYTHON3_11)" != "/bin/false" ]; then $(PYTHON3_11) $(srcdir)/setup.py install --prefix $(dir_for_binding_install); fi;fi
