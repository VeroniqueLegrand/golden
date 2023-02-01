/* list.h - Databases files list functions */
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
#ifndef __LIST_H_
#define __LIST_H_
#include <stdbool.h>

typedef struct {
  int oldnb;
  int newnb;
}slist_inc;

/* Functions prototypes */
slist_inc list_append(char *, char *, char *,char *,bool);
char *list_name(char *, int);
int list_check(void);
int list_nb(char * , char * );
void list_new(char *);
char * list_get(char *);

#endif /* __LIST_H_ */

