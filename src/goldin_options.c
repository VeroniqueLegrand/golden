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

// static int dump_flag;
static int concat_sflg=0;
static int concat_oflg=0;
static int idx_input_flg=0;
static int purge_flg=0;

struct option long_options[] =
{
  /* These options set a flag. */
  {"sort",   no_argument, &concat_sflg, 's'}, // sort indexes and keep doublons, not like default behavior. Allows only 1 filename arg
  {"concat",  no_argument, &concat_oflg, 'c'}, // concatenates all indexes and do not sort them; keep doublons.
  {"idx_input", no_argument, &idx_input_flg, 1}, // indicates that filenames given in argument are base name for index files.
  /* These options don't set a flag. */
  {"idx_dir",  required_argument, 0, 'b'}, // indicates place where to put produced index files. default value is "."
  {"purge", no_argument, &purge_flg, 'p'},
  {NULL, 0, 0, 0}
};

void init_goldin_parms(goldin_parms * p_parms,int argc, char **argv) {
  int i;
  int option_index = 0;
  char * prog = basename(*argv);
  p_parms->new_index_dir=".";
  p_parms->loc = 0;
  p_parms->acc = 0;
  p_parms->wrn = 1;
  p_parms->dir = NULL;
  while((i = getopt_long(argc, argv, "ab:cd:hipqs",long_options,&option_index)) != -1) {
      switch(i) {
      case 0 :
          // printf("Option: %s \n",long_options[option_index].name);
        break;
      case 'a':
        p_parms->acc = 1; break;
      case 'b':
        p_parms->new_index_dir=optarg;break;
      case 'c':
        p_parms->co_flag=1;
        concat_oflg=1;
        break;
      case 'd':
        p_parms->dir = optarg; break;
      case 'h':
        usage(EXIT_SUCCESS,prog); break;
      case 'i':
        p_parms->loc = 1; break;
      case 'p':
        p_parms->purge_flag=1;
        purge_flg=1;
        break;
      case 'q':
        p_parms->wrn = 0; break;
      case 's':
        p_parms->csort_flag=1;
        concat_sflg=1;
        break;
      default:
        usage(EXIT_FAILURE,prog); break; }
    }
    p_parms->idx_input_flag=idx_input_flg;
    p_parms->co_flag=concat_oflg;
    p_parms->csort_flag=concat_sflg;
    p_parms->purge_flag=purge_flg;
    if ((p_parms->co_flag || p_parms->csort_flag || p_parms->purge_flag)  && p_parms->dir!=NULL) usage(EXIT_FAILURE,prog);
    if (!p_parms->idx_input_flag && (p_parms->co_flag || p_parms->purge_flag || p_parms->csort_flag)) usage(EXIT_FAILURE,prog);
    if (!p_parms->idx_input_flag) p_parms->serial_behavior=1;
    if (p_parms->idx_input_flag && ((p_parms->co_flag && p_parms->purge_flag) && !p_parms->csort_flag)) usage(EXIT_FAILURE,prog);
    /*if (!p_parms->csort_flag && !p_parms->co_flag && !p_parms->purge_flag && !p_parms->idx_input_flag) p_parms->serial_behavior=1;
    if (p_parms->purge_flag && (p_parms->co_flag )) usage(EXIT_FAILURE,prog);
    if ((p_parms->purge_flag && !p_parms->idx_input_flag) && !p_parms->csort_flag) usage(EXIT_FAILURE,prog);
    if (p_parms->idx_input_flag && p_parms->dir !=NULL) usage(EXIT_FAILURE,prog);
    if ((p_parms->idx_input_flag && (!p_parms->co_flag && !p_parms->csort_flag && !p_parms->purge_flag)) ) usage(EXIT_FAILURE,prog); // TODO later? Implement merge of existing index files.
    if (p_parms->co_flag && p_parms->csort_flag) usage(EXIT_FAILURE,prog);*/
    if (argc - optind < 2) usage(EXIT_FAILURE,prog);
    p_parms->dbase = argv[optind];
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
  (void)fprintf(f, "  --idx_dir   ... Specify directory where to put generated index files. \n");
  (void)fprintf(f, "  --idx_input   ...Process input files as index files and not as flat files. \n");
  (void)fprintf(f, "  -c, --concat  ... concatenates index files whose base name are given in argument. \n");
  (void)fprintf(f, "  -s, --sort    ... sort file given in input. \n");
  (void)fprintf(f, "  -p, --purge   ...Removes doublons from index file. Must be used together with --concat_sort or on a index file that was previously sorted. \n");
  exit(status); }
