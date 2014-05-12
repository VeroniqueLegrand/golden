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

array_indix_t fic_index_load(const char * file) {
  FILE * g;
  uint64_t nb_idx;
  array_indix_t fic_indix;
  indix_t cur;
  fic_indix.l_idx=NULL;
  int i=0;
  if ((g = fopen(file, "r")) == NULL) err(errno,"cannot open file: %s.",file);
  if (fread(&nb_idx, sizeof(nb_idx), 1, g) != 1) err(errno,"cannot read index number from file: %s.",file);
  fic_indix.nb_idx=nb_idx;
  if ((fic_indix.l_idx = (indix_t *)realloc(fic_indix.l_idx, nb_idx*sizeof(indix_t))) == NULL) err(errno,"cannot allocate memory");
  for(i=0;i<nb_idx;i++) {
    if (fread(&cur, sizeof(cur), 1, g) != 1) err(errno,"cannot read index from file: %s.",file);
    fic_indix.l_idx[i]=cur;
  }
  if (fclose(g) == EOF) err(errno,"error closing file: %s.",file);
  return fic_indix;
}


/*
 Load an index file into a memory structure. Mostly used for unit tests. Or else,trying to load huge files (X Gb) in memory
 would fail.
 */
// all_indix_t index_load(char * flat_filename, char * file,int typ) {
all_indix_t index_load(const char *idx_dir, const char * dbase,const char * suff) {
  char * file;
  array_indix_t t_idx;
  all_indix_t all_idx;
  int load_all=0;
  init_all_indix_t(&all_idx);
  
  if (suff==NULL) load_all=1; // load all indexes, both AC and locus.
  if (load_all==1) {
    file = index_file(idx_dir, dbase, ACCSUF);
    t_idx=fic_index_load(file);
    all_idx.accnb=t_idx.nb_idx;
    all_idx.l_accind=t_idx.l_idx;
    file = index_file(idx_dir, dbase, LOCSUF);
    t_idx=fic_index_load(file);
    all_idx.locnb=t_idx.nb_idx;
    all_idx.l_locind=t_idx.l_idx;
  } else {
    file = index_file(idx_dir, dbase, suff);
    t_idx=fic_index_load(file);
  }
  if (suff==LOCSUF) {
    all_idx.locnb=t_idx.nb_idx;
    all_idx.l_locind=t_idx.l_idx;
  } else if (suff==ACCSUF){
    all_idx.accnb=t_idx.nb_idx;
    all_idx.l_accind=t_idx.l_idx;
  }
  return all_idx;
}


/*
 to remove old index files
 */
void index_hl_remove(int acc,int loc,char *new_index_dir,char *dbase) {
  char * dbx_file;
  dbx_file = index_file(new_index_dir, dbase, LSTSUF);
  if (access(dbx_file, F_OK) == 0) {
    if (remove(dbx_file)==-1) err(errno, "Couldn't remove : %s",dbx_file);
  }
  if (acc) {
    char * acx_file = index_file(new_index_dir, dbase, ACCSUF);
    if (access(acx_file, F_OK) == 0) if (remove(acx_file)==-1) err(errno, "Couldn't remove : %s",acx_file);
  }
  if (loc) {
    char * icx_file = index_file(new_index_dir, dbase, LOCSUF);
    if (access(icx_file, F_OK) == 0) if (remove(icx_file)==-1) err(errno, "Couldn't remove : %s",icx_file);
  }
}



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
    err(0,"index_search : bad list in input");
  }

  if (stat(file, &st) == -1) {
    err("index file : %s does not exist.",file); }

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

