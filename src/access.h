/* access.h - Accession numbers functions */

#ifndef __ACCESS_H_
#define __ACCESS_H_

#include "index.h"

/*
typedef struct {
	result_t ** addrArray;
	int arrSize;
} ArrayOfResAddr;*/

/* Functions prototypes */
int access_merge(char *, long, indix_t *);
//ArrayOfResAddr access_search(result_t ** , int , char * , int * );
void access_search(result_t ** lst_current_db, int lst_size, char * db_name, int * nb_AC_not_found);

#endif /* __ACCESS_H_ */
