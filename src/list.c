/* list.c - Database files list functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

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

#include "error.h"
#include "index.h"
#include "list.h"

#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t long
#endif

#define BUFINC 100

/* Append database flat file list && return file nb
 * The files argument is a list of file names separated by \n.*/
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
        if (strcmp(buf, q) == 0) warn("%s",q, "duplicate file in database");
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
      else (void)fprintf(f, "%s\n", dir, q);
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
