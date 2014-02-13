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

#include "index.h"

char * data_file="../test/unit/enzyme_extract.dat";
char * acx_file="enzyme_extract_idx1.acx";
char * icx_file="enzyme_extract_idx1.icx";
char * rac_filename="enzyme_extract_idx1";

// void test_index_sort();
all_indix_t test_index_create();


void test_index_sort() {
  index_sort(icx_file,LOC_IDX);
}


all_indix_t test_index_create() {
  all_indix_t t_idx=create_index(data_file,0,1);
  assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.l_accind==NULL);
  assert(t_idx.accnb==0);
  assert(t_idx.l_locind==NULL);
  assert(t_idx.locnb==0);

  t_idx=create_index(data_file,1,0);
  assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.accnb==0);
  assert(t_idx.l_accind==NULL);
  assert(t_idx.l_locind!=NULL);
  assert(t_idx.locnb==21);

  // return indexes for later tests.
  return t_idx;
}

void test_index_dump_load(all_indix_t t_idx) {
  struct stat st;
  int ret=index_dump(rac_filename,APPEND_INDEXES,t_idx.accnb, t_idx.l_accind);
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==0);
  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx.accnb, t_idx.l_accind);
  assert(stat(acx_file, &st) != -1);
  assert(st.st_size==0);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx.locnb, t_idx.l_locind);
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size!=0);
  int old_size=st.st_size;

  ret=index_dump(rac_filename,MERGE_INDEXES,t_idx.locnb, t_idx.l_locind);
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);

  ret=index_dump(rac_filename,APPEND_INDEXES,t_idx.locnb, t_idx.l_locind);
  assert( stat(icx_file, &st)!= -1);
  int expected_size=2*old_size;
  assert(st.st_size==2*old_size);

  ret=index_dump(rac_filename,REPLACE_INDEXES,t_idx.locnb, t_idx.l_locind);
  assert( stat(icx_file, &st)!= -1);
  assert(st.st_size==old_size);

  all_indix_t t_idx2=index_load(data_file,acx_file,ACC_IDX);
  assert(strcmp(t_idx2.flatfile_name,data_file)==0);
  assert(t_idx2.accnb==0);
  assert(t_idx2.locnb==0);
  assert(t_idx2.l_accind==NULL);
  assert(t_idx2.l_locind==NULL);

  t_idx2=index_load(data_file,icx_file,LOC_IDX);
  assert(strcmp(t_idx2.flatfile_name,data_file)==0);
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
  // TODO : clean index files.
}

