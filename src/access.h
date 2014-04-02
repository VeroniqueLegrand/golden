/* access.h - Accession numbers functions */

#ifndef __ACCESS_H_
#define __ACCESS_H_

#include "index.h"


/* Functions prototypes */
int access_merge(char *, long, indix_t *,char *);
int access_concat(char *, long, indix_t *, char * );
void access_search(WDBQueryData wData, char * db_name, int * nb_AC_not_found);
result_t * access_search_deprecated(char *dbase, char *name);
#endif /* __ACCESS_H_ */
