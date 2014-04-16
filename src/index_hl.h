/*
 * index_hl.h
 *
 *  Created on: Apr 15, 2014
 *      Author: vlegrand
 *
 *      In this file I gather structures and function working on indexes but at a "higher level" than the basic index functions in index.h.
 *      These functions/structures use those in index.h and are used by locus.h and access.h.
 *      Motivation : index.c was getting too big with new concat, merge,etc functions.
 */

#ifndef INDEX_HL_H_
#define INDEX_HL_H_

#include "index.h"

/* Storage structures for information on flat files that were just parsed.*/
typedef struct {
  indix_t *l_locind; //array of index for the flat file
  long locnb; // size of l_locind.
  indix_t *l_accind;
  long accnb; // size of l_accind.
} all_indix_t;


void init_all_indix_t();
void freeAllIndix(); // free memory allocated for the elements of the all_indix_t structure.

typedef struct {
  result_t ** start_l;
  int len_l; // in fact number of cards that we have to look for in the DB.
} WDBQueryData;

typedef struct {
  WDBQueryData * l_infoDB;
  int nb_db;
} LWDBQueryData;

typedef struct {
  result_t ** lst_work; // array of pointers to result_t structures sorted in alphabetical order of db_name.
  int nb_cards; // size of the previous array.
  LWDBQueryData meta_lst_work; // meta information about the array ; pointer to "sub arays" for each db_name + size of sub array.
} WAllQueryData;

all_indix_t create_index(char *,int,int,int );
int index_search(char *file, char * db_name, WDBQueryData wData, int * nb_not_found);
all_indix_t index_load(char *,char *);
int index_concat(char *, long, indix_t *);
int index_merge(char *, long, indix_t *);
all_indix_t fic_index_load(char * file,char * suff);

#endif /* INDEX_HL_H_ */
