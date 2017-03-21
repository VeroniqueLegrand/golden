/* entry.c - Databases entries functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#include <ctype.h>
#include <unistd.h>

#include "error.h"
#include "entry.h"

#ifndef HAVE_FSEEKO
#define ftello ftell
#define fseeko fseek
#define off_t uint64_t
#endif

/* Buffers values */
#define BUFINC 100
#define BUFMAX (1024 * 1024)


/* Parse database flat file entry */
int entry_parse(FILE *f, entry_t *ent) {
  int i, n;
  char *p, *q, *buf;
  int buflen;

  buflen = BUFINC;
  if ((buf = (char *)malloc((size_t)buflen+1)) == NULL) {
    error_fatal("memory", NULL); }

  *ent->locus = *ent->access = '\0'; ent->offset = -1;
  i = 0;
  while(fgets(buf, buflen, f) != NULL) {

    /* Check for long line */
    if (feof(f) == 0 && strrchr(buf, '\n') == NULL) {
      buflen += BUFINC;
      if (buflen >= BUFMAX) {
	error_fatal(buf, "line too long"); }
      if ((buf = (char *)realloc(buf, (size_t)buflen+1)) == NULL) {
	error_fatal("memory", NULL); }
      if (fseeko(f, -1 * (off_t)strlen(buf), SEEK_CUR) == -1) {
	error_fatal("file", NULL); }
      continue; }

    /* Checks for entry end */
    if (*buf == '/' && *(buf+1) == '/') {
      i = 0; break; }

    /* Checks for locus line (entry offset) */
    if ((*buf == 'I' && strncmp(buf, "ID ", 3) == 0) ||
        (*buf == 'L' && strncmp(buf, "LOCUS ", 6) == 0) ||
        (*buf == 'E' && strncmp(buf, "ENTRY ", 6) == 0)) {
      if (i++ > 0) { error_warn(ent->locus, "truncated entry"); }
      ent->offset = ftello(f) - strlen(buf);
      p = buf; q = ent->locus; n = NAMLEN;
      while(*p && !isspace((int)*p)) { p++; }
      while(*p && isspace((int)*p)) { p++; }
      while(*p && *p != ';' && !isspace((int)*p) && n--) { *q++ = *p++; }
      *q = '\0';
      if (n < 0 && *p != ';' && !isspace((int)*p)) {
	error_fatal("locus", "name too long"); }
      continue; }

    /* Checks for primary accession */
    if (*buf == 'A' && (strncmp(buf, "AC ", 3) == 0 ||
                        strncmp(buf, "ACCESSION ", 10) == 0 ||
                        strncmp(buf, "ACCESSIONS ", 11) == 0)) {
      p = buf; q = ent->access; n = NAMLEN;
      if (*q != '\0') { continue; }
      while(*p && !isspace((int)*p)) { p++; }
      while(*p && isspace((int)*p)) { p++; }
      while(*p && *p != ';' && !isspace((int)*p) && n--) { *q++ = *p++; }
      *q = '\0';
      if (n < 0 && *p != ';' && !isspace((int)*p)) {
	error_fatal("accession", "name too long"); }
      continue; }

  }

  free(buf);

  if (feof(f) && ent->offset == -1) {
    return 1; }

  return 0; }


/* Display database flat file entry */
int entry_display(FILE *f, int fd) {
  static char *buf;
  int len;
  size_t read_len;
  int didnt_read_anything;

  len = BUFINC;
  if ((buf = (char *)malloc((size_t)len+1)) == NULL) {
    error_fatal("memory", NULL); }
    didnt_read_anything=1;
  while(fgets(buf, len, f) != NULL) {
      didnt_read_anything=0;
    /* Print entry line */
    read_len=strlen(buf);
    write(fd,buf,read_len);
    /* Checks for entry end */
    if (*buf == '/' && *(buf+1) == '/') { break; }
  }

  free(buf);
    if (didnt_read_anything) error_fatal("couldn't read anything from flat file; offset indicated in index is probably wrong.","It is likely that golden indexes are broken");

  return 0; }



