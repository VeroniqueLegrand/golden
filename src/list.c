/* list.c - Database files list functions */
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>

#include "error.h"
#include "index.h"
#include "list.h"

#ifndef HAVE_FSEEKO
#define fseeko fseek
#define off_t uint64_t
#endif

#define BUFINC 100
#define DEBUG
#include <time.h>

/*
 * Acquires or releases (depending on l_type value) an exclusive write lock on list file.
 * l_type value is either F_WRLCK or F_UNLCK.
 */
void set_list_lock(int fd, int l_type) {
  struct flock excl_wr_lock; // exclusive write lock.
  int res;
  excl_wr_lock.l_type=l_type; //F_WRLCK;
  excl_wr_lock.l_whence=SEEK_SET;
  excl_wr_lock.l_start=0;
  excl_wr_lock.l_len=0;
  res=fcntl(fd,F_SETLKW,&excl_wr_lock);
  if (res==-1) err(errno,"list_lock failed");
}


void list_lock(int fd) {
  set_list_lock(fd,F_WRLCK);
}

void list_unlock(int fd) {
  set_list_lock(fd,F_UNLCK);
}


/*
 Checks if a filename (a_fic) is already contained in the list of filenames (files_orig).
 */
void check_doublon(char * a_fic,char *files_orig) {
  char * p,*q;
  size_t len_files=strlen(files_orig);
  int cnt=0;
  char * files=strdup(files_orig);
  char * a=files;
  char * new_file=files;
  size_t len_a_fic=strlen(a_fic);
  if (a_fic[len_a_fic-1]=='/') return;
  while (new_file!=NULL) {
    while (*a!='\n' && cnt!=len_files) { // check for end of current filename un the list of filenames given in argument.
      a++;
      cnt++;
    }
    if (cnt!=len_files) a[0]='\0';
    /*if ((p = strrchr(new_file, '/')) != NULL) ++p;
    else p=new_file;*/
    p=new_file;
    /*if ((q = strrchr(a_fic, '/')) != NULL) ++q;
    else q=a_fic;*/
    q=a_fic;
    if (strcmp(p, q) == 0) errx(FAILURE, "duplicate file in database : %s,%s",p,q); // UNCOMMENT this is simply fr debugging purpose.
    if (cnt==len_files) {
      new_file=NULL;
    } else {
      a++;
      new_file=a;
      cnt++;
    }
  }
  free(files);
}



void read_chunk(int f, char * remain,char * l_buf, off_t nb_to_read) {
  size_t len_remain;
  ssize_t nb_read;
  len_remain=strlen(remain);
  if (len_remain>0) {
     strcpy(l_buf,remain);
     strcpy(remain,"");
  }
  if ((nb_read=read(f,l_buf+len_remain,(size_t) nb_to_read))==-1) err(FAILURE,"Error while reading list file.");
  l_buf[nb_to_read+len_remain]='\0';
  if (l_buf[nb_to_read+len_remain-1]!='\n') {
    // Here : look for last \n caracter in l_buf and copy what is after in remain. Do not modify remain in caller method.
    size_t i=(size_t) nb_to_read+len_remain;
    while (i>0 && l_buf[i]!='\n') {
      i--;
    }
    if (l_buf[i]=='\n') {
      char * last;
      i++;
      last=&l_buf[i];
      strcpy(remain,last);
      l_buf[i]='\0';
    }
  }
  
#ifdef DEBUG
  printf("read : %zu bytes.\n",nb_read);
  printf("len_remain+nb_to_read=%llu \n",len_remain+nb_to_read);
  printf("l_buf=%s\n",l_buf);
#endif
}


