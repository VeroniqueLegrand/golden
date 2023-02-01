/* locus.c - Entry names functions */
/*
Copyright (C) 2001-2023  Institut Pasteur

  This program is part of the golden software.

  This program  is free software:  you can  redistribute it  and/or modify it  under the terms  of the GNU
  General Public License as published by the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the  GNU General Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

  Contact:

   Veronique Legrand                                                           veronique.legrand@pasteur.fr

 */
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
#ifdef DEBUG
  result_t ** start_l=wData.start_l; // for printing debug info
#endif
  
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
int locus_merge(char *dbase, uint64_t nb, indix_t *ind, char * new_index_dir) {
  int i;
  char *file;
  // char * idx_dir=index_dir();
  file = index_file(new_index_dir, dbase, LOCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }

int locus_concat(char *dbase, uint64_t nb, indix_t *ind, char * new_index_dir) {
  int i=IDX_ERR;
  char *file;

  file = index_file(new_index_dir, dbase, LOCSUF);
  i = index_concat(file, nb, ind);
  free(file);
  return i;
}
