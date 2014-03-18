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

#include "list.h"
#include "index.h"

char * data_file="../test/unit/enzyme_extract.dat";
char * acx_file="enzyme_extract_idx1.acx";
char * icx_file="enzyme_extract_idx1.idx";
char * icx_unsorted_file="../test/unit/enzyme_extract_unsorted_idx1.idx";
char * rac_filename="enzyme_extract_idx1";

// void test_index_sort();
all_indix_t test_index_create();

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
  
  all_indix_t t_idx=fic_index_load(icx_unsorted_file,LOCSUF);
  index_dump("enzyme_test",REPLACE_INDEXES,t_idx,LOCSUF,"."); // creates tst_file
  index_sort(tst_file,21);
  // load test file and icx_file (already sorted), and check that both results are the same.
  all_indix_t t_idx3=index_load("enzyme_test",LOCSUF);
  assert(t_idx3.accnb==0);
  assert(t_idx3.locnb==21);
  
  all_indix_t t_idx_ref=index_load(rac_filename,LOCSUF);
  while (i<21) {
    assert(strcmp(t_idx_ref.l_locind[i].name,t_idx3.l_locind[i].name)==0);
    i++;
  }
}

void test_list_append() {
  char * expected_content="toto/titi/premier_fichier.dat\ntoto/titi/second_fichier.dat\ntoto/titi/troisieme_fichier.dat\n\
toto1.dat\ntiti/toto1.dat\n\
titi/toto1.dat\ntiti/toto2.dat\ntiti/toto3.dat\n";
  
  // write expected content for tests
  /*
  int fd_tmp=open("/tmp/unit_test.txt",O_WRONLY);
  write(fd_tmp,expected_content,strlen(expected_content));
  close(fd_tmp);*/
  char * dbase="db_test_tmp";
  struct stat st;
  char * buf;
  int fd;
  copy_file("../test/unit/db_test.dbx", "../test/unit/db_test_tmp.dbx");
  int nb=list_append(dbase,NULL,"toto1.dat","../test/unit");
  assert(nb==4);
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  
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
}

void test_index_merge() {
  // 1- mimic old behavior : merge 1 file with new indexes.
  // 2- test new behavior : merge several index files.
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
  int ret=index_dump(rac_filename,APPEND_INDEXES,t_idx, ACCSUF,".");
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==16); // index file should only contain an indicator saying that there are 0 indexes.
  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx,ACCSUF,".");
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==16);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx,LOCSUF,".");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size>16);
  int old_size=st.st_size;
  
  all_indix_t t_idx3=index_load(rac_filename,LOCSUF);
  assert(t_idx3.accnb==0);
  assert(t_idx3.locnb==21);
  
  qsort(t_idx3.l_locind, (size_t) t_idx3.locnb, sizeof(*t_idx3.l_locind), index_compare);
  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx3,LOCSUF,".");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);
  
  freeAllIndix(t_idx3);

  ret=index_dump(rac_filename,MERGE_INDEXES,t_idx,LOCSUF,".");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);

  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx,LOCSUF,".");
  assert( stat(icx_file, &st)!= -1);
  int expected_size=2*old_size;
  expected_size=expected_size-8;
  assert(st.st_size==expected_size);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx,LOCSUF,".");
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);

  // all_indix_t t_idx2=index_load(datta_file,acx_file,ACC_IDX);
  
  all_indix_t t_idx2=index_load(rac_filename,ACCSUF);
  //t_idx2.flatfile_name=strdup(data_file);
  // assert(strcmp(t_idx2.flatfile_name,data_file)==0);
  assert(t_idx2.accnb==0);
  assert(t_idx2.locnb==0);
  // assert(t_idx2.l_accind==NULL);
  assert(t_idx2.l_locind==NULL);

  t_idx2=index_load(rac_filename,LOCSUF);
  // assert(strcmp(t_idx2.flatfile_name,data_file)==0);
  assert(t_idx2.accnb==0);
  assert(t_idx2.locnb==21);
  assert(t_idx2.l_accind==NULL);
  assert(t_idx2.l_locind!=NULL);

  freeAllIndix(t_idx2);
  
}


int main(int argc, char **argv) {
  all_indix_t t_idx=test_index_create();
  test_index_dump_load(t_idx);
  freeAllIndix(t_idx);
  test_index_sort();
  test_list_append();
  // TODO : clean index files.
}

