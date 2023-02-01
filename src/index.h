/* index.h - Golden indexes functions */
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
#ifndef __INDEX_H_
#define __INDEX_H_

#include <inttypes.h>

#include "entry.h"


/* Index files suffixes */
#define LOCSUF "idx"
#define ACCSUF "acx"
#define LSTSUF "dbx"
#define VIRSUF "vix"

// to indicate whether we are loading acc indexes or locus indexes.
#define LOC_IDX 0
#define ACC_IDX 1


/* constants used to indicate that an AC or entry name was found nowhere. */
#define NOT_FOUND -1

/* error codes for index functions */
#define IDX_ERR -1

/* Index structure definition */
typedef struct {
  char name[NAMLEN+1];
  uint32_t filenb;
  uint64_t offset; } indix_t;


typedef struct {
  char *name, *dbase;
  char *real_dbase;
  int filenb;
  off_t offset; } result_t;


/* Functions prototypes */
const char *index_dir(void);
char *index_file(const char *, const char *, const char *);
void print_wrk_struct(result_t ** lst_work,int nb_cards,int);
void index_sort(char *, uint64_t);
void create_missing_idxfile(char *);
char *index_temp(const char *dir);
int index_compare(const void *a, const void *b);
uint64_t iswap64(uint64_t val);
uint32_t iswap32(uint32_t val);
uint64_t index_file_concat(int fd_d,int prev_nb, uint64_t, int, uint64_t);
struct flock index_file_lock(int fd, off_t l_start, off_t l_len );
void index_file_unlock(int fd, struct flock lock_t);
void index_purge(const char * fic);


#endif /* __INDEX_H_ */

