#ifndef Py_CONFIG_H
#define Py_CONFIG_H

#if defined(__aarch64__) || defined(__arm64__)
#  include "../android_arm64/pyconfig/pyconfig.h"
#elif defined(__x86_64__)
#  include "../android_x86_64/pyconfig/pyconfig.h"
#else
#  error "Unsupported architecture for Python"
#endif

#if defined(__APPLE__)
#  undef HAVE_ASM_TYPES_H
#  undef HAVE_SYS_SYSMACROS_H
#  undef HAVE_LINUX_NETLINK_H
#  undef HAVE_NETPACKET_PACKET_H
#  undef HAVE_SYS_EPOLL_H
#  undef HAVE_LINUX_TIPC_H
#  undef HAVE_SEM_TIMEDWAIT
#  undef HAVE_GETHOSTBYNAME_R
#  undef HAVE_GETHOSTBYNAME_R_3_ARG
#  undef HAVE_GETHOSTBYNAME_R_5_ARG
#  undef HAVE_GETHOSTBYNAME_R_6_ARG
#  undef HAVE_CHROOT
#  undef HAVE_SYSTEM
#  undef HAVE_SETRESUID
#  undef HAVE_SETRESGID
#  undef HAVE_GETRESUID
#  undef HAVE_GETRESGID
#endif

#endif /* Py_CONFIG_H */
