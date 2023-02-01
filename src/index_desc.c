/*
 * index_meta.c
 *
 *  Created on: Apr 3, 2014
 *      Author: vlegrand
 */
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
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
// #include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include "index.h"
#include "list.h"
#include "index_desc.h"

#define BUFINC 100

/* Initialize data structure*/
void init_idx_desc(index_desc * p_sdesc) {
  p_sdesc->d_facx=-1; // for .acx files
  p_sdesc->d_ficx=-1; // for .icx files
  p_sdesc->accnb=0;
  p_sdesc->locnb=0;
  p_sdesc->max_filenb=0;
  // printf("printf call to init_idx_desc\n");
}



fic_index_desc get_ficidx_desc(char *cur_index_dir, char *dbase,char* suff, char *opn_mode, int create_flg) {
  char * idx_file;
  fic_index_desc cur_idx;
  int ret;
  uint64_t indnb;
  int flg;
  if ((strcmp(opn_mode,"r")!=0) && (strcmp(opn_mode,"r+")!=0)) err(0,"opn_mode must be \"r\" or \"r+\"");
  if (strcmp(opn_mode,"r")==0) flg=O_RDONLY;
  else flg=O_RDWR;
  cur_idx.d_fidx=-1;
  cur_idx.idxnb=0;

  idx_file=index_file(cur_index_dir,dbase,suff);
  ret=access(idx_file, F_OK);
  if (( ret!= 0) && create_flg) create_missing_idxfile(idx_file);
  else if (ret!=0) err(errno,"%s should exist", idx_file);

  if ((cur_idx.d_fidx = open(idx_file, flg)) == -1) err(errno,"error opening file : %s", idx_file);
  if (read(cur_idx.d_fidx, &indnb, sizeof(indnb)) != sizeof(indnb)) err(errno,"error reading file : %s", idx_file);
  cur_idx.idxnb=indnb;
  
  /*
  if (indnb>=1) {
    indix_t my_idx;
    if (read(cur_idx.d_fidx, &my_idx, sizeof(my_idx)) != sizeof(my_idx)) err(errno,"error reading file : %s", idx_file);
  }
  */

  free(idx_file);
  return cur_idx;
}


index_desc get_source_index_desc(int acc,int loc,char * s_index_dir, char * dbase) {
  index_desc s_idx;
  fic_index_desc w_idx;

  init_idx_desc(&s_idx);
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

index_desc get_dest_index_desc(int acc,int loc,char * new_index_dir, char * dbase) {
  index_desc d_idx;
  fic_index_desc w_idx;

  // printf("Call to get_dest_index_desc\n");
  // remove old index files.No! concatenation will be done by several different processes. Process N must not erase what process M has written!
  // index_hl_remove(acc,loc,new_index_dir,dbase);
  init_idx_desc(&d_idx);
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
  // dbx_file=index_file(new_index_dir,dbase,LSTSUF);
  // just create the file in fact.
  // list_new(dbx_file);
  // free(dbx_file);
  return d_idx;
}

void close_index_desc(index_desc * p_ddesc) {
  if (p_ddesc->d_ficx!=-1) {
    if (close(p_ddesc->d_ficx) == -1) err(errno,"error closing destination locus index file");
  }
  if (p_ddesc->d_facx!=-1) {
    if (close(p_ddesc->d_facx) == -1) err(errno,"error closing destination AC index file");
  }
  init_idx_desc(p_ddesc);
}
