/*
 * index_meta.c
 *
 *  Created on: Apr 3, 2014
 *      Author: vlegrand
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include "index.h"
#include "list.h"
#include "index_desc.h"

#define BUFINC 100

/* Initialize data structure*/
void init_sidx_desc(source_index_desc * p_sdesc) {
  p_sdesc->d_facx=NULL; // for .acx files
  p_sdesc->d_ficx=NULL; // for .icx files
  p_sdesc->accnb=0;
  p_sdesc->locnb=0;
  p_sdesc->max_filenb=0;
}

void init_didx_desc(dest_index_desc * p_ddesc) {
  p_ddesc->d_facx=NULL; // for .acx files
  p_ddesc->d_ficx=NULL; // for .icx files
  p_ddesc->d_fdbx=NULL; // for .dbx files.
  p_ddesc->accnb=0;
  p_ddesc->locnb=0;
  p_ddesc->max_filenb=0;
}


fic_index_desc get_ficidx_desc(char *cur_index_dir, char *dbase,char* suff, char *opn_mode, int create_flg) {
  char * idx_file;
  fic_index_desc cur_idx;
  int ret;
  uint64_t indnb;
  if ((strcmp(opn_mode,"r")!=0) && (strcmp(opn_mode,"r+")!=0)) err(0,"opn_mode must be \"r\" or \"r+\"");
  cur_idx.d_fidx=NULL;
  cur_idx.idxnb=0;
  if (strcmp(suff,ACCSUF)==0) idx_file=index_file(cur_index_dir,dbase,ACCSUF);
  else if (strcmp(suff,LOCSUF)==0) idx_file=index_file(cur_index_dir,dbase,LOCSUF);

  ret=access(idx_file, F_OK);
  if (( ret!= 0) && create_flg) create_missing_idxfile(idx_file);
  else if (ret!=0) err(errno,"%s should exist", idx_file);

  if ((cur_idx.d_fidx = fopen(idx_file, opn_mode)) == NULL) err(errno,"error opening file : %s", idx_file);
  if (fread(&indnb, sizeof(indnb), 1, cur_idx.d_fidx) != 1) err(errno,"error reading file : %s", idx_file);
  cur_idx.idxnb=indnb;
  return cur_idx;
}

void close_source_index_desc(source_index_desc * p_sdesc) {
  if (fclose(p_sdesc->d_ficx) == EOF) err(errno,"error closing source locus index file.");
  if (fclose(p_sdesc->d_facx) == EOF) err(errno,"error closing source AC index file.");
  init_sidx_desc(p_sdesc);
}

source_index_desc get_source_index_desc(int acc,int loc,char * s_index_dir, char * dbase) {
  char *acx_file, *icx_file, *dbx_file;
  uint64_t indnb;
  source_index_desc s_idx;
  fic_index_desc w_idx;
  int ret;
  size_t len;
  char * buf, *p;
  FILE * dbx_fd;

  init_sidx_desc(&s_idx);
  s_idx.max_filenb=list_nb(s_index_dir, dbase);
  if (acc) {
      w_idx=get_ficidx_desc(s_index_dir, dbase, ACCSUF, "r", 0);
      s_idx.accnb=w_idx.idxnb;
      s_idx.d_facx=w_idx.d_fidx;
    }
  if (loc) {
    w_idx=get_ficidx_desc(s_index_dir, dbase, LOCSUF, "r", 0);
    s_idx.locnb=w_idx.idxnb;
    s_idx.d_ficx=w_idx.d_fidx;
  }
  return  s_idx;
}

dest_index_desc get_dest_index_desc(int acc,int loc,char * new_index_dir, char * dbase) {
  char *acx_file, *icx_file, *dbx_file;
  uint64_t indnb;
  dest_index_desc d_idx;
  fic_index_desc w_idx;
  int ret;
  size_t len;
  char * buf, *p;
  FILE * dbx_fd;

  init_didx_desc(&d_idx);
  if (acc) {
     w_idx=get_ficidx_desc(new_index_dir, dbase, ACCSUF, "r+", 1);
     d_idx.accnb=w_idx.idxnb;
     d_idx.d_facx=w_idx.d_fidx;
  }
  if (loc) {
    w_idx=get_ficidx_desc(new_index_dir, dbase, LOCSUF, "r+", 1);
    d_idx.locnb=w_idx.idxnb;
    d_idx.d_ficx=w_idx.d_fidx;
  }
  dbx_file=index_file(new_index_dir,dbase,LSTSUF);
  ret=access(dbx_file, F_OK);
  if (ret!=0) {
    list_new(dbx_file);
    if ((d_idx.d_fdbx = fopen(dbx_file, "r+")) == NULL) err(errno,"error opening file : %s", dbx_file);
  } else {
    if ((d_idx.d_fdbx = fopen(dbx_file, "r+")) == NULL) err(errno,"error opening file : %s", dbx_file);
    len = BUFINC;
    if ((buf = (char *)malloc(len+1)) == NULL) err(errno,"memory");
    while(fgets(buf, (int)len, d_idx.d_fdbx) != NULL) {
      /* Checks for long line */
      if ((p = strrchr(buf, '\n')) == NULL) {
        len += BUFINC;
        if ((buf = (char *)realloc(buf, len+1)) == NULL) err(errno, "memory");
        if (fseeko(d_idx.d_fdbx, -1 * (off_t)strlen(buf), SEEK_CUR) != 0) err(errno,"error seeking in file : %s", dbx_file);
          continue;
      }
      d_idx.max_filenb++;
    }
    free(buf);
  }
  return d_idx;
}

void close_dest_index_desc(dest_index_desc * p_ddesc) {
  if (fclose(p_ddesc->d_ficx) == EOF) err(errno,"error closing destination locus index file");
  if (fclose(p_ddesc->d_facx) == EOF) err(errno,"error closing destination AC index file");
  if (fclose(p_ddesc->d_fdbx) == EOF) err(errno,"error closing destination LST file");
  init_didx_desc(p_ddesc);
}
