/* list.c - Database files list functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <err.h>
#include <fcntl.h>

#include "error.h"
#include "index.h"
#include "list.h"

#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t long
#endif

#define BUFINC 100
// #define LOCK_DEBUG
#include <time.h>

/*
 * Acquires or releases (depending on l_type value) an exclusive write lock on list file.
 * l_type value is either F_WRLCK or F_UNLCK.
 */
void set_list_lock(int fd, int l_type) {
  struct flock excl_wr_lock; // exclusive write lock.
  excl_wr_lock.l_type=l_type; //F_WRLCK;
  excl_wr_lock.l_whence=SEEK_SET;
  excl_wr_lock.l_start=0;
  excl_wr_lock.l_len=0;
  int res=fcntl(fd,F_SETLKW,&excl_wr_lock);
  if (res==-1) err(errno,"list_lock failed");
}


void list_lock(int fd) {
#ifdef LOCK_DEBUG
  clock_t start_t, stop_t, total_t;
  printf("Going to try to lock list file for writing.\n");
  start_t=clock();
#endif
  set_list_lock(fd,F_WRLCK);
#ifdef LOCK_DEBUG
  stop_t=clock();
  total_t=(double) (stop_t -start_t) / CLOCKS_PER_SEC;
  printf("list file locked for writing : %lu.\n",total_t);
#endif
}

void list_unlock(int fd) {
#ifdef LOCK_DEBUG
  clock_t start_t, stop_t, total_t;
  printf("Going to try to unlock list file for writing.\n");
  start_t=clock();
#endif
  set_list_lock(fd,F_UNLCK);
#ifdef LOCK_DEBUG
  stop_t=clock();
  total_t=(double) (stop_t -start_t) / CLOCKS_PER_SEC;
  printf("list file unlocked for writing : %lu.\n",total_t);
#endif
}

/* Append database flat file list && return file nb
 * The files argument is a list of file names separated by \n.
 * Parameters :
 * dir : as usual, the alternate dir where to find the .dat files.
 * dbase : base name for the bank; useful to retrieve index file name.
 * files : list of dbx files which content must be appended to existing .dbx file.
 * new_index_dir : location for index files. In previous versions of goldin/golden,
 * they were always generated in the execution directory but I do not find this 
 * very practical for unit testing and other kind of testing as well. 
 * Just put "." if you want previous behavior.
 * buf : a buffer to store content of the .dbx file before appending data to it.
 */
int list_append(char *dbase, char *dir, char *files,char * new_index_dir, char **buf) {
  // FILE *f;
  struct stat st;
  int f,nb_read;
  int nb;
  char *p, *q,*name;
  
  char * new_file, *old_file;
  char * l_files;
  char ** arr_buf=NULL;
  int cnt_fic;

  char * l_buf=*buf;
  nb = 0;
  name = index_file(new_index_dir, dbase, LSTSUF);
  f = open(name, O_RDWR|O_APPEND|O_CREAT, 0666);
  if (f == -1) err(errno,"Cannot open file : %s",name);
  
  if (stat(name, &st) == -1) err(errno,name, NULL);
  list_lock(f);

  //len=st.st_size;
  if ((l_buf = (char *)realloc(l_buf,st.st_size+1)) == NULL) err(errno,"memory");
  *buf=l_buf;
  if ((nb_read=read(f,l_buf,st.st_size))!=st.st_size) err(errno,"Error while reading list file.");
  l_buf[st.st_size]='\0';
  // printf("list_nb : read \n%s\n from existing file.\n",buf);
  char * a_fic=strtok(l_buf,"\n");
  while (a_fic!=NULL) {
    // printf("list_nb, a_fic=%s\n",a_fic);
    nb++;
    arr_buf=realloc(arr_buf,nb*sizeof(char *));
    arr_buf[nb-1]=a_fic;
    a_fic=strtok(NULL,"\n");
  }
  
  /* Checks for duplicate file */
  for (cnt_fic=0;cnt_fic<nb;cnt_fic++) {
    old_file=arr_buf[cnt_fic];
    l_files=strdup(files);
    new_file=strtok(l_files,"\n");
    while (new_file!=NULL) {
      if ((p = strrchr(new_file, '/')) != NULL) ++p;
      else p=new_file;
      if ((q = strrchr(old_file, '/')) != NULL) ++q;
      else q=old_file;
      if (strcmp(p, q) == 0) warn("duplicate file in database : %s",q);
      new_file=strtok(NULL,"\n");
    }
    free(l_files);
  }

  // printf("list_append : before adding files, nb= %d \n",nb);
  /* Add new files (even if they are duplicate) */
  l_files=strdup(files);
  // printf("going to add files : %s\n",l_files);
  new_file=strtok(l_files,"\n");
  while (new_file!=NULL) {
    // printf("strtok returned : %s \n",new_file);
    q = new_file; if ((p = strrchr(q, '/')) != NULL) q = ++p;
    if (dir!=NULL) {
      int tmp=write(f,dir,strlen(dir));
      tmp=write(f,"/",1);
    }
    // printf("going to write : %s , len: %ld to dbx file\n",q,strlen(q));
    write(f,q,strlen(q));
    write(f,"\n",1);
    new_file=strtok(NULL,"\n");
    nb++;
  }
  free(l_files);
  list_unlock(f);
  if (close(f)!=0) err(errno,"Error while closing file : %s",name);
  free(name);
  // free(buf);
  // buf=NULL;
  free(arr_buf);
  // printf("list_append : going to return nb=%d \n",nb);
  return nb;
}



