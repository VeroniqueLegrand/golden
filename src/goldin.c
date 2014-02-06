/* goldin.c - Golden indexer main functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <libgen.h>

#include "access.h"
#include "entry.h"
#include "error.h"
#include "list.h"
#include "locus.h"


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#define BUFINC 100

/* Functions prototypes */
static void usage(int);

/* Global variables */
static char *prog;


/* Main function */
int main(int argc, char **argv) {
  FILE *f;
  int i, nb, loc, acc, wrn;
  char *p, *dbase, *file, *dir;
  entry_t ent;
  long locnb, accnb, indnb;
  indix_t *cur, *locind, *accind;
  size_t len;
  struct stat st;

  /* Inits */
  prog = basename(*argv);

  /* Checks command line options & arguments */
  i = loc = acc = 0; wrn = 1; dir = NULL;
  while((i = getopt(argc, argv, "ad:hiq")) != -1) {
    switch(i) {
    case 'a':
      acc = 1; break;
    case 'd':
      dir = optarg; break;
    case 'h':
      usage(EXIT_SUCCESS); break;
    case 'i':
      loc = 1; break;
    case 'q':
      wrn = 0; break;
    default:
      usage(EXIT_FAILURE); break; }
  }
  if ((loc + acc) == 0) { loc = acc = 1; }
  if (argc - optind < 2) usage(EXIT_FAILURE);

  /* Proceed all input files */
  dbase = argv[optind]; indnb = 0; locind = accind = NULL;
  if (dir == NULL) { dir = dbase; }
  for(i = optind + 1; i < argc; i++) {
    file = argv[i];

    /* Check for regular file */
    if (stat(file, &st) == -1) {
      error_fatal(file, NULL); }
    if ((st.st_mode & S_IFMT) != S_IFREG) {
      error_fatal(file, "not a regular file"); }

    /* Add file to list */
    nb = list_append(dbase, dir, file);

    /* Proceed all flat file entries */
    locnb = accnb = 0;
    if ((f = fopen(file, "r")) == NULL)
      error_fatal(file, NULL);
    while(entry_parse(f, &ent) != 1) {
      /* Checks for reallocation */
      if (locnb >= indnb || accnb >= indnb) {
        indnb += BUFINC; len = (size_t)indnb * sizeof(indix_t);
        if ((locind = (indix_t *)realloc(locind, len)) == NULL ||
            (accind = (indix_t *)realloc(accind, len)) == NULL)
          error_fatal("memory", NULL);
      }
      /* Store entry name & accession number indexes */
      if (loc && ent.locus[0] != '\0') {
        cur = locind + locnb; locnb++;
        (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
        (void)strncpy(cur->name, ent.locus, (size_t)NAMLEN);
        p = cur->name;
        while (*p) { *p = toupper((unsigned char)*p); p++;
        }
        cur->filenb = nb; cur->offset = ent.offset;
      }
      if (acc && ent.access[0] != '\0') {
        cur = accind + accnb; accnb++;
        (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
        (void)strncpy(cur->name, ent.access, (size_t)NAMLEN);
        p = cur->name;
        while (*p) { *p = toupper((unsigned char)*p); p++;
        }
        cur->filenb = nb; cur->offset = ent.offset;
      }
    }
    if (fclose(f) == EOF)
      error_fatal(file, NULL);

    if (wrn && (locnb + accnb) == 0) {
      error_warn(file, "file contains no entries");
      continue; }

    /* Merge indexes */
    if (loc) {
      if (locus_merge(dbase, locnb, locind))
        error_fatal(dbase, "entry names indexes failed"); }
    if (acc) {
      if (access_merge(dbase, accnb, accind))
        error_fatal(dbase, "accession numbers indexes failed"); }
    }

  free(accind); free(locind);

  return EXIT_SUCCESS; }


/* Usage display */
static void usage(int status) {
  FILE *f = stdout;

  (void)fprintf(f, "usage: %s [options] <dbase> <file> ...\n\n", prog);
  (void)fprintf(f, "options:\n");
  (void)fprintf(f, "  -a       ... Make accession numbers indexes.\n");
  (void)fprintf(f, "  -d <dir> ... Use alternate data <dir>.\n");
  (void)fprintf(f, "  -h       ... Prints this message and exit.\n");
  (void)fprintf(f, "  -i       ... Make entry names indexes.\n");
  (void)fprintf(f, "  -q       ... Be quiet, do not display some warnings.\n");

  exit(status); }
