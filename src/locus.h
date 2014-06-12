/* locus.h - Entry name functions */

#ifndef __LOCUS_H_
#define __LOCUS_H_

#include "index.h"
#include "index_hl.h"

/* Functions prototypes */
int locus_merge(char *, uint64_t, indix_t *,char * new_index_dir);
int locus_concat(char *dbase, uint64_t, indix_t *ind, char * new_index_dir);
void locus_search(WDBQueryData wData, char * dbase, int * nb_locus_not_found);

#endif /* __LOCUS_H_ */
