/* access.h - Accession numbers functions Copyright (C) 2001-2023  Institut Pasteur

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

#ifndef __ACCESS_H_
#define __ACCESS_H_

#include "index.h"
#include "index_hl.h"


/* Functions prototypes */
int access_merge(char *, uint64_t, indix_t *,char *);
int access_concat(char *, uint64_t, indix_t *, char * );
void access_search(WDBQueryData wData, char * db_name, int * nb_AC_not_found);
result_t * access_search_deprecated(char *dbase, char *name);
#endif /* __ACCESS_H_ */
