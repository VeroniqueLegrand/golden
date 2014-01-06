/* locus.h - Entry name functions */

#ifndef __LOCUS_H_
#define __LOCUS_H_

#include "index.h"

/* Functions prototypes */
int locus_merge(char *, long, indix_t *);
ArrayOfResAddr locus_search(result_t ** ,int ,char *, int * );

#endif /* __LOCUS_H_ */
