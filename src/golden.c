/* golden.c - Golden retriever main functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <libgen.h>

#include "access.h"
#include "entry.h"
#include "error.h"
#include "list.h"
#include "locus.h"


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t long
#endif

#define DEBUG




/* Functions prototypes */
static void usage(int);
void res_display(result_t res, int chk, char * file);
int goldenLstQuery(result_t** ,int, int ,int);
int performGoldenQuery(WAllQueryData, int,int);
static int compare_dbase (void const *a, void const *b);
char * build_query(const int from_file, int optind, const int argc, char ** argv);
WAllQueryData prepareQueryData(char *, result_t * ,int);
void freeQueryData(WAllQueryData wData);
int get_nbCards(char * my_list);
void print_results(int nb_res,int chk, result_t * res,char * file);
void logEntriesNotFound(WAllQueryData wData,int nb_notFound);

/* Global variables */
static char *prog;

// for sepqrqting elements in the list containing the AC.
// const char* separator=" ";


/* Main function */
int main(int argc, char **argv) {
  FILE  *h;
  int i, loc, acc, chk, nb_res,from_file;
  int nb_cards;
  char *file=NULL;
  result_t *res;
  char * my_list=NULL;

  /* Inits */
  prog = basename(*argv);

  /* Checks command line options & arguments */
  i = loc = acc = chk = from_file = 0; file = NULL;
  while((i = getopt(argc, argv, "achilo:f")) != -1) {
    switch(i) {
    case 'a':
      acc = 1; break;
    case 'c':
      chk = 1; break;
    case 'h':
      usage(EXIT_SUCCESS); break;
    case 'i':
      loc = 1; break;
    case 'l':
      if (list_check()) {
        error_fatal("databases", "cannot retrieve list"); }
      return EXIT_SUCCESS;
    case 'o':
      file = optarg; break;
    case 'f':
      from_file=1; break;
    default:
      usage(EXIT_FAILURE); break; }
  }
  if ((loc + acc) == 0) { loc = acc = 1; }
  if (optind==argc) {
      error_fatal("arguments", "no input file or list of bank:AC provided.");
  }
  my_list=build_query(from_file,optind, argc, argv);
  nb_cards=get_nbCards(my_list);
  // instantiate storage for query results.
  res=(result_t*) malloc(sizeof(result_t)*nb_cards);
  WAllQueryData wData=prepareQueryData(my_list,res,nb_cards);
  nb_res=performGoldenQuery(wData,acc,loc);
  
  logEntriesNotFound(wData,nb_cards-nb_res);
  // print results
  print_results(wData.nb_cards,chk,res,file);
  
  free(my_list);
  freeQueryData(wData);
  free(res);
  return EXIT_SUCCESS; }


/*
 From input arguments provided by the user, build a char string containing a list of bank:AC stuff separated by blanks.
 Returns this string. The caller must free the memory allocated for the returned string.
 */
