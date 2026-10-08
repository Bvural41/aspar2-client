#ifndef Py_CONFIG_H
#define Py_CONFIG_H

#if defined(__aarch64__)
#  include "../android_arm64/pyconfig/pyconfig.h"
#elif defined(__x86_64__)
#  include "../android_x86_64/pyconfig/pyconfig.h"
#else
#  error "Unsupported architecture for Python"
#endif

#endif /* Py_CONFIG_H */
