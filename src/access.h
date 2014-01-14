/* access.h - Accession numbers functions */

#ifndef __ACCESS_H_
#define __ACCESS_H_

#include "index.h"


/* Functions prototypes */
int access_merge(char *, long, indix_t *);
void access_search(WDBQueryData wData, char * db_name, int * nb_AC_not_found);
#endif /* __ACCESS_H_ */
