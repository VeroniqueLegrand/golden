
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
char * unit_tst_dir="../test/unit";




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
  free(file);
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
  if (stat(fdest, &st) != -1) {
    if (remove(fdest)==-1) err(errno, "Couldn't remove dest file.");
  }
  if (stat(fsource, &st) == -1) err(errno, "Couldn't find source file: %s.",fsource);
  
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
  char * tst_file="../test/unit/enzyme_test.idx";
  int i=1;
  
  all_indix_t t_idx=index_load("../test/unit","enzyme_extract_unsorted_idx1",LOCSUF);
#ifdef DEBUG
  int j;
  for (j=0;j<21;j++) {
    printf("%s\n",t_idx.l_locind[j].name);
  }
  printf("--------------------------------------------------------------------------------\n");
#endif
  //
  index_dump("enzyme_test",REPLACE_INDEXES,t_idx,LOCSUF,"../test/unit"); // creates tst_file
  index_sort(tst_file,21);
  // load test file and icx_file (already sorted), and check that both results are the same.
  all_indix_t t_idx3=index_load("../test/unit","enzyme_test",LOCSUF);
  assert(t_idx3.accnb==0);
  assert(t_idx3.locnb==21);
  
#ifdef DEBUG
  for (j=0;j<21;j++) {
    printf("%s\n",t_idx3.l_locind[j].name);
  }
#endif

  while (i<21) {
    char * v1=t_idx3.l_locind[i-1].name;
    char * v2=t_idx3.l_locind[i].name;
    assert(strcmp(t_idx3.l_locind[i-1].name,t_idx3.l_locind[i].name)<=0);
    i++;
  }
}

void test_list_append() {
  char * expected_content="toto/titi/premier_fichier.dat\ntoto/titi/second_fichier.dat\ntoto/titi/troisieme_fichier.dat\n\
toto1.dat\ntiti/totoX.dat\n\
titi/totoY.dat\ntiti/toto2.dat\ntiti/toto3.dat\n";
  
  char * dbase="db_test_tmp";
  struct stat st;
  char * buf=NULL;
  char * list_buf=NULL;
  int fd;

  list_new("../test/unit/new_tmp.dbx");
  assert(stat("../test/unit/new_tmp.dbx", &st) != -1);
  int n_nb=list_nb("../test/unit","new_tmp");
  assert(n_nb==0);

  //char * list_get(char * file)
  char * l_flat=list_get("../test/unit/new_tmp.dbx");
  assert(strcmp(l_flat,"")==0);
  free(l_flat);

  copy_file("../test/unit/db_test.dbx","../test/unit/db_test_tmp.dbx");
  int nb=list_append(dbase,NULL,"toto1.dat","../test/unit");
  assert(nb==4);
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  n_nb=list_nb("../test/unit","db_test_tmp");
  assert(n_nb==4);
  
  nb=list_append(dbase,"titi","totoX.dat\n","../test/unit");
  assert(stat("../test/unit/db_test_tmp.dbx", &st) != -1);
  assert(nb==5);
  // TODO : check that we got a warning for duplicate toto1.dat
  nb=list_append(dbase,"titi","totoY.dat\ntoto2.dat\ntoto3.dat\n","../test/unit");
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
  if (list_buf!=NULL) free(list_buf);
  
  // new test with a file containing long path
  list_new("../test/unit/long_tmp.dbx");
  copy_file("../test/unit/long_path.dbx","../test/unit/long_tmp.dbx");
  nb=list_append("long_tmp","test","blabla.dat","../test/unit");
  assert(nb=17);
}

void test_index_merge() {
  // 1- mimic old behavior : merge 1 file with new indexes.
  // 2- test new behavior : merge several index files.
}


void create_idx_for_concat() {
  char * buf=NULL;
  all_indix_t t_idx=create_index("../test/unit/wgs_extract.1.gnp",1,1,1);
  int nb=list_append("wgs1",NULL,"../test/unit/wgs_extract.1.gnp","../test/unit");
  int ret=index_dump("wgs1",REPLACE_INDEXES,t_idx, ACCSUF,"../test/unit");
  ret=index_dump("wgs1",REPLACE_INDEXES,t_idx, LOCSUF,"../test/unit");
  freeAllIndix(t_idx);

  t_idx=create_index("../test/unit/wgs_extract.2.gnp",1,1,1);
  nb=list_append("wgs2",NULL,"../test/unit/wgs_extract.2.gnp","../test/unit");
  ret=index_dump("wgs2",REPLACE_INDEXES,t_idx, ACCSUF,"../test/unit");
  ret=index_dump("wgs2",REPLACE_INDEXES,t_idx, LOCSUF,"../test/unit");
  freeAllIndix(t_idx);

  t_idx=create_index("../test/unit/wgs_extract.3.gnp",1,1,1);
  nb=list_append("wgs3",NULL,"../test/unit/wgs_extract.3.gnp","../test/unit");
  ret=index_dump("wgs3",REPLACE_INDEXES,t_idx, ACCSUF,"../test/unit");
  ret=index_dump("wgs3",REPLACE_INDEXES,t_idx, LOCSUF,"../test/unit/");
  freeAllIndix(t_idx);
  if (buf!=NULL) free(buf);
}

