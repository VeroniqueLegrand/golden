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

#include <libgen.h>

#ifndef EXIT_SUCCESS // TODO : this is redundant with definitions in golden.c. Find a common place to put them.
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif
#include "goldin_options.h"

void init_goldin_parms(goldin_parms * p_parms,int argc, char **argv) {
  int i;
  int option_index = 0;
  char * prog = basename(*argv);
  p_parms->new_index_dir=".";
  p_parms->loc = 0;
  p_parms->acc = 0;
  p_parms->wrn = 1;
  p_parms->dir = NULL;
  while((i = getopt_long(argc, argv, "ad:hiq",long_options,&option_index)) != -1) {
      switch(i) {
      case 0 :
          printf("Option: %s \n",long_options[option_index].name);
        break;
      case 'a':
        p_parms->acc = 1; break;
      case 'd':
        p_parms->dir = optarg; break;
      case 'h':
        usage(EXIT_SUCCESS,prog); break;
      case 'i':
        p_parms->loc = 1; break;
      case 'q':
        p_parms->wrn = 0; break;
      /* long options */
      case 'b':
        p_parms->new_index_dir=optarg;break;
      default:
        usage(EXIT_FAILURE,prog); break; }
    }
    p_parms->idx_input_flag=idx_input_flg;
    p_parms->co_flag=concat_oflg;
    p_parms->csort_flag=concat_sflg;
    if (!p_parms->csort_flag && !p_parms->co_flag && !p_parms->idx_input_flag) p_parms->serial_behavior=1;
    if (p_parms->idx_input_flag && p_parms->dir !=NULL) usage(EXIT_FAILURE,prog);
    if ((p_parms->idx_input_flag && (!p_parms->co_flag && !p_parms->csort_flag)) ) usage(EXIT_FAILURE,prog); // TODO later? Implement merge of existing index files.
    if ((p_parms->idx_input_flag && (p_parms->co_flag && p_parms->csort_flag)) ) usage(EXIT_FAILURE,prog); 
    if (argc - optind < 2) usage(EXIT_FAILURE,prog);
    p_parms->dbase = argv[optind];
    // if (p_parms->dir == NULL) { p_parms->dir = p_parms->dbase; }
    if ((p_parms->loc + p_parms->acc) == 0) { p_parms->loc = p_parms->acc = 1; }
}

/* Usage display */
void usage(int status,char * prog) {
  FILE *f = stdout;

  (void)fprintf(f, "usage: %s [options] <dbase> <file> ...\n\n", prog);
  (void)fprintf(f, "options:\n");
  (void)fprintf(f, "  -a       ... Make accession numbers indexes.\n");
  (void)fprintf(f, "  -d <dir> ... Use alternate data <dir>.\n");
  (void)fprintf(f, "  -h       ... Prints this message and exit.\n");
  (void)fprintf(f, "  -i       ... Make entry names indexes.\n");
  (void)fprintf(f, "  -q       ... Be quiet, do not display some warnings.\n");
  (void)fprintf(f, "  --index_dir   ... Specify directory where to put generated index files. \n");
  (void)fprintf(f, "  --idx_input   ...Process input files as index files and not as flat files. \n");
  (void)fprintf(f, "  --concat_only ... concatenate index files whose base name are given in argument; result is new index files (acx and/or idx). \n");
  (void)fprintf(f, "  --concat_sort ... Concatenates and sort index files. \n");
  exit(status); }
