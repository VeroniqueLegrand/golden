/* golden.c - Golden retriever main functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <err.h>
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
#include "query.h"


#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t uint64_t
#endif

//#define DEBUG




/* Functions prototypes */
static void usage(int);
void res_display(result_t res, int chk, int file);
void print_results(int nb_res,int chk, result_t * res,char * file);
void logEntriesNotFound(WAllQueryData wData,int nb_notFound);
char * build_query_from_files(int optind, const int argc, char ** argv);
char * build_query_from_str(int optind, const int argc, char ** argv);


/* Global variables */
static char *prog;


/* Main function */
int main(int argc, char **argv) {
  int i, loc, acc, chk, nb_res,from_file;
  int nb_cards;
  char *file=NULL;
  result_t *res;
  char * my_list=NULL;
    
    // printf("hello \n");

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
      usage(SUCCESS_ALL_FOUND); break;
    case 'i':
      loc = 1; break;
    case 'l':
      if (list_check()) {
        error_fatal("databases", "cannot retrieve list"); }
      return SUCCESS_ALL_FOUND;
    case 'o':
      file = optarg; break;
    case 'f':
      from_file=1; break;
    default:
      usage(FAILURE); break; }
  }
  if ((loc + acc) == 0) { loc = acc = 1; }
  if (optind==argc) {
      error_fatal("arguments", "no input file or list of bank:AC provided.");
  }
  if (from_file) {
    my_list=build_query_from_files(optind, argc, argv);
  } else {
    my_list=build_query_from_str(optind, argc, argv);
  }
  // printf("%s\n",my_list);
  nb_cards=get_nbCards(my_list);
  // printf("nb_cards=%d \n",nb_cards);
  // instantiate storage for query results.
  res=(result_t*) malloc(sizeof(result_t)*nb_cards);
  WAllQueryData wData=prepareQueryData(my_list,res,nb_cards);
  nb_res=performGoldenQuery(wData,acc,loc);
  // printf("nb_res=%d\n",nb_res);
  logEntriesNotFound(wData,nb_cards-nb_res);
  // print results
  print_results(wData.nb_cards,chk,res,file);
  free(my_list);
  freeQueryData(wData);
  free(res);
  if (nb_res==nb_cards) return SUCCESS_ALL_FOUND;
  return SUCCESSS_NOT_ALL_FOUND;
}

/*
 Builds query from files whose names are passed as arguments to the command line.
 */
char * build_query_from_files(int optind, const int argc, char ** argv) {
  struct stat buff;
  int fd;
  int nb_read;
  int new_siz,prev_siz=0;
  char * my_list=NULL;
#ifdef DEBUG
  printf("optind=%d\n",optind);
  printf("%s\n",argv[optind]);
#endif
  while (optind<argc) {
    /* read input file(s)if any into a list of bank:AC stuff. */
    fd=open(argv[optind],O_RDONLY);
    if (fd==-1) { // maybe see errno in that case.
      err(errno,"cannot open file: %s.",argv[optind]);
    }
    if (fstat(fd, &buff)==-1) {
      err(errno,"cannot get file size: %s.",argv[optind]);
    }
    new_siz=prev_siz+buff.st_size;
    new_siz+=2; // add 1 for separator and 1 for 0 (end of string car).
    if ((my_list=(char *) realloc(my_list,new_siz*sizeof(char)))==NULL) err(errno, "cannot allocate memory to store requested ACs.");
    
    if ((nb_read=read(fd,&my_list[prev_siz],buff.st_size))==-1) {
      err(errno,"cannot read file: %s.",argv[optind]);
    }
    close(fd);
    if (my_list[prev_siz+nb_read-1]!='\n') {
      my_list[prev_siz+nb_read]='\n';
      my_list[prev_siz+nb_read+1]=0;
      prev_siz=new_siz-1;
    } else {
      prev_siz=new_siz-2; // don't need the separator; \n was already at the end of the file and 0 only matters if we parsed all files.
    }
    optind++;
  }
  return my_list;
}

/*
 Builds query from arguments from the command line.
 */