void test_index_desc(index_desc* d_descr,index_desc* ls_descr) {
  int nb_source=3;// number of elements in ls_descr.
  int i;
  struct stat st;
  *d_descr=get_dest_index_desc(1,1,"../test/unit","wgs_c");
  assert(stat("../test/unit/wgs_c.acx", &st) != -1);
  assert(stat("../test/unit/wgs_c.idx", &st) != -1);
  // assert(stat("../test/unit/wgs_c.dbx", &st) != -1);
  assert(d_descr->d_facx!=-1);
  assert(d_descr->d_ficx!=-1);
  //assert(d_descr->d_fdbx!=NULL);
  assert(d_descr->max_filenb==0);
  assert(d_descr->locnb==0);
  assert(d_descr->accnb==0);
  ls_descr[0]=get_source_index_desc(1,1,"../test/unit","wgs1");
  ls_descr[1]=get_source_index_desc(1,1,"../test/unit","wgs2");
  ls_descr[2]=get_source_index_desc(1,1,"../test/unit","wgs3");

  for(i=0; i<nb_source; i++) {
    assert(ls_descr[i].d_facx!=-1);
    assert(ls_descr[i].d_ficx!=-1);
    assert(ls_descr[i].max_filenb=1);
  }

  assert(ls_descr[0].accnb==2);
  assert(ls_descr[0].locnb==2);

  index_desc tmp=ls_descr[1];
  assert(ls_descr[1].accnb==5);
  assert(ls_descr[1].locnb==5);

  tmp=ls_descr[2];
  assert(ls_descr[2].accnb==3);
  assert(ls_descr[2].locnb==3);
}

/*
 * concatenates index in memory with index file on disk.
 */
void test_index_concat() {
  struct stat st;
  char * buf=NULL;
  all_indix_t t_idx=create_index("../test/unit/wgs_extract.1.gnp",1,1,1);
  int nb=list_append("wgs_orig",NULL,"../test/unit/wgs_extract.1.gnp","../test/unit");
  char * acx_file=index_file("../test/unit","wgs_orig",ACCSUF);
  int new_nb=index_concat(acx_file, nb, t_idx.l_accind);

  assert(new_nb==nb);
  assert((stat("../test/unit/wgs_orig.acx", &st) != -1));
  new_nb=index_concat(acx_file, nb, t_idx.l_accind);
  assert(new_nb==2*nb);

  all_indix_t t_idx_2=index_load("../test/unit","wgs_orig",ACCSUF);

  assert(strcmp(t_idx.l_accind[0].name,t_idx_2.l_accind[0].name)==0);
  assert(strcmp(t_idx.l_accind[nb-1].name,t_idx_2.l_accind[new_nb-1].name)==0);
  assert(strcmp(t_idx.l_accind[0].name,t_idx_2.l_accind[nb].name)==0);

  freeAllIndix(t_idx);
  freeAllIndix(t_idx_2);
  free(acx_file);
  if (buf!=NULL) free(buf);
}

/*
 Concatenates several unsorted index files That may contain doublons.
 */
