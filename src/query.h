/*
 *  Here, I gather functions that are useful for performing queries and that are used by
 *  both main C golden and 'Python' Golden.
 */


#ifndef __QUERY_H_
#define __QUERY_H_

// static int compare_dbase (void const *a, void const *b);
int performGoldenQuery(WAllQueryData, int,int);
WAllQueryData prepareQueryData(char *, result_t * ,int);
void freeQueryData(WAllQueryData wData);
int get_nbCards(char * my_list);

#endif
