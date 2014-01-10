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



/* Structure used to store intermediate results : ie, list of AC that were not found yet and tha
 t have to be looked for in other DB. */

typedef struct {
  result_t ** addrArray;
  int arrSize;
} ArrayOfResAddr;

void initArrayOfResAddr(ArrayOfResAddr * const ia_arr);  // deprecated

/* Try to make code simpler. 
 I work on an array of adresses of result_t structures.
 This array is sorted in alphabetical order of result_t->db_name.
 This structure is a utilty to make the manipulation of the array by the goldenLstQuery more simple.
 */
/*
typedef struct {
  result_t ** start_l;
  int len_l; // in fact number of cards that we have to look for in the DB.
} InfoDbResAddr;

typedef struct {
  InfoDbResAddr * l_infoDB;
  int nb_db;
} LInfoDbResAddr;*/

/* Functions prototypes */
int index_merge(char *, long, indix_t *);
//int index_search(char *, char *, result_t ** , int, result_t *** const  , int * );
int index_search(char *file, char * db_name, result_t ** lst, int lst_size, int * nb_not_found);
const char *index_dir(void);
char *index_file(const char *, const char *, const char *);

#endif /* __INDEX_H_ */

