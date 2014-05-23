/*
 * index_desc.h
 *
 *  Created on: Apr 3, 2014
 *      Author: vlegrand
 *
 *  Here, gather structures to handle index files. These structures are more complex than the basic stuff in index.h.
 *  These new structures are used for the the new functionnality (concatenation of existing index files)
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
 * Here, I make a distinction between structures related to source indexes and structures related to "destination" index.
 * The reason is the following :
 * We'll want to parallelize the "index concatenation" step in the future.
 * We'll then have several processes (one per "source index") that will write to the same destination index file.
 */

/*
 * Used to keep description of source index files that are being processed (read).
 */
typedef struct {
  int d_facx; // for .acx files
  int d_ficx; // for .icx files
  int accnb;
  int locnb;
  // TODO : dbx files are not that big, maybe storing their content in memory would be more efficient... Try that later.
  int max_filenb; // number of lines in .dbx files.
} source_index_desc;

// void init_sidx_desc(source_index_desc *);

/*
 * Used to keep description of destination index files that are being processed (write, update).
 */
typedef struct {
  int d_facx; // for .acx files
  int d_ficx; // for .icx files
  int d_fdbx; // for .dbx files
  int accnb;
  int locnb;
  int max_filenb; // number of lines in .dbx files.
} dest_index_desc;

//void init_didx_desc(dest_index_desc *);


fic_index_desc get_ficidx_desc(char *, char *,char*, char *,int);
void close_source_index_desc(source_index_desc *);
source_index_desc get_source_index_desc(int acc,int loc,char * s_index_dir, char * dbase);
void close_dest_index_desc(dest_index_desc * );
dest_index_desc get_dest_index_desc(int acc,int loc,char * new_index_dir, char * dbase);
#endif /* INDEX_DESC_H_ */