/*
 Counts existing elements and check for doublons in new elements.
*/
int count_check_doublons(char * name, int f, char * files) {
  struct stat st;
  // bool is_cut;
  char * a_fic;
  int nb;
  off_t nb_to_read;
  char l_buf[2*PATH_MAX+1];
  off_t offset;
  char remain[PATH_MAX+1]="";
  if (stat(name, &st) == -1) err(FAILURE,name, NULL);
#ifdef DEBUG
  printf("%s size of list file before writing: %lld\n",__func__,(long long) st.st_size);
  printf("%s list_append, File inode: %llu\n",__func__,st.st_ino);
  printf("%s reading : \n",__func__);
#endif
  nb = 0;
  nb_to_read=st.st_size;
  //is_cut=true;
#ifdef DEBUG
  printf("nb_to_read-PATH_MAX=%lld\n",nb_to_read-PATH_MAX);
#endif
  while (nb_to_read-PATH_MAX>=0) {
    read_chunk(f,remain,l_buf,PATH_MAX);
    a_fic=strtok(l_buf,"\n");
    while (a_fic!=NULL) {
#ifdef DEBUG
      printf("a_fic=%s\n",a_fic);
      if (strcmp(a_fic,"")==0) printf("Warning a_fic shouldn't be empty!");
#endif
      check_doublon(a_fic,files);
      nb++;
      // strcpy(remain,a_fic);
      a_fic=strtok(NULL,"\n");
    }
    // if (is_cut) nb--; // if read cut a path, then full path will be process with the next data read.
    nb_to_read=nb_to_read-PATH_MAX;
  }
  read_chunk(f,remain,l_buf,nb_to_read);
  a_fic=strtok(l_buf,"\n");
  while (a_fic!=NULL) {
#ifdef DEBUG
    printf("a_fic=%s\n",a_fic);
#endif
    check_doublon(a_fic,files);
    nb++;
    // strcpy(remain,a_fic);
    a_fic=strtok(NULL,"\n");
  }
#ifdef DEBUG
  printf("%s : before adding files, nb= %d \n",__func__,nb);
  printf("%s : list_append, going to write : %ld bytes \n", __func__,strlen(files));
  offset = lseek( f, 0, SEEK_CUR ) ;
  printf("%s, current offset : %lld \n", __func__,offset);
#endif
  return nb;
}

/* Append database flat file list && return file nb
 * The files argument is a list of file names separated by \n.
 * Parameters :
 * dir : as usual, the alternate dir where to find the .dat files.
 * dbase : base name for the bank; useful to retrieve index file name.
 * files : list of dbx files which content must be appended to existing .dbx file.
 * new_index_dir : location for index files. In previous versions of goldin/golden,
 * they were always generated in the execution directory but I do not find this 
 * very practical for unit testing and other kind of testing as well. 
 * Just put "." if you want previous behavior.
 * buf : a buffer to store content of the .dbx file before appending data to it.
 */
slist_inc list_append(char *dbase, char *dir, char *files,char * new_index_dir, bool keep_path) {
  // struct stat st;
  int f;
  int nb;
  slist_inc l_nb;
  char *p, *q,*name;
  char * new_file;
  char * l_files;
  char l_buf[2*PATH_MAX+1];
  int len_remain;
#ifdef DEBUG
  struct stat st;
#endif

  l_buf[0]='\0';
  len_remain=0;
  name = index_file(new_index_dir, dbase, LSTSUF);
#ifdef DEBUG  
  printf("going to open and lock : %s \n",name);
#endif
  f = open(name, O_RDWR|O_CREAT, 0666);
  if (f == -1) err(FAILURE,"Cannot open file : %s",name);
  list_lock(f);
  // 1rst part, count elements and check that there are no dublons.
  l_nb.oldnb=count_check_doublons(name, f, files);
  nb=l_nb.oldnb;
  // printf("list_append, count_check_doublons returned : %d \n",nb);
  /* 2nd part: Add new files (even if they are duplicate) */
  l_files=strdup(files);

  new_file=strtok(l_files,"\n");
  while (new_file!=NULL) {
    q = new_file; if ((p = strrchr(q, '/')) != NULL) q = ++p;
    if (!keep_path) {
      if (dir!=NULL) {
        ssize_t tmp=write(f,dir,strlen(dir));
        tmp=write(f,"/",1);
      }
      write(f,q,strlen(q));
    } else { // case of concatenation
      write(f,new_file,strlen(new_file));
    }
    write(f,"\n",1);
    new_file=strtok(NULL,"\n");
    nb++;
  }
  free(l_files);
  if (fsync(f) ==-1) err(FAILURE,"Error while fsync list file");
#ifdef DEBUG
  if (fstat(f, &st) == -1) err(FAILURE,name, NULL);
  printf("list_append, size of list file after writing: %lld\n",st.st_size);
  printf("list_append, File inode: %llu\n",st.st_ino);
  printf("list_append, before adding files, nb=%d\n",nb);
#endif  
  list_unlock(f);
  if (close(f)!=0) err(errno,"Error while closing file : %s",name);
  free(name);
  l_nb.newnb=nb;
  return l_nb;
}



