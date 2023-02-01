/* error.c - Error functions

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#include <errno.h>

#include "error.h"


#ifndef HAVE_STRERROR
char *strerror(int errnum) {
  extern char *sys_errlist[];
  extern int sys_nerr;

  if (errnum > 0 && errnum < sys_nerr) {
    return sys_errlist[errnum]; }

  return (char *)"Unknown error type"; }
#endif /* HAVE_STRERROR */


/* Abort on fatal error */
void error_fatal(const char *str, const char *err) {
    printf("error fatal called \n");

  // printf("error fatal called with str=%s \n and \n err=%s \n",str,err);
  if (err == NULL) { err = strerror(errno); }
  (void)fprintf(stderr, "Fatal: %s: %s.\n", str, err);

  exit(FAILURE);
}

/* Warn for non fatal error */
void error_warn(const char *str, const char *err) {

  if (err == NULL) { err = strerror(errno); }
  (void)fprintf(stderr, "Warning: %s: %s.\n", str, err);

  return; }
