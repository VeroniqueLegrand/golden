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

/*
 * Here : aim is to make goldin.c simplier.
 * I will gather here "high level" functions that are used only by goldin.
 * These functions may use directly those in index.c, index_hl.c, list.c,locus.c,access.c.
 */
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include "index.h"
#include "index_hl.h"
#include "goldin_options.h"
#include "goldin_utils.h"
#include "access.h"
#include "locus.h"

void all_index_sort(goldin_parms s_parms,all_indix_nb tot_idx) {
  char *file;
  if (s_parms.loc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, LOCSUF);
    index_sort(file,tot_idx.locnb);
    free(file);
  }
  if (s_parms.acc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, ACCSUF);
    index_sort(file,tot_idx.accnb);
    free(file);
  }
}

/*
 * Remove doublons from index files; for a same AC or locus, keep only the record with the biggest filenb.
 * WARNING : it is crucial that index files were previously sorted or else, the result of this function is undefined.
 */
void all_index_purge(goldin_parms s_parms) {
  char *file;
   if (s_parms.loc) {
      file = index_file(s_parms.new_index_dir, s_parms.dbase, LOCSUF);
      index_purge(file);
      free(file);
    }
    if (s_parms.acc) {
      file = index_file(s_parms.new_index_dir, s_parms.dbase, ACCSUF);
      index_purge(file);
      free(file);
    }
}

/*
* Concatenates all indexes in memory (locus+AC) with existing index file on disk.
*/
/*
all_indix_nb all_index_mconcat(all_indix_t file_l_indix,goldin_parms s_parms) {
  all_indix_nb tot_idx;

  if (s_parms.loc) {
    tot_idx.locnb=locus_concat(s_parms.dbase, file_l_indix.locnb, file_l_indix.l_locind,s_parms.new_index_dir);
    if (tot_idx.locnb==IDX_ERR) err(errno,"entry names indexes concatenation failed : %s",s_parms.dbase);
  }
  if (s_parms.acc) {
    tot_idx.accnb=access_concat(s_parms.dbase, file_l_indix.accnb, file_l_indix.l_accind,s_parms.new_index_dir);
    if (tot_idx.accnb==IDX_ERR) err(errno,"accession numbers indexes concatenation failed : %s",s_parms.dbase);
  }
  return tot_idx;
}*/


/*
 * Merge new indexes with existing index file.
 * Compute cpu time and wall time for this operation if program was built with PERF_PROFILE
 */
void all_index_mmerge(all_indix_t file_l_indix,goldin_parms s_parms) {

  /* Merge indexes */
  if (s_parms.loc) {
    if (locus_merge(s_parms.dbase, file_l_indix.locnb, file_l_indix.l_locind,s_parms.new_index_dir)) err(errno,"entry names indexes failed : %s",s_parms.dbase);
  }
  if (s_parms.acc) {
    if (access_merge(s_parms.dbase, file_l_indix.accnb, file_l_indix.l_accind,s_parms.new_index_dir)) err(errno,"accession numbers indexes failed : %s",s_parms.dbase);
  }

}
