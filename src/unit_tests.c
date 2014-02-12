/*
This file is dedicated to unit testing.
Unit testing differs from the already existing tests in the "test" directory in the fact
that its aim is to test small functions (index_merge, index_sort etc) and not the whole tool (goldin).
It is lower level.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "index.h"

char * data_file="../test/unit/enzyme_extract.dat";

void test_index_sort();
all_indix_t test_index_create();

void test_index_sort() {



}

all_indix_t test_index_create() {
  all_indix_t t_idx=create_index(data_file,0,1);
  assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.l_accind==NULL);
  assert(t_idx.l_locind==NULL);

  t_idx=create_index(data_file,1,0);
  assert(strcmp(t_idx.flatfile_name,data_file)==0);
  assert(t_idx.l_accind==NULL);
  assert(t_idx.l_locind!=NULL);

  // return indexes for later tests.
  return t_idx;
}

void test_index_dump(all_indix_t t_idx) {
  int index_dump("enzyme_extract_idx1", int mode, long nb, t_idx);
}

int main(int argc, char **argv) {
  test_index_create();
}

