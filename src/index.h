/* index.h - Golden indexes functions */

#ifndef __INDEX_H_
#define __INDEX_H_

#include <inttypes.h>

#include "entry.h"


/* Index files suffixes */
#define LOCSUF "idx"
#define ACCSUF "acx"
#define LSTSUF "dbx"
#define VIRSUF "vix"


/* constants used to indicate that an AC or entry name was found nowhere. */
#define NOT_FOUND -1



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

void print_wrk_struct(int nb_cards,result_t ** lst_work,int);


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



/* Functions prototypes */
int index_merge(char *, long, indix_t *);
//int index_search(char *, char *, result_t ** , int, result_t *** const  , int * );
int index_search(char *file, char * db_name, result_t ** lst, int lst_size, int * nb_not_found);
const char *index_dir(void);
char *index_file(const char *, const char *, const char *);

#endif /* __INDEX_H_ */

