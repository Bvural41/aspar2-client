// Python 2.7 minimal stub for Android ARM64
// Provides enough symbols to link the Metin2 engine.
// The real Python runtime will be initialized via JNI from Java side.
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

typedef ssize_t Py_ssize_t;

// Forward declare PyObject and PyTypeObject
struct _typeobject;
struct _object {
    int ob_refcnt;
    struct _typeobject *ob_type;
};
typedef struct _object PyObject;

struct _typeobject {
    int ob_refcnt;
    struct _typeobject *ob_type;
    long ob_size;
    const char *tp_name;
};
typedef struct _typeobject PyTypeObject;

// PyTypeObject instances
PyTypeObject PyType_Type = {1, NULL, 0, "type"};
PyTypeObject PyBaseObject_Type = {1, NULL, 0, "object"};
PyTypeObject PySuper_Type = {1, NULL, 0, "super"};
PyTypeObject PyCode_Type = {1, NULL, 0, "code"};
PyTypeObject PyFrame_Type = {1, NULL, 0, "frame"};
PyTypeObject PyFunction_Type = {1, NULL, 0, "function"};
PyTypeObject PyClassMethod_Type = {1, NULL, 0, "classmethod"};
PyTypeObject PyStaticMethod_Type = {1, NULL, 0, "staticmethod"};
PyTypeObject PyCFunction_Type = {1, NULL, 0, "builtin_function_or_method"};
PyTypeObject PyMethod_Type = {1, NULL, 0, "instancemethod"};
PyTypeObject PyClass_Type = {1, NULL, 0, "class"};
PyTypeObject PyInstance_Type = {1, NULL, 0, "instance"};
PyTypeObject PyModule_Type = {1, NULL, 0, "module"};
PyTypeObject PyList_Type = {1, NULL, 0, "list"};
PyTypeObject PyTuple_Type = {1, NULL, 0, "tuple"};
PyTypeObject PyDict_Type = {1, NULL, 0, "dict"};
PyTypeObject PyString_Type = {1, NULL, 0, "str"};
PyTypeObject PyBaseString_Type = {1, NULL, 0, "basestring"};
PyTypeObject PyUnicode_Type = {1, NULL, 0, "unicode"};
PyTypeObject PyInt_Type = {1, NULL, 0, "int"};
PyTypeObject PyLong_Type = {1, NULL, 0, "long"};
PyTypeObject PyFloat_Type = {1, NULL, 0, "float"};
PyTypeObject PyBool_Type = {1, NULL, 0, "bool"};
PyTypeObject PyComplex_Type = {1, NULL, 0, "complex"};
PyTypeObject PyFile_Type = {1, NULL, 0, "file"};
PyTypeObject PyBuffer_Type = {1, NULL, 0, "buffer"};
PyTypeObject PyMemoryView_Type = {1, NULL, 0, "memoryview"};
PyTypeObject PyByteArray_Type = {1, NULL, 0, "bytearray"};
PyTypeObject PyByteArrayIter_Type = {1, NULL, 0, "bytearray_iterator"};
PyTypeObject PySet_Type = {1, NULL, 0, "set"};
PyTypeObject PyFrozenSet_Type = {1, NULL, 0, "frozenset"};
PyTypeObject PyNone_Type = {1000000000, NULL, 0, "NoneType"};
PyTypeObject PySlice_Type = {1, NULL, 0, "slice"};
PyTypeObject PyEllipsis_Type = {1, NULL, 0, "ellipsis"};
PyTypeObject PyRange_Type = {1, NULL, 0, "range"};
PyTypeObject PySeqIter_Type = {1, NULL, 0, "iterator"};
PyTypeObject PyCallIter_Type = {1, NULL, 0, "callable-iterator"};
PyTypeObject PyEnum_Type = {1, NULL, 0, "enumerate"};
PyTypeObject PyReversed_Type = {1, NULL, 0, "reversed"};
PyTypeObject PyGen_Type = {1, NULL, 0, "generator"};
PyTypeObject PyProperty_Type = {1, NULL, 0, "property"};
PyTypeObject PyWrapperDescr_Type = {1, NULL, 0, "wrapper_descriptor"};
PyTypeObject PyDictProxy_Type = {1, NULL, 0, "dictproxy"};
PyTypeObject PyGetSetDescr_Type = {1, NULL, 0, "getset_descriptor"};
PyTypeObject PyMemberDescr_Type = {1, NULL, 0, "member_descriptor"};
PyTypeObject PyDictIterKey_Type = {1, NULL, 0, "dictionary-keyiterator"};
PyTypeObject PyDictIterValue_Type = {1, NULL, 0, "dictionary-valueiterator"};
PyTypeObject PyDictIterItem_Type = {1, NULL, 0, "dictionary-itemiterator"};
PyTypeObject PyDictKeys_Type = {1, NULL, 0, "dict_keys"};
PyTypeObject PyDictItems_Type = {1, NULL, 0, "dict_items"};
PyTypeObject PyDictValues_Type = {1, NULL, 0, "dict_values"};
PyTypeObject PyCell_Type = {1, NULL, 0, "cell"};
PyTypeObject PyCObject_Type = {1, NULL, 0, "CObject"};
PyTypeObject PyCapsule_Type = {1, NULL, 0, "capsule"};
PyTypeObject PyTraceBack_Type = {1, NULL, 0, "traceback"};
PyTypeObject PySTEntry_Type = {1, NULL, 0, "symtable entry"};
PyTypeObject PyNullImporter_Type = {1, NULL, 0, "NullImporter"};
PyTypeObject _PyWeakref_RefType = {1, NULL, 0, "weakref"};
PyTypeObject _PyWeakref_ProxyType = {1, NULL, 0, "weakproxy"};
PyTypeObject _PyWeakref_CallableProxyType = {1, NULL, 0, "weakcallableproxy"};

int PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b) { return (a == b); }
PyObject* PyType_GenericAlloc(PyTypeObject *type, int nitems) { return (PyObject*)calloc(1, 64); }
PyObject* PyType_GenericNew(PyTypeObject *type, PyObject *args, PyObject *kwds) { return (PyObject*)calloc(1, 64); }
int PyType_Ready(PyTypeObject *type) { return 0; }

// PyNone singleton
extern PyObject _Py_NoneStruct;
#define Py_None (&_Py_NoneStruct)

// -------------------------------------------------------
// Reference counting
// -------------------------------------------------------
void Py_IncRef(PyObject* o) { if (o) o->ob_refcnt++; }
void Py_DecRef(PyObject* o) { if (o && --o->ob_refcnt <= 0) {} }

// -------------------------------------------------------
// Object protocol
// -------------------------------------------------------
PyObject* PyInt_FromLong(long v) { return Py_None; }
long PyInt_AsLong(PyObject* o) { return 0; }
PyObject* PyLong_FromLong(long v) { return Py_None; }
long PyLong_AsLong(PyObject* o) { return 0; }
PyObject* PyFloat_FromDouble(double v) { return Py_None; }
double PyFloat_AsDouble(PyObject* o) { return 0.0; }
PyObject* PyBool_FromLong(long v) { return Py_None; }
PyObject* PyString_FromString(const char* s) { return Py_None; }
PyObject* PyString_FromStringAndSize(const char* s, long sz) { return Py_None; }
const char* PyString_AsString(PyObject* o) { return ""; }
PyObject* PyUnicode_FromString(const char* s) { return Py_None; }

// -------------------------------------------------------
// Tuple protocol
// -------------------------------------------------------
int PyTuple_Size(PyObject* o) { return 0; }
PyObject* PyTuple_GetItem(PyObject* o, int i) { return Py_None; }
PyObject* PyTuple_New(int size) { return Py_None; }
int PyTuple_SetItem(PyObject* o, int i, PyObject* v) { return 0; }

// -------------------------------------------------------
// List protocol
// -------------------------------------------------------
PyObject* PyList_New(int size) { return Py_None; }
int PyList_Append(PyObject* list, PyObject* item) { return 0; }
int PyList_Size(PyObject* list) { return 0; }
PyObject* PyList_GetItem(PyObject* list, int i) { return Py_None; }

// -------------------------------------------------------
// Dict protocol
// -------------------------------------------------------
static PyObject s_global_dict = {1000000000, &PyDict_Type};
static PyObject s_main_module = {1000000000, &PyModule_Type};
static PyObject s_builtin_module = {1000000000, &PyModule_Type};

PyObject* PyDict_New(void) { return &s_global_dict; }
int PyDict_SetItemString(PyObject* d, const char* k, PyObject* v) { return 0; }
PyObject* PyDict_GetItemString(PyObject* d, const char* k) { return Py_None; }

// -------------------------------------------------------
// Module support
// -------------------------------------------------------
PyObject* Py_InitModule4(const char* name, void* methods, const char* doc, PyObject* self, int apiver) { return &s_main_module; }
int PyModule_AddIntConstant(PyObject* mod, const char* name, long val) { return 0; }
int PyModule_AddStringConstant(PyObject* mod, const char* name, const char* val) { return 0; }
int PyModule_AddObject(PyObject* mod, const char* name, PyObject* val) { return 0; }

