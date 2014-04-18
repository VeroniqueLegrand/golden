/*
This file is dedicated to unit testing.
Unit testing differs from the already existing tests in the "test" directory in the fact
that its aim is to test small functions (index_merge, index_sort etc) and not the whole tool (goldin).
It is lower level.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
//#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include "index.h"
#include "list.h"
#include "index_hl.h"
#include "index_desc.h"



char * data_file="../test/unit/enzyme_extract.dat";
char * acx_file="../test/unit/enzyme_extract_idx1.acx";
char * icx_file="../test/unit/enzyme_extract_idx1.idx";
char * icx_unsorted_file="../test/unit/enzyme_extract_unsorted_idx1.idx";
char * rac_filename="../test/unit/enzyme_extract_idx1";




int APPEND_INDEXES=1;
int MERGE_INDEXES=2;
int REPLACE_INDEXES=3;

// void test_index_sort();
all_indix_t test_index_create();

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


// utility method to copy a file before sorting it. Only used for unit tests.
void copy_file(char* fsource, char* fdest) {
  char mode[]="0777";
  int mod;
  struct stat st;
  int fd, fd_dest,ret;
  int nb_read;
  char * buf;
  mod=strtol(mode, 0, 8);
  if (stat(fsource, &st) != -1) {
    if (remove(fdest)==-1) err(errno, "Source file does not exist.");
  }
  if ((fd=open(fsource,O_RDONLY))==-1) err(errno, "Cannot open source file.");
  if ((fd_dest=open(fdest,O_WRONLY|O_CREAT))==-1) err(errno, "Cannot open destination file.");
  if ((buf=malloc(st.st_size))==NULL) err(errno, "Cannot allocate memory");
  if ((nb_read=read(fd,buf,st.st_size))==-1) err(errno,"Error while reading source file.");
  if (write(fd_dest,buf,nb_read)==-1) err(errno,"Error while writing dest file.");
  close(fd);
  close(fd_dest);
  if ((ret=chmod(fdest, mod)) == -1) err(errno,"Cannot set permissions.");
}

void test_index_sort() {
  char * tst_file="enzyme_test.idx";
  int i=0;
  
  all_indix_t t_idx=index_load("../test/unit","enzyme_extract_unsorted_idx1",LOCSUF);
  index_dump("enzyme_test",REPLACE_INDEXES,t_idx,LOCSUF,"../test/unit"); // creates tst_file
  index_sort(tst_file,21);
  // load test file and icx_file (already sorted), and check that both results are the same.
  all_indix_t t_idx3=index_load("../test/unit","enzyme_test",LOCSUF);
  assert(t_idx3.accnb==0);
  assert(t_idx3.locnb==21);
  
  all_indix_t t_idx_ref=index_load("../test/unit",rac_filename,LOCSUF);
  while (i<21) {
    assert(strcmp(t_idx_ref.l_locind[i].name,t_idx3.l_locind[i].name)==0);
    i++;
  }
}

void test_list_append() {
  char * expected_content="toto/titi/premier_fichier.dat\ntoto/titi/second_fichier.dat\ntoto/titi/troisieme_fichier.dat\n\
toto1.dat\ntiti/toto1.dat\n\
titi/toto1.dat\ntiti/toto2.dat\ntiti/toto3.dat\n";
  
  char * dbase="db_test_tmp";
  struct stat st;
  char * buf;
  int fd;

  list_new("../test/unit/new_tmp.dbx");
  assert(stat("../test/unit/new_tmp.dbx", &st) != -1);
  int n_nb=list_nb("../test/unit","new_tmp");
  assert(n_nb==0);

  //char * list_get(char * file)
  char * l_flat=list_get("../test/unit/new_tmp.dbx");
  assert(strcmp(l_flat,"")==0);
  free(l_flat);

  copy_file("../test/unit/db_test.dbx", "../test/unit/db_test_tmp.dbx");
  int nb=list_append(dbase,NULL,"toto1.dat","../test/unit");
  assert(nb==4);
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  n_nb=list_nb("../test/unit","db_test_tmp");
  assert(n_nb==4);
  
  nb=list_append(dbase,"titi","toto1.dat\n","../test/unit");
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  assert(nb==5);
  // TODO : check that we got a warning for duplicate toto1.dat
  nb=list_append(dbase,"titi","toto1.dat\ntoto2.dat\ntoto3.dat\n","../test/unit");
  assert(nb==8);
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  buf= malloc(st.st_size);
  fd=open("../test/unit/db_test_tmp.dbx",O_RDONLY);
  assert(read(fd,buf,st.st_size)!=-1);
  assert(strcmp(buf,expected_content)==0);
  close(fd);
  free(buf);
  
  l_flat=list_get("../test/unit/db_test_tmp.dbx");
  assert(strcmp(l_flat,expected_content)==0);
  free(l_flat);

  // check new dbx file creation for new indexes.
  nb=list_append("db_test_tmp2",NULL,"toto1.dat","../test/unit");
  assert(nb==1);
  assert(stat("../test/unit/db_test_tmp2.dbx", &st) != -1);

}

void test_index_merge() {
  // 1- mimic old behavior : merge 1 file with new indexes.
  // 2- test new behavior : merge several index files.
}


void create_idx_for_concat() {
  all_indix_t t_idx=create_index("../test/unit/wgs_extract.1.gnp",1,1,1);
  int ret=index_dump("wgs1",APPEND_INDEXES,t_idx, ACCSUF,".");
  ret=index_dump("wgs1",APPEND_INDEXES,t_idx, LOCSUF,".");
  freeAllIndix(t_idx);

  t_idx=create_index("../test/unit/wgs_extract.2.gnp",1,1,1);
  ret=index_dump("wgs2",APPEND_INDEXES,t_idx, ACCSUF,"../test/unit");
  ret=index_dump("wgs2",APPEND_INDEXES,t_idx, LOCSUF,"../test/unit");
  freeAllIndix(t_idx);

  t_idx=create_index("../test/unit/wgs_extract.3.gnp",1,1,1);
  ret=index_dump("wgs3",APPEND_INDEXES,t_idx, ACCSUF,"../test/unit");
  ret=index_dump("wgs3",APPEND_INDEXES,t_idx, LOCSUF,"../test/unit/");
  freeAllIndix(t_idx);
}

void test_index_desc(dest_index_desc* d_descr,source_index_desc* ls_descr) {
  int nb_source=3;// number of elements in ls_descr.
  int i;
  struct stat st;
  *d_descr=get_dest_index_desc(1,1,"../test/unit","wgs_c");
  assert(stat("../test/unit/wgs_c.acx", &st) != -1);
  assert(stat("../test/unit/wgs_c.icx", &st) != -1);
  assert(stat("../test/unit/wgs_c.dbx", &st) != -1);
  assert(d_descr->d_facx!=NULL);
  assert(d_descr->d_ficx!=NULL);
  assert(d_descr->d_fdbx!=NULL);
  assert(d_descr->max_filenb==0);
  assert(d_descr->locnb==0);
  assert(d_descr->accnb==0);
  ls_descr[0]=get_source_index_desc(1,1,"../test/unit","wgs1");
  ls_descr[1]=get_source_index_desc(1,1,"../test/unit","wgs2");
  ls_descr[2]=get_source_index_desc(1,1,"../test/unit","wgs2");

  for(i=0; i<nb_source; i++) {
    assert(ls_descr[i].d_facx!=NULL);
    assert(ls_descr[i].d_ficx!=NULL);
    assert(ls_descr[i].max_filenb=1);
  }

  assert(ls_descr[0].accnb=0);
  assert(ls_descr[0].locnb=2);

  assert(ls_descr[1].accnb=0);
  assert(ls_descr[1].locnb=5);

  assert(ls_descr[2].accnb=0);
  assert(ls_descr[2].locnb=3);
}

/*
 Concatenates several unsorted index files That may contain doublons.
 */
