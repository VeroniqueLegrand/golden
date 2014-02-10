/* entry.h - Databases entries functions */
// #define DEBUG
#ifndef __ENTRY_H_
#define __ENTRY_H_

#include <stdio.h>

/* Entries names maximum length */
#define NAMLEN 27

/* Entry structure definition */
typedef struct {
  char locus[NAMLEN+1];
  char access[NAMLEN+1];
  off_t offset; } entry_t;



/* Functions prototypes */
int entry_parse(FILE *, entry_t *);
// int entry_display(FILE *, FILE *);
int entry_display(FILE *f, int fd);

#endif /* __ENTRY_H_ */
