/*
 * index_desc.h
 *
 *  Created on: Apr 3, 2014
 *      Author: vlegrand
 *
 *  Here, gather structures to handle index files. These structures are more complex than the basic stuff in index.h.
 *  These new structures are used for the the new functionnality (concatenation of existing index files)
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

#ifndef INDEX_DESC_H_
#define INDEX_DESC_H_

/*
 Used to store description of 1 physical index file (index or locus, it doesn't matter).
 */
typedef struct {
  int d_fidx;
  int idxnb;
}fic_index_desc;


/*
 * Used to keep description of index files that are being processed (read or write).
 */
typedef struct {
  int d_facx; // for .acx files
  int d_ficx; // for .icx files
  int accnb;
  int locnb;
  // TODO : dbx files are not that big, maybe storing their content in memory would be more efficient... Try that later.
  int max_filenb; // number of lines in .dbx files.
} index_desc;



fic_index_desc get_ficidx_desc(char *, char *,char*, char *,int);
void close_index_desc(index_desc *);
index_desc get_source_index_desc(int acc,int loc,char * s_index_dir, char * dbase);
index_desc get_dest_index_desc(int acc,int loc,char * new_index_dir, char * dbase);
#endif /* INDEX_DESC_H_ */
