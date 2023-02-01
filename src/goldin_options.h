/*
Copyright (C) 2001-2023  Institut Pasteur

  This program is part of the golden software.

  This program  is free software:  you can  redistribute it  and/or modify it  under the terms  of the GNU
  General Public License as published by the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the  GNU General Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

  Contact:

   Veronique Legrand                                                           veronique.legrand@pasteur.fr

 */

#ifndef __GOLDIN_OPTIONS_H_
#define __GOLDIN_OPTIONS_H_

#include <getopt.h>



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
  int purge_flag;
  int idx_input_flag;
  char * dbase;
} goldin_parms;


/* Functions prototypes */
void usage(int,char *);
void init_goldin_parms(goldin_parms * p_parms,int argc, char **argv);

#endif
