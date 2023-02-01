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

#ifndef INDEX_HL_H_
#define INDEX_HL_H_

#include "index.h"

/* Storage structures for information on flat files that were just parsed.*/
typedef struct {
  indix_t *l_locind; //array of index for the flat file
  uint64_t locnb; // size of l_locind.
  indix_t *l_accind;
  uint64_t accnb; // size of l_accind.
} all_indix_t;

/* storage structure for the content of 1 index file. */
typedef struct {
  indix_t *l_idx; //array of index for the flat file
  uint64_t nb_idx; // size of array.
} array_indix_t;

// void init_array_indix_t();


void init_all_indix_t(all_indix_t *);
void freeAllIndix(all_indix_t); // free memory allocated for the elements of the all_indix_t structure.

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
all_indix_t index_load(const char *,const char *,const char *);
int index_concat(char *, uint64_t, indix_t *);
int index_merge(char *, uint64_t, indix_t *);
array_indix_t fic_index_load(const char * file);
// void index_hl_remove(int,int,char *,char *);

#endif /* INDEX_HL_H_ */
