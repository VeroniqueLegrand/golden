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

#include <goldin_options.h>

#define PERF_PROFILE

#ifdef PERF_PROFILE
#include <time.h>
#endif

#include <fcntl.h>

/* Global variables */
// static char *prog;


void process_databank_files(int,int,char ** ,goldin_parms);
all_indix_nb process_databank_file(goldin_parms, char * );
void all_index_sort(goldin_parms,all_indix_nb);

/* Main function */
int main(int argc, char **argv) {
  FILE *f;
  int i;
  char *p, *dbase, *file;
  entry_t ent;
  long locnb, accnb, indnb;
  indix_t *cur, *locind, *accind;
  size_t len;
  goldin_parms s_parms;
  //char * lst_merge_fic=NULL;
  /* Inits */
  // prog = basename(*argv);

  /* Checks command line options & arguments */
  init_goldin_parms(&s_parms,argc, argv);
  /* Proceed all input files */
  indnb = 0; locind = accind = NULL;

  if (!s_parms.idx_input_flag) {
    process_databank_files(optind,argc,argv,s_parms);
  } else {/* I am expecting index files as input */
    process_index_files(optind,argc,argv,s_parms);
  };

    
  return EXIT_SUCCESS;
}

void process_databank_files(int optind,int argc,char ** argv,goldin_parms s_parms) {
  int i;
  char* file;
  all_indix_nb tot_idx;
  for(i = optind + 1; i < argc; i++) {
     file = argv[i];
     tot_idx=process_databank_file(s_parms,file);
  }
  if (s_parms.csort_flag) { // sort index file.
    all_index_sort(s_parms,tot_idx);
  }
}

void all_index_sort(goldin_parms s_parms,all_indix_nb tot_idx) {
  char *file;
  if (s_parms.loc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, LOCSUF);
    index_sort(file,tot_idx.locnb);
  }
  if (s_parms.acc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, ACCSUF);
    index_sort(file,tot_idx.accnb);
  }
}

all_indix_nb process_databank_file(goldin_parms s_parms , char * file) {
  struct stat st;
  int nb;
  int ret;
  all_indix_nb tot_idx;
  /* Check for regular file */
  if (stat(file, &st) == -1) err(errno,file, NULL);
  if ((st.st_mode & S_IFMT) != S_IFREG) err(errno, file, "not a regular file");
      
#ifdef PERF_PROFILE
  clock_t cpu_time_start=clock();
  time_t wall_time_start=time(NULL);
  srand(wall_time_start);
#endif

  /* Add file to list */
  nb = list_append(s_parms.dbase, s_parms.dir, file,s_parms.new_index_dir);
  all_indix_t file_l_indix=create_index(file,nb,s_parms.loc,s_parms.acc);
  tot_idx.accnb=file_l_indix.accnb;
  tot_idx.locnb=file_l_indix.accnb;
  if (s_parms.wrn && (file_l_indix.locnb + file_l_indix.accnb) == 0) {
     warn("%s %s",file, "file contains no entries");
     return tot_idx;
  }
  if (!s_parms.csort_flag && !s_parms.co_flag) { // same behavior as in previous versions.
    index_merge_with_existing(file_l_indix,s_parms.dbase,s_parms.loc,s_parms.acc);
    freeAllIndix(file_l_indix);
    return tot_idx;
  }
  // concatenate index files
  tot_idx=index_concat_with_existing(file_l_indix,s_parms.dbase,s_parms.loc,s_parms.acc);
  freeAllIndix(file_l_indix);
  return tot_idx;
}

void process_index_files(int optind,int argc,char ** argv,goldin_parms s_parms) {
  struct stat st;
  int i,fd,nb_read,nb;
  char* rac_file;
  all_indix_nb tot_idx;
  char * d_dbx_file, *d_acx_file, *d_icx_file;
  char * s_dbx_file, *s_acx_file, *s_icx_file;

  d_dbx_file=index_file(s_parms.new_index_dir,s_parms.dbase,LSTSUF);
  if (s_parms.acc) d_acx_file=index_file(s_parms.new_index_dir,s_parms.dbase,ACCSUF);
  if (s_parms.loc) d_icx_file=index_file(s_parms.new_index_dir,s_parms.dbase,LOCSUF);


  for(i = optind + 1; i < argc; i++) {
     rac_file = argv[i];
     s_dbx_file=index_file(s_parms.new_index_dir,rac_file,LSTSUF);
     if (stat(s_dbx_file, &st) == -1) err(errno,s_dbx_file, NULL);
     int len=st.st_size;
     char * lst_to_concat= malloc(len+1);
     if ((fd=open(s_dbx_file,O_RDONLY))==-1) err(errno, "Cannot open source file.");
     if ((nb_read=read(fd,lst_to_concat,st.st_size))==-1) err(errno,"Error while reading source file.");
     close(fd);
     nb = list_append(s_parms.dbase, s_parms.dir,lst_to_concat,s_parms.new_index_dir);


     if (s_parms.acc) s_acx_file=index_file(s_parms.new_index_dir,rac_file,ACCSUF);
     if (s_parms.loc) s_icx_file=index_file(s_parms.new_index_dir,rac_file,LOCSUF);

  }
  if (s_parms.csort_flag) { // sort index file.
    all_index_sort(s_parms,tot_idx);
  }
}




