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

// #include <mcheck.h>


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
static char *index_temp(const char *);
static int index_move(const char *, const char *);
static int index_compare(const void *, const void *);
static uint64_t iswap64(uint64_t);
static uint32_t iswap32(uint32_t);

// initializes the structure where we store info about the AC and locus that were not found.
void initArrayOfResAddr(ArrayOfResAddr * const ia_arr) {
  ia_arr->addrArray=NULL;
  ia_arr->arrSize=0;
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
int index_search(char *file, char * db_name, result_t ** lst, int lst_size, result_t *** const const lst_notFound , int * nb_res_found) {
  FILE *f;
  int i, swap;
  uint64_t indnb;
  long min, cur, max;
  off_t pos, chk;
  size_t len;
  struct stat st;
  indix_t inx;
  int nb_not_found;

  nb_not_found=0;
  if (lst==NULL) return 0;
  if (*lst==NULL) {
	  error_fatal("index_search : bad list in input", NULL);
  }

  if (stat(file, &st) == -1) {
    error_fatal(file, NULL); }

#ifdef DEBUG
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

  char * cur_db=(*lst)->dbase;
  int idx=0;

  while ((idx<lst_size) && (strcmp((*lst)->dbase,cur_db)==0))
  {
	 char * name= (*lst)->name;
#ifdef DEBUG
	 printf("name : %s\n",name);
#endif
	 len = strlen(name);
	 if (len > NAMLEN) {
	    error_fatal(name, "name too long");
	 }

	 min = 0; max = (long)indnb - 1;
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
	   	/* Not found */
	 if (i != 0) {
	    if ((lst_notFound==NULL) || ((*lst_notFound)==NULL)) {
	    	// should never happen, raise exc. an adress were to store the list of entries that were not found must be provided by the caller.
	    	error_fatal("mis-use of index_search, you should provide an adress in memory to list of entries that ere not found.",NULL);
	   	}
	   	else {
	   		nb_not_found++;
	   		result_t ** aux_lst_notFound=*lst_notFound; // for readability
	   		aux_lst_notFound=realloc(aux_lst_notFound, nb_not_found*sizeof(result_t *));
	   		if (aux_lst_notFound==NULL) {
	   			 error_fatal("index_search: memory", NULL);
	   		}
	   		aux_lst_notFound[nb_not_found-1]=*lst;
	   		*lst_notFound=aux_lst_notFound; // adress value aux_lst_notFound may have changed following realloc.
#ifdef DEBUG
	   		printf("index_search: Adding %s to aux_lst_notFound",((*lst)->name));
#endif
	   	}

	 } else {
	   	(*lst)->filenb = inx.filenb;
	   	(*lst)->offset = inx.offset;
	   	printf("index_search, setting db_name to : %s", db_name);
	   	(*lst)->real_dbase=strdup(db_name);
	   	if (swap == 1) {
	   		(*lst)->filenb = iswap32((*lst)->filenb);
	   		(*lst)->offset = iswap64((*lst)->offset); }
	   	*nb_res_found=*nb_res_found+1;
	 }
	 ++lst;
	 ++idx;
  }
  if (fclose(f) == EOF) {
    error_fatal(file, NULL); }
#ifdef DEBUG
	 printf("\n index_search, closed file : %s, for cur_db : %s, nb_not_found=%d, nb_res_found=%d \n",file,cur_db,nb_not_found,*nb_res_found);
	 printf("lst_notFound content :\n");
	 int j=0;
	 result_t * cur_res;
	 while (j<nb_not_found) {
	  cur_res=(*lst_notFound)[j];
	  printf("%s:%s ",cur_res->dbase,cur_res->name);
	  j++;
  }
#endif
  return nb_not_found;}


/* Merge indexes */
int index_merge(char *file, long nb, indix_t *ind) {
  FILE *f, *g;
  int i;
  char *new;
  const char *dir;
  uint64_t newnb, oldnb, totnb;
  indix_t *cur, old;

  if ((dir = getenv("TMPDIR")) == NULL) { dir = TMPDIR; }

  /* Sort indexes */
  qsort(ind, (size_t)nb, sizeof(*ind), index_compare);
  newnb = nb; totnb = 0; cur = ind;

  /* Create missing empty index file */
  if (access(file, F_OK) != 0) {
    if ((g = fopen(file, "w")) == NULL) {
      error_fatal(file, NULL); }
    if (fwrite(&totnb, sizeof(totnb), 1, g) != 1) {
      error_fatal(file, NULL); }
    if (fclose(g) == EOF) {
      error_fatal(file, NULL); }
  }

  if (nb == 0) { return 0; }

  /* Make new file */
  if ((new = index_temp(dir)) == NULL) {
    error_fatal(dir, "cannot create temporary file"); }
  if ((f = fopen(new, "w")) == NULL) {
    error_fatal(new, NULL); }
  if (fwrite(&totnb, sizeof(totnb), 1, f) != 1) {
    error_fatal(new, NULL); }

  /* Process old index file & merge with new */
  if ((g = fopen(file, "r")) == NULL) {
    error_fatal(file, NULL); }
  if (fread(&oldnb, sizeof(oldnb), 1, g) != 1) {
    error_fatal(file, NULL); }
  while(oldnb) {
    if (fread(&old, sizeof(old), 1, g) != 1) {
      error_fatal(file, NULL); }

    /* Insert new entries */
    while(newnb && (i = index_compare(cur, &old)) < 0) {
      if (fwrite(cur, sizeof(*cur), 1, f) != 1) {
	error_fatal(new, NULL); }
      newnb--; cur++; totnb++; }

    /* Update existing entries */
    if (newnb && i == 0) {
      if (fwrite(cur, sizeof(*cur), 1, f) != 1) {
	error_fatal(new, NULL); }
      newnb--; oldnb--; cur++; totnb++; continue; }

    /* Write old entry */
    if (fwrite(&old, sizeof(old), 1, f) != 1) {
      error_fatal(new, NULL); }
    oldnb--; totnb++; }

  if (fclose(g) == EOF) {
    error_fatal(file, NULL); }

  /* Add new remaining indexes */
  while(newnb) {
    if (fwrite(cur, sizeof(*cur), 1, f) != 1) {
      error_fatal(new, NULL); }
    newnb--; cur++; totnb++; }

  /* Write entries number */
  if (fseeko(f, 0, SEEK_SET) == -1) {
    error_fatal(new, NULL); }
  if (fwrite(&totnb, sizeof(totnb), 1, f) != 1) {
    error_fatal(new, NULL); }

  if (fclose(f) == EOF) {
    error_fatal(new, NULL); }

  /* Rename file */
  if (index_move(file, new) != 0) {
    error_fatal(new, NULL); }

  free(new);

  return 0; }


/* Generate temporary index filename */
static char *index_temp(const char *dir) {
  char *tmp;
  size_t len;

  len = strlen(dir) + 1 + 4 + 6;
  if ((tmp = (char *)malloc(len+1)) == NULL) {
    error_fatal("memory", NULL); }
  (void)sprintf(tmp, "%s/goldXXXXXX", dir);
  if (mktemp(tmp) != NULL) { return tmp; }
  free(tmp);

  return NULL; }


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


/* Compare indexes by names */
static int index_compare(const void *a, const void *b) {
  const indix_t *p, *q;

  p = (const indix_t *)a; q = (const indix_t *)b;

  return strncmp(p->name, q->name, (size_t)NAMLEN); }


/* Swap values ... */
static uint64_t iswap64(uint64_t val) {
  return ((val << 56) & 0xff00000000000000ULL) |
         ((val << 40) & 0x00ff000000000000ULL) |
         ((val << 24) & 0x0000ff0000000000ULL) |
         ((val <<  8) & 0x000000ff00000000ULL) |
         ((val >>  8) & 0x00000000ff000000ULL) |
         ((val >> 24) & 0x0000000000ff0000ULL) |
         ((val >> 40) & 0x000000000000ff00ULL) |
         ((val >> 56) & 0x00000000000000ffULL); }

static uint32_t iswap32(uint32_t val) {
  return ((val << 24) & 0xff000000) |
         ((val <<  8) & 0x00ff0000) |
         ((val >>  8) & 0x0000ff00) |
         ((val >> 24) & 0x000000ff); }
