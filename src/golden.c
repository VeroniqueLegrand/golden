/* golden.c - Golden retriever main functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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
// result_t* goldenLstQuery(char * lst,int acc,int loc , int * nb_res);
void goldenLstQuery(result_t** ,int, int ,int , int * );
static int compare_dbase (void const *a, void const *b);

/* Global variables */
static char *prog;


/* Main function */
int main(int argc, char **argv) {
  FILE  *h;
  int i, loc, acc, chk, nb_res;
  char *file, *input_file;
  result_t *res;
  char * my_list=NULL;

  /* Inits */
  prog = basename(*argv);
  nb_res=0;
  input_file = NULL;

  /* Checks command line options & arguments */
  i = loc = acc = chk = 0; file = NULL;
  while((i = getopt(argc, argv, "achilo:f:")) != -1) {
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
      /*if (argc!=0) {
        error_fatal("arguments", "no arguments allowed with -f option");
        return EXIT_SUCCESS;
      }*/
      input_file = optarg; break;
      /*
       * input_file contains many bank:AC infos
       */
    default:
      usage(EXIT_FAILURE); break; }
  }
  if ((loc + acc) == 0) { loc = acc = 1; }

  /* read input file into a list of bank:AC stuff. */
  if (input_file!=NULL) {
	  h= fopen(input_file, "r");
	  if (h == NULL) {
		  error_fatal(input_file,"Couldn't open file");
		  return EXIT_SUCCESS; }
	  fseek(h, 0, SEEK_END);
	  int input_size = ftell(h);
	  fseek(h, 0, SEEK_SET);
	  my_list=(char *) malloc(input_size*sizeof(char));
	  if (my_list==NULL) {
		  error_fatal("memory", NULL);
	  } else {
	        fread(my_list, sizeof(char), input_size, h);
	  }
	  char * end_substr=strstr(my_list,"\n");
	  while (end_substr!=NULL) {
		  *end_substr=' ';
		  end_substr=strstr(my_list,"\n");
	  }

#ifdef DEBUG
	  printf ("Read: ");
	  printf("%s ",my_list);
#endif
	  if (fclose(h ) == EOF) {
            error_fatal(input_file, "Couldn't close file");
	  }
  }
  else my_list=argv[argc-1];

  char *end;
  // Trim leading space
  while(isspace(*my_list)) my_list++;
  // Trim trailing space
  end = my_list + strlen(my_list) - 1;
  while(end > my_list && isspace(*end)) end--;
  // Write new null terminator
  *(end+1) = 0;

  // instantiate storage for query results.
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
  res=(result_t*) malloc(sizeof(result_t)*nb_cards);
  // instantiate data structure for work
  result_t ** lst_work=(result_t **) malloc(sizeof(result_t *)*nb_cards);

  char * elm;
  int len;
  char * dbase, *name, *p, *q;
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
  printf("\nlst_work content :");
  i=0;
  result_t * cur_res;
  while (i<nb_cards) {
	  cur_res=lst_work[i];
	  printf("%s:%s ",cur_res->dbase,cur_res->name);
	  i++;
  }
  printf("\n");
#endif

  // now, sort "work" data structures so that e can work with it.
  qsort (lst_work,nb_cards, sizeof(result_t *), compare_dbase);

#ifdef DEBUG
  printf("\nlst_work content :");
  i=0;
  // result_t * cur_res;
  while (i<nb_cards) {
	  cur_res=lst_work[i];
	  printf("%s:%s ",cur_res->dbase,cur_res->name);
	  i++;
  }
  printf("\n");
#endif


  goldenLstQuery(lst_work,nb_cards,acc,loc,&nb_res);

#ifdef DEBUG
  printf("going to print : %d results \n",nb_res);
#endif

  // first version prints output. Don't want to change main's prototype at the beginning.
  for (i=0; i<nb_res; i++) {
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
  if (input_file!=NULL) {
    free(my_list);
  }
  free(lst_work);
  free(res);

  return EXIT_SUCCESS; }


/*
 * Utility method that compares 2 result_t structures based on result_t.dbase comparison.
 */