/* Get database list flat file name */
char *list_name(char *dbase, int nb) {
  FILE *f;
  const char *p;
  char *q, *file, *buf, *name;
  size_t len;

  file = index_file(NULL, dbase, LSTSUF);

  if ((f = fopen(file, "r")) == NULL)
    error_fatal(file, NULL);
  len = BUFINC;
  if ((buf = (char *)malloc(len+1)) == NULL)
    error_fatal("memory", NULL);

  while(fgets(buf, (int)len, f) != NULL) {

    /* Checks for long line */
    if ((q = strrchr(buf, '\n')) == NULL) {
      len += BUFINC;
      if ((buf = (char *)realloc(buf, len+1)) == NULL)
	error_fatal("memory", NULL);
      if (fseeko(f, -1 * (off_t)strlen(buf), SEEK_SET) != 0)
	error_fatal(file, NULL);
      continue; }
    *q = '\0';

    /* Checks for number */
    if (--nb) continue;
    break; }

  if (fclose(f) == EOF)
    error_fatal(file, NULL);

  free(file);

  p = index_dir();

  /* Be compatible with old list format (without dir prefix) */
  if (strchr(buf, '/') == NULL) {
    len = strlen(p) + 1 + strlen(dbase) + 1 + strlen(buf);
    if ((name = (char *)malloc(len+1)) == NULL)
      error_fatal("memory", NULL);
    (void)sprintf(name, "%s/%s/%s", p, dbase, buf); }
  else {
    len = strlen(p) + 1 + strlen(buf);
    if ((name = (char *)malloc(len+1)) == NULL)
      error_fatal("memory", NULL);
    (void)sprintf(name, "%s/%s", p, buf); }

  free(buf);

  return name; }


/* Checks for available databases in index directory */
int list_check(void) {
  const char *p;
  char *q;
  int nb;
  DIR *d;
  struct dirent *dir;

  nb = 0;
  p = index_dir();
  if ((d = opendir(p)) == NULL)
    error_fatal(p, NULL);
  while((dir = readdir(d)) != NULL) {
    if ((q = strrchr(dir->d_name, '.')) == NULL) continue;
    if (strcmp(q+1, LSTSUF) != 0 && strcmp(q+1, VIRSUF) !=0) continue;
    *q = '\0'; nb++;
    (void)fprintf(stdout, "%s\n", dir->d_name); }
  if (nb == 0) {
    (void)fprintf(stdout, "No available databases.\n"); }
  if (closedir(d) == -1)
    error_fatal(p, NULL);

  return 0; }

/*
 * Returns the number of flat files already indexed for dbase.
 * If dbx file doesn't exist, an error is displayed and program exits..
 */
int list_nb(char * new_index_dir, char * dbase) {
  char * dbx_file,* buf, *p;
  FILE * dbx_fd;
  int ret,len;
  int nb_flat=0;
  dbx_file=index_file(new_index_dir,dbase,LSTSUF);
  ret=access(dbx_file, F_OK);
  if (ret!=0) err(0,"%s file doesn't exist",dbx_file);
  if ((dbx_fd = fopen(dbx_file, "r+")) == NULL) err(errno,"error opening file : %s", dbx_file);
  len = BUFINC;
  if ((buf = (char *)malloc(len+1)) == NULL) err(errno,"memory");
  while(fgets(buf, (int)len, dbx_fd) != NULL) {
    /* Checks for long line */
    if ((p = strrchr(buf, '\n')) == NULL) {
      len += BUFINC;
      if ((buf = (char *)realloc(buf, len+1)) == NULL) err(errno, "memory");
      if (fseeko(dbx_fd, -1 * (off_t)strlen(buf), SEEK_CUR) != 0) err(errno,"error seeking in file : %s", dbx_file);
        continue;
    }
    nb_flat++;
  }
  free(buf);
  free(dbx_file);
  fclose(dbx_fd);
  return nb_flat;
}

/*
 * Creates new, empty dbx file.
 */
void list_new(char *file) {
  FILE *g;
  /* Create empty index file*/
  if ((g = fopen(file, "w")) == NULL) err(errno,"cannot create file : %s",file);
  if (fclose(g) == EOF) err(errno,"cannot close file : %s",file);
}

/*
 * Get all database flat file names.
 * Allocates memory.
 */
char * list_get(char * file) {
  struct stat st;
  int fd;
  ssize_t nb_read;
  off_t len;
  char * lst_to_concat;
  if (stat(file, &st) == -1) err(errno,file, NULL);
  len=st.st_size;
  lst_to_concat= malloc((size_t) len+1);
  if ((fd=open(file,O_RDONLY))==-1) err(errno, "Cannot open source file.");
  if ((nb_read=read(fd,lst_to_concat,(size_t) st.st_size))==-1) err(errno,"Error while reading source file.");
  close(fd);
  lst_to_concat[len]='\0';
  // printf("read : %s from file : %s \n",lst_to_concat,file);
  return lst_to_concat;
}
