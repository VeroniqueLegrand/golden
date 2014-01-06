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
ArrayOfResAddr locus_search(result_t ** lst_current_db,int lst_size,char *dbase, int * nb_res_found) {
  FILE *f;
  char *p, *file, buf[1024];
  *nb_res_found=0;
  int fgetCalls=0; // need to count that for memory allocation.
  ArrayOfResAddr in_lst_notFound;


  /* Virtual database indexes */
  file = index_file(NULL, dbase, VIRSUF);
#ifdef DEBUG
  printf("Searching in file : %s \n",file);
#endif
   if (access(file, F_OK) != -1) {
    if ((f = fopen(file, "r")) == NULL) {
      error_fatal("memory", NULL); }
    while (fgets(buf, 1023, f) != NULL) {
      if ((p = strrchr(buf, '\n')) != NULL) { *p = '\0'; }
      in_lst_notFound= locus_search(lst_current_db, lst_size,buf,  nb_res_found);
      if (in_lst_notFound.arrSize!=0) {
    	  if (fgetCalls>=1) {
    	      free(lst_current_db); // have to free that since it points to memory allocated by index_search during the previous call to locus_search (for the previous file in the virtual database indexes).
    	  }
    	  lst_current_db=in_lst_notFound.addrArray; // search only for results that were not found previously.
    	  lst_size=in_lst_notFound.arrSize;
    	  fgetCalls++;
    	  continue;
      }
      break; }
    if (fgetCalls>1 && in_lst_notFound.arrSize==0) { free(lst_current_db);} // else lst_current_db is lstGoldenQuery's lst_work; and it is free in main. Or, we need it to log info about what was not found.
    if (fclose(f) == EOF) {
      error_fatal("memory", NULL); }
    free(file);
    return in_lst_notFound;
   }


  /* Real database indexes */
  file = index_file(NULL, dbase, LOCSUF);
#ifdef DEBUG
  printf("Searching in file : %s \n",file);
#endif
  in_lst_notFound.addrArray=malloc(sizeof(result_t *));
  in_lst_notFound.arrSize=0;
  in_lst_notFound.arrSize=index_search(file, dbase, lst_current_db,lst_size, &in_lst_notFound.addrArray, nb_res_found);
  free(file);
  return in_lst_notFound; }


/* Merge locus indexes with existing file */
int locus_merge(char *dbase, long nb, indix_t *ind) {
  int i;
  char *file;

  file = index_file(".", dbase, LOCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }
