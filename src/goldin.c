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
#include <getopt.h>


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif



#define PERF_PROFILE

#ifdef PERF_PROFILE
#include <time.h>
#endif

/* Functions prototypes */
static void usage(int);


/* Global variables */
static char *prog;
static int dump_flag;
static int sort_flag;
static int merge_flag;

static struct option long_options[] =
{
  /* These options set a flag. */
  {"dump", no_argument,       &dump_flag, 1}, // dump indexes right after their creation; no merge; no sort.
  {"sort",   no_argument,       &sort_flag, 1}, // sort indexes.
  {"merge",  no_argument, &merge_flag, 1}, // merges all index files given in argument into a new one (its dbase is given in argument). if --sort is not specified, files are simply concatenated.
  /* These options don't set a flag.
   We distinguish them by their indices. */
  {"index_dir",  required_argument, 0, 'b'} // indicates place where to put index files. default value is "."
  
  /*{"add",     no_argument,       0, 'a'},*/
  };


/*
 Depending on the option flags, arguments do not have the same signification.
 If --merge is specified, <file> is in fact 'dbase' for existing index files to merge and
 <dbase> is as usual for new index file.
 */
void processArgs() {
  
}


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
  char * new_index_dir=".";
  char * lst_merge_fic=NULL;
  
#ifdef PERF_PROFILE
  clock_t cpu_time_start, cpu_time_stop;
  time_t wall_time_start, wall_time_stop;
#endif
  /* Inits */
  prog = basename(*argv);
  int option_index = 0;
  /* Checks command line options & arguments */
  i = loc = acc = 0; wrn = 1; dir = NULL;
  while((i = getopt_long(argc, argv, "ad:hiq",long_options,&option_index)) != -1) {
    switch(i) {
    case 0 :
        printf("Option: %s \n",long_options[option_index].name);
      break;
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
    /* long options */
    case 'b':
      new_index_dir=optarg;break;
    default:
      usage(EXIT_FAILURE); break; }
  }
  if ((loc + acc) == 0) { loc = acc = 1; }
  if (dump_flag && merge_flag) usage(EXIT_FAILURE);
  if ((sort_flag && !dump_flag) && (sort_flag && !merge_flag)) usage(EXIT_FAILURE);
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
    
#ifdef PERF_PROFILE
    cpu_time_start=clock();
    wall_time_start=time(NULL);
    srand(wall_time_start);
#endif

    /* Add file to list */
    nb = list_append(dbase, dir, file,new_index_dir);
    all_indix_t file_l_indix=create_index(file,nb,loc,acc);

    if (sort_flag) {
      qsort(file_l_indix.l_locind, (size_t) file_l_indix.locnb, sizeof(*file_l_indix.l_locind), index_compare);
      qsort(file_l_indix.l_accind, (size_t) file_l_indix.accnb, sizeof(*file_l_indix.l_accind), index_compare);
    }
    if (dump_flag) {
      int ret=index_dump(dbase,REPLACE_INDEXES,file_l_indix,LOCSUF,new_index_dir);
      ret=index_dump(dbase,REPLACE_INDEXES,file_l_indix,ACCSUF,new_index_dir);
#ifdef PERF_PROFILE
      cpu_time_stop=clock();
      wall_time_stop=time(NULL);
      
      // compute time spent reading flat files
      clock_t cpu_time_read_flat=cpu_time_stop-cpu_time_start;
      time_t wall_time_read_flat=wall_time_stop-wall_time_start;
      
      printf("processor time spent in creating indexes for flat file %s : %lu clock ticks\n",file,(unsigned long)	cpu_time_read_flat);
      printf("wall time spent in creating indexes for flat files %s : %ld seconds \n",file,(long)	wall_time_read_flat);
#endif
      // if (strcmp(new_index_dir,".")!=0) free(new_index_dir);
      return EXIT_SUCCESS;
    }

    
    if (wrn && (file_l_indix.locnb + file_l_indix.accnb) == 0) {
      error_warn(file, "file contains no entries");
      continue; }

#ifdef PERF_PROFILE
    cpu_time_start=clock();
    wall_time_start=time(NULL);
#endif
    /* Merge indexes */
    if (loc) {
      if (locus_merge(dbase, file_l_indix.locnb, file_l_indix.l_locind))
        error_fatal(dbase, "entry names indexes failed"); }
    if (acc) {
      if (access_merge(dbase, file_l_indix.accnb, file_l_indix.l_accind))
        error_fatal(dbase, "accession numbers indexes failed"); }
#ifdef PERF_PROFILE
    cpu_time_stop=clock();
    wall_time_stop=time(NULL);
    
    // compute time spent merging files
    clock_t cpu_time_merge_index=cpu_time_stop-cpu_time_start;
    time_t wall_time_merge_index=wall_time_stop-wall_time_start;
    
    printf("processor time spent merging indexes : %lu clock ticks\n",(unsigned long)	cpu_time_merge_index);
    printf("wall time spent merging indexes: %ld seconds\n",(long)	wall_time_merge_index);
#endif
    freeAllIndix(file_l_indix);
  }
  // if (strcmp(new_index_dir,".")!=0) free(new_index_dir);
  return EXIT_SUCCESS;
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
  (void)fprintf(f, "  --dump   ... Dump indexes without sorting nor merging. \n");
  (void)fprintf(f, "  --merge  ... merge indexe whose base name are given in argument; result is new index files (acx and/or idx). \n");
  (void)fprintf(f, "  --sort   ... sort indexes. \n");
  exit(status); }
