#ifndef __GOLDIN_OPTIONS_H_
#define __GOLDIN_OPTIONS_H_

#include <getopt.h>

// static int dump_flag;
static int concat_sflg;
static int concat_oflg;
static int idx_input_flg;

static struct option long_options[] =
{
  /* These options set a flag. */
  {"concat_sort",   no_argument, &concat_sflg, 1}, // concatenates all indexes and sort them (keep doublons, not like default behavior).
  {"concat_only",  no_argument, &concat_oflg, 1}, // concatenates all indexes and do not sort them; keep doublons.
  {"idx_input", no_argument, &idx_input_flg, 1}, // indicates that filenames given in argument are base name for index files.
  /* These options don't set a flag. */
  {"index_dir",  required_argument, 0, 'b'} // indicates place where to put produced index files. default value is "."
};

/*
 This structure is used to store the goldin command line options.
 It is filled by the main program and passed as argument to other function.
 Aim is to have a limited number of function argument for code readability.
 */
typedef struct {
  int loc; // indicates if indexing locus
  int acc; // indicates if indexing accession numbers.
  int wrn; // indicates if one want to display a warning in case input flat file contains no AC/locus.
  char * dir; // alternate directory for data bank flat files.
  char * new_index_dir; // directory where to put generated index files.
  int serial_behavior; // indicates if processing is "serial" like in previous goldin version (this is the 3.0). This version aims at offering users the possibility to parallelize certain steps of the indexation process.
  int csort_flag;
  int co_flag;
  int idx_input_flag;
  char * dbase;
} goldin_parms;


/* Functions prototypes */
void usage(int,char *);
void init_goldin_parms(goldin_parms * p_parms,int argc, char **argv);

#endif
