
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Python.h>

#include "access.h"
#include "list.h"

#define MAXNAME 100

static int bank_exist(char *bank, char *suf) {
  static char sav[MAXNAME];
  static int ini = 0, prv;
  const char *dir;
  char *file;
  int ret;

  if (ini != 0 && strcmp(bank, sav) == 0) { return prv; }

  dir = index_dir(); ret = 0;
  file = index_file(dir, bank, VIRSUF);
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

static PyObject *Golden_access_new(PyObject *self, PyObject *args) {
  char * l_args,elm;
  int t_args,nb_cards;
  static int cur_res=0;
  
  if (cur_res==0) { // first call perform query
    /* get and validate list parameter*/
    if (!PyArg_ParseTuple(args, "s", &l_args, &t_args)) {
      return NULL; }
  
    if (*l_args == '\0') {
      PyErr_SetString(PyExc_ValueError, "list of bank:AC cannot be empty.");
      return NULL; }
  
    nb_cards=get_nbCards(my_list);
    // instantiate storage for query results.
    res=(result_t*) malloc(sizeof(result_t)*nb_cards);
    WAllQueryData wData=prepareQueryData(my_list,res,nb_cards);
    int nb_res=performGoldenQuery(wData,acc,loc);
  }
  
  // logEntriesNotFound(wData,nb_cards-nb_res); // TODO? : How to display that to the python caller?
  if (cur_res==nb_cards) { // iteration is over.
    freeQueryData(wData);
    free(res);
    Py_RETURN_NONE;
  }
  if (res->filenb == NOT_FOUND) {
    free(res->dbase);
    free(res->name);
    cur_res++;
    return Py_BuildValue("Entry not found"); }
  
  str = entry_load(res);
  free(res->dbase);
  free(res->name);
  if (res->real_dbase!=NULL) {
    free(res->real_dbase);
  }
  cur_res++;
  return str;
}


static PyMethodDef Golden_methods[] = {
  { "access", (PyCFunction)Golden_access, METH_VARARGS, NULL },
  { "access_new", (PyCFunction)Golden_access_new, METH_VARARGS, NULL },
  { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initGolden() {
  Py_InitModule3("Golden", Golden_methods, "???"); }
