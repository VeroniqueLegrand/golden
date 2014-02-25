/*
 * The following code aims at testing?profiling performances of parts of golden and goldin.
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

#include <time.h>

#include "index.h"
#include "query.h"

#define RET_OK 0
#define RET_NOK 1

#define SPLIT_MAX 30000

char * huge_file_dir="/tmp_banques/index/";
char * huge_index_file="embl_est.acx";
char * split_file_dir="/tmp_banques/index/split"; // should exist.
char * test_file_dir="/tmp_banques/index/perf_test_idx"; // where I store sorted index files of different size



/*
 * Need index files of different sizes for performance testing.
 * This function splits a given index file into smaller index files containing nb indexes each
 */
int index_split(char * dir_source,char * file_to_split, char * dir_dest,int nb) {
  int cnt=0;
  int tot_idx_read=0,cur_read=0;
  FILE * f_output;
  uint64_t nb_idx;
  uint64_t s_nb_idx= (uint64_t) nb;
  indix_t cur;
  char * path_file_to_split=malloc(strlen(dir_source)+1+strlen(file_to_split)+1);
  char * cur_split_file=malloc(strlen(dir_dest)+strlen(file_to_split)+6); // 6 for cnt plus \0
  sprintf(path_file_to_split,"%s/%s",dir_source,file_to_split);
  FILE * f_input=fopen(path_file_to_split,"r");

  if (fread(&nb_idx, sizeof(nb_idx), 1, f_input) != 1) err(errno,"cannot read index number from file: %s.",path_file_to_split);
  while (cnt<SPLIT_MAX && tot_idx_read<nb_idx) {
    cur_read=0;
    sprintf(cur_split_file,"%s/%s%d",dir_dest,file_to_split,cnt);
    f_output=fopen(cur_split_file,"w");
    fwrite(&s_nb_idx,sizeof(s_nb_idx),1,f_output);
    while (cur_read<nb && tot_idx_read<nb_idx) {
      if (fread(&cur, sizeof(cur), 1, f_input) != 1) err(errno,"cannot read index from file: %s.",path_file_to_split);
      if (fwrite(&cur, sizeof(cur), 1, f_output) != 1) err(errno,"Cannot write index to : %s",cur_split_file);
      cur_read++;
      tot_idx_read++;
    }
    if (tot_idx_read==nb_idx) {
      s_nb_idx=(uint64_t) cur_read;
      if (fseeko(f_output, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.",cur_split_file);
      fwrite(&s_nb_idx,sizeof(s_nb_idx),1,f_output);
    }
    fclose(f_output);
    cnt++;
  }
  fclose(f_input);
  if (cnt==SPLIT_MAX) {
    printf("Cannot split : %s into more than 30000 files.\n",path_file_to_split);
    return RET_NOK;
  }
  free(cur_split_file);
  free(path_file_to_split);
  return RET_OK;
}


/*
 Aim is to measure if index file size has any impact on index_search duration.
 In order to run this test, GOLDENDATA must be set to where you test index files are located.
 */
void perf_mimic_golden(char * my_list) {
  int nb_cards=get_nbCards(my_list);
  // instantiate storage for query results.
  result_t* res=(result_t*) malloc(sizeof(result_t)*nb_cards);
  WAllQueryData wData=prepareQueryData(my_list,res,nb_cards);
  
  clock_t cpu_time_start=clock();
  time_t wall_time_start=time(NULL);
  int nb_res=performGoldenQuery(wData,1,1);
  clock_t cpu_time_stop=clock();
  time_t wall_time_stop=time(NULL);
  
  freeQueryData(wData);
  free(res);
  
  // compute time spent searching for the card.
  clock_t cpu_time_read_flat=cpu_time_stop-cpu_time_start;
  time_t wall_time_read_flat=wall_time_stop-wall_time_start;
  
  printf("processor time spent searching for AC : %lu clock ticks\n",(unsigned long)	cpu_time_read_flat);
  printf("wall time spent searching for AC : %ld seconds \n",(long)	wall_time_read_flat);

}


/*
 * Measures execution time (in CPU clock ticks) evolution for searching a given AC/locus in different index files.
 * The aim of the test is to determine if index file size has a significant influence on golden search execution time.
 */
void test_index_search() {
  perf_mimic_golden("embl_est_20000:10000"); // 800 Ko file
  perf_mimic_golden("embl_est_40000:10000"); //1,6 Mo file
  perf_mimic_golden("embl_est_400000:10000"); //16 Mo file
  perf_mimic_golden("embl_est_2000000:10000"); //80 Mo file
  perf_mimic_golden("embl_est_8000000:10000"); // 320 Mo file
  perf_mimic_golden("embl_est_32000000:10000"); // 1 Go file
  perf_mimic_golden("embl_est_40000000:10000"); // 1,6 Go file
  perf_mimic_golden("embl_est_48000000:10000"); // 1,9 Go file
  perf_mimic_golden("embl_est_120000000:10000"); // 4,8 Go file
  perf_mimic_golden("embl_est_X:10000"); // 10,8 Go file
}

void perf_index_sort(char * file, int nb_idx) {
  clock_t cpu_time_start=clock();
  time_t wall_time_start=time(NULL);
  
  index_sort(file,nb_idx);
  
  clock_t cpu_time_stop=clock();
  time_t wall_time_stop=time(NULL);
  
  // compute time spent sorting indexes using mmap.
  clock_t cpu_time_read_flat=cpu_time_stop-cpu_time_start;
  time_t wall_time_read_flat=wall_time_stop-wall_time_start;
  
  printf("processor time spent mmap sorting: %s : %lu clock ticks\n",file,(unsigned long)	cpu_time_read_flat);
  printf("wall time spent mmap sorting :%s : %ld seconds \n",file,(long)	wall_time_read_flat);
}

/*
 Tests the new index_sort fonction that uses mmap.
 Aim is : 
 - to compare with previous version (index_merge)
 - see if the use of mmap makes anything better.
 - try to determine a file size limit or using mmap.
 */
void test_index_sort() {
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_20000.acx",20000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_40000.acx",40000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_400000.acx",400000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_2000000.acx",2000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_8000000.acx",8000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_32000000.acx",32000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_40000000.acx",40000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_48000000.acx",48000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_120000000.acx",120000000);
  perf_index_sort("/tmp_banques/index/perf_test_idx/embl_est_X.acx",270978004);
}


int main(int argc, char **argv) {
  // int ret=index_split(huge_file_dir,huge_index_file,split_file_dir, 120000000);
  test_index_search(); // mimic golden behavior but for 1 card.
  test_index_sort();
  return RET_OK;
}
