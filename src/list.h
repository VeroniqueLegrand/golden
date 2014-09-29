/* list.h - Databases files list functions */

#ifndef __LIST_H_
#define __LIST_H_
#include <stdbool.h>

typedef struct {
  int oldnb;
  int newnb;
}slist_inc;

/* Functions prototypes */
slist_inc list_append(char *, char *, char *,char *,bool);
char *list_name(char *, int);
int list_check(void);
int list_nb(char * , char * );
void list_new(char *);
char * list_get(char *);

#endif /* __LIST_H_ */