/* Get database list flat file name */
char *list_name(char *dbase, int nb) {
  FILE *f;
  const char *p;
  char *q, *file, *buf, *name;
  size_t len;

  file = index_file(NULL, dbase, LSTSUF);

  if ((f = fopen(file, "r")) == NULL)
    error_fatal(file, NULL);
  len = BUFINC;
  if ((buf = (char *)malloc(len+1)) == NULL)
    error_fatal("memory", NULL);

  while(fgets(buf, (int)len, f) != NULL) {

    /* Checks for long line */
    if ((q = strrchr(buf, '\n')) == NULL) {
      len += BUFINC;
      if ((buf = (char *)realloc(buf, len+1)) == NULL)
	error_fatal("memory", NULL);
      if (fseeko(f, -1 * (off_t)strlen(buf), SEEK_SET) != 0)
	error_fatal(file, NULL);
      continue; }
    *q = '\0';

    /* Checks for number */
    if (--nb) continue;
    break; }

  if (fclose(f) == EOF)
    error_fatal(file, NULL);

  free(file);

  p = index_dir();

  /* Be compatible with old list format (without dir prefix) */
  if (strchr(buf, '/') == NULL) {
    len = strlen(p) + 1 + strlen(dbase) + 1 + strlen(buf);
    if ((name = (char *)malloc(len+1)) == NULL)
      error_fatal("memory", NULL);
    (void)sprintf(name, "%s/%s/%s", p, dbase, buf); }
  else {
    len = strlen(p) + 1 + strlen(buf);
    if ((name = (char *)malloc(len+1)) == NULL)
      error_fatal("memory", NULL);
    (void)sprintf(name, "%s/%s", p, buf); }

  free(buf);

  return name; }


/* Checks for available databases in index directory */
int list_check(void) {
  const char *p;
  char *q;
  int nb;
  DIR *d;
  struct dirent *dir;

  nb = 0;
  p = index_dir();
  if ((d = opendir(p)) == NULL)
    error_fatal(p, NULL);
  while((dir = readdir(d)) != NULL) {
    if ((q = strrchr(dir->d_name, '.')) == NULL) continue;
    if (strcmp(q+1, LSTSUF) != 0 && strcmp(q+1, VIRSUF) !=0) continue;
    *q = '\0'; nb++;
    (void)fprintf(stdout, "%s\n", dir->d_name); }
  if (nb == 0) {
    (void)fprintf(stdout, "No available databases.\n"); }
  if (closedir(d) == -1)
    error_fatal(p, NULL);

  return 0; }

/*
 * Returns the number of flat files already indexed for dbase.
 * If dbx file doesn't exist, an error is displayed and program exits..
 */
int list_nb(char * new_index_dir, char * dbase) {
  char * dbx_file,* buf, *p;
  FILE * dbx_fd;
  int ret,len;
  int nb_flat=0;
  dbx_file=index_file(new_index_dir,dbase,LSTSUF);
  ret=access(dbx_file, F_OK);
  if (ret!=0) err(0,"%s file doesn't exist",dbx_file);
  if ((dbx_fd = fopen(dbx_file, "r+")) == NULL) err(errno,"error opening file : %s", dbx_file);
  len = BUFINC;
  if ((buf = (char *)malloc(len+1)) == NULL) err(errno,"memory");
  while(fgets(buf, (int)len, dbx_fd) != NULL) {
    /* Checks for long line */
    if ((p = strrchr(buf, '\n')) == NULL) {
      len += BUFINC;
      if ((buf = (char *)realloc(buf, len+1)) == NULL) err(errno, "memory");
      if (fseeko(dbx_fd, -1 * (off_t)strlen(buf), SEEK_CUR) != 0) err(errno,"error seeking in file : %s", dbx_file);
        continue;
    }
    nb_flat++;
  }
  free(buf);
  free(dbx_file);
  fclose(dbx_fd);
  return nb_flat;
}

/*
 * Creates new, empty dbx file.
 */
void list_new(char *file) {
  FILE *g;
  /* Create empty index file*/
  if ((g = fopen(file, "w")) == NULL) err(errno,"cannot create file : %s",file);
  if (fclose(g) == EOF) err(errno,"cannot close file : %s",file);
}

/*
 * Get all database flat file names.
 * Allocates memory.
 */
char * list_get(char * file) {
  struct stat st;
  int fd,nb_read;
  if (stat(file, &st) == -1) err(errno,file, NULL);
  int len=st.st_size;
  char * lst_to_concat= malloc(len+1);
  if ((fd=open(file,O_RDONLY))==-1) err(errno, "Cannot open source file.");
  if ((nb_read=read(fd,lst_to_concat,st.st_size))==-1) err(errno,"Error while reading source file.");
  close(fd);
  lst_to_concat[len]='\0';
  // printf("read : %s from file : %s \n",lst_to_concat,file);
  return lst_to_concat;
}