// -------------------------------------------------------
// Build value / arg parsing
// -------------------------------------------------------
PyObject* Py_BuildValue(const char* format, ...) { return Py_None; }
PyObject* _Py_BuildValue_SizeT(const char* format, ...) { return Py_None; }
int PyArg_ParseTuple(PyObject* args, const char* format, ...) { return 0; }
int PyArg_ParseTupleAndKeywords(PyObject* args, PyObject* kwds, const char* format, char** kwlist, ...) { return 0; }

// -------------------------------------------------------
// Error handling
// -------------------------------------------------------
static PyObject _PyExc_RuntimeError = {1000000000, &PyNone_Type};
void PyErr_SetString(PyObject* exc, const char* msg) {}
void PyErr_SetNone(PyObject* exc) {}
PyObject* PyErr_Occurred(void) { return NULL; }
void PyErr_Clear(void) {}
PyObject* PyErr_Format(PyObject* exc, const char* format, ...) { return NULL; }

// -------------------------------------------------------
// Interpreter lifecycle
// -------------------------------------------------------
void Py_Initialize(void) {}
void Py_Finalize(void) {}
int Py_IsInitialized(void) { return 1; }
void PySys_SetArgv(int argc, char** argv) {}
int PyRun_SimpleString(const char* s) { return 0; }
PyObject* PyImport_ImportModule(const char* name) { return &s_builtin_module; }
PyObject* PyImport_AddModule(const char* name) { return &s_main_module; }
PyObject* PyObject_GetAttrString(PyObject* o, const char* name) { return Py_None; }
int PyObject_SetAttrString(PyObject* o, const char* name, PyObject* v) { return 0; }
PyObject* PyObject_CallObject(PyObject* callable, PyObject* args) { return Py_None; }
PyObject* PyObject_CallFunction(PyObject* callable, const char* format, ...) { return Py_None; }
PyObject* PyObject_CallMethod(PyObject* obj, const char* name, const char* format, ...) { return Py_None; }
int PyObject_IsTrue(PyObject* o) { return 0; }
long PyObject_Hash(PyObject* o) { return 0; }
PyObject* PyObject_Repr(PyObject* o) { return Py_None; }
PyObject* PyObject_Str(PyObject* o) { return Py_None; }
int PyCallable_Check(PyObject* o) { return 0; }

// -------------------------------------------------------
// Sys module
// -------------------------------------------------------
PyObject* PySys_GetObject(const char* name) { return &s_global_dict; }
int PySys_SetObject(const char* name, PyObject* v) { return 0; }

// -------------------------------------------------------
// Type checks
// -------------------------------------------------------
int PyInt_Check(PyObject* o) { return 0; }
int PyString_Check(PyObject* o) { return 0; }
int PyList_Check(PyObject* o) { return 0; }
int PyTuple_Check(PyObject* o) { return 0; }
int PyDict_Check(PyObject* o) { return 0; }
int PyFloat_Check(PyObject* o) { return 0; }

// -------------------------------------------------------
// GIL
// -------------------------------------------------------
void PyEval_InitThreads(void) {}
void PyEval_RestoreThread(void* tstate) {}
void* PyEval_SaveThread(void) { return NULL; }

static void dummy_dealloc(PyObject *op) {}

// -------------------------------------------------------
// Extra Python 2.7 symbols & internal singletons
// -------------------------------------------------------
PyObject _Py_NoneStruct = {1000000000, &PyNone_Type};
PyObject _Py_EllipsisObject = {1000000000, &PyNone_Type};
PyObject _Py_NotImplementedStruct = {1000000000, &PyNone_Type};
PyObject _Py_ZeroStruct = {1000000000, &PyNone_Type};
PyObject _Py_TrueStruct = {1000000000, &PyNone_Type};
PyObject* _PyTrash_delete_later = NULL;
int _PyTrash_delete_nesting = 0;
char* _Py_PackageContext = NULL;
const char* Py_FileSystemDefaultEncoding = "utf-8";
int _Py_CheckInterval = 100;
int _Py_CheckRecursionLimit = 1000;
volatile int _Py_Ticker = 0;
long _Py_RefTotal = 0;
void* _PyThreadState_Current = NULL;
void* _PyThreadState_GetFrame = NULL;
void* _PyOS_ReadlineTState = NULL;
void* PyOS_ReadlineFunctionPointer = NULL;
void* PyOS_InputHook = NULL;
void* PyImport_FrozenModules = NULL;
void* PyImport_Inittab = NULL;

