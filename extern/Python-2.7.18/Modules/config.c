/* Module configuration for Android Metin2 Mobile */

#include "Python.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void initthread(void);
extern void initsignal(void);
extern void initposix(void);
extern void initerrno(void);
extern void initpwd(void);
extern void init_sre(void);
extern void init_codecs(void);
extern void init_weakref(void);
extern void initzipimport(void);
extern void init_symtable(void);
extern void initarray(void);
extern void initcmath(void);
extern void initmath(void);
extern void init_struct(void);
extern void inittime(void);
extern void initoperator(void);
extern void init_random(void);
extern void init_collections(void);
extern void init_heapq(void);
extern void inititertools(void);
extern void initstrop(void);
extern void init_functools(void);
extern void initdatetime(void);
extern void init_bisect(void);
extern void initunicodedata(void);
extern void init_io(void);
extern void initfcntl(void);
extern void initselect(void);
extern void initmmap(void);
extern void init_csv(void);
extern void init_socket(void);
extern void inittermios(void);
extern void init_md5(void);
extern void init_sha(void);
extern void init_sha256(void);
extern void init_sha512(void);
extern void initbinascii(void);
extern void initparser(void);
extern void initcStringIO(void);
extern void initcPickle(void);
extern void initxxsubtype(void);
extern void initfuture_builtins(void);
extern void init_json(void);
extern void initzlib(void);

extern void PyMarshal_Init(void);
extern void initimp(void);
extern void initgc(void);
extern void init_ast(void);
extern void _PyWarnings_Init(void);

struct _inittab _PyImport_Inittab[] = {
    {"thread", initthread},
    {"signal", initsignal},
    {"posix", initposix},
    {"errno", initerrno},
    {"pwd", initpwd},
    {"_sre", init_sre},
    {"_codecs", init_codecs},
    {"_weakref", init_weakref},
    {"zipimport", initzipimport},
    {"_symtable", init_symtable},
    {"array", initarray},
    {"cmath", initcmath},
    {"math", initmath},
    {"_struct", init_struct},
    {"time", inittime},
    {"operator", initoperator},
    {"_random", init_random},
    {"_collections", init_collections},
    {"_heapq", init_heapq},
    {"itertools", inititertools},
    {"strop", initstrop},
    {"_functools", init_functools},
    {"datetime", initdatetime},
    {"_bisect", init_bisect},
    {"unicodedata", initunicodedata},
    {"_io", init_io},
    {"fcntl", initfcntl},
    {"select", initselect},
    {"mmap", initmmap},
    {"_csv", init_csv},
    {"_socket", init_socket},
    {"termios", inittermios},
    {"_md5", init_md5},
    {"_sha", init_sha},
    {"_sha256", init_sha256},
    {"_sha512", init_sha512},
    {"binascii", initbinascii},
    {"parser", initparser},
    {"cStringIO", initcStringIO},
    {"cPickle", initcPickle},
    {"xxsubtype", initxxsubtype},
    {"future_builtins", initfuture_builtins},
    {"_json", init_json},
    {"zlib", initzlib},

    /* Core internal modules */
    {"marshal", PyMarshal_Init},
    {"imp", initimp},
    {"_ast", init_ast},
    {"__main__", NULL},
    {"__builtin__", NULL},
    {"sys", NULL},
    {"exceptions", NULL},
    {"gc", initgc},
    {"_warnings", _PyWarnings_Init},

    {0, 0}
};

#ifdef __cplusplus
}
#endif
