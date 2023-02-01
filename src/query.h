/*
 *  Here, I gather functions that are useful for performing queries and that are used by
 *  both main C golden and 'Python' Golden.
 */
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

#ifndef __QUERY_H_
#define __QUERY_H_

#include "index_hl.h"

// static int compare_dbase (void const *a, void const *b);
int performGoldenQuery(WAllQueryData, int,int);
WAllQueryData prepareQueryData(char *, result_t * ,int);
void freeQueryData(WAllQueryData wData);
int get_nbCards(char * my_list);

#endif