char * build_query(const int from_file, int optind, const int argc, char ** argv) {
    struct stat buff;
    int fd;
    int new_siz,prev_siz=0;
    char * my_list=NULL;
    if (from_file) {
        /* read input file(s)if any into a list of bank:AC stuff. */
        while (optind<argc) {
          printf("%s\n",argv[optind]);
            fd=open(argv[optind],O_RDONLY);
            if (fd==-1) { // maybe see errno in that case.
                printf( "Error opening file: %s\n", strerror( errno ) );
                error_fatal("argv[optind]", "cannot open file.");
            }
            if (fstat(fd, &buff)==-1) {
                error_fatal("argv[optind]", "cannot get file size.");
            }
            if (my_list==NULL) {
                if ((my_list=(char *) malloc(buff.st_size*sizeof(char)))==NULL) error_fatal("memory", "cannot allocate memory to store requested ACs.");
                new_siz=buff.st_size;
            } else {
                prev_siz+=1; // allocate 1car for separator
                new_siz=prev_siz+buff.st_size;
                if ((my_list=(char *) realloc(my_list,new_siz*sizeof(char)))==NULL) error_fatal("memory", "cannot allocate memory to store requested ACs.");
                strcat(my_list," ");
            }
            if (read(fd,&my_list[prev_siz],buff.st_size)==-1) {
                error_fatal("argv[optind]", "cannot read file.");
            }
            prev_siz=new_siz;
            close(fd);
            optind++;
        }
    } else {
        // argv[optind] looks like: "base:name"
        while (optind<argc) {
            //printf("%s\n",argv[optind]);
            if (my_list==NULL) {
                new_siz=strlen(argv[optind]);
                // printf("Allocating %d bytes for my_list\n",new_siz);
                if ((my_list=(char *) malloc(new_siz*sizeof(char)))==NULL) error_fatal("memory", "cannot allocate memory to store requested ACs.");
                strncpy(my_list,argv[optind],strlen(argv[optind]));
            } else {
                prev_siz+=1; // allocate space for separator
                int arg_len=strlen(argv[optind])+1; //keep place for the \0 char.
                new_siz=prev_siz+arg_len;
                my_list = (char *) realloc(my_list,new_siz*sizeof(char));
                // printf("Re-allocating %d bytes for my_list\n",new_siz);
                if (my_list == NULL) {
                    error_fatal("memory", "cannot allocate memory to store requested ACs.");
                }
                //printf("%s\n",my_list);
                strcat(my_list," ");
                // my_list[prev_siz-1]=' ';
                // printf("%s\n",my_list);
                int new_len=strlen(my_list);
                strncat(my_list,argv[optind],arg_len-1);
                // printf("%s\n",my_list);
            }
            prev_siz=new_siz;
            optind++;
        }
        printf("%s\n",my_list);
    }
    
    
    char *end;
    // Trim leading space
    while(isspace(*my_list)) my_list++;
    // Trim trailing space
    end = my_list + strlen(my_list) - 1;
    while(end > my_list && isspace(*end)) end--;
    // Write new null terminator
    *(end+1) = 0;
    // replace '\n' with spaces.
    char * p_cr;
    while ((p_cr=strchr(my_list,'\n')) != NULL) {
        *p_cr=' ';
    }
    return my_list;
}


/*
 returns the number of cards in the query.
 */
int get_nbCards(char * my_list) {
    // count spaces to know how many cards we are expecting.
    char * tmp_list=my_list;
    char * blank_pos;
    int nb_cards=0;
    blank_pos=strstr(tmp_list," ");
    while (blank_pos!=NULL) {
        nb_cards++;
        tmp_list=blank_pos;
        tmp_list+=1;
        blank_pos=strstr(tmp_list," ");
    }
    nb_cards++;
    return nb_cards;
}



/*
 prints results to screen or file and free memory allocated for strings in result_t structures.
 TODO : move instructions that free memory somewhere else; it doesn't seem a good idea to do that here.
 */
