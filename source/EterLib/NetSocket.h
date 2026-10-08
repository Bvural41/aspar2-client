#pragma once

#ifdef _WIN32

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <winsock.h>

	#pragma comment(lib, "ws2_32.lib")

	typedef SOCKET socket_t;
	typedef int socklen_t;

	#define SOCKET_ERR_WOULDBLOCK   WSAEWOULDBLOCK
	#define SOCKET_ERR_INPROGRESS   WSAEINPROGRESS
	#define SOCKET_ERR_CONNRESET    WSAECONNRESET
	#define SOCKET_ERR_ALREADY      WSAEALREADY

	inline int socket_close(socket_t s)
	{
		return closesocket(s);
	}

	inline int socket_error_code()
	{
		return WSAGetLastError();
	}

	inline bool socket_is_wouldblock(int err)
	{
		return (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS || err == WSAEALREADY);
	}

	inline int socket_set_nonblocking(socket_t s, bool nonblocking)
	{
		u_long mode = nonblocking ? 1 : 0;
		return ioctlsocket(s, FIONBIO, &mode);
	}

	inline bool socket_is_valid(socket_t s)
	{
		return (s != INVALID_SOCKET);
	}

#else // Non-Windows: POSIX BSD Sockets (Android, iOS, Linux, macOS)

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>

	typedef int socket_t;
	typedef int SOCKET;

	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET  (-1)
	#endif

	#ifndef SOCKET_ERROR
		#define SOCKET_ERROR    (-1)
	#endif

	typedef struct sockaddr     SOCKADDR;
	typedef struct sockaddr*    PSOCKADDR;
	typedef struct sockaddr_in  SOCKADDR_IN;
	typedef struct hostent      HOSTENT;
	typedef struct timeval      TIMEVAL;

	#define SOCKET_ERR_WOULDBLOCK   EWOULDBLOCK
	#define SOCKET_ERR_INPROGRESS   EINPROGRESS
	#define SOCKET_ERR_CONNRESET    ECONNRESET
	#define SOCKET_ERR_ALREADY      EALREADY

	#define WSAEWOULDBLOCK          EWOULDBLOCK
	#define WSAEINPROGRESS          EINPROGRESS
	#define WSAECONNRESET           ECONNRESET
	#define WSAEALREADY             EALREADY
	#define closesocket(s)          close(s)
	#define WSAGetLastError()       errno

	inline int socket_close(socket_t s)
	{
		return close(s);
	}

	inline int socket_error_code()
	{
		return errno;
	}

	inline bool socket_is_wouldblock(int err)
	{
		return (err == EWOULDBLOCK || err == EAGAIN || err == EINPROGRESS || err == EALREADY);
	}

	inline int socket_set_nonblocking(socket_t s, bool nonblocking)
	{
		int flags = fcntl(s, F_GETFL, 0);
		if (flags == -1)
			return -1;
		flags = nonblocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
		return fcntl(s, F_SETFL, flags);
	}

	inline bool socket_is_valid(socket_t s)
	{
		return (s >= 0);
	}

#endif
