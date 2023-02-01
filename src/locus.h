/* locus.h - Entry name functions */
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
#ifndef __LOCUS_H_
#define __LOCUS_H_

#include "index.h"
#include "index_hl.h"

/* Functions prototypes */
int locus_merge(char *, uint64_t, indix_t *,char * new_index_dir);
int locus_concat(char *dbase, uint64_t, indix_t *ind, char * new_index_dir);
void locus_search(WDBQueryData wData, char * dbase, int * nb_locus_not_found);

#endif /* __LOCUS_H_ */
