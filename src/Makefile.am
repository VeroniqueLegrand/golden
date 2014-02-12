
LINTDEFS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)

AM_CPPFLAGS = -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS = golden goldin unit_tests
noinst_LIBRARIES = libgolden.a
noinst_HEADERS = $(HDR)
pkgdata_DATA =

golden_SOURCES = golden.c
golden_LDADD = libgolden.a

goldin_SOURCES = goldin.c
goldin_LDADD = libgolden.a

unit_tests_SOURCES = unit_tests.c

libgolden_a_SOURCES = $(SRC)
SRC = access.c entry.c error.c index.c list.c locus.c query.c
HDR = access.h entry.h error.h index.h list.h locus.h query.h

TESTS = unit_tests.sh

## Maintainer parano check
parano:
	$(LINT) $(LINTFLAGS) $(LINTDEFS) $(golden_SOURCES) \
	  $(libgolden_a_SOURCES)
	$(LINT) $(LINTFLAGS) $(LINTDEFS) $(goldin_SOURCES) \
	  $(libgolden_a_SOURCES)

