#include "pymodule.h"
#include "wavegen.h"

PyObject *modulateFunc(PyObject *self, PyObject *const *args, Py_ssize_t nargs) noexcept
{
    if (nargs != 2)
    {
        PyErr_SetString(PyExc_TypeError, "function expects exactly 2 arguments");
        return nullptr;
    }

    auto pAttr = PyObject_GetAttrString(self, THIS_ATTR);
    auto zis = static_cast<WaveGen*>(PyCapsule_GetPointer(pAttr, nullptr));
    Py_XDECREF(pAttr);

    const auto bs = PyFloat_AsDouble(args[0]),
               mw = (PyFloat_AsDouble(args[1]) + 1.0) / 2.0,
               d = zis->modulationDepth();

    return PyFloat_FromDouble(bs * (bs >= 0.0 ? mw * d - d + 1.0 : 1.0 - mw * d));
}

static PyMethodDef s_funcDefs[] = {
    {
        "modulate",
        reinterpret_cast<PyCFunction>(modulateFunc),
        METH_FASTCALL,
        "Modulation function"
    },
    { nullptr, nullptr, 0 , nullptr }
};

static PyModuleDef s_modDef = {
    PyModuleDef_HEAD_INIT,
    "wavegen",
    nullptr,
    -1,
    s_funcDefs,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyObject *PyInit_wavegen()
{
    return PyModule_Create(&s_modDef);
}

PyObject *getPythonModule()
{
    return PyState_FindModule(&s_modDef);
}
