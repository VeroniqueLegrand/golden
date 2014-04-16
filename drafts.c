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
/*
 Dump all indexes to the file given in input.
 If file doesn't exist, it is created.
 If mode is set to APPEND_INDEXES, new indexes are added at the end of the file.
 If mode is set to REPLACE_INDEXES, file is erased, only new indexes will be in it.
 If mode is set to MERGE_INDEXES, new indexes are merged with old ones.
    Prerequisite: old indexes must have been sorted previously.
    This is the default behavior that was in previous goldin version.
 */
int index_dump(char *dbase, int mode, all_indix_t file_l_indix,char * SUF, const char * index_dir) {
  char *file;
  char o_flg[3];
  FILE *f;
  int i;
  uint64_t nb_idx=0;
  long nb;
  indix_t *ind;

  if (strcmp(SUF,LOCSUF)==0) {
    nb=file_l_indix.locnb;
    ind=file_l_indix.l_locind;
  } else {
    nb=file_l_indix.accnb;
    ind=file_l_indix.l_accind;
  }

  file = index_file(index_dir, dbase, SUF);
  if (mode==MERGE_INDEXES) return index_merge(file,nb,ind);

  if (mode==APPEND_INDEXES) strcpy(o_flg,"a");
  else strcpy(o_flg,"w");

  if((access(file,F_OK) != -1 ) && (o_flg[0]=='a')) {
    strcpy(o_flg,"r+");
  }

  if ((f = fopen(file, o_flg)) == NULL) err(errno,"cannot open file: %s.",file);
  if (o_flg[0]=='r') {
    if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.",file);
    if (fread(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error reading number of indexes");
    if (fseeko(f, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.",file);
  } else if (fwrite(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error initializing number of indexes");

  i=0;

  while(i<nb) {
    if (fwrite(ind, sizeof(*ind), 1, f) != 1) {
      err(errno,"error writing index"); }
    i++;
    ind++;
  }

  // udate number of indexes in file.
  if (mode==APPEND_INDEXES) nb_idx+=nb;
  if (mode==REPLACE_INDEXES) nb_idx=nb;

  if (fseeko(f, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.",file);
  if (fwrite(&nb_idx, sizeof(nb_idx), 1, f) != 1) err(errno,"error writing number of indexes");

  if (fclose(f) == EOF)err(errno,"cannot close file: %s.",file);
  return 0;
}

