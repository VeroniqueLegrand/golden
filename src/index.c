/* index.c - Golden indexes functions */

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

#include "error.h"
#include "index.h"
#include <errno.h>
#include <err.h>
#include <ctype.h>
#include <fcntl.h>

#include <sys/mman.h>

// buffer size for reading file.
#define BUFINC 100


#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t long
#endif

#ifndef DATADIR
#define DATADIR "/usr/local/share"PACKAGE
#endif

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

/* Functions prototypes */

static int index_move(const char *, const char *);
/*static int index_compare(const void *, const void *);
static uint64_t iswap64(uint64_t);
static uint32_t iswap32(uint32_t);*/



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

/* Get golden indexes directory */
const char *index_dir(void) {
  const char *p;
  if ((p = getenv("GOLDENDATA")) == NULL) {
    p = DATADIR; }
  return p; }


/* Get golden index file name */
char *index_file(const char *dir, const char *dbase, const char *suf) {
  const char *p;
  char *file;
  size_t len;

  if ((p = dir) == NULL) { p = index_dir(); }

  len = strlen(p) + 1 + strlen(dbase) + 1 + strlen(suf);
  if ((file = (char *)malloc(len+1)) == NULL) {
    error_fatal("memory", NULL); }
  (void)sprintf(file, "%s/%s.%s", p, dbase, suf);

  return file; }


/* Search database indexes */
//
/* lst is an array containing the adresses of result_t structures in the order expected by the user.
 It points on the first element that index_search has to look for in file for dbase.
 The information in lst (filenb,offset) is filled for the elements that are found.
 index_search returns the number of elements that were not found.
 index_search allocates memory for lst_not_found. It is up to the caller to free it.
 */
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



