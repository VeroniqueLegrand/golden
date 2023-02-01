/* index.c - Golden indexes functions */
/*
Copyright (C) 2001-2023  Institut Pasteur

  This program is part of the golden software.

  This program  is free software:  you can  redistribute it  and/or modify it  under the terms  of the GNU
  General Public License as published by the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the  GNU General Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

  Contact:

   Veronique Legrand                                                           veronique.legrand@pasteur.fr

 */
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
#define off_t uint64_t
#endif

#ifndef DATADIR
#define DATADIR "/usr/local/share"PACKAGE
#endif

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

// #define LOCK_DEBUG
#include <time.h>

#define MAX_IDX_READ 10240 // maximum number of indix_t structures that can be read from an index file.


#define DEBUG
/* Functions prototypes */

static int index_move(const char *, const char *);


/*
  Debug utility : prints content of data structure used for work (array of addresses of result_t structures).
 */
void print_wrk_struct(result_t ** lst_work, int nb_cards, int missing_only) {
    int i;
    result_t * cur_res;
    printf("\nlst_work content :");
    i=0;
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
#ifdef DEBUG
  if ((p->filenb==120) || (q->filenb==120)) {
    printf("comparing : %s, %u, %lld",p->name,p->filenb,p->offset);
    printf(" with : %s, %u, %lld\n",q->name,q->filenb,q->offset);
  }
#endif
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



void index_sort(char *file, uint64_t nb) {
  // FILE *g;
  int fd;
  const char *dir;
  indix_t * old;
  void * fmap, *fmap_orig;
  struct stat statbuf;  
  int result;
  off_t length;
  indix_t * old2;
#ifdef DEBUG
  uint64_t i;
#endif

  if (nb == 0) return;
  if ((dir = getenv("TMPDIR")) == NULL) { dir = TMPDIR; }

  if (access(file, F_OK) != 0) {
      err(errno, "file doesn't exist : %s",file);
      exit(1);
  }
  // figure out how big it is
  result = stat (file, &statbuf);
  if (result == -1) err(errno, "Cannot stat file : %s",file);
  length = statbuf.st_size;
  // if ((g = fopen(file, "r")) == NULL) err(errno, "Cannot open file : %s",file);
  if ((fd=open(file,O_RDWR))==-1) err(errno, "Cannot open file : %s",file);
  // mmap it
  fmap = (void *) mmap (NULL, (size_t) length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (fmap == NULL) err(errno, "Cannot mmap file : %s",file);
  fmap_orig=fmap;
  fmap=fmap+sizeof(uint64_t);
  old=(indix_t *) fmap;

#ifdef DEBUG
  old2=old;
  printf("AC/locus before sort:\n.");
  for (i=0;i<nb;i++) {
    printf("%s, %u, %lld\n",old2->name,old2->filenb,old2->offset);
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
    printf("%s, %u, %lld\n",old->name,old->filenb,old->offset);
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
uint64_t index_concat(char *file, uint64_t nb, indix_t *ind) {
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

void set_lock(int fd,struct flock lck) {
  int res = fcntl(fd, F_SETLKW, &lck);
  if (res == -1) err(1, "lock failed on destination file.");
}

/*
 * Sets a write lock on a region of the file pointed by fd.
 * Region locked is delimited by l_start and l_len
 */
struct flock index_file_lock(int fd, off_t l_start, off_t l_len ) {
  struct flock lock_t;
  lock_t.l_whence = SEEK_SET;
  lock_t.l_start = l_start;
  lock_t.l_len = l_len;
  lock_t.l_type = F_WRLCK;
  set_lock(fd,lock_t);
  return lock_t;
}


void index_file_unlock(int fd, struct flock lock_t) {
  lock_t.l_type = F_UNLCK;
  set_lock(fd,lock_t);
}

/*
 * reads at most MAX_IDX_READ indexes from source index file, update their filenb and write
 * them to destination file.
 */
void index_append(int fd_d,int prev_nb_flat,uint64_t nb_to_read,int fd_s,indix_t * buf) {
  uint64_t cnt;
  indix_t * inx;
#ifdef DEBUG
  printf("index.c, index_append filenb will be incremented of : %d\n",prev_nb_flat);
#endif
  if (read(fd_s,buf, (ssize_t) nb_to_read*sizeof(indix_t)) == -1) err(errno,"Cannot read index from source file");
  inx=buf;
  for (cnt=0;cnt<nb_to_read;cnt++) {
    inx->filenb=inx->filenb+prev_nb_flat;
    inx++;
  }
  if (write(fd_d,buf, (size_t) nb_to_read*sizeof(indix_t)) != nb_to_read*sizeof(indix_t)) err(errno,"Cannot write index to destination file");
}

/* concatenates indexes that have already been written to a file by a previous call to goldin. */
uint64_t index_file_concat(int fd_d,int prev_nb_flat, uint64_t nb_idx, int fd_s, uint64_t prev_nb_idx) {
  // uint64_t totnb=prev_nb_idx;
  uint64_t totnb;
  struct flock lock_t; // lock used to perform the truncate operation
  struct flock lock_w; // lock to perform the writing in the "reserved" area of the destination file.
  struct stat s_dest, s_source;
  int res;
  size_t s_to_add;
  indix_t * buf=NULL;
  int nb;

#ifdef DEBUG
  printf("index.c, index_file_concat called with prev_nb_flat=%d\n",prev_nb_flat);
#endif
  lock_t=index_file_lock(fd_d,0,sizeof(prev_nb_idx)); // lock used to perform the truncate operation
  if (lseek(fd_d, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.acx","wgs_c");
  
  // re-read number of indexes before writing because this value may have been changed by another process since the last time we read it.
  if (read(fd_d, &totnb, sizeof(totnb)) != sizeof(totnb)) err(errno,"error reading destination index file");
#ifdef DEBUG
  printf("index.c, index_file_concat previous number of index=%lld new number of indexes=%lld \n",totnb,totnb+nb_idx);
#endif
  if (lseek(fd_d, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.acx","wgs_c");
  totnb+=nb_idx;
  if (write(fd_d,&totnb,sizeof(totnb))!=sizeof(totnb)) err(errno,"Cannot write index to destination file");
  res = fstat(fd_d, &s_dest);
  if (res == -1) err(1, "stat failed on destination file");

  res = fstat(fd_s, &s_source);
  if (res == -1) err(1, "stat failed on source file");

  s_to_add = (size_t) s_source.st_size-sizeof(nb_idx); // do not concatenate number of indexes in index file.
  if (lseek(fd_d, 0, SEEK_END) == -1) err(errno,"index_file_concat: error while getting at the end of dest index file.");
  res = ftruncate(fd_d, s_dest.st_size + s_to_add);
  if (res == -1 && S_ISREG(s_dest.st_mode)) err(1, "Truncate failed");
  
#ifdef DEBUG
  printf("index.c, index_file_concat locking destination index file from octet : %lld to octet: %lld for writing\n",s_dest.st_size,s_dest.st_size+s_source.st_size-sizeof(nb_idx));
#endif
  
  lock_w=index_file_lock(fd_d,s_dest.st_size,s_source.st_size-sizeof(nb_idx));
  
  index_file_unlock(fd_d,lock_t);

  nb=(int) nb_idx;
  if (buf==NULL) buf=malloc(MAX_IDX_READ*sizeof(indix_t));
  while (nb-MAX_IDX_READ>0) {
   index_append(fd_d,prev_nb_flat,MAX_IDX_READ,fd_s,buf);
   nb=nb-MAX_IDX_READ;
  }
  index_append(fd_d,prev_nb_flat,nb,fd_s,buf);
  free(buf);
  index_file_unlock(fd_d,lock_w);
  return totnb;
}


/* Merge indexes */
int index_merge(char *file, uint64_t nb, indix_t *ind) {
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
    if (fread(&old, sizeof(old), 1, g) != 1) err(errno,"Cannot read indexes from file %s : %s",__func__,file);

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
  int i;
  size_t ret;

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
  if (fread(&cur1, sizeof(cur1), 1, g) != 1) err(errno,"Cannot read indexes from file %s: %s",__func__,fic);
  newnb=0;

  while(oldnb-1) {
    ret=fread(&cur2, sizeof(cur2), 1, g);
    if (ret != 1) err(errno,"Cannot read indexes from file %s: %s",__func__,fic);
    i=index_compare(&cur1,&cur2);
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









