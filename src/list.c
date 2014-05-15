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
 */
int list_append(char *dbase, char *dir, char *files,char * new_index_dir) {
  FILE *f;
  int nb;
  char *p, *q, *buf, *name;
  size_t len;
  char * file;
  char * cp_files=strdup(files);

  nb = 0;
  name = index_file(new_index_dir, dbase, LSTSUF);

  if (access(name, F_OK) != -1) {
    if ((f = fopen(name, "r+")) == NULL) err(errno,"Cannot open file : %s",name);
    len = BUFINC;
    if ((buf = (char *)malloc(len+1)) == NULL) err(errno,"memory");
    while(fgets(buf, (int)len, f) != NULL) {
      /* Checks for long line */
      if ((p = strrchr(buf, '\n')) == NULL) {
        len += BUFINC;
        if ((buf = (char *)realloc(buf, len+1)) == NULL) err(errno, "memory");
        if (fseeko(f, -1 * (off_t)strlen(buf), SEEK_CUR) != 0) err(errno,"error seeking in file : %s", name);
        continue;
      }
      /* Checks for existing file */
      *p = '\0';
      file=strtok(cp_files,"\n");
      while (file!=NULL) {
        q = file; if ((p = strrchr(q, '/')) != NULL) q = ++p;
        if (strcmp(buf, q) == 0) warn("duplicate file in database : %s",q);
        file=strtok(NULL,"\n");
      }
      nb++;
    }
    free(buf);
    /* Append new files to list */
    strcpy(cp_files,files);
    file=strtok(cp_files,"\n");
    while (file!=NULL) {
      q = file; if ((p = strrchr(q, '/')) != NULL) q = ++p;
      if (dir!=NULL) (void)fprintf(f, "%s/%s\n", dir, q);
      else (void)fprintf(f, "%s\n", q);
      nb++;
      file=strtok(NULL,"\n");
    }
    if (fclose(f) == EOF) error_fatal(name, NULL);
  } else {
    if ((f = fopen(name, "a")) == NULL) err(errno,"Cannot open file : %s",name);
    /* Append new files to list */
    strcpy(cp_files,files);
    file=strtok(cp_files,"\n");
    while (file!=NULL) {
      q = file; if ((p = strrchr(q, '/')) != NULL) q = ++p;
      if (dir!=NULL) (void)fprintf(f, "%s/%s\n", dir, q);
      else (void)fprintf(f, "%s\n", q);
      nb++;
      file=strtok(NULL,"\n");
    }
    if (fclose(f) == EOF) error_fatal(name, NULL);
  }
  free(name);
  free(cp_files);
  return (nb);
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
  return lst_to_concat;
}