void test_index_file_concat(index_desc* d_descr,index_desc* ls_descr,int nb_source) {
  int new_nb,i;
  char * s_dbx_file, *l_flats;
  char source_base[5];
  char * buf=NULL;

  // concatenate
  for(i=1; i<=nb_source; i++) {
    sprintf(source_base,"%s%d","wgs",i);
    s_dbx_file=index_file("../test/unit",source_base,LSTSUF);
    l_flats=list_get(s_dbx_file);
    free(s_dbx_file);
    new_nb = list_append("wgs_c",NULL,l_flats,"../test/unit");
    assert(new_nb==i);
    free(l_flats);

    // if (lseek(d_descr->d_facx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.acx","wgs_c");
    d_descr->accnb=index_file_concat(d_descr->d_facx,d_descr->max_filenb, ls_descr[i-1].accnb, ls_descr[i-1].d_facx,d_descr->accnb);

    // if (lseek(d_descr->d_ficx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.icx","wgs_c");
    d_descr->locnb=index_file_concat(d_descr->d_ficx,d_descr->max_filenb, ls_descr[i-1].locnb, ls_descr[i-1].d_ficx,d_descr->locnb);

    d_descr->max_filenb=new_nb;
    close_index_desc(&ls_descr[i-1]);
  }
  if (buf!=NULL) free(buf);
  
  close_index_desc(d_descr);

  // check resulting index file.
  int nb = list_nb("../test/unit","wgs_c");
  assert(nb==new_nb);
  all_indix_t wgs_idx=index_load("../test/unit","wgs_c",NULL);
  assert(wgs_idx.accnb==10);
  assert(wgs_idx.locnb==10);
  indix_t i0=wgs_idx.l_locind[0];
  indix_t i9=wgs_idx.l_locind[9];
  indix_t i4=wgs_idx.l_locind[4];
  
  /*
  int j;
  for (j=0;j<wgs_idx.accnb; j++){
    printf("%s %d\n",wgs_idx.l_locind[j].name,wgs_idx.l_locind[j].filenb);
  }
  */

  assert(strcmp(i0.name,"EYS95825")==0);
  assert(i0.filenb==1);

  assert(strcmp(i4.name,"EYS92515")==0);
  assert(i4.filenb==2);

  assert(strcmp(i9.name,"EYS68660")==0);
  assert(i9.filenb==3);
  
  }

all_indix_t test_index_create() {
  int nb;
  char * buf=NULL;
  nb = list_append("enzyme_extract",NULL, data_file,"../test/unit");
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
  if (buf!=NULL) free(buf);
  // return indexes for later tests.
  return t_idx;
}

/*
 Inside the given open file, go back to the initial position for beginning reading indix_t structures.
 */