void test_index_concat(dest_index_desc* d_descr,source_index_desc* ls_descr,int nb_source) {
  int new_nb,i;
  char * s_dbx_file, *l_flats;
  char source_base[5];

  // concatenate
  for(i=0; i<nb_source; i++) {
    sprintf(source_base,"%s%d","wgs",i);
    s_dbx_file=index_file("../test/unit",source_base,LSTSUF);
    l_flats=list_get(s_dbx_file);
    new_nb = list_append("wgs_c",NULL,l_flats,"../test/unit");
    assert(new_nb=i+1);
    free(l_flats);

    if (fseeko(d_descr->d_facx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.acx","wgs_c");
    d_descr->accnb=fic_index_concat(d_descr->d_facx,d_descr->accnb, ls_descr[i].accnb, ls_descr[i].d_facx,ls_descr[i].max_filenb);

    if (fseeko(d_descr->d_ficx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.icx","wgs_c");
    d_descr->locnb=fic_index_concat(d_descr->d_ficx,d_descr->locnb, ls_descr[i].locnb, ls_descr[i].d_ficx,ls_descr[i].max_filenb);

    close_source_index_desc(&ls_descr[i]);
  }
  d_descr->max_filenb=new_nb;
  close_dest_index_desc(d_descr);

  // check resulting index file.
  int nb = list_nb("../test/unit","wgs_c.dbx");
  assert(nb==new_nb);
  all_indix_t wgs_idx=index_load("../test/unit","wgs_c",NULL);
  assert(wgs_idx.accnb==0);
  assert(wgs_idx.locnb==10);
  indix_t i0=wgs_idx.l_locind[0];
  indix_t i9=wgs_idx.l_locind[9];
  indix_t i4=wgs_idx.l_locind[4];
  assert(strcmp(i0.name,"EYS95825")==0);
  assert(i0.filenb==1);

  assert(strcmp(i0.name,"EYS68660")==0);
  assert(i4.filenb==2);

  assert(strcmp(i9.name,"EYS68660")==0);
  assert(i9.filenb==3);
}

all_indix_t test_index_create() {
  int nb;
  nb = list_append(data_file, data_file, data_file,".");
  all_indix_t t_idx=create_index(data_file,nb,0,1);
  // assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.accnb==0);
  assert(t_idx.locnb==0);

  t_idx=create_index(data_file,nb,1,0);
  // assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.accnb==0);
  assert(t_idx.locnb==21);
  indix_t idx=t_idx.l_locind[20];
  assert(strcmp(idx.name,"7.1.1.4")==0);
  idx=t_idx.l_locind[0];
  assert(strcmp(idx.name,"1.1.1.36")==0);

  // return indexes for later tests.
  return t_idx;
}

// int index_dump(char *dbase, int mode, long nb, indix_t *ind,char * SUF)
void test_index_dump_load(all_indix_t t_idx) {
  struct stat st;
  int ret=index_dump(rac_filename,APPEND_INDEXES,t_idx, ACCSUF,"../test/unit");
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==16); // index file should only contain an indicator saying that there are 0 indexes.
  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx,ACCSUF,"../test/unit");
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==16);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx,LOCSUF,"../test/unit");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size>16);
  int old_size=st.st_size;
  
  all_indix_t t_idx3=index_load("../test/unit",rac_filename,LOCSUF);
  assert(t_idx3.accnb==0);
  assert(t_idx3.locnb==21);
  
  qsort(t_idx3.l_locind, (size_t) t_idx3.locnb, sizeof(*t_idx3.l_locind), index_compare);
  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx3,LOCSUF,"../test/unit");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);
  
  freeAllIndix(t_idx3);

  ret=index_dump(rac_filename,MERGE_INDEXES,t_idx,LOCSUF,"../test/unit");
  assert(stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);

  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx,LOCSUF,"../test/unit");
  assert( stat(icx_file, &st)!= -1);
  int expected_size=2*old_size;
  expected_size=expected_size-8;
  assert(st.st_size==expected_size);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx,LOCSUF,"../test/unit");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);
  
  all_indix_t t_idx2=index_load("../test/unit",rac_filename,ACCSUF);
  assert(t_idx2.accnb==0);
  assert(t_idx2.locnb==0);
  // assert(t_idx2.l_accind==NULL);
  assert(t_idx2.l_locind==NULL);

  t_idx2=index_load("../test/unit",rac_filename,LOCSUF);
  // assert(strcmp(t_idx2.flatfile_name,data_file)==0);
  assert(t_idx2.accnb==0);
  assert(t_idx2.locnb==21);
  assert(t_idx2.l_accind==NULL);
  assert(t_idx2.l_locind!=NULL);

  freeAllIndix(t_idx2);
  
}


int main(int argc, char **argv) {
  all_indix_t t_idx=test_index_create();
  dest_index_desc d_descr;
  source_index_desc * ls_desc=malloc(3*sizeof(source_index_desc));
  test_index_dump_load(t_idx);
  freeAllIndix(t_idx);
  test_index_sort();
  test_list_append();
  test_index_desc(&d_descr,ls_desc);

  // TODO : clean index files.
}

