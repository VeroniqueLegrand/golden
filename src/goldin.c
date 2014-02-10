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
#include <errno.h>
#include "list.h"
#include "locus.h"
#include <err.h>


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#define BUFINC 100

// #define PERF_PROFILE

#ifdef PERF_PROFILE
#include <time.h>
#endif

/* Functions prototypes */
static void usage(int);
all_indix_t create_index(char *,int,int );

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

  clock_t cpu_time_start;
  time_t wall_time_start;

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
    all_indix_t file_l_indix=create_index(file,loc,acc);

#ifdef PERF_PROFILE
    clock_t cpu_time_stop=clock();
    time_t wall_time_stop=time(NULL);
    
    // compute time spent reading flat files
    clock_t cpu_time_read_flat=cpu_time_stop-cpu_time_start;
    time_t wall_time_read_flat=wall_time_stop-wall_time_start;
    
    printf("processor time spent reading flat file %s : %lu clock ticks\n",file,(unsigned long)	cpu_time_read_flat);
    printf("wall time spent reading flat files %s : %ld seconds \n",file,(long)	wall_time_read_flat);
#endif
    
    if (wrn && (locnb + accnb) == 0) {
      error_warn(file, "file contains no entries");
      continue; }

#ifdef PERF_PROFILE
    cpu_time_start=clock();
    wall_time_start=time(NULL);
#endif
    /* Merge indexes */
    if (loc) {
      if (locus_merge(dbase, locnb, file_l_indix.l_locind))
        error_fatal(dbase, "entry names indexes failed"); }
    if (acc) {
      if (access_merge(dbase, accnb, file_l_indix.l_accind))
        error_fatal(dbase, "accession numbers indexes failed"); }
#ifdef PERF_PROFILE
    cpu_time_stop=clock();
    wall_time_stop=time(NULL);
    
    // compute time spent reading flat files
    clock_t cpu_time_merge_index=cpu_time_stop-cpu_time_start;
    time_t wall_time_merge_index=wall_time_stop-wall_time_start;
    
    printf("processor time spent merging indexes : %lu clock ticks\n",(unsigned long)	cpu_time_merge_index);
    printf("wall time spent merging indexes: %ld seconds\n",(long)	wall_time_merge_index);
#endif
    freeAllIndix(file_l_indix);
  }
  // free(accind); free(locind);

  return EXIT_SUCCESS;
}

/*
 * First step : create indexes for given file.
 */
all_indix_t create_index(char * file, int loc, int acc) {
  FILE *f;
  char *p;
  long locnb, accnb,indnb;
  entry_t ent;
  size_t len;
  indix_t *cur;
  int nb;
  all_indix_t fic_indix;

  locnb = accnb = 0;
  indnb = 0;
  fic_indix.l_locind=NULL;
  fic_indix.l_accind=NULL;
  fic_indix.flatfile_name=strdup(file);
#ifdef PERF_PROFILE
  clock_t cpu_time_start=clock();
  time_t wall_time_start=time(NULL);
  srand(wall_time_start);
#endif
  if ((f = fopen(file, "r")) == NULL)
	  err(errno,"cannot open file: %s.",file);
  while(entry_parse(f, &ent) != 1) {
    /* Checks for reallocation */
    if (locnb >= indnb || accnb >= indnb) {
      indnb += BUFINC; len = (size_t)indnb * sizeof(indix_t);
      if ((fic_indix.l_locind = (indix_t *)realloc(fic_indix.l_locind, len)) == NULL ||
          (fic_indix.l_accind = (indix_t *)realloc(fic_indix.l_accind, len)) == NULL)
    	err(errno,"cannot reallocate memory");
    }
    /* Store entry name & accession number indexes */
    if (loc && ent.locus[0] != '\0') {
      cur = fic_indix.l_locind + locnb; locnb++;
      (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
      (void)strncpy(cur->name, ent.locus, (size_t)NAMLEN);
      p = cur->name;
      while (*p) { *p = toupper((unsigned char)*p); p++;
      }
      cur->filenb = nb; cur->offset = ent.offset;
    }
    if (acc && ent.access[0] != '\0') {
      cur = fic_indix.l_accind + accnb; accnb++;
      (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
      (void)strncpy(cur->name, ent.access, (size_t)NAMLEN);
      p = cur->name;
      while (*p) { *p = toupper((unsigned char)*p); p++;
      }
      cur->filenb = nb; cur->offset = ent.offset;
    }
    
#ifdef PERF_PROFILE
    // for the needs of performance testing, modify cur->name so that index file grows bigger and bigger.
    sprintf(cur->name,"%d",rand());
#endif
  }
  if (fclose(f) == EOF)
    err(errno,"cannot close file: %s.",file);
	  // error_fatal(file, NULL);

  return fic_indix;
}


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
