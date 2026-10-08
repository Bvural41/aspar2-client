#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "windows.h"

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) close(s)
#define ioctlsocket(s, cmd, argp) ioctl(s, cmd, argp)
#define WSAGetLastError() errno

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in *LPSOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr *LPSOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr *LPIN_ADDR;
typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent *LPHOSTENT;
typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger *LPLINGER;
typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval *LPTIMEVAL;
typedef fd_set FD_SET;

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAENOTSOCK ENOTSOCK
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAEMSGSIZE EMSGSIZE
#define WSAEPROTOTYPE EPROTOTYPE
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN ENETDOWN
#define WSAENETUNREACH ENETUNREACH
#define WSAENETRESET ENETRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAEISCONN EISCONN
#define WSAENOTCONN ENOTCONN
#define WSAESHUTDOWN ESHUTDOWN
#define WSAETOOMANYREFS ETOOMANYREFS
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAELOOP ELOOP
#define WSAENAMETOOLONG ENAMETOOLONG
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEHOSTUNREACH EHOSTUNREACH

#define SD_RECEIVE 0
#define SD_SEND    1
#define SD_BOTH    2

typedef struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
} WSADATA, *LPWSADATA;

#ifdef __cplusplus
inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) { return 0; }
inline int WSACleanup(void) { return 0; }
#endif