int PyFrame_GetLineNumber(void* f) { return 0; }

PyObject* PyLong_FromLongLong(long long v) { return NULL; }
PyObject* PyLong_FromUnsignedLong(unsigned long v) { return NULL; }
PyObject* PyLong_FromUnsignedLongLong(unsigned long long v) { return NULL; }
long long PyLong_AsLongLong(PyObject* o) { return 0; }

int PyObject_HasAttrString(PyObject* o, const char* name) { return 0; }

int PyDict_Size(PyObject* d) { return 0; }
int PyDict_Next(PyObject* d, int* ppos, PyObject** pkey, PyObject** pvalue) { return 0; }
int PyDict_SetItem(PyObject* d, PyObject* k, PyObject* v) { return 0; }
int PyList_SetItem(PyObject* l, int i, PyObject* v) { return 0; }

// -------------------------------------------------------
// Python 2.7 Global Flags & Error Handlers
// -------------------------------------------------------
int Py_DebugFlag = 0;
int Py_VerboseFlag = 0;
int Py_InteractiveFlag = 0;
int Py_InspectFlag = 0;
int Py_OptimizeFlag = 0;
int Py_NoSiteFlag = 1;
int Py_BytesWarningFlag = 0;
int Py_UseClassExceptionsFlag = 0;
int Py_FrozenFlag = 0;
int Py_TabcheckFlag = 0;
int Py_UnicodeFlag = 0;
int Py_IgnoreEnvironmentFlag = 1;
int Py_DivisionWarningFlag = 0;
int Py_DontWriteBytecodeFlag = 1;
int Py_NoUserSiteDirectory = 1;
int _Py_QnewFlag = 0;
int Py_Py3kWarningFlag = 0;
int Py_HashRandomizationFlag = 0;

void Py_FatalError(const char *message) {}
void Py_SetProgramName(char *name) {}

void PyErr_Fetch(PyObject **ptype, PyObject **pvalue, PyObject **ptraceback) {
    if (ptype) *ptype = NULL;
    if (pvalue) *pvalue = NULL;
    if (ptraceback) *ptraceback = NULL;
}
void PyErr_Restore(PyObject *type, PyObject *value, PyObject *traceback) {}
void PyErr_Print(void) {}
void PyErr_BadArgument(void) {}
PyObject* PyErr_NoMemory(void) { return NULL; }

PyObject* PyEval_EvalCode(void *co, PyObject *globals, PyObject *locals) { return Py_None; }
int PyEval_GetRestricted(void) { return 0; }
void PyEval_SetTrace(void *func, PyObject *arg) {}

PyObject* PyMarshal_ReadObjectFromString(char *data, long len) { return Py_None; }
PyObject* PyMarshal_ReadLastObjectFromFile(void *fp) { return Py_None; }
PyObject* PyMarshal_WriteObjectToString(PyObject *v, int version) { return Py_None; }

