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

all_indix_nb index_concat_with_existing(all_indix_t file_l_indix,char *dbase, int loc, int acc) {
  all_indix_nb tot_idx;
#ifdef PERF_PROFILE
    clock_t cpu_time_start=clock();
    time_t wall_time_start=time(NULL);
#endif

  if (loc) {
    tot_idx.locnb=locus_concat(dbase, file_l_indix.locnb, file_l_indix.l_locind);
    if (tot_idx.locnb==IDX_ERR) err(errno,"entry names indexes concatenation failed : %s",dbase);
  }
  if (acc) {
    tot_idx.accnb=access_concat(dbase, file_l_indix.accnb, file_l_indix.l_accind);
    if (tot_idx.accnb==IDX_ERR) err(errno,"accession numbers indexes concatenation failed : %s",dbase);
  }

#ifdef PERF_PROFILE
  clock_t cpu_time_stop=clock();
  time_t wall_time_stop=time(NULL);

  // compute time spent concatenating indexes
  clock_t cpu_time_merge_index=cpu_time_stop-cpu_time_start;
  time_t wall_time_merge_index=wall_time_stop-wall_time_start;

  printf("processor time spent concatenating indexes : %lu clock ticks\n",(unsigned long) cpu_time_merge_index);
  printf("wall time spent concatenating indexes: %ld seconds\n",(long)  wall_time_merge_index);
#endif

  return tot_idx;
}

/*
 * Merge new indexes with existing index file.
 * Compute cpu time and wall time for this operation if program was built with PERF_PROFILE
 */
void index_merge_with_existing(all_indix_t file_l_indix,char *dbase, int loc, int acc) {
#ifdef PERF_PROFILE
    clock_t cpu_time_start=clock();
    time_t wall_time_start=time(NULL);
#endif
  /* Merge indexes */
  if (loc) {
    if (locus_merge(dbase, file_l_indix.locnb, file_l_indix.l_locind)) err(errno,"entry names indexes failed : %s",dbase);
  }
  if (acc) {
    if (access_merge(dbase, file_l_indix.accnb, file_l_indix.l_accind)) err(errno,"accession numbers indexes failed : %s",dbase);
  }
#ifdef PERF_PROFILE
  clock_t cpu_time_stop=clock();
  time_t wall_time_stop=time(NULL);

  // compute time spent merging files
  clock_t cpu_time_merge_index=cpu_time_stop-cpu_time_start;
  time_t wall_time_merge_index=wall_time_stop-wall_time_start;

  printf("processor time spent merging indexes : %lu clock ticks\n",(unsigned long) cpu_time_merge_index);
  printf("wall time spent merging indexes: %ld seconds\n",(long)  wall_time_merge_index);
#endif

}




void init_index_fd(cur_index_descr * p_idx_fd) {
  p_idx_fd->d_fa=NULL;
  p_idx_fd->d_fl=NULL;
  p_idx_fd->accnb=0;
  p_idx_fd->locnb=0;
}


void close_index_desc(cur_index_descr * p_idx_fd, char * dbase) {
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
}


/*
 * Returns information about index files.
 */
cur_index_descr get_index_desc(int acc,int loc,char * new_index_dir, char * dbase,char * opn_mode, int create_flg) {
  // struct stat st;
  char *s_acx_file, *s_icx_file;
  uint64_t indnb;
  cur_index_descr cur_idx;
  int ret;

  if ((strcmp(opn_mode,"r")!=0) && (strcmp(opn_mode,"r+")!=0)) {
    error_fatal("opn_mode must be \"r\" or \"r+\"", NULL);
  }
  init_index_fd(&cur_idx);
  if (acc) {
      s_acx_file=index_file(new_index_dir,dbase,ACCSUF);
      ret=access(s_acx_file, F_OK);
      if (( ret!= 0) && create_flg) create_missing_idxfile(s_acx_file);
      else if (ret!=0) error_fatal(s_acx_file, NULL);

      if ((cur_idx.d_fa = fopen(s_acx_file, opn_mode)) == NULL) error_fatal(s_acx_file, NULL);
      if (fread(&indnb, sizeof(indnb), 1, cur_idx.d_fa) != 1) error_fatal(s_acx_file, NULL);
      cur_idx.accnb=indnb;
    }
    if (loc) {
      s_icx_file=index_file(new_index_dir,dbase,LOCSUF);
      ret=access(s_icx_file, F_OK);
      if (( ret!= 0) && create_flg) create_missing_idxfile(s_icx_file);
      else if (ret!=0) error_fatal(s_acx_file, NULL);
      if ((cur_idx.d_fl = fopen(s_icx_file, opn_mode)) == NULL) error_fatal(s_icx_file, NULL);
      if (fread(&indnb, sizeof(indnb), 1, cur_idx.d_fl) != 1) error_fatal(s_icx_file, NULL);
      cur_idx.locnb=indnb;
    }
    return  cur_idx;
}
