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


// #define DEBUG
/* Functions prototypes */

static int index_move(const char *, const char *);


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
  return strncmp(p->name, q->name, (size_t)NAMLEN);
}

/* Compare indexes by name and filenb */
int index_compare_fnb(const void *a, const void *b) {
  const indix_t *p, *q;
  int ret;
  p = (const indix_t *)a; q = (const indix_t *)b;
  ret=strncmp(p->name, q->name, (size_t)NAMLEN);
  if (ret==0) {
    if (p->filenb==q->filenb) return 0;
    else if (p->filenb<q->filenb) return -1;
    else if (p->filenb>q->filenb) return 1;
  }
  return ret;
}


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



// #define DEBUG


void index_sort(char *file, long nb) {
  // FILE *g;
  int fd;
  const char *dir;
  indix_t * old;
  void * fmap, *fmap_orig;
  // uint64_t nb_idx;

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

#ifdef DEBUG
  indix_t * old2=old;
  printf("AC/locus before sort:\n.");
  int i;
  for (i=0;i<nb;i++) {
    printf("%s\n",old2->name);
    old2++;
  }
  printf("--------------------------------------------------\n");
#endif

  /* Sort indexes */
  qsort(old, (size_t)nb, sizeof(*old), index_compare);

#ifdef DEBUG
  // to check that it works.
  printf("AC/locus sorted in alphabetical order:\n.");
  for (i=0;i<nb;i++) {
    printf("%s\n",old->name);
    old++;
  }
  printf("--------------------------------------------------\n");
#endif

  // if ( msync(fmap_orig, (size_t) length , MS_SYNC ) < 0 ) err(errno, "msync failed for file : %s",file);
  if (close(fd) == EOF) err(errno, "Cannot close file : %s",file);
  if (munmap(fmap_orig,(size_t) length) == -1) err(errno, "Cannot unmap file : %s",file);

  return;
}


/* concatenates new indexes in memory with index file on disk.
 * If index file doesn't exist, create it.
 * returns the total number of indexes.
 */
int index_concat(char *file, long nb, indix_t *ind) {
  indix_t *cur;
  FILE *g;
  uint64_t newnb, oldnb;
  cur = ind;

  /* Create empty index file if missing */
  if (access(file, F_OK) != 0) create_missing_idxfile(file);
  if (nb == 0) { return 0; }
  if ((g = fopen(file, "r+")) == NULL) err(errno,"Cannot open file : %s",file);
  if (fread(&oldnb, sizeof(oldnb), 1, g) != 1) err(errno,"Cannot read number of indexes from file : %s",file);
  newnb=oldnb+nb;

  /* Add new remaining indexes */
  if (fseeko(g, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.",file);
  while(nb) {
    if (fwrite(cur, sizeof(*cur), 1, g) != 1) err(errno,"Cannot write index to : %s",file);
    nb--;
    cur++;
  }

  if (fseeko(g, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s",file);
  if (fwrite(&newnb, sizeof(newnb), 1, g) != 1) err(errno,"Cannot write to : %s", file);

  if (fclose(g) == EOF) err(errno,"Cannot close file : %s",file);
  return newnb;
}

/* concatenates indexes that have already been written to a file by a previous call to goldin. */
long index_file_concat(FILE * fd_d,int prev_nb_flat, long nb_idx, FILE * fd_s, long prev_nb_idx) {
  long totnb=prev_nb_idx;
  long cnt;
  indix_t inx;
  for (cnt=0;cnt<nb_idx;cnt++) {
    if (fread(&inx, sizeof(inx), 1, fd_s) != 1) error_fatal("Cannot read index from source file", NULL);
    inx.filenb=inx.filenb+prev_nb_flat;
    if (fwrite(&inx, sizeof(inx), 1, fd_d) != 1) err(errno,"Cannot write index to destination file",NULL);
    totnb++;
  }
  return totnb;
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


void index_purge(const char * fic) {
  char * dir, *new;
  FILE * f, *g;
  uint64_t newnb, oldnb;
  indix_t cur1,cur2;

  if (access(fic, F_OK) != 0) err(errno,"file doesn't exist : %s",fic);
  if ((dir = getenv("TMPDIR")) == NULL) { dir = TMPDIR; }
  if ((new = index_temp(dir)) == NULL) err(errno,"cannot create temporary file : %s",dir);
  if ((f = fopen(new, "w")) == NULL) err(errno,"Cannot open file : %s",new);

  if ((g = fopen(fic, "r")) == NULL) err(errno,"Cannot open file : %s",fic);
  if (fread(&oldnb, sizeof(oldnb), 1, g) != 1) err(errno,"Cannot read number of indexes from file : %s",fic);
  if ((oldnb==0) || (oldnb==1)) {
    if (fclose(g) == EOF) err(errno,"Cannot close file : %s",fic);
    return;
  }

  if (fwrite(&oldnb, sizeof(oldnb), 1, f) != 1) err(errno,"Cannot write to : %s", new);
  if (fread(&cur1, sizeof(cur1), 1, g) != 1) err(errno,"Cannot read indexes from file : %s",fic);
  newnb=0;

  while(oldnb-1) {
    /*if (feof(g)) {
      printf("end of file reached. ");
    }*/
    int ret=fread(&cur2, sizeof(cur2), 1, g);
    // printf("fread returned : %d\n",ret);
    if (ret != 1) {
      /*printf("%d \n",errno);
      if (ferror(g)) {
        printf("At file: %ld\n", ftell(g));
        perror("read error: ");
      }*/
      err(errno,"Cannot read indexes from file : %s",fic);
    }
    int i=index_compare(&cur1,&cur2);
    if (i!=0) {
      if (fwrite(&cur1, sizeof(cur1), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
      cur1=cur2;
      newnb++;
    } else {
      if (cur1.filenb<cur2.filenb) cur1=cur2;
    }
    oldnb--;
  }
  if (fwrite(&cur1, sizeof(cur1), 1, f) != 1) err(errno,"Cannot write index to : %s",new);
  newnb++;
  if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s",new);
  if (fwrite(&newnb, sizeof(newnb), 1, f) != 1) err(errno,"Cannot write to : %s", new);
  if (fclose(f) == EOF) err(errno,"Cannot close file : %s",new);
  if (fclose(g) == EOF) err(errno,"Cannot close file : %s",fic);
  
  /* Rename file */
  if (index_move(fic, new) != 0) err(errno,"Cannot move : %s to : %s",new,fic);

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