// -------------------------------------------------------
// Exception types (exported as PyObject*)
// -------------------------------------------------------
PyObject* PyExc_BaseException = &_PyExc_RuntimeError;
PyObject* PyExc_Exception = &_PyExc_RuntimeError;
PyObject* PyExc_StopIteration = &_PyExc_RuntimeError;
PyObject* PyExc_GeneratorExit = &_PyExc_RuntimeError;
PyObject* PyExc_StandardError = &_PyExc_RuntimeError;
PyObject* PyExc_ArithmeticError = &_PyExc_RuntimeError;
PyObject* PyExc_LookupError = &_PyExc_RuntimeError;
PyObject* PyExc_AssertionError = &_PyExc_RuntimeError;
PyObject* PyExc_AttributeError = &_PyExc_RuntimeError;
PyObject* PyExc_EOFError = &_PyExc_RuntimeError;
PyObject* PyExc_FloatingPointError = &_PyExc_RuntimeError;
PyObject* PyExc_EnvironmentError = &_PyExc_RuntimeError;
PyObject* PyExc_IOError = &_PyExc_RuntimeError;
PyObject* PyExc_OSError = &_PyExc_RuntimeError;
PyObject* PyExc_ImportError = &_PyExc_RuntimeError;
PyObject* PyExc_IndexError = &_PyExc_RuntimeError;
PyObject* PyExc_KeyError = &_PyExc_RuntimeError;
PyObject* PyExc_KeyboardInterrupt = &_PyExc_RuntimeError;
PyObject* PyExc_MemoryError = &_PyExc_RuntimeError;
PyObject* PyExc_NameError = &_PyExc_RuntimeError;
PyObject* PyExc_OverflowError = &_PyExc_RuntimeError;
PyObject* PyExc_RuntimeError = &_PyExc_RuntimeError;
PyObject* PyExc_NotImplementedError = &_PyExc_RuntimeError;
PyObject* PyExc_SyntaxError = &_PyExc_RuntimeError;
PyObject* PyExc_IndentationError = &_PyExc_RuntimeError;
PyObject* PyExc_TabError = &_PyExc_RuntimeError;
PyObject* PyExc_ReferenceError = &_PyExc_RuntimeError;
PyObject* PyExc_SystemError = &_PyExc_RuntimeError;
PyObject* PyExc_SystemExit = &_PyExc_RuntimeError;
PyObject* PyExc_TypeError = &_PyExc_RuntimeError;
PyObject* PyExc_UnboundLocalError = &_PyExc_RuntimeError;
PyObject* PyExc_UnicodeError = &_PyExc_RuntimeError;
PyObject* PyExc_UnicodeEncodeError = &_PyExc_RuntimeError;
PyObject* PyExc_UnicodeDecodeError = &_PyExc_RuntimeError;
PyObject* PyExc_UnicodeTranslateError = &_PyExc_RuntimeError;
PyObject* PyExc_ValueError = &_PyExc_RuntimeError;
PyObject* PyExc_ZeroDivisionError = &_PyExc_RuntimeError;
PyObject* PyExc_WindowsError = &_PyExc_RuntimeError;
PyObject* PyExc_VMSError = &_PyExc_RuntimeError;
PyObject* PyExc_BufferError = &_PyExc_RuntimeError;
PyObject* PyExc_MemoryErrorInst = &_PyExc_RuntimeError;
PyObject* PyExc_RecursionErrorInst = &_PyExc_RuntimeError;
PyObject* PyExc_Warning = &_PyExc_RuntimeError;
PyObject* PyExc_UserWarning = &_PyExc_RuntimeError;
PyObject* PyExc_DeprecationWarning = &_PyExc_RuntimeError;
PyObject* PyExc_PendingDeprecationWarning = &_PyExc_RuntimeError;
PyObject* PyExc_SyntaxWarning = &_PyExc_RuntimeError;
PyObject* PyExc_RuntimeWarning = &_PyExc_RuntimeError;
PyObject* PyExc_FutureWarning = &_PyExc_RuntimeError;
PyObject* PyExc_ImportWarning = &_PyExc_RuntimeError;
PyObject* PyExc_UnicodeWarning = &_PyExc_RuntimeError;
PyObject* PyExc_BytesWarning = &_PyExc_RuntimeError;

// -------------------------------------------------------
// Python 2.7 Runtime & Parsing Functions
// -------------------------------------------------------
PyObject* _PyLong_New(Py_ssize_t size) { return (PyObject*)calloc(1, 64); }
PyObject* _PyLong_FromByteArray(const unsigned char* bytes, size_t n, int little_endian, int is_signed) { return (PyObject*)calloc(1, 64); }

int Py_FlushLine(void) { return 0; }
int PyCode_Addr2Line(void *co, int addrq) { return 0; }
void* PyCode_New(int argcount, int nlocals, int stacksize, int flags,
                 PyObject *code, PyObject *consts, PyObject *names,
                 PyObject *varnames, PyObject *freevars, PyObject *cellvars,
                 PyObject *filename, PyObject *name, int firstlineno,
                 PyObject *lnotab) { return (void*)calloc(1, 128); }

PyObject* PyComplex_FromCComplex(double real, double imag) { return (PyObject*)calloc(1, 64); }
long PyImport_GetMagicNumber(void) { return 62211; }
unsigned long PyLong_AsUnsignedLong(PyObject *pylong) { return 0; }
PyObject* PyModule_GetDict(PyObject *m) { return Py_None; }
int PyNumber_Check(PyObject *o) { return 1; }

int PyObject_AsCharBuffer(PyObject *obj, const char **buffer, Py_ssize_t *buffer_len) {
    if (buffer) *buffer = "";
    if (buffer_len) *buffer_len = 0;
    return 0;
}

PyObject* PyObject_GetAttr(PyObject *o, PyObject *attr_name) { return Py_None; }
PyObject* PyRun_StringFlags(const char *str, int start, PyObject *globals, PyObject *locals, void *flags) { return Py_None; }
PyObject* PyString_InternFromString(const char *cp) { return PyString_FromString(cp); }
PyObject* PyUnicodeUCS2_DecodeUTF8(const char *s, Py_ssize_t size, const char *errors) { return (PyObject*)calloc(1, 64); }