void index_fbegin_go(int fidx,char * filename) {
  uint64_t indnb;
  if (lseek(fidx, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s",filename);
  if (read(fidx,&indnb, sizeof(indnb)) != sizeof(indnb)) err(errno,"error reading file : %s", filename);
}

void create_files_for_purge() {
  char * buf=NULL;
  index_desc my_dest;
  my_dest=get_dest_index_desc(1,1,"../test/unit","wgs_cfp");
  char * s_dbx_file = index_file("../test/unit","wgs2",LSTSUF);
  char * l_flats=list_get(s_dbx_file);
  free(s_dbx_file);
  int new_nb = list_append("wgs_cfp",NULL,l_flats,"../test/unit");
  
  index_desc my_source=get_source_index_desc(1,1,"../test/unit","wgs2");
  my_dest.accnb=index_file_concat(my_dest.d_facx,my_dest.max_filenb, my_source.accnb, my_source.d_facx,my_dest.accnb);
  my_dest.max_filenb=new_nb;
  new_nb = list_append("wgs_cfp",NULL,"T3","../test/unit");
  index_fbegin_go(my_source.d_facx,"wgs2.acx");
  
  
  my_dest.accnb=index_file_concat(my_dest.d_facx,my_dest.max_filenb, my_source.accnb, my_source.d_facx,my_dest.accnb);
  my_dest.max_filenb=new_nb;
  new_nb = list_append("wgs_cfp",NULL,"T4","../test/unit");
  
  index_fbegin_go(my_source.d_facx,"wgs2.acx");

  my_dest.accnb=index_file_concat(my_dest.d_facx,my_dest.max_filenb, my_source.accnb, my_source.d_facx,my_dest.accnb);
  my_dest.max_filenb=new_nb;
  new_nb = list_append("wgs_cfp",NULL,"T5","../test/unit");

  if (buf!=NULL) free(buf);
  
  index_fbegin_go(my_source.d_facx,"wgs2.acx");
  
  my_dest.accnb=index_file_concat(my_dest.d_facx,my_dest.max_filenb, my_source.accnb, my_source.d_facx,my_dest.accnb);
  my_dest.max_filenb=new_nb;
  close_index_desc(&my_source);
  
  if (lseek(my_dest.d_facx, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s","wgs_cfp");
  write(my_dest.d_facx, &my_dest.accnb, sizeof(my_dest.accnb));
  int nb_idx=my_dest.accnb;
  close_index_desc(&my_dest);
  
  char * acx_file=index_file("../test/unit","wgs_cfp",ACCSUF);
  index_sort(acx_file,nb_idx);
  free(acx_file);

  // create empty index file
  my_dest=get_dest_index_desc(1,1,"../test/unit","wgs_cfp_empty");
  close_index_desc(&my_dest);
  
  // create index file with only 1 element
  my_dest=get_dest_index_desc(1,1,"../test/unit","wgs_cfp_single");
  if (lseek(my_dest.d_facx, 0, SEEK_SET) == -1) err(errno,"Cannot go to beginning of file : %s","wgs_cfp_single");
  int s_accnb=1;
  indix_t s_idx;
  strcpy(s_idx.name,"abc");
  s_idx.filenb=1;
  s_idx.offset=22500;
  write(my_dest.d_facx,&s_accnb, sizeof(s_accnb));
  close_index_desc(&my_dest);
}

array_indix_t fic_index_load_test(const char * file) {
  int g;
  uint64_t nb_idx;
  array_indix_t fic_indix;
  indix_t cur;
  fic_indix.l_idx=NULL;
  int i=0;
  if ((g = open(file, O_RDONLY)) == -1) err(errno,"cannot open file: %s.",file);
  if (read(g,&nb_idx, sizeof(nb_idx)) != sizeof(nb_idx)) err(errno,"cannot read index number from file: %s.",file);
  fic_indix.nb_idx=nb_idx;
  if ((fic_indix.l_idx = (indix_t *)realloc(fic_indix.l_idx, nb_idx*sizeof(indix_t))) == NULL) err(errno,"cannot allocate memory");
  for(i=0;i<nb_idx;i++) {
    if (read(g,&cur, sizeof(cur)) != sizeof(cur)) err(errno,"cannot read index from file: %s.",file);
    fic_indix.l_idx[i]=cur;
  }
  if (close(g) == -1) err(errno,"error closing file: %s.",file);
  return fic_indix;
}


/* use index files created by create_idx_for_concat for that.*/
void test_index_purge() {
  all_indix_t idx_bf_purge=index_load("../test/unit","wgs_cfp",ACCSUF);
  assert(idx_bf_purge.accnb==20);
  char * idx_file=index_file("../test/unit","wgs_cfp",ACCSUF);
  /*array_indix_t tmp=fic_index_load_test(idx_file); // seems to work
  int i=tmp.nb_idx;
  while (i-1){
    printf("%s %d\n",tmp.l_idx[i-1].name,tmp.l_idx[i-1].filenb);
    i--;
  }*/
  
  /*int i=idx_bf_purge.accnb;
  while (i){
    printf("%s %d\n",idx_bf_purge.l_accind[i-1].name,idx_bf_purge.l_accind[i-1].filenb);
    i--;
  }*/
  
  index_purge(idx_file);
  free(idx_file);
  all_indix_t idx_af_purge=index_load("../test/unit","wgs_cfp",ACCSUF);
  assert(idx_af_purge.accnb==5);
  int i;
  for (i=0;i<5;i++) {
    assert(idx_af_purge.l_accind[i].filenb==4);
  }
  // freeAllIndix(idx_bf_purge);
  freeAllIndix(idx_af_purge);

  // just check that purge doesn't crash on empty index files or index files that contains only 1 element.
  idx_file=index_file("../test/unit","wgs_cfp_empty",ACCSUF);
  index_purge(idx_file);
  free(idx_file);

  idx_file=index_file("../test/unit","wgs_cfp_single",ACCSUF);
  index_purge(idx_file);
  free(idx_file);
}


// clean all files generated on disk
void clean() {
  struct stat st;
  int i;
  char f_idx_wgs[22];
  char f_acx_wgs[22];
  char f_dbx_wgs[22];
  
  if (stat("../test/unit/db_test_tmp.dbx", &st) != -1) {
    if (remove("../test/unit/db_test_tmp.dbx")==-1) err(errno, "Couldn't remove ../test/unit/db_test_tmp.dbx");
  }
  
  if (stat("../test/unit/db_test_tmp2.dbx", &st) != -1) {
    if (remove("../test/unit/db_test_tmp2.dbx")==-1) err(errno, "Couldn't remove ../test/unit/db_test_tmp2.dbx");
  }

  
  if (stat("../test/unit/enzyme_extract.dbx", &st) != -1) {
    if (remove("../test/unit/enzyme_extract.dbx")==-1) err(errno, "Couldn't remove ../test/unit/enzyme_extract.dbx");
  }
  
  if (stat("../test/unit/enzyme_test.idx", &st) != -1) {
    if (remove("../test/unit/enzyme_test.idx")==-1) err(errno, "Couldn't remove ../test/unit/enzyme_test.idx");
  }
  
  if (stat("../test/unit/new_tmp.dbx", &st) != -1) {
    if (remove("../test/unit/new_tmp.dbx")==-1) err(errno, "Couldn't remove ../test/unit/new_tmp.dbx");
  }
  
  for (i=1; i<=3; i++) {
    sprintf(f_idx_wgs,"../test/unit/wgs%d.idx",i);
    if (stat(f_idx_wgs, &st) != -1) {
      if (remove(f_idx_wgs)==-1) err(errno, "Couldn't remove %s",f_idx_wgs);
    }
  
    sprintf(f_acx_wgs,"../test/unit/wgs%d.acx",i);
    if (stat(f_acx_wgs, &st) != -1) {
      if (remove(f_acx_wgs)==-1) err(errno, "Couldn't remove %s",f_acx_wgs);
    }
  
    sprintf(f_dbx_wgs,"../test/unit/wgs%d.dbx",i);
    if (stat(f_dbx_wgs, &st) != -1) {
      if (remove(f_dbx_wgs)==-1) err(errno, "Couldn't remove %s",f_dbx_wgs);
    }
  }
  
  if (stat("../test/unit/wgs_c.acx", &st) != -1) {
    if (remove("../test/unit/wgs_c.acx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_c.acx");
  }

  if (stat("../test/unit/wgs_c.idx", &st) != -1) {
    if (remove("../test/unit/wgs_c.idx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_c.idx");
  }
  
  if (stat("../test/unit/wgs_c.dbx", &st) != -1) {
    if (remove("../test/unit/wgs_c.dbx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_c.dbx");
  }
  
  if (stat("../test/unit/wgs_cfp.dbx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp.dbx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp.dbx");
  }

  if (stat("../test/unit/wgs_cfp.acx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp.acx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_acx.dbx");
  }

  if (stat("../test/unit/wgs_cfp.idx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp.idx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_acx.idx");
  }
  
  if (stat("../test/unit/wgs_cfp_empty.dbx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp_empty.dbx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_empty.dbx");
  }

  if (stat("../test/unit/wgs_cfp_empty.acx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp_empty.acx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_empty.dbx");
  }

  if (stat("../test/unit/wgs_cfp_empty.idx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp_empty.idx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_empty.idx");
  }

  if (stat("../test/unit/wgs_cfp_single.dbx", &st) != -1) {
      if (remove("../test/unit/wgs_cfp_single.dbx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_single.dbx");
  }

  if (stat("../test/unit/wgs_cfp_single.acx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp_single.acx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_single.dbx");
  }

  if (stat("../test/unit/wgs_cfp_single.idx", &st) != -1) {
    if (remove("../test/unit/wgs_cfp_single.idx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_cfp_single.idx");
  }
  
  if (stat("../test/unit/wgs_orig.dbx", &st) != -1) {
    if (remove("../test/unit/wgs_orig.dbx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_orig.dbx");
  }

  if (stat("../test/unit/wgs_orig.acx", &st) != -1) {
    if (remove("../test/unit/wgs_orig.acx")==-1) err(errno, "Couldn't remove ../test/unit/wgs_orig.acx");
  }

  if (stat("../test/unit/long_tmp.dbx", &st) != -1) {
    if (remove("../test/unit/long_tmp.dbx")==-1) err(errno, "Couldn't remove ../test/unit/long_tmp.dbx");
  }
}



int main(int argc, char **argv) {
  clean();
  /*
  all_indix_t tmp_idx=index_load("/volumes/@home/wgs_tmp_index","wgs_concat_ns",ACCSUF);
  // all_indix_t tmp_idx2=index_load("/Users/vlegrand/Desktop/golden-3.0/test/tmp_new","wgs_ac_c1_sorted",ACCSUF);
  printf("%llu\n",tmp_idx.accnb);
  int i;
  for (i=0;i<tmp_idx.accnb;i++) {
    printf("%s %d %lu\n",tmp_idx.l_accind[i].name,tmp_idx.l_accind[i].filenb,tmp_idx.l_accind[i].offset);
  }

  freeAllIndix(tmp_idx);
  // freeAllIndix(tmp_idx2); */// old debug stuff

  all_indix_t t_idx=test_index_create();
  freeAllIndix(t_idx);
  index_desc d_descr;
  index_desc * ls_desc=malloc(3*sizeof(index_desc));
  // test_index_dump_load(t_idx);
  test_index_sort();
  test_list_append();
  create_idx_for_concat();
  test_index_desc(&d_descr,ls_desc);
  test_index_file_concat(&d_descr,ls_desc,3);
  test_index_concat();
  create_files_for_purge();
  test_index_purge();
  free(ls_desc);
  // clean files on disk
  clean();
}


/*
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
*/

