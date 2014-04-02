/* access.c - Accession numbers functions */

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

#include "access.h"
#include "error.h"
#include "index.h"

//#define DEBUG


/*
 keep that for compatibility with old export python.
 */
result_t * access_search_deprecated(char *dbase, char *name) {
  result_t * res;
  // result_t * adr_res=&res;
  int nb_AC_not_found=1;
  
  if ((res=malloc(sizeof(result_t)))==NULL) error_fatal("memory", NULL);
  
  res->name=strdup(name);
  res->dbase=strdup(dbase);
  res->filenb=NOT_FOUND;
  
  WDBQueryData wData;
  wData.len_l=1;
  wData.start_l=&res;
  
  access_search(wData,dbase,&nb_AC_not_found);
#ifdef DEBUG
  if (res->filenb!=NOT_FOUND) {
    printf("access_search_deprecated, found : \n");
    printf("%s:%s\n,real_dbase; %s\n, filenb=%d\n, offset=%lld\n",res->dbase,res->name,res->real_dbase,res->filenb,res->offset);
  } else {
    printf("access_search_deprecated, didn't find : %s:%s \n",res->dbase,res->name);
  }
#endif
  return res;}



/* Search in accession indexes */
/* VL: keep db_name parameter for recursive calls when searching in virtual database indexes.
 * lst_notFound points to a memory area allocated by the caller to store adresses of result_t structures for which no match was found.
 * It is used and "free" by the caller.
 * For recursive calls to access_search and calls to index_search, use memory areas allocated by access_search.
 */
void access_search(WDBQueryData wData, char * db_name, int * nb_AC_not_found) {
  FILE *f;
  char *p, *file, buf[1024];
#ifdef DEBUG
  result_t ** start_l=wData.start_l; // for printing debug info
#endif
  // cur_base may be virtual.
  /* Virtual database indexes */
  file = index_file(NULL, db_name, VIRSUF);
  //printf("Searching in file : %s \n",file);
  if (access(file, F_OK) != -1) {
    if ((f = fopen(file, "r")) == NULL) {
      error_fatal("memory", NULL); }

    while (fgets(buf, 1023, f) != NULL) {
      if ((p = strrchr(buf, '\n')) != NULL) { *p = '\0'; }
      access_search(wData, buf, nb_AC_not_found);
#ifdef DEBUG
      // dump list of accession numbers that were not found yet.
      print_wrk_struct(start_l,wData.len_l,1);
#endif
      if (*nb_AC_not_found!=0) {
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
    return;  }

  /* Real database indexes */
  file = index_file(NULL, db_name, ACCSUF);
#ifdef DEBUG
  printf("Searching in file : %s \n",file);
#endif
    int nb_AC_found=index_search(file, db_name, wData, nb_AC_not_found);
#ifdef DEBUG
  printf("access_search : returned from index_search DB : %s : nb_not_found_for_db=%d, nb_res_found=%d \n",db_name,*nb_AC_not_found,nb_AC_found);
#endif
  free(file);
  return;	}


/* Merge accession indexes */
int access_merge(char *dbase, long nb, indix_t *ind,char * new_index_dir) {
  int i;
  char *file;

  file = index_file(new_index_dir, dbase, ACCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }

int access_concat(char *dbase, long nb, indix_t *ind, char * new_index_dir) {
  int i=IDX_ERR;
  char *file;

  file = index_file(new_index_dir, dbase, ACCSUF);
  i = index_concat(file, nb, ind);
  free(file);
  return i;
}