/* Only used for performance testing for the moment; optimized version of index_merge.*/
void index_sort(char *file, long nb) {
  // FILE *g;
  int fd;
  const char *dir;
  indix_t * old;
  void * fmap, *fmap_orig;
  uint64_t nb_idx;

  if (nb == 0) return;
  if ((dir = getenv("TMPDIR")) == NULL) { dir = TMPDIR; }

  if (access(file, F_OK) != 0) err(errno, "file doesn't exist : %s",file);// create_missing_idxfile(file);
  // figure out how big it is
  struct stat statbuf;
  int result = stat (file, &statbuf);
  if (result == -1) err(errno, "Cannot stat file : %s",file);
  off_t length = statbuf.st_size;
  // if ((g = fopen(file, "r")) == NULL) err(errno, "Cannot open file : %s",file);
  if ((fd=open(file,O_RDWR))==-1) err(errno, "Cannot open file : %s",file);
  // mmap it
  fmap = (void *) mmap (NULL, (size_t) length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (fmap == NULL) err(errno, "Cannot mmap file : %s",file);
  fmap_orig=fmap;
  fmap=fmap+sizeof(uint64_t);
  old=(indix_t *) fmap;
  /* Sort indexes */
  qsort(old, (size_t)nb, sizeof(*old), index_compare);

#ifdef DEBUG
  // to check that it works.
  printf("AC/locus sorted in alphabetical order:\n.");
  int i;
  for (i=0;i<nb;i++) {
    printf("%s\n",old->name);
    old++;
  }
#endif

  if (close(fd) == EOF) err(errno, "Cannot close file : %s",file);
  if (munmap(fmap_orig,(size_t) length) == -1) err(errno, "Cannot unmap file : %s",file);

  return;
}


/* Merge indexes */
int index_merge(char *file, long nb, indix_t *ind) {
  FILE *f, *g;
  int i;
  char *new;
  const char *dir;
  uint64_t newnb, oldnb, totnb;
  indix_t *cur, old;

  if ((dir = getenv("TMPDIR")) == NULL) { dir = TMPDIR; }

  /* Sort new indexes */
  qsort(ind, (size_t)nb, sizeof(*ind), index_compare);
  newnb = nb; totnb = 0; cur = ind;

  /* Create empty index file if missing */
  if (access(file, F_OK) != 0) create_missing_idxfile(file);

  if (nb == 0) { return 0; }

  /* Make new file */
  if ((new = index_temp(dir)) == NULL) err(errno,"cannot create temporary file : %s",dir);
  if ((f = fopen(new, "w")) == NULL) err(errno,"Cannot open file : %s",new);
  if (fwrite(&totnb, sizeof(totnb), 1, f) != 1) err(errno,"Cannot write to : %s", new);

  /* Process old index file & merge with new indexes in new temporary index file. */
  if ((g = fopen(file, "r")) == NULL) err(errno,"Cannot open file : %s",file);
  if (fread(&oldnb, sizeof(oldnb), 1, g) != 1) err(errno,"Cannot read number of indexes from file : %s",file);
  
  while(oldnb) {
    if (fread(&old, sizeof(old), 1, g) != 1) err(errno,"Cannot read indexes from file : %s",file);

    /* Insert new entries */
    while(newnb && (i = index_compare(cur, &old)) < 0) {
      if (fwrite(cur, sizeof(*cur), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
      newnb--; cur++; totnb++;
    }

    /* Update existing entries */
    if (newnb && i == 0) {
      if (fwrite(cur, sizeof(*cur), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
      newnb--; oldnb--; cur++; totnb++; continue;
    }

    /* Write old entry */
    if (fwrite(&old, sizeof(old), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
    oldnb--; totnb++;
  }

  if (fclose(g) == EOF) err(errno,"Cannot close file : %s",file);

  /* Add new remaining indexes */
  while(newnb) {
    if (fwrite(cur, sizeof(*cur), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
    newnb--; cur++; totnb++;
  }

  /* Update entries number */
  if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s",new);
  if (fwrite(&totnb, sizeof(totnb), 1, f) != 1) err(errno,"Cannot write to : %s", new);
  if (fclose(f) == EOF) err(errno,"Cannot close file : %s",new);

  /* Rename file */
  if (index_move(file, new) != 0) err(errno,"Cannot move : %s to : %s",new,file);

  free(new);
  return 0;
}


/*
 * merge multiple index files.
 * files is in fact a list of dbase, separated by ' '. It is given by the user.
 * For each given dbase, I have to find the coresponding .dbx, .acx, .idx files.
 */
void index_merge_multi(char *files,char * dbase, int loc, int acc, char * source_dir, char * dest_dir) {
  char * elm;
  char * s_dbx_file, *s_acx_file, *s_icx_file;
  char * d_dbx_file, *d_acx_file, *d_icx_file;

  d_dbx_file=index_file(dest_dir,dbase,LSTSUF);
  if (acc) d_acx_file=index_file(dest_dir,dbase,ACCSUF);
  if (loc) d_icx_file=index_file(dest_dir,dbase,LOCSUF);

  // if dest files already exist, merge new files into it.

  elm = strtok (files," ");
  while (elm!=NULL) {
    s_dbx_file=index_file(source_dir,elm,LSTSUF);
    if (acc) s_acx_file=index_file(source_dir,elm,ACCSUF);
    if (loc) s_icx_file=index_file(source_dir,elm,LOCSUF);
    elm = strtok (NULL,"\n");
  }
}

/* Move indexe file */
static int index_move(const char *dst, const char *src) {
  FILE *f, *g;
  char *p, *q, *tmp;
  uint64_t n;
  indix_t cur;

  /* Rename file */
  if (rename(src, dst) == 0) { return 0; }

  /* Copy+Remove file */
  q = strdup(dst); p = dirname(q);
  if ((tmp = index_temp(p)) == NULL) {
    error_fatal(p, "cannot create temporary file"); }
  if ((f = fopen(src, "r")) == NULL) {
    error_fatal(src, NULL); }
  if ((g = fopen(tmp, "w")) == NULL) {
    error_fatal(tmp, NULL); }
  if (fread(&n, sizeof(n), 1, f) != 1) {
    error_fatal(src, NULL); }
  if (fwrite(&n, sizeof(n), 1, g) != 1) {
    error_fatal(tmp, NULL); }
  while (n--) {
    if (fread(&cur, sizeof(cur), 1, f) != 1) {
      error_fatal(src, NULL); }
    if (fwrite(&cur, sizeof(cur), 1, g) != 1) {
      error_fatal(tmp, NULL); }
  }
  if (fclose(g) == EOF) {
    error_fatal(tmp, NULL); }
  if (fclose(f) == EOF) {
    error_fatal(src, NULL); }
  if (rename(tmp, dst) != 0) {
    error_fatal(dst, NULL); }
  free(tmp); free(q);
  if (unlink(src) != 0) {
    error_fatal(src, NULL); }

  return 0; }

/*
 Dump all indexes to the file given in input.
 If file doesn't exist, it is created.
 If mode is set to APPEND_INDEXES, new indexes are added at the end of the file.
 If mode is set to REPLACE_INDEXES, file is erased, only new indexes will be in it.
 If mode is set to MERGE_INDEXES, new indexes are merged with old ones.
    Prerequisite: old indexes must have been sorted previously.
    This is the default behavior that was in previous goldin version.
 */
int index_dump(char *dbase, int mode, all_indix_t file_l_indix,char * SUF, const char * index_dir) {
  char *file;
  char o_flg[3];
  FILE *f;
  int i;
  uint64_t nb_idx=0;
  long nb;
  indix_t *ind;

  if (strcmp(SUF,LOCSUF)==0) {
    nb=file_l_indix.locnb;
    ind=file_l_indix.l_locind;
  } else {
    nb=file_l_indix.accnb;
    ind=file_l_indix.l_accind;
  }

  file = index_file(index_dir, dbase, SUF);
  if (mode==MERGE_INDEXES) return index_merge(file,nb,ind);

  if (mode==APPEND_INDEXES) strcpy(o_flg,"a");
  else strcpy(o_flg,"w");

  if((access(file,F_OK) != -1 ) && (o_flg[0]=='a')) {
    strcpy(o_flg,"r+");
  }

  if ((f = fopen(file, o_flg)) == NULL) err(errno,"cannot open file: %s.",file);
  if (o_flg[0]=='r') {
    if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.",file);
    if (fread(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error reading number of indexes");
    if (fseeko(f, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.",file);
  } else if (fwrite(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error initializing number of indexes");

  i=0;
  
  while(i<nb) {
    if (fwrite(ind, sizeof(*ind), 1, f) != 1) {
      err(errno,"error writing index"); }
    i++;
    ind++;
  }

  // udate number of indexes in file.
  if (mode==APPEND_INDEXES) nb_idx+=nb;
  if (mode==REPLACE_INDEXES) nb_idx=nb;

  if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.",file);
  if (fwrite(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error writing number of indexes");

  if (fclose(f) == EOF)err(errno,"cannot close file: %s.",file);
  return 0;
}