void print_results(int nb_res,int chk,result_t * res,char * file) {
  int i;
#ifdef DEBUG
  printf("going to print : %d results \n",nb_res);
#endif
  // first version prints output. Don't want to change main's prototype at the beginning.
  for (i=0; i<nb_res; i++) {
    if (res[i].filenb==NOT_FOUND) continue; // only call print for results that were found. 
	  res_display(res[i],chk, file);
	  printf("###############################################################################");
	  printf("                                                                               ");
	  result_t cur_res;
	  cur_res=res[i];
	  free(cur_res.name); free(cur_res.dbase);
	  if (cur_res.real_dbase!=NULL) {
		  free(cur_res.real_dbase);
	  }
  }

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




/*
 *Utility method that displays one result.
 * Note : it is up to the caller to free the memory allocated for res after display:
 // free(res->name); free(res->dbase);
 //  free(res);
 */
void res_display(result_t res, int chk, char * file) {
	char * p;
	FILE *f, *g;
	/* Get database flat file name */
	if (res.filenb==NOT_FOUND || res.real_dbase==NULL) {
		return;
	}
	p = list_name(res.real_dbase, res.filenb);

	/* Set output stream */
	g = stdout;
	if (file != NULL && (g = fopen(file, "a")) == NULL) {
	    error_fatal(file, NULL); }

	/* Display database entry */
	if (chk == 0) {
	    if ((f = fopen(p, "r")) == NULL) {
	      error_fatal(p, NULL); }
	    if (fseeko(f, res.offset, SEEK_SET) != 0) {
	      error_fatal(p, NULL); }
	    if (entry_display(f, g)) {
	      error_fatal(p, NULL); }
	    if (fclose(f) == EOF) {
	      error_fatal(p, NULL); }
	    free(p); }


	if (file != NULL && fclose(g) == EOF) {
	    error_fatal(file, NULL); }
}



void logEntriesNotFound(WAllQueryData wData,int nb_notFound) {
	fprintf(stderr, "entries not found : ");
	result_t * cur_res;
	int i=0;
  int j=0;
	while ((i<wData.nb_cards) && (j<nb_notFound)) {
		cur_res=wData.lst_work[i];
    if (cur_res->filenb==NOT_FOUND) {
      fprintf(stderr, "%s:%s ",cur_res->dbase,cur_res->name);
      j++;
    }
		i++;
	}
  fprintf(stderr, "\n");
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
  elm = strtok (my_list," ");
  for (i=0;i<nb_cards;i++) {
	  len = strlen(elm);
	  if (strchr(elm,':') == NULL) {
      printf("%s",elm);
      error_fatal(elm, "invalid query value"); }
	  if ((dbase = (char *)malloc(len+1)) == NULL ||
	  	  (name = (char *)malloc(len+1)) == NULL) {
      error_fatal("memory", NULL); }
	  p=elm;
	  q=dbase;
	  while(*p && *p != ':') *q++ = *p++; *q = '\0'; p++;
	  q = name; while(*p) *q++ = toupper((unsigned char)*p++); *q = '\0';
	  res[i].dbase=dbase;
	  res[i].name=name;
	  res[i].filenb=NOT_FOUND;
	  res[i].real_dbase=NULL;
	  lst_work[i]=&res[i];
	  elm = strtok (NULL," ");
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
 Free memory allocated for working data structures.
 */
void freeQueryData(WAllQueryData wData) {
  free(wData.lst_work);
  free(wData.meta_lst_work.l_infoDB);
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
  int loc4base,idx_db;
  char * cur_dbname;
  int tot_nb_res_found=0;
  int tot_nb_res_not_found=0;
  int nb_AC_not_found,nb_locus_not_found;
  
  int nb_db=wData.meta_lst_work.nb_db;
  // WDBQueryData * l_infoDB=wData.meta_lst_work.l_infoDB[];
  for (idx_db=0;idx_db<nb_db;idx_db++) {
    WDBQueryData queryDB=wData.meta_lst_work.l_infoDB[idx_db];
    loc4base=loc;
    nb_AC_not_found=queryDB.len_l;
    nb_locus_not_found=queryDB.len_l;
    cur_dbname=(*queryDB.start_l)->dbase;
    // printf("cur_dbname : %s",cur_dbname);
    if (acc) {
			access_search(queryDB,cur_dbname, &nb_AC_not_found);
      if (nb_AC_not_found==0) loc4base=0;
      nb_locus_not_found=nb_AC_not_found;
      tot_nb_res_not_found+=nb_AC_not_found;
    }
    if (loc4base) {
      locus_search(queryDB,cur_dbname,&nb_locus_not_found);
      tot_nb_res_not_found+=nb_locus_not_found;
    } else {
      tot_nb_res_not_found+=nb_AC_not_found;
    }
  }
  // log entries not found
  
  // logEntriesNotFound(wData,tot_nb_res_not_found);
  tot_nb_res_found=wData.nb_cards-tot_nb_res_not_found;
  return tot_nb_res_found;
}




/* Usage display */
static void usage(int status) {
  FILE *f = stdout;

  (void)fprintf(f, "usage: %s [options] <dbase:name>\n\n", prog);
  (void)fprintf(f, "options:\n");
  (void)fprintf(f, "  -a        ... Search query by accession number.\n");
  (void)fprintf(f, "  -c        ... Check query.\n");
  (void)fprintf(f, "  -h        ... Prints this message and exit.\n");
  (void)fprintf(f, "  -i        ... Search query by entry name.\n");
  (void)fprintf(f, "  -l        ... List available databases.\n");
  (void)fprintf(f, "  -o <file> ... Place output into <file>.\n");

  exit(status); }
