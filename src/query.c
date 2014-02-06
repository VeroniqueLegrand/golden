#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <err.h>

#include "index.h"
#include "access.h"
#include "locus.h"
#include "query.h"

// #define DEBUG

/*
 returns the number of cards in the query.
 */
int get_nbCards(char * my_list) {
    // count ':' to know how many cards we are expecting.
    int nb_cards=0;
    int l_siz=strlen(my_list);
    int i;
    for (i=0;i<l_siz;i++) {
      if (my_list[i]==':') {
        nb_cards++;
    }
  }
    return nb_cards;
}

/*
 * Utility method that compares 2 result_t structures based on result_t.dbase comparison.
 */
static int compare_dbase (void const *a, void const *b)
{
  result_t const * const *pa = a;
  result_t const * const *pb = b;
  /*
   #ifdef DEBUG
   printf("\nComparing  pa->dbase and  pb->dbase : ");
   printf("%s ",(*pa)->dbase);
   printf("%s ",(*pb)->dbase);
   #endif*/
  int res=strcmp((*pa)->dbase, (*pb)->dbase);
  return res;
}


// parse query data; instantiate and fill data structure for later work (when running query).
WAllQueryData prepareQueryData(char * my_list, result_t * res,int nb_cards) {
  WAllQueryData wData;
  result_t ** lst_work=(result_t **) malloc(sizeof(result_t *)*nb_cards);
  wData.lst_work=lst_work;
  wData.nb_cards=nb_cards;

  char * elm;
  int len;
  char * dbase, *name, *p, *q;
  int i;
#ifdef DEBUG
  printf("my_list=%s\n",my_list);
  printf("nb_cards=%d\n",nb_cards);
#endif
  elm = strtok (my_list,"\n");
  for (i=0;i<nb_cards;i++) {
    len = strlen(elm);
    if (strchr(elm,':') == NULL) {
      //printf("%s",elm);
      err(errno,"invalid query value: %s.",elm);
      }
      if ((dbase = (char *)malloc(len+1)) == NULL ||
         (name = (char *)malloc(len+1)) == NULL) {
      //error_fatal("memory", NULL);
      err(errno,"memory");}
      p=elm;
      q=dbase;
      while(*p && *p != ':') *q++ = *p++; *q = '\0'; p++;
      q = name; while(*p) *q++ = toupper((unsigned char)*p++); *q = '\0';
      res[i].dbase=dbase;
      res[i].name=name;
      res[i].filenb=NOT_FOUND;
      res[i].real_dbase=NULL;
      lst_work[i]=&res[i];
      elm = strtok (NULL,"\n");
  }
  // here, debug stuff, check that lst_work is filled correctly.
#ifdef DEBUG
  print_wrk_struct(lst_work,nb_cards,0);
#endif
  // now, sort "work" data structures so that we can work with it.
  qsort (lst_work,nb_cards, sizeof(result_t *), compare_dbase);
#ifdef DEBUG
  print_wrk_struct(lst_work,nb_cards,0);
#endif

  char * curDBName=lst_work[0]->dbase;
  int cnt_cards4db=0;
  int nb_db=1;

  WDBQueryData infoCurDB;
  LWDBQueryData l_infoCurDB;
  l_infoCurDB.l_infoDB=(WDBQueryData *) malloc(sizeof(WDBQueryData));
  // fill meta data
  infoCurDB.start_l=lst_work;
  for (i=0;i<nb_cards;i++) {
    if (strcmp(curDBName,lst_work[i]->dbase)==0) {
      cnt_cards4db++;
    } else {
      infoCurDB.len_l=cnt_cards4db;
      l_infoCurDB.l_infoDB[nb_db-1]=infoCurDB;
      nb_db+=1;
      l_infoCurDB.l_infoDB=(WDBQueryData *) realloc(l_infoCurDB.l_infoDB,nb_db*sizeof(WDBQueryData));
      cnt_cards4db=1;
      curDBName=lst_work[i]->dbase;
      infoCurDB.start_l=&lst_work[i];
    }
  }
  infoCurDB.len_l=cnt_cards4db;
  l_infoCurDB.l_infoDB[nb_db-1]=infoCurDB;
  l_infoCurDB.nb_db=nb_db;

  wData.meta_lst_work=l_infoCurDB;
  return wData;
}

/*
  Golden version that works on a list of database:accession_nbr or database:entry_names stuffs.
  Parameters :
    acc : flag indicating if we look for AC.
    loc : flag indicating if we look for locus.
 returns :
    tot_nb_res_found : total number of cards found
*/
int performGoldenQuery(WAllQueryData wData,int acc,int loc) {
  int idx_db,nb_db;
  char * cur_dbname;
  WDBQueryData queryDB;
  int tot_nb_res_found=0;
  int tot_nb_res_not_found=0;
  int nb_AC_not_found,nb_locus_not_found;

  nb_db=wData.meta_lst_work.nb_db;

  for (idx_db=0;idx_db<nb_db;idx_db++) {
    queryDB=wData.meta_lst_work.l_infoDB[idx_db];
    // loc4base=loc;
    nb_AC_not_found=queryDB.len_l;
    nb_locus_not_found=queryDB.len_l;
    cur_dbname=(*queryDB.start_l)->dbase;
    // printf("cur_dbname : %s\n",cur_dbname);
    if (acc) {
      access_search(queryDB,cur_dbname, &nb_AC_not_found);
      //if (nb_AC_not_found==0) loc4base=0;
      nb_locus_not_found=nb_AC_not_found;
      tot_nb_res_not_found+=nb_AC_not_found;
    }
    if (nb_AC_not_found && loc) {
      locus_search(queryDB,cur_dbname,&nb_locus_not_found);
      tot_nb_res_not_found+=nb_locus_not_found;
    } else {
      tot_nb_res_not_found+=nb_AC_not_found;
    }
  }
  tot_nb_res_found=wData.nb_cards-tot_nb_res_not_found;
  return tot_nb_res_found;
}

/*
 Free memory allocated for working data structures.
 */
void freeQueryData(WAllQueryData wData) {
  free(wData.lst_work);
  free(wData.meta_lst_work.l_infoDB);
}
