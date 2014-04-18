/*
 * drafts.c
 *
 *  Created on: Apr 15, 2014
 *      Author: vlegrand
 *      Maybe I will nedd these stuff one day. just for remind, don't compile this file.
 */


/*
 * Returns information about index files.
 */


void update_index_desc(cur_index_descr * d_descr,int cnt_filenb) {
  d_descr->ficnb=cnt_filenb;
}

int index_dump(char *dbase, int mode, all_indix_t file_l_indix,char * SUF, const char * index_dir);

