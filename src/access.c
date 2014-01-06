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



/* Search in accession indexes */
/* VL: keep db_name parameter for recursive calls when searching in virtual database indexes.
 * lst_notFound points to a memory area allocated by the caller to store adresses of result_t structures for which no match was found.
 * It is used and "free" by the caller.
 * For recursive calls to access_search and calls to index_search, use memory areas allocated by access_search.
 */
ArrayOfResAddr access_search(result_t ** lst_current_db, int lst_size, char * db_name, int * nb_res_found) {
  FILE *f;
  char *p, *file, buf[1024];
  int fgetCalls=0; // need to count that for memory allocation.
  ArrayOfResAddr in_lst_notFound;



  // cur_base may be virtual.
  /* Virtual database indexes */
  file = index_file(NULL, db_name, VIRSUF);
  if (access(file, F_OK) != -1) {
    if ((f = fopen(file, "r")) == NULL) {
      error_fatal("memory", NULL); }

    while (fgets(buf, 1023, f) != NULL) {
      if ((p = strrchr(buf, '\n')) != NULL) { *p = '\0'; }
      in_lst_notFound=access_search(lst_current_db, lst_size, buf, nb_res_found);
#ifdef DEBUG
      // dump in_lst_notFound
      int i;
      result_t * cur_res;
      printf("Still looking for : ");
      for (i=0; i<in_lst_notFound.arrSize; i++) {
    	  cur_res=in_lst_notFound.addrArray[i];
    	  printf("%s ",cur_res->name);
      }
      printf("\n");
#endif
      if (in_lst_notFound.arrSize!=0) {
#ifdef DEBUG
    	  printf("access_search : all results were not found ; continue \n");
#endif
    	  if (fgetCalls>=1) {
    		  free(lst_current_db); // have to free that since it points to memory allocated by index_search during the previous call to access_search (for the previous file in the virtual database indexes).
    	  }
    	  lst_current_db=in_lst_notFound.addrArray; // search only for results that were not found previously.
    	  lst_size=in_lst_notFound.arrSize;
    	  fgetCalls++;
    	  continue;
      }
      break; }
    if (fgetCalls>1 && in_lst_notFound.arrSize==0) { free(lst_current_db);} // if fgetCalls=1 or 0,lst_current_db is lstGoldenQuery's lst_work; and it is free in main.
                                                                            // if in_lst_notFound.arrSize!=0 : memory may need to be used after when looking for locus. => free it only if all AC were found.
    if (fclose(f) == EOF) {
      error_fatal("memory", NULL); }
    free(file);
    return in_lst_notFound;  }

  /* Real database indexes */
  file = index_file(NULL, db_name, ACCSUF);
#ifdef DEBUG
  //printf("Searching in file : %s \n",file);
#endif
  in_lst_notFound.addrArray=malloc(sizeof(result_t *));
  in_lst_notFound.arrSize=0;
  in_lst_notFound.arrSize=index_search(file, db_name, lst_current_db,lst_size, &in_lst_notFound.addrArray, nb_res_found);
#ifdef DEBUG
  printf("access_search : returned from index_search: nb_not_found_for_db=%d, nb_res_found=%d \n",in_lst_notFound.arrSize,*nb_res_found);
#endif
  // if everything was found, free memory.
  if (in_lst_notFound.arrSize==0) {
	  free(in_lst_notFound.addrArray); // pb here.
	  initArrayOfResAddr(&in_lst_notFound);
  }
  free(file);
  return in_lst_notFound;	}


/* Merge accession indexes */
int access_merge(char *dbase, long nb, indix_t *ind) {
  int i;
  char *file;

  file = index_file(".", dbase, ACCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }
