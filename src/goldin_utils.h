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

#ifndef __GOLDIN_UTILS_H_
#define __GOLDIN_UTILS_H_

/* Storage structure for number of indexes in case of index concatenation. */
typedef struct {
  int accnb;
  int locnb;
} all_indix_nb;

/* these methods use both access.h, locus.h and index.h. That's why they are here. */
void all_index_sort(goldin_parms,all_indix_nb);
void all_index_purge(goldin_parms);
void all_index_mmerge(all_indix_t file_l_indix,goldin_parms);
// all_indix_nb all_index_mconcat(all_indix_t file_l_indix,goldin_parms);

#endif
