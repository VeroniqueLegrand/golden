#ifndef __GOLDIN_UTILS_H_
#define __GOLDIN_UTILS_H_



/* these methods use both access.h, locus.h and index.h. That's why they are here. */
void all_index_sort(goldin_parms,all_indix_nb);
void mem_index_merge(all_indix_t file_l_indix,goldin_parms);
all_indix_nb mem_index_concat(all_indix_t file_l_indix,goldin_parms);

#endif
