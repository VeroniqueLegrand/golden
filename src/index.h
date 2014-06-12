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

