
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Python.h>

#include "access.h"
#include "list.h"
#include "query.h"
#include "index.h"
#include "index_hl.h"
#include "entry.h"

#define MAXNAME 100
//#define DEBUG

#if PY_MAJOR_VERSION >=3
#define IS_PY3K
#endif

static int bank_exist(char *bank, char *suf) {
  static char sav[MAXNAME];
  static int ini = 0, prv;
  const char *dir;
  char *file;
  int ret;

  if (ini != 0 && strcmp(bank, sav) == 0) { return prv; }

  dir = index_dir(); ret = 0;
  file = index_file(dir, bank, VIRSUF);
  /*printf("inside bank_exists: \n");
	 printf("%s",file);*/
  if (access(file, F_OK) != -1) { ret++; }
  free(file);
  file = index_file(dir, bank, suf);
  if (access(file, F_OK) != -1) { ret++; }
  free(file);

  snprintf(sav, sizeof(sav), "%s", bank);
  ini = 1; prv = ret;

  return ret; }


static PyObject *entry_load(result_t *res) {
  FILE *f;
  char *file;
  char tmp[1024], *buf = NULL;
  size_t tlen, len = 0;
  PyObject *str;
  char err_msg[200];

  file = list_name(res->real_dbase, res->filenb);
  if (file == NULL) {
    PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->dbase);
    return NULL; }

  f = fopen(file, "r");
  free(file);
  if (f == NULL) {
    PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->dbase);
    return NULL; }
  if (fseeko(f, res->offset, SEEK_SET) != 0) {
    PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->dbase);
    return NULL; }
  while (fgets(tmp, 1024, f) != NULL) {
    tlen = strlen(tmp); len += tlen;
    buf = realloc(buf, len+1);
    if (buf == NULL) {
      PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->name);
      return NULL; }
    memmove(buf+len-tlen, tmp, tlen);
    if (strcmp(tmp, "//\n") == 0) break; }
    if (buf==NULL) {
        /* Peculiar case where indexes are broken. fssek is asked to go beyond end-of-file and it does it without raising an error.
           Then fgets is asked to read at that place and it doesn't read anything but it returns NULL.*/
        sprintf(err_msg,"%s:%s Nothing was read, golden indexes are probably broken",res->dbase,res->name);
        PyErr_SetString(PyExc_RuntimeError,err_msg);
        PyErr_Print();
        str = Py_BuildValue("s", ""); // do that since python interpreter crashes when we return NULL. But execution has to stop.
        if (fclose(f) == EOF) {
            PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->dbase);
            return NULL; }
        return str;
    }
  *(buf+len) = '\0';
  if (fclose(f) == EOF) {
    PyErr_SetFromErrnoWithFilename(PyExc_IOError, res->dbase);
    return NULL; }

  str = Py_BuildValue("s", buf);
  free(buf);

  return str; }


static PyObject *Golden_access(PyObject *self, PyObject *args) {
  char *bank, *name;
  result_t *res;
  PyObject *str;

  /* Get/Validate bank & name parameters */
  if (!PyArg_ParseTuple(args, "ss", &bank, &name)) {
    return NULL; }
  if (*bank == '\0' || *name == '\0') {
    PyErr_SetString(PyExc_ValueError, "arguments cannot be empty");
    return NULL; }

  /* Ensure that the bank exists */
  if (bank_exist(bank, ACCSUF) == 0) {
#ifdef DEBUG
    printf("Bank doesn't exist\n");
#endif
    PyErr_SetFromErrnoWithFilename(PyExc_IOError, bank);
    return NULL; }

  /* Search indexes for name */
  res = access_search_deprecated(bank, name);
  if (res->filenb == NOT_FOUND) {
    free(res->dbase);
    free(res->name);
    return Py_BuildValue("s", NULL); }

  /* Load entry if exists */
  str = entry_load(res);

  free(res->dbase);
  free(res->name);
  if (res->real_dbase!=NULL) {
    free(res->real_dbase);
  }
  free(res);

  return str; }

static PyObject *Golden_access_multi(PyObject *self, PyObject *args) {
  char * l_args;
  static int nb_cards;
  static int idx_cur_res=0;
  static result_t * res=NULL;
  static WAllQueryData wData;
  PyObject *str;
  int nb_res;
  int i;
  int len,cnt;
  result_t cur_res; 
 
  //printf("%d call to Golden_access_new\n",idx_cur_res);
  if (idx_cur_res==0) { // first call perform query
    /* get and validate list parameter*/
    if (!PyArg_ParseTuple(args, "s", &l_args)) {
      return NULL; }
  
    if (*l_args == '\0') {
      PyErr_SetString(PyExc_ValueError, "list of bank:AC cannot be empty.");
      return NULL; }
  
    nb_cards=get_nbCards(l_args);
    // instantiate storage for query results.
    res=(result_t*) malloc(sizeof(result_t)*nb_cards);
    if (res==NULL) {
        PyErr_SetString(PyExc_MemoryError, "Couldn't allocate memory for storing results ");
        return NULL;
    }
    // check that l_args is correct.
    len=(int) strlen(l_args);
    cnt=0;
    for (i=0;i<len;i++) {
        l_args[i]=='\n'?cnt++:cnt;
    }
    if (cnt!=nb_cards) {
        PyErr_SetString(PyExc_ValueError, "input arguments must be like:\nbank:AC\\nbank:AC\\nbank:AC\\n");
        return NULL;
    }
    wData=prepareQueryData(l_args,res,nb_cards);
    nb_res=performGoldenQuery(wData,1,0);
  }
  
  // logEntriesNotFound(wData,nb_cards-nb_res); // TODO? : How to display that to the python caller?
  if (idx_cur_res==nb_cards) { // iteration is over.
    // printf("iteration over results is over, free memory\n");
    freeQueryData(wData);
    free(res);
    idx_cur_res=0;
    Py_RETURN_NONE;
  }
  cur_res=res[idx_cur_res];
  if (cur_res.filenb == NOT_FOUND) {
    str=Py_BuildValue("ss",cur_res.name," Entry not found");
  } else {
    str = entry_load(&cur_res);
  }
  free(cur_res.dbase);
  free(cur_res.name);
  if (cur_res.real_dbase!=NULL) {
      free(cur_res.real_dbase);
  }
  idx_cur_res++;
  return  str;
}


static PyMethodDef Golden_methods[] = {
  { "access", (PyCFunction)Golden_access, METH_VARARGS, NULL },
  { "access_new", (PyCFunction) Golden_access_multi, METH_VARARGS, NULL }, // keep this alias for old python scripts. Please note that it is deprecated.
  { "access_multi", (PyCFunction) Golden_access_multi, METH_VARARGS, NULL },
  { NULL, NULL, 0, NULL }
};

#ifdef IS_PY3K
  static struct PyModuleDef Goldendef= {
          PyModuleDef_HEAD_INIT,
          "Golden",
          "Contains functions that allow the user to query databank directly from a python script.",
          -1,
          Golden_methods,
          NULL,
          NULL,
          NULL,
          NULL,
  };
#endif

#ifdef IS_PY3K
PyMODINIT_FUNC PyInit_Golden() {
   return PyModule_Create(&Goldendef);
}
#else
PyMODINIT_FUNC initGolden() {
  Py_InitModule3("Golden", Golden_methods, "???");
}
#endif
