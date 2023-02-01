/* error.h - Error functions

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

#ifndef __ERROR_H_
#define __ERROR_H_

#define SUCCESS_ALL_FOUND 0
#define SUCCESSS_NOT_ALL_FOUND 1
#define FAILURE 2

/* Functions prototypes */
void error_fatal(const char *, const char *);  // TODO : remove these functions and use those in err.h instead.
void error_warn(const char *, const char *);
#endif /* __ERROR_H_ */
