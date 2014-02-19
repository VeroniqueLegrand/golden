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

// to indicate whether we are loading acc indexes or locus indexes.
#define LOC_IDX 0
#define ACC_IDX 1


/* constants used to indicate that an AC or entry name was found nowhere. */
#define NOT_FOUND -1

/* Modes for index_dump */
#define APPEND_INDEXES 1
#define REPLACE_INDEXES 2
#define MERGE_INDEXES 3


/* Index structure definition */
typedef struct {
  char name[NAMLEN+1];
  uint32_t filenb;
  uint64_t offset; } indix_t;

/* Storage structures for information on flat files that were just parsed.*/
typedef struct {
  // char * flatfile_name;// name of the flat file. // never ued; remove it.
  indix_t *l_locind; //array of index for the flat file
  long locnb; // size of l_locind.
  indix_t *l_accind;
  long accnb; // size of l_accind.
} all_indix_t;

void init_all_indix_t();

typedef struct {
  char *name, *dbase;
  char *real_dbase;
  int filenb;
  off_t offset; } result_t;

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
int index_search(char *file, char * db_name, WDBQueryData wData, int * nb_not_found);
const char *index_dir(void);
char *index_file(const char *, const char *, const char *);
void freeAllIndix(); // free memory allocated for the elements of the all_indix_t structure.
all_indix_t create_index(char *,int,int,int );
int index_dump(char *, int , long , indix_t *,char *);
// all_indix_t index_load(char * ,char *, int );
all_indix_t index_load(char *,char *);
void print_wrk_struct(result_t ** lst_work,int nb_cards,int);
void index_sort(char *, long);
void create_missing_idxfile(char *);
char *index_temp(const char *dir);
int index_compare(const void *a, const void *b);
uint64_t iswap64(uint64_t val);
uint32_t iswap32(uint32_t val);
int list_append(char *dbase, char *dir, char *file);

#endif /* __INDEX_H_ */