static int compare_dbase (void const *a, void const *b)
{
   /* definir des pointeurs type's et initialise's
      avec les parametres */
   result_t const * const *pa = a;
   result_t const * const *pb = b;

   /* evaluer et retourner l'etat de l'evaluation (tri croissant) */
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
	if (file != NULL && (g = fopen(file, "w")) == NULL) {
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



void logEntriesNotFound(result_t ** lst_NotFound,int nb_notFound,char *cur_dbname) {
	fprintf(stderr, "entries not found in %s : ", cur_dbname);
	result_t * cur_res;
	int i=0;
	while (i<nb_notFound) {
		cur_res=lst_NotFound[i];
		fprintf(stderr, "%s ",cur_res->name);
		i++;
	}
}


/*
  Golden version that works on a list of database:accession_nbr or database:entry_names stuffs
*/
void goldenLstQuery(result_t** lst_searched_for,int lst_siz,int acc,int loc , int * nb_res_found) {
	char * elm;
	int len;
	char * dbase, *name, *p, *q;
	int nb_bases=0;
	int i;
	int loc4base;
	int lst_notFound_siz;

	result_t *res_locus=NULL;
	int nb_res_acc, nb_res_loc, nb_res_expected_for_db;
	result_t ** lst_notFound=NULL; // array of pointers towards AC that were not found in the indicated DB.
	ArrayOfResAddr lst_AC_notFound_db;
	// result_t ** lst_locusNotFound=NULL; // array f pointers towards entry names that were not found in the indicated db.
	ArrayOfResAddr lst_LOC_notFound_db;
	result_t ** lst_current_db=lst_searched_for;
	char * cur_dbname=NULL; // for debug only
	int idx=0; // to iterate over lst_searched_for
	int prev_idx=0;
	int delta=0;
	int orig_lst_siz=lst_siz;

	initArrayOfResAddr(&lst_AC_notFound_db);
	initArrayOfResAddr(&lst_LOC_notFound_db);

	while ((idx<orig_lst_siz) && (lst_current_db!=NULL)) {
		cur_dbname=(*lst_current_db)->dbase;
		lst_siz=orig_lst_siz-prev_idx;
#ifdef DEBUG
		printf("goldenLstQuery, entering loop for db : %s\n",cur_dbname);
#endif
		loc4base=loc;
		nb_res_acc=0;
		nb_res_loc=0;
		if (acc) {
			lst_AC_notFound_db=access_search(lst_current_db,lst_siz,cur_dbname, &nb_res_acc); // name is still a char * but it contains several names (accession numbers) separated by spaces.
			*nb_res_found=*nb_res_found+nb_res_acc;
			lst_notFound=lst_AC_notFound_db.addrArray;
			lst_notFound_siz=lst_AC_notFound_db.arrSize;
			if (lst_AC_notFound_db.arrSize==0) loc4base = 0; // all "name" were found in AC indexes for that base.
			idx=idx+nb_res_acc;
#ifdef DEBUG
			printf("%d res found in .acx file for db: %s \n",nb_res_acc,cur_dbname);
#endif
		} else {
			lst_notFound=lst_current_db; // in that case, the user indicated that they only wanted to look for locus names. (1)
			lst_notFound_siz=lst_siz;
		}
		if (loc4base) {
			/* in that case, 2 possibilities :
			- user specified that they wanted to look only for locus (1),
			- user didn't specify locus or AC but previous call to access_search didn't find all the AC (2).
			*/
			lst_LOC_notFound_db=locus_search(lst_notFound,lst_notFound_siz,cur_dbname,&nb_res_loc);
			*nb_res_found=*nb_res_found+nb_res_loc;
			idx=idx+nb_res_loc;
#ifdef DEBUG
			printf("%d res found in .lcx file for db: %s \n",nb_res_loc,cur_dbname);
#endif
			if (acc) { // case (2), lst_notFound=lst_AC_notFound_db.addrArray
				free(lst_notFound);
			}
			if (lst_LOC_notFound_db.arrSize!=0) { // not all locus (or ac+locus) were found, log information and free memory.
				idx=idx+lst_LOC_notFound_db.arrSize;
				logEntriesNotFound(lst_LOC_notFound_db.addrArray,lst_LOC_notFound_db.arrSize,cur_dbname);
				free(lst_LOC_notFound_db.addrArray);
				initArrayOfResAddr(&lst_LOC_notFound_db);
			}
		} else {
			/* - user specified they wanted only AC or, (3)
			   - all requested stuff were found in .acx files.
			 */
			if (lst_AC_notFound_db.arrSize!=0) { // case (3)
				// in that case, the user is only looking for AC and some of them were not found: log it somewhere! and free memory.
				idx=idx+lst_AC_notFound_db.arrSize;
				logEntriesNotFound(lst_AC_notFound_db.addrArray,lst_AC_notFound_db.arrSize,cur_dbname);
				free(lst_AC_notFound_db.addrArray);
				initArrayOfResAddr(&lst_AC_notFound_db);
			}
		}
		if (prev_idx==0) {
			lst_current_db=lst_current_db+idx;
		} else {
			delta=idx-prev_idx;
			lst_current_db=lst_current_db+delta;
		}
		prev_idx=idx;
	}
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
