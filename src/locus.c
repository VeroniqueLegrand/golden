/* locus.c - Entry names functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "error.h"
#include "index.h"
#include "locus.h"


/* Search in entry names indexes */
void locus_search(WDBQueryData wData, char * dbase, int * nb_locus_not_found) {
  FILE *f;
  char *p, *file, buf[1024];
  result_t ** start_l=wData.start_l; // for printing debug info
  
  /* Virtual database indexes */
  file = index_file(NULL, dbase, VIRSUF);
  if (access(file, F_OK) != -1) {
    if ((f = fopen(file, "r")) == NULL) {
      error_fatal("memory", NULL); }
    while (fgets(buf, 1023, f) != NULL) {
      if ((p = strrchr(buf, '\n')) != NULL) { *p = '\0'; }
      locus_search(wData,buf, nb_locus_not_found);
#ifdef DEBUG
      // dump list of accession numbers that were not found yet.
      print_wrk_struct(start_l,wData.len_l,1);
#endif
      if (*nb_locus_not_found!=0) {
#ifdef DEBUG
        printf("access_search : all results were not found ; continue \n");
#endif
        continue;
      }
      break;
    }
    if (fclose(f) == EOF) {
      error_fatal("memory", NULL); }
    free(file);
    return;
   }


  /* Real database indexes */
  file = index_file(NULL, dbase, LOCSUF);
#ifdef DEBUG
  printf("Searching in file : %s \n",file);
#endif
  index_search(file, dbase, wData,nb_locus_not_found);
  free(file);
  return ; }


/* Merge locus indexes with existing file */
int locus_merge(char *dbase, long nb, indix_t *ind, char * new_index_dir) {
  int i;
  char *file;
  // char * idx_dir=index_dir();
  file = index_file(new_index_dir, dbase, LOCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }

int locus_concat(char *dbase, long nb, indix_t *ind, char * new_index_dir) {
  int i=IDX_ERR;
  char *file;

  file = index_file(new_index_dir, dbase, LOCSUF);
  i = index_concat(file, nb, ind);
  free(file);
  return i;
}
