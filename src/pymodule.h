#ifndef _PYMODULE_H_
#define _PYMODULE_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include <Python.h>

PyObject *PyInit_wavegen();
PyObject *getPythonModule();

static const char *const THIS_ATTR = "this";

#endif
