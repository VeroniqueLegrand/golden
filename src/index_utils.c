/* index.c - Goldin utility functions for indexes. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <libgen.h>

#include "index.h"
#include <errno.h>
#include <err.h>
#include <ctype.h>


/*
  Debug utility : prints content of data structure used for work (array of addresses of result_t structures).
 */
void print_wrk_struct(result_t ** lst_work, int nb_cards, int missing_only) {
    printf("\nlst_work content :");
    int i=0;
    result_t * cur_res;
    while (i<nb_cards) {
      cur_res=lst_work[i];
      if ((missing_only && (cur_res->filenb==NOT_FOUND)) || (!missing_only)){
        printf("%s:%s ",cur_res->dbase,cur_res->name);
      }
      i++;
    }
    printf("\n");
}


void init_all_indix_t(all_indix_t * sToInit) {
  sToInit->accnb=0;
  sToInit->flatfile_name=NULL;
  sToInit->l_accind=NULL;
  sToInit->l_locind=NULL;
  sToInit->locnb=0;
}


void freeAllIndix(all_indix_t sToFree) {
  if (sToFree.flatfile_name!=NULL) free(sToFree.flatfile_name);
  if (sToFree.l_locind!=NULL) free(sToFree.l_locind);
  if (sToFree.l_accind!=NULL) free(sToFree.l_accind);
}


/*
 Load an index file into a memory structure. Mostly used for unit tests. Or else,trying to load huge files (X Gb) in memory
 would fail.
 */
// all_indix_t index_load(char * flat_filename, char * file,int typ) {
all_indix_t index_load(char * dbase,char * suff) {
  FILE * g;
  char * file;
  indix_t cur;
  uint64_t nb_idx;
  all_indix_t fic_indix;
  int i=0;
  // fic_indix.flatfile_name=strdup(flat_filename);
  fic_indix.locnb=0;
  fic_indix.accnb=0;
  fic_indix.l_locind=NULL;
  fic_indix.l_accind=NULL;
  file = index_file(".", dbase, suff);
  if ((g = fopen(file, "r")) == NULL) err(errno,"cannot open file: %s.",file);
  if (fread(&nb_idx, sizeof(nb_idx), 1, g) != 1) err(errno,"cannot read index from file: %s.",file);
  if (suff==LOCSUF) {
    fic_indix.locnb=nb_idx;
    if ((fic_indix.l_locind = (indix_t *)realloc(fic_indix.l_locind, nb_idx)) == NULL) err(errno,"cannot allocate memory");
  } else {
    fic_indix.accnb=nb_idx;
    if ((fic_indix.l_accind = (indix_t *)realloc(fic_indix.l_accind, nb_idx)) == NULL) err(errno,"cannot reallocate memory");
  }

  while(i<nb_idx) {
    if (fread(&cur, sizeof(cur), 1, g) != 1) err(errno,"cannot read index from file: %s.",file);
    if (suff==LOCSUF) fic_indix.l_locind[i]=cur;
    else fic_indix.l_accind[i]=cur;
    i++;
  }
  if (fclose(g) == EOF) err(errno,"error closing file: %s.",file);
  return fic_indix;
}

void create_missing_idxfile(char *file) {
  FILE *g;
  uint64_t nb=0;
  /* Create empty index file*/
  if ((g = fopen(file, "w")) == NULL) {
     err(errno,"cannot open file : %s",file); }
  if (fwrite(&nb, sizeof(nb), 1, g) != 1) {
     err(errno,"cannot write to file : %s",file); }
  if (fclose(g) == EOF) err(errno,"cannot close file : %s",file);
}

/* Generate temporary index filename */
char *index_temp(const char *dir) {
  char *tmp;
  size_t len;

  len = strlen(dir) + 1 + 4 + 6;
  if ((tmp = (char *)malloc(len+1)) == NULL) {
    err(errno,"memory"); }
  (void)sprintf(tmp, "%s/goldXXXXXX", dir);
  if (mktemp(tmp) != NULL) { return tmp; }
  free(tmp);

  return NULL; }


/* Compare indexes by names */
int index_compare(const void *a, const void *b) {
  const indix_t *p, *q;

  p = (const indix_t *)a; q = (const indix_t *)b;

  return strncmp(p->name, q->name, (size_t)NAMLEN); }

/* Swap values ... */
uint64_t iswap64(uint64_t val) {
  return ((val << 56) & 0xff00000000000000ULL) |
         ((val << 40) & 0x00ff000000000000ULL) |
         ((val << 24) & 0x0000ff0000000000ULL) |
         ((val <<  8) & 0x000000ff00000000ULL) |
         ((val >>  8) & 0x00000000ff000000ULL) |
         ((val >> 24) & 0x0000000000ff0000ULL) |
         ((val >> 40) & 0x000000000000ff00ULL) |
         ((val >> 56) & 0x00000000000000ffULL); }

uint32_t iswap32(uint32_t val) {
  return ((val << 24) & 0xff000000) |
         ((val <<  8) & 0x00ff0000) |
         ((val >>  8) & 0x0000ff00) |
         ((val >> 24) & 0x000000ff); }

