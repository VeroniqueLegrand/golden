/* index_utility.c - low-level tility functions for indexes. */

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
#include "index_hl.h"
#include <errno.h>
#include <err.h>
#include <ctype.h>

#define BUFINC 100




void init_all_indix_t(all_indix_t * sToInit) {
  sToInit->accnb=0;
  // sToInit->flatfile_name=NULL;
  sToInit->l_accind=NULL;
  sToInit->l_locind=NULL;
  sToInit->locnb=0;
}


void freeAllIndix(all_indix_t sToFree) {
  // if (sToFree.flatfile_name!=NULL) free(sToFree.flatfile_name);
  if (sToFree.l_locind!=NULL) free(sToFree.l_locind);
  if (sToFree.l_accind!=NULL) free(sToFree.l_accind);
}

all_indix_t fic_index_load(char * file,char * suff) {
  FILE * g;
  uint64_t nb_idx;
  all_indix_t fic_indix;
  indix_t cur;
  int i=0;
  init_all_indix_t(&fic_indix);
  if ((g = fopen(file, "r")) == NULL) err(errno,"cannot open file: %s.",file);
  if (fread(&nb_idx, sizeof(nb_idx), 1, g) != 1) err(errno,"cannot read index number from file: %s.",file);
  if (suff==LOCSUF) {
    fic_indix.locnb=nb_idx;
    if ((fic_indix.l_locind = (indix_t *)realloc(fic_indix.l_locind, nb_idx*sizeof(indix_t))) == NULL) err(errno,"cannot allocate memory");
  } else {
    fic_indix.accnb=nb_idx;
    if ((fic_indix.l_accind = (indix_t *)realloc(fic_indix.l_accind, nb_idx*sizeof(indix_t))) == NULL) err(errno,"cannot reallocate memory");
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


/*
 Load an index file into a memory structure. Mostly used for unit tests. Or else,trying to load huge files (X Gb) in memory
 would fail.
 */
// all_indix_t index_load(char * flat_filename, char * file,int typ) {
all_indix_t index_load(char * dbase,char * suff) {
  char * file;
  file = index_file(".", dbase, suff);
  return fic_index_load(file,suff);
}

/*void close_index_desc(cur_index_descr * p_idx_fd, char * dbase) {
  if (p_idx_fd->d_fa!=NULL) {
    if (fclose(p_idx_fd->d_fa) == EOF) err(errno,"error closing file: %s.acx",dbase);
    p_idx_fd->d_fa=NULL;
  }
  if (p_idx_fd->d_fl!=NULL) {
    if (fclose(p_idx_fd->d_fl) == EOF) err(errno,"error closing file: %s.icx",dbase);
    p_idx_fd->d_fl=NULL;
  }

  p_idx_fd->accnb=0;
  p_idx_fd->locnb=0;
  p_idx_fd->ficnb=0;
}*/



/*
 * First step : create indexes for given file.
 */
all_indix_t create_index(char * file, int filenb, int loc, int acc) {
  FILE *f;
  char *p;
  long indnb;
  entry_t ent;
  size_t len;
  indix_t *cur;
  all_indix_t fic_indix;

  fic_indix.locnb = 0;
  fic_indix.accnb = 0;
  indnb = 0;
  fic_indix.l_locind=NULL;
  fic_indix.l_accind=NULL;
  // fic_indix.flatfile_name=strdup(file);
#ifdef PERF_PROFILE
  clock_t cpu_time_start=clock();
  time_t wall_time_start=time(NULL);
  srand(wall_time_start);
#endif

  if ((f = fopen(file, "r")) == NULL)
    err(errno,"cannot open file: %s.",file);
  while(entry_parse(f, &ent) != 1) {
    /* Checks for reallocation */
    if (fic_indix.locnb >= indnb || fic_indix.accnb >= indnb) {
      indnb += BUFINC; len = (size_t)indnb * sizeof(indix_t);
      if ((fic_indix.l_locind = (indix_t *)realloc(fic_indix.l_locind, len)) == NULL ||
          (fic_indix.l_accind = (indix_t *)realloc(fic_indix.l_accind, len)) == NULL)
      err(errno,"cannot reallocate memory");
    }
    /* Store entry name & accession number indexes */
    if (loc && ent.locus[0] != '\0') {
      cur = fic_indix.l_locind + fic_indix.locnb; fic_indix.locnb++;
      (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
      (void)strncpy(cur->name, ent.locus, (size_t)NAMLEN);
      p = cur->name;
      while (*p) { *p = toupper((unsigned char)*p); p++;
      }
      cur->filenb = filenb;
      cur->offset = ent.offset;
    }
    if (acc && ent.access[0] != '\0') {
      cur = fic_indix.l_accind + fic_indix.accnb; fic_indix.accnb++;
      (void)memset(cur->name, 0x0, (size_t)NAMLEN+1);
      (void)strncpy(cur->name, ent.access, (size_t)NAMLEN);
      p = cur->name;
      while (*p) { *p = toupper((unsigned char)*p); p++;
      }
      cur->filenb = filenb; cur->offset = ent.offset;
    }

#ifdef PERF_PROFILE
    // for the needs of performance testing, modify cur->name so that index file grows bigger and bigger.
    sprintf(cur->name,"%d",rand());
#endif
  }
  if (fclose(f) == EOF)
    err(errno,"cannot close file: %s.",file);
    // error_fatal(file, NULL);

  return fic_indix;
}

/* Search database indexes */
int index_search(char *file, char * db_name, WDBQueryData wData, int * nb_not_found) {
  FILE *f;
  int i, swap;
  uint64_t indnb;
  long min, cur, max;
  off_t pos, chk;
  size_t len;
  struct stat st;
  indix_t inx;
  int nb_found;

  nb_found=0;

  result_t ** lst=wData.start_l; // for work
  int lst_size=wData.len_l;

  if (lst==NULL) return 0;
  if (*lst==NULL) {
    error_fatal("index_search : bad list in input", NULL);
  }

  if (stat(file, &st) == -1) {
    error_fatal(file, NULL); }

#ifdef DEBUG
    result_t ** start_l=wData.start_l; // for printing debug info only.
    printf("index_search called on : %s\n",file);
#endif


  if ((f = fopen(file, "r")) == NULL) {
    error_fatal(file, NULL); }
  if (fread(&indnb, sizeof(indnb), 1, f) != 1) {
    error_fatal(file, NULL); }

  /* Check indexes endianness */
    chk = sizeof(indnb) + indnb * sizeof(inx);
    if ((swap = (chk != st.st_size)) == 1) {
      indnb = iswap64(indnb); }

  int idx_card;
  //printf("lst_size=%d\n",lst_size);
  for (idx_card=0;idx_card<lst_size;idx_card++)
  {
    result_t * cur_res=lst[idx_card];
    if (cur_res->filenb!=NOT_FOUND) continue;
    char * name=cur_res->name;
#ifdef DEBUG
    printf("name : %s\n",name);
#endif
    len = strlen(name);
    if (len > NAMLEN) {
      error_fatal(name, "name too long");
    }

    min = 0; max = (long)indnb - 1;
    i=-1; // to avoid pb in case indnb=0 and i is not initialized.
#ifdef DEBUG
    printf("entering while search loop : min=%ld max=%ld\n",min,max);
#endif
    while(min <= max)
    {
    /* Set current position */
      cur = (min + max) / 2;
      pos = sizeof(indnb) + cur * sizeof(inx);
      if (fseeko(f, pos, SEEK_SET) == -1) {
        error_fatal(file, NULL); }
        /* Check current name */
      if (fread(&inx, sizeof(inx), 1, f) != 1) {
        error_fatal(file, NULL); }
        if ((i = strcmp(name, inx.name)) == 0) break;
        /* Set new limits */
        if (i < 0) { max = cur - 1; }
        if (i > 0) { min = cur + 1; }
    }
    if (i==0) { // found
      (*nb_not_found)--;
      cur_res->filenb = inx.filenb;
      cur_res->offset = inx.offset;
      cur_res->real_dbase=strdup(db_name);
      if (swap == 1) {
        cur_res->filenb = iswap32(cur_res->filenb);
        cur_res->offset = iswap64(cur_res->offset);
      }
      nb_found++;
    }
    if (*nb_not_found==0) break;
  }
  if (fclose(f) == EOF) {
    error_fatal(file, NULL); }
#ifdef DEBUG
  printf("\n index_search, closed file : %s, for cur_db : %s, nb_not_found=%d, nb_res_found=%d \n",file,db_name,*nb_not_found,nb_found);
  print_wrk_struct(start_l,lst_size, 1);
#endif
  return nb_found;}