char * build_query_from_str(int optind, const int argc, char ** argv) {
  int arg_len,str_len;
  int new_siz,prev_siz=0;
  char * my_list=NULL;
  int first_arg=1;
  // argv[optind] looks like: "base:name"
  while (optind<argc) {
    arg_len=strlen(argv[optind]);
    new_siz=prev_siz+arg_len;
    new_siz+=1; // add 1 for \n separator
    if (optind+1==argc) { // last argument in the command line
      // add 1 for the "end of string" caracter.
      new_siz+=1;
    }
    if ((my_list = (char *) realloc(my_list,new_siz*sizeof(char))) == NULL) err(errno, "cannot allocate memory to store requested ACs.");
    if (first_arg) {
      strncpy(my_list,argv[optind],arg_len);
      my_list[arg_len]='\0';
      first_arg=0;
    } else {
      strncat(my_list,argv[optind],arg_len);
      my_list[prev_siz+arg_len]='\0';
    }
    str_len=strlen(my_list);
    my_list[str_len]='\n';
    prev_siz=new_siz;
    optind++;
  }
  my_list[str_len+1]=0;
  return my_list;
}



/*
 prints results to screen or file and free memory allocated for strings in result_t structures.
 TODO : move instructions that free memory somewhere else; it doesn't seem a good idea to do that here.
 */
void print_results(int nb_res,int chk,result_t * res,char * file) {
  int i;
  int g=fileno(stdout);
  /* Set output stream */
  // g = stdout;
  if (file != NULL && (g = open(file, O_WRONLY|O_CREAT,0666)) == -1) err(errno,"cannot open file: %s.",file);

  // first version prints output. Don't want to change main's prototype at the beginning.
    //printf("nb_res=%d\n",nb_res);
  for (i=0; i<nb_res; i++) {
      // printf("i=%d\n",i);
      // if (res[i].filenb==NOT_FOUND) printf("entry not found, continue\n");
    if (res[i].filenb==NOT_FOUND) continue; // only call print for results that were found. 
      res_display(res[i],chk, g);
      // printf("\n#####\n");
      result_t cur_res;
      cur_res=res[i];
      free(cur_res.name); free(cur_res.dbase);
      if (cur_res.real_dbase!=NULL) {
        free(cur_res.real_dbase);
      }
  }
  if (file!=NULL) close(g);
}



/*
 *Utility method that displays one result.
 * Note : it is up to the caller to free the memory allocated for res after display:
*/
void res_display(result_t res, int chk, int fd) {
    char * p;
    FILE *f;
    /* Get database flat file name */
    if (res.filenb==NOT_FOUND || res.real_dbase==NULL) {
        return;
    }
    p = list_name(res.real_dbase, res.filenb);
    // printf("In res_display, p=%s\n",p);

    /* Display database entry */
    if (chk == 0) {
        if ((f = fopen(p, "r")) == NULL) {
          error_fatal(p, NULL); }
        if (fseeko(f, res.offset, SEEK_SET) != 0) {
          error_fatal(p, NULL); }
        if (entry_display(f, fd)) {
          error_fatal(p, NULL); }
        if (fclose(f) == EOF) {
          error_fatal(p, NULL); }
        free(p); }
}


/*
 Displays the list of entries that were not found.
 */
void logEntriesNotFound(WAllQueryData wData,int nb_notFound) {
  // fprintf(stderr, "entries not found : ");
  int cnt_nf=0;
  result_t * cur_res;
  int i=0;
  int j=0;
  while ((i<wData.nb_cards) && (j<nb_notFound)) {
    cur_res=wData.lst_work[i];
    if (cur_res->filenb==NOT_FOUND) {
      if (cnt_nf==0) fprintf(stderr, "entries not found :");
      cnt_nf++;
      fprintf(stderr, " \"%s:%s\"",cur_res->dbase,cur_res->name);
      j++;
    }
    i++;
  }
  fprintf(stderr, "\n");
}



/* Usage display */
static void usage(int status) {
  FILE *f = stdout;

  (void)fprintf(f, "usage: %s [options] <dbase:name> <dbase:name> <dbase:name>...\n\n", prog);
  (void)fprintf(f, "options:\n");
  (void)fprintf(f, "  -a        ... Search query by accession number.\n");
  (void)fprintf(f, "  -c        ... Check query.\n");
  (void)fprintf(f, "  -h        ... Prints this message and exit.\n");
  (void)fprintf(f, "  -i        ... Search query by entry name.\n");
  (void)fprintf(f, "  -l        ... List available databases.\n");
  (void)fprintf(f, "  -o <file> ... Place output into <file>.\n");
  (void)fprintf(f, "  -f <file1> <file2> <file3>... Read input from <file1> <file2> <file3>.\n");
  exit(status); }
