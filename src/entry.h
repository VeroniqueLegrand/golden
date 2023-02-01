/* entry.h - Databases entries functions

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
// #define DEBUG
#ifndef __ENTRY_H_
#define __ENTRY_H_

#include <stdio.h>

/* Entries names maximum length */
#define NAMLEN 27

/* Entry structure definition */
typedef struct {
  char locus[NAMLEN+1];
  char access[NAMLEN+1];
  off_t offset; } entry_t;



/* Functions prototypes */
int entry_parse(FILE *, entry_t *);
// int entry_display(FILE *, FILE *);
int entry_display(FILE *f, int fd);

#endif /* __ENTRY_H_ */
