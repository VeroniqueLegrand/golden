
LINTDEFS = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)

AM_CPPFLAGS = -DDATADIR=\"$(pkgdatadir)\"

bin_PROGRAMS = golden goldin unit_tests perf_tests
noinst_LIBRARIES = libgolden.a
noinst_HEADERS = $(HDR)
pkgdata_DATA =

golden_SOURCES = golden.c
golden_LDADD = libgolden.a

goldin_SOURCES = goldin.c
goldin_LDADD = libgolden.a

unit_tests_SOURCES = unit_tests.c
unit_tests_LDADD = libgolden.a

perf_tests_SOURCES = perf_tests.c
perf_tests_LDADD = libgolden.a

libgolden_a_SOURCES = $(SRC)
SRC = access.c entry.c error.c index.c index_hl.c index_desc.c list.c locus.c query.c goldin_options.c goldin_utils.c
HDR = access.h entry.h error.h index.h index_hl.h index_desc.h list.h locus.h query.h goldin_options.h goldin_utils.h

## TESTS = unit_tests.sh ## for a later use.

## Maintainer parano check
parano:
	$(LINT) $(LINTFLAGS) $(LINTDEFS) $(golden_SOURCES) \
	  $(libgolden_a_SOURCES)
	$(LINT) $(LINTFLAGS) $(LINTDEFS) $(goldin_SOURCES) \
	  $(libgolden_a_SOURCES)

