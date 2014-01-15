
SUFFIXES = .pod .man

man1_MANS = golden.man goldin.man

EXTRA_DIST = $(man1_MANS) golden.pod goldin.pod

PODARGS = --center='User Manuals' --release='Unix'

## Extra rules
.pod.man:
	$(POD2MAN) $(PODARGS) $< > $@ && touch $@

## Maintainer parano check
parano:
	(cd $(srcdir) && for p in *.pod; do podchecker $$p; done)