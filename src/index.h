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

/* Structure used to store intermediate results : ie, list of AC that were not found yet and thst have to be looked for in other DB. */
typedef struct {
  result_t ** addrArray;
  int arrSize;
} ArrayOfResAddr;

void initArrayOfResAddr(ArrayOfResAddr * const ia_arr);

/* Functions prototypes */
int index_merge(char *, long, indix_t *);
int index_search(char *, char *, result_t ** , int, result_t *** const  , int * );
const char *index_dir(void);
char *index_file(const char *, const char *, const char *);

#endif /* __INDEX_H_ */

