/* Copyright (c) 2018-2025 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_SOCKET_C
#define GUARD_DEX_SOCKET_SOCKET_C 1
#define DEE_SOURCE

#include "libnet.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

typedef DeeSocketObject Socket;

PRIVATE ATTR_COLD int DCALL
err_no_af_support(neterrno_t error, sa_family_t af) {
	return DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
	                          "Address family %R is not supported",
	                          sock_getafnameorid(af));
}

PRIVATE NONNULL((1)) int DCALL
socket_ctor(Socket *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	int af, type, proto;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("socket", params: "
	af:?X2?Dint?Dstring,type:?X3?Dint?Dstring?N=!N,proto:?X3?Dint?Dstring?N=!N
", docStringPrefix: "socket", defineKwList: true);]]]*/
	static DEFINE_KWLIST(socket_kwlist, { KEX("af", 0x5b382189, 0x6dfc80eaad0fec70), KEX("type", 0x79f23513, 0xd08fc7c8724a760a), KEX("proto", 0x3046135, 0x87e38aec26b344e), KEND });
#define socket_socket_params "af:?X2?Dint?Dstring,type:?X3?Dint?Dstring?N=!N,proto:?X3?Dint?Dstring?N=!N"
	struct {
		DeeObject *af;
		DeeObject *type;
		DeeObject *proto;
	} args;
	args.type = Dee_None;
	args.proto = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, socket_kwlist, "o|oo:socket", &args))
		goto err;
/*[[[end]]]*/
	if (sock_getafof(args.af, &af))
		goto err;
	if (sock_gettypeof(args.type, &type))
		goto err;
	if (sock_getprotoof(args.proto, &proto))
		goto err;
	if (af == AF_AUTO) {
		DeeError_Throwf(&DeeError_ValueError,
		                "AF_AUTO cannot be used during socket construction");
		goto err;
	}
	Dee_atomic_rwlock_init(&self->s_lock);
	self->s_state                 = SOCKET_FOPENED;
	self->s_sockaddr.sa.sa_family = (sa_family_t)af;
	self->s_type                  = type;
	self->s_proto                 = proto;
	/* Create the socket descriptor. */
	DBG_ALIGNMENT_DISABLE();
	self->s_socket = socket(af, type, proto);
	DBG_ALIGNMENT_ENABLE();
	if (self->s_socket == INVALID_SOCKET) {
		/* Check for errors. */
		neterrno_t err;
		DBG_ALIGNMENT_DISABLE();
		err = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (err == EAFNOSUPPORT) {
			err_no_af_support(err, (sa_family_t)af);
		} else if (err == EPROTONOSUPPORT) {
			DeeNet_ThrowErrorf(&DeeError_NoSupport, err,
			                   "Protocol %R is not supported or cannot be used with address family %R",
			                   sock_getprotonameorid(proto),
			                   sock_getafnameorid((sa_family_t)af));
		} else if (err == EPROTOTYPE) {
			DeeNet_ThrowErrorf(&DeeError_NoSupport, err,
			                   "Socket type %R cannot be used with protocol %R",
			                   sock_gettypenameorid(type),
			                   sock_getprotonameorid(proto));
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, err,
			                   "Failed to create `socket(%R, %R, %R)'",
			                   sock_getafnameorid((sa_family_t)af),
			                   sock_gettypenameorid(type),
			                   sock_getprotonameorid(proto));
		}
		goto err;
	}
#if defined(AF_INET6) && defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
	if (af == AF_INET6) {
		int value = 0;
		DBG_ALIGNMENT_DISABLE();
		setsockopt(self->s_socket,
		           IPPROTO_IPV6,
		           IPV6_V6ONLY,
		           (char const *)&value,
		           sizeof(value));
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* AF_INET6 && IPPROTO_IPV6 && IPV6_V6ONLY */
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
socket_fini(Socket *__restrict self) {
	if (self->s_state & SOCKET_FOPENED) {
		DBG_ALIGNMENT_DISABLE();
		(void)closesocket(self->s_socket);
		DBG_ALIGNMENT_ENABLE();
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
socket_bool(Socket *__restrict self) {
	return !!(self->s_state & SOCKET_FOPENED);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_GetSockName(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result,
                      bool throw_error) {
	uint16_t state;
again:
	socket_read(self);
	state = self->s_state;
	if unlikely(!(state & (SOCKET_FBOUND | SOCKET_FCONNECTED | SOCKET_FOPENED))) {
		socket_endread(self);
		if (state & (SOCKET_FBINDING | SOCKET_FCONNECTING)) {
			/* Socket is currently binding/connecting (wait a bit more) */
			DeeThread_SleepNoInt(1000);
			goto again;
		}
		if (throw_error) {
			if (!(state & SOCKET_FOPENED)) {
				err_socket_closed(EINVAL, self);
			} else {
				DeeNet_ThrowErrorf(&DeeError_NotConnected, ENOTCONN,
				                   "Socket %k is neither bound nor connected",
				                   self);
			}
		}
		return -1;
	}
	if (!(state & SOCKET_FHASSOCKADDR)) {
		/* Load the socket's name on first access. */
		socklen_t addrlen;
		int error;
		addrlen = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family, self->s_proto);
		DBG_ALIGNMENT_DISABLE();
		error = getsockname(self->s_socket, &result->sa, &addrlen);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(error < 0) {
			socket_endread(self);
			if (throw_error) {
				DBG_ALIGNMENT_DISABLE();
				error = GET_NET_ERROR();
				DBG_ALIGNMENT_ENABLE();
				if (error == EBADF || error == ENOTSOCK || error == EINVAL) {
					err_socket_closed(error, self);
				} else if (error == EOPNOTSUPP) {
					DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
					                   "The socket's protocol %K does not support socket names",
					                   sock_getprotonameorid(self->s_proto));
				} else {
					DeeNet_ThrowErrorf(&DeeError_NetError, error,
					                   "Failed to get name of socket %k",
					                   self);
				}
			}
			return -1;
		}
		memcpy(&self->s_sockaddr, result, sizeof(SockAddr));
		atomic_or(&self->s_state, SOCKET_FHASSOCKADDR);
		socket_endread(self);
	} else {
		socket_endread(self);
		memcpy(result, &self->s_sockaddr, sizeof(SockAddr));
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_GetPeerAddr(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result,
                      bool throw_error) {
	socklen_t socklen;
	int ok = 0;
	socket_read(self);
	if (self->s_state & SOCKET_FHASPEERADDR) {
		memcpy(result, &self->s_peeraddr, sizeof(SockAddr));
		socket_endread(self);
		return 0;
	}
	socket_endread(self);

	socklen = (socklen_t)SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,
	                                     self->s_proto);
	socket_read(self);
	DBG_ALIGNMENT_DISABLE();
	if unlikely(getpeername(self->s_socket, &result->sa, &socklen) < 0) {
		ok = -1;
	} else if (!(self->s_state & SOCKET_FHASPEERADDR)) {
		memcpy(&self->s_peeraddr, result, sizeof(SockAddr));
		self->s_state |= SOCKET_FHASPEERADDR;
	}
	DBG_ALIGNMENT_ENABLE();
	socket_endread(self);
	if unlikely(ok != 0 && throw_error) {
		neterrno_t err;
		DBG_ALIGNMENT_DISABLE();
		err = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (err == ENOTCONN) {
			DeeNet_ThrowErrorf(&DeeError_NotConnected, err,
			                   "Socket %k is not connected", self);
		} else if (err == EOPNOTSUPP) {
			DeeNet_ThrowErrorf(&DeeError_NoSupport, err,
			                   "Cannot query peer address for socket protocol %R",
			                   sock_getprotonameorid(self->s_proto));
		} else if (err == EINVAL) {
			err_socket_closed(err, self);
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, err,
			                   "Failed to query peer address of socket %k",
			                   self);
		}
	}
	return ok;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeSockAddrObject *DCALL
socket_sockname_get(Socket *__restrict self) {
	DREF DeeSockAddrObject *result;
	result = DeeObject_MALLOC(DeeSockAddrObject);
	if unlikely(!result)
		goto done;
	if unlikely(DeeSocket_GetSockName(self, &result->sa_addr, true))
		goto err_r;
	DeeObject_Init(result, &DeeSockAddr_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeSockAddrObject *DCALL
socket_peeraddr_get(Socket *__restrict self) {
	DREF DeeSockAddrObject *result;
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeObject_MALLOC(DeeSockAddrObject);
	if unlikely(!result)
		goto done;
	if unlikely(DeeSocket_GetPeerAddr(self, &result->sa_addr, true))
		goto err_r;
	DeeObject_Init(result, &DeeSockAddr_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE DEFINE_STRING(shutdown_all, "rw");

PRIVATE WUNUSED NONNULL((1)) int DCALL
socket_do_shutdown(Socket *__restrict self, int how) {
	int error;
	DBG_ALIGNMENT_DISABLE();
	error = shutdown(self->s_socket, how);
	if (error < 0 && GET_NET_ERROR() == ENOTCONN)
		error = 0;
	DBG_ALIGNMENT_ENABLE();
	return error;
}


PRIVATE ATTR_COLD int DCALL
err_invalid_shutdown_how(neterrno_t error, int how) {
	return DeeNet_ThrowErrorf(&DeeError_ValueError, error,
	                          "Invalid shutdown mode %x", how);
}

PRIVATE ATTR_COLD int DCALL
err_shutdown_failed(Socket *__restrict self, neterrno_t error) {
	return DeeNet_ThrowErrorf(&DeeError_NetError, error,
	                          "Failed to shutdown socket %k",
	                          self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_close(Socket *self, size_t argc, DeeObject *const *argv) {
	sock_t socket_handle;
	DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
	DeeArg_Unpack0Or1(err, argc, argv, "close", &shutdown_mode);
	if (!DeeString_Check(shutdown_mode) ||
	    !DeeString_IsEmpty(shutdown_mode)) {
		int error, mode;
		uint16_t new_state;
		if unlikely(get_shutdown_modeof(shutdown_mode, &mode))
			goto err;
		/* First of: acquire read-access and call shutdown(). */
		if (mode == SHUT_RD) {
			new_state = SOCKET_FSHUTDOWN_R;
		} else if (mode == SHUT_WR) {
			new_state = SOCKET_FSHUTDOWN_W;
		} else {
			new_state = SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W;
		}
again_shutdown:
		/* Test for interrupts. */
		if (DeeThread_CheckInterrupt())
			goto err;
		socket_read(self);
		if (atomic_fetchor(&self->s_state, SOCKET_FSHUTTINGDOWN) &
		    SOCKET_FSHUTTINGDOWN) {
			/* Special case: Another shutdown operation is still in progress. */
			socket_endread(self);
			DeeThread_SleepNoInt(1000);
			goto again_shutdown;
		}
		/* We'll have to do the shutdown. */
		error = socket_do_shutdown(self, mode);
		if unlikely(error < 0) {
			DBG_ALIGNMENT_DISABLE();
			error = GET_NET_ERROR();
			DBG_ALIGNMENT_ENABLE();
			atomic_and(&self->s_state, ~SOCKET_FSHUTTINGDOWN);
			socket_endread(self);
			if (error == EINVAL) {
				err_invalid_shutdown_how(error, mode);
			} else {
				err_shutdown_failed(self, error);
			}
			goto err;
		}
		/* Indicate that shutdown has been completed. */
		atomic_or(&self->s_state, new_state);
		atomic_and(&self->s_state, ~SOCKET_FSHUTTINGDOWN);
		socket_upgrade(self); /* it's ok if this blocks. */
	} else {
		socket_write(self);
	}
	/* Actually close the socket. */
	socket_handle  = self->s_socket;
	self->s_socket = INVALID_SOCKET;
	/* Indicate that the socket has been closed by deleting all flags. */
	self->s_state = 0;
	COMPILER_WRITE_BARRIER();
	socket_endwrite(self);
	/* Close the old socket handle. */
	DBG_ALIGNMENT_DISABLE();
	(void)closesocket(socket_handle);
	DBG_ALIGNMENT_ENABLE();
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_shutdown(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
	DeeArg_Unpack0Or1(err, argc, argv, "shutdown", &shutdown_mode);
	if (!DeeString_Check(shutdown_mode) ||
	    !DeeString_IsEmpty(shutdown_mode)) {
		int error, mode;
		uint16_t new_state;
		if unlikely(get_shutdown_modeof(shutdown_mode, &mode))
			goto err;
		/* First of: acquire read-access and call shutdown(). */
		if (mode == SHUT_RD) {
			new_state = SOCKET_FSHUTDOWN_R;
		} else if (mode == SHUT_WR) {
			new_state = SOCKET_FSHUTDOWN_W;
		} else {
			new_state = SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W;
		}
again_shutdown:
		/* Test for interrupts. */
		if (DeeThread_CheckInterrupt())
			goto err;
		socket_read(self);
		if (atomic_fetchor(&self->s_state, SOCKET_FSHUTTINGDOWN) &
		    SOCKET_FSHUTTINGDOWN) {
			/* Special case: The socket is already being shut down. */
			socket_endread(self);
			DeeThread_SleepNoInt(1000);
			goto again_shutdown;
		}
		/* Actually do the shutdown. */
		error = socket_do_shutdown(self, mode);
		if unlikely(error < 0) {
			DBG_ALIGNMENT_DISABLE();
			error = GET_NET_ERROR();
			DBG_ALIGNMENT_ENABLE();
			if (error == ENOTCONN) {
				/* Ignore not-connected errors. */
			} else {
				atomic_and(&self->s_state, ~SOCKET_FSHUTTINGDOWN);
				socket_endread(self);
				if (error == EINVAL) {
					err_invalid_shutdown_how(error, mode);
				} else if (error == EBADF || error == ENOTSOCK) {
					err_socket_closed(error, self);
				} else {
					err_shutdown_failed(self, error);
				}
				goto err;
			}
		}
		/* Indicate that shutdown has been completed. */
		atomic_or(&self->s_state, new_state);
		atomic_and(&self->s_state, ~SOCKET_FSHUTTINGDOWN);
		socket_endread(self);
	}
	return_none;
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_addr_not_available(neterrno_t error,
                       SockAddr const *__restrict addr,
                       int prototype) {
	return DeeNet_ThrowErrorf(&DeeError_AddrNotAvail, error,
	                          "The specified address %K is not available from the local machine",
	                          SockAddr_ToString(addr, prototype, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_Bind(DeeSocketObject *__restrict self,
               SockAddr const *__restrict addr) {
	int error;
	uint16_t state;
	neterrno_t error_code;
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
	/* Enter the binding-state. */
	if (atomic_fetchor(&self->s_state, SOCKET_FBINDING) & SOCKET_FBINDING) {

		socket_endread(self);
		/* The socket is already in the middle of a binding-call. */
		DeeThread_SleepNoInt(1000);
		goto again;
	}
	DBG_ALIGNMENT_DISABLE();
#if defined(SOL_SOCKET) && defined(SO_REUSEADDR) /* Enable local address reuse */
	{
		int yes = 1;
		setsockopt(self->s_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes));
	}
#endif
	/* Do the bind system call. */
	error = bind(self->s_socket, (struct sockaddr *)addr, (socklen_t)SockAddr_Sizeof(addr->sa.sa_family, self->s_proto));
	DBG_ALIGNMENT_ENABLE();
	if likely(error >= 0) {
		/* Save the (now) active socket address. */
		memcpy(&self->s_sockaddr, addr, sizeof(SockAddr));
		COMPILER_WRITE_BARRIER();
		atomic_or(&self->s_state, SOCKET_FBOUND | SOCKET_FHASSOCKADDR);
	}
	/* Unset the binding-flag. */
	state = atomic_fetchand(&self->s_state, ~SOCKET_FBINDING);
	socket_endread(self);
	if likely(error >= 0)
		return 0;
	DBG_ALIGNMENT_DISABLE();
	error_code = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
	/* Handle errors. */
	if (error_code == EADDRINUSE) {
		DeeNet_ThrowErrorf(&DeeError_AddrInUse, error_code,
		                   "The specified address %K is already in use",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	} else if (error_code == EADDRNOTAVAIL) {
		err_addr_not_available(error_code, addr, self->s_proto);
	} else if (error_code == EBADF || error_code == ENOTSOCK) {
		goto err_closed;
	} else if (error_code == EINVAL) {
		if (!(state & SOCKET_FOPENED) ||
		    (state & SOCKET_FSHUTDOWN_R)) {
err_closed:
			err_socket_closed(error_code, self);
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, error_code,
			                   "Cannot rebind socket %k of address family %K to address %K", self,
			                   sock_getafnameorid(self->s_proto),
			                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
		}
	} else if (error_code == EOPNOTSUPP) {
		DeeNet_ThrowErrorf(&DeeError_NoSupport, error_code,
		                   "The socket protocol %K does allow address binding",
		                   sock_getafnameorid(self->s_proto));
	} else if (error_code == EISCONN) {
		DeeNet_ThrowErrorf(&DeeError_IsConnected, error_code,
		                   "socket %k is already connected", self);
	} else {
		DeeNet_ThrowErrorf(&DeeError_NetError, error_code,
		                   "Failed to bind socket %k to address %K", self,
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	}
err:
	return -1;
}


#ifdef CONFIG_HOST_WINDOWS
/* On windows, accept() isn't interruptible (and neither is `select()')
 * But to still ensure that this blocking call can be interrupted by
 * `Thread.interrupt()', we need to use this system call right here: */
PRIVATE DWORD DCALL
select_interruptible(SOCKET hSocket, LONG lNetworkEvents, DWORD dwTimeout) {
	DWORD dwResult;
	HANDLE hSockEvent;
	/* NOTE: No need to do further error checking.
	 *       These API functions will propagate errors for us automatically. */
	DBG_ALIGNMENT_DISABLE();
	hSockEvent = WSACreateEvent();
	(void)WSAEventSelect(hSocket, hSockEvent, lNetworkEvents);
	dwResult = WSAWaitForMultipleEvents(1, &hSockEvent, FALSE, dwTimeout, TRUE);
	/* Apparently this is required to prevent the event also closing the socket??? */
	(void)WSAEventSelect(hSocket, hSockEvent, 0);
	(void)WSACloseEvent(hSockEvent);
	DBG_ALIGNMENT_ENABLE();
	return dwResult;
}
#endif /* CONFIG_HOST_WINDOWS */

#define SELECT_TIMEOUT_NANOSECONDS UINT64_C(100000000)
#define SELECT_TIMEOUT_MICROSECONDS \
	((uint32_t)(SELECT_TIMEOUT_NANOSECONDS / 1000))


PRIVATE ATTR_COLD NONNULL((2, 3)) int DCALL
err_network_down(neterrno_t error, Socket *__restrict socket,
                 SockAddr const *__restrict addr) {
	return DeeNet_ThrowErrorf(&DeeError_NetUnreachable, error,
	                          "No route to network of %K can be established",
	                          SockAddr_ToString(addr, socket->s_proto,
	                                            SOCKADDR_STR_FNOFAIL |
	                                            SOCKADDR_STR_FNODNS));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_Connect(DeeSocketObject *__restrict self,
                  SockAddr const *__restrict addr) {
	int error;
	socklen_t addrlen;
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
	/* Enter the connecting-state. */
	if (atomic_fetchor(&self->s_state, SOCKET_FCONNECTING) & SOCKET_FCONNECTING) {
		socket_endread(self);
		/* The socket is already in the middle of a connecting-call.
		 * Wait for that other connect() call to complete, then try again. */
		DeeThread_SleepNoInt(1000);
		goto again;
	}
	/* Do the connect system call. */
	addrlen = SockAddr_Sizeof(addr->sa.sa_family, self->s_proto);
	DBG_ALIGNMENT_DISABLE();
	error = connect(self->s_socket, (struct sockaddr *)addr, addrlen);
	DBG_ALIGNMENT_ENABLE();
	if (error < 0) {
		DBG_ALIGNMENT_DISABLE();
		error = (int)GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (error == EINPROGRESS
#ifdef EINTR
		    || error == EINTR
#endif /* EINTR */
		    ) {
			/* Non-blocking connect still going on
			 * Use 'select' to wait for the socket to finish */
			socklen_t errlen;
restart_select:
			socket_endread(self);
			if (DeeThread_CheckInterrupt())
				goto err_connect_failed;
			socket_read(self);
#ifdef CONFIG_HOST_WINDOWS
			error = select_interruptible(self->s_socket,
			                             FD_CONNECT | FD_CLOSE,
			                             SELECT_TIMEOUT_MICROSECONDS / 1000);
			if (error != WSA_WAIT_EVENT_0) {
				if (error == WSA_WAIT_IO_COMPLETION ||
				    error == WSA_WAIT_TIMEOUT)
					goto restart_select; /* Timeout */
				goto err_connect_failure;
			}
#else /* CONFIG_HOST_WINDOWS */
			{
				struct timeval timeout;
				fd_set wfds;
				timeout.tv_sec  = (long)(SELECT_TIMEOUT_MICROSECONDS / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT_MICROSECONDS % 1000000);
				FD_ZERO(&wfds);
				FD_SET(self->s_socket, &wfds);
				DBG_ALIGNMENT_DISABLE();
				error = select(self->s_socket + 1, NULL, &wfds, NULL, &timeout);
				DBG_ALIGNMENT_ENABLE();
				if unlikely(error <= 0) {
					if (error == 0)
						goto restart_select;
					DBG_ALIGNMENT_DISABLE();
					error = (int)GET_NET_ERROR();
					DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
					if (error == EINTR)
						goto restart_select;
#endif /* EINTR */
					goto err_connect_failure;
				}
			}
#endif /* !CONFIG_HOST_WINDOWS */
			errlen = sizeof(error);
			/* Check for an error in the connect operation */
			DBG_ALIGNMENT_DISABLE();
			if (getsockopt(self->s_socket, SOL_SOCKET, SO_ERROR,
			               (char *)&error, &errlen))
				error = 0;
			DBG_ALIGNMENT_ENABLE();
			if unlikely(error)
				goto err_connect_failure;
		}
	}
	atomic_or(&self->s_state, SOCKET_FCONNECTED | SOCKET_FHASSOCKADDR);
	/* Unset the connecting-flag. */
	atomic_and(&self->s_state, ~SOCKET_FCONNECTING);
	socket_endread(self);
	return 0;
err_connect_failed:
	atomic_and(&self->s_state, ~SOCKET_FCONNECTING);
	goto err;
err_connect_failure:
	atomic_and(&self->s_state, ~SOCKET_FCONNECTING);
	socket_endread(self);
	/* Handle errors. */
	if (error == EADDRNOTAVAIL) {
		err_addr_not_available((neterrno_t)error, addr, self->s_proto);
	} else if (error == EAFNOSUPPORT) {
		err_no_af_support((neterrno_t)error, addr->sa.sa_family);
	} else if (error == EBADF || error == ENOTSOCK) {
		err_socket_closed(error, self);
	} else if (error == EOPNOTSUPP) {
		DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
		                   "The socket %k is currently listening and cannot connect",
		                   self);
	} else if (error == EALREADY || error == EISCONN) {
		DeeNet_ThrowErrorf(&DeeError_IsConnected, error,
		                   "Socket %k is already connected or connecting", self);
	} else if (error == ECONNRESET) {
		DeeNet_ThrowErrorf(&DeeError_ConnectReset, error,
		                   "The target %K reset the connection request before it could complete",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	} else if (error == ECONNREFUSED) {
		DeeNet_ThrowErrorf(&DeeError_ConnectRefused, error,
		                   "Target %K is not listening or has refused to connect",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	} else if (error == ENETUNREACH) {
		err_network_down(error, self, addr);
	} else if (error == EHOSTUNREACH) {
		DeeNet_ThrowErrorf(&DeeError_HostUnreachable, error,
		                   "The target host %K cannot be reached",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	} else if (error == EPROTOTYPE) {
		DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
		                   "The address %K uses a different type than socket %k",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS), self);
	} else if (error == ETIMEDOUT) {
		DeeNet_ThrowErrorf(&DeeError_TimedOut, error,
		                   "Timed out while attempting to connect to %K",
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	} else {
		DeeNet_ThrowErrorf(&DeeError_NetError, error,
		                   "Failed to connect socket %k with address %K", self,
		                   SockAddr_ToString(addr, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
	}
err:
	return -1;
}


#ifdef CONFIG_NO_NOTIFICATIONS

PRIVATE int DCALL get_default_backlog(void) {
	return 5;
}

#else /* CONFIG_NO_NOTIFICATIONS */

DEFINE_NOTIFY_ENVIRON_INTEGER(uint16_t, maxbacklog, 5, "DEEMON_MAXBACKLOG");
PRIVATE int DCALL get_default_backlog(void) {
	uint16_t result;
	if (maxbacklog_get(&result))
		goto err;
	DBG_ALIGNMENT_DISABLE();
	return (int)(unsigned int)result;
err:
	return -1;
}

#endif /* !CONFIG_NO_NOTIFICATIONS */


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSocket_Listen(DeeSocketObject *__restrict self, int max_backlog) {
	int error;
	uint16_t state;
	neterrno_t error_code;
	if (max_backlog < 0) {
		max_backlog = get_default_backlog();
		DBG_ALIGNMENT_ENABLE();
		if unlikely(max_backlog < 0)
			goto err;
	}
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
	/* Enter the listening-state. */
	if (atomic_fetchor(&self->s_state, SOCKET_FSTARTLISTENING) &
	    SOCKET_FSTARTLISTENING) {

		socket_endread(self);
		/* The socket is already in the middle of a listening-call. */
		DeeThread_SleepNoInt(1000);
		goto again;
	}
	/* Do the listen system call. */
	DBG_ALIGNMENT_DISABLE();
	error = listen(self->s_socket, max_backlog);
	DBG_ALIGNMENT_ENABLE();
	if likely(error >= 0)
		atomic_or(&self->s_state, SOCKET_FLISTENING);
	/* Unset the binding-flag. */
	state = atomic_fetchand(&self->s_state, ~SOCKET_FSTARTLISTENING);
	socket_endread(self);
	if likely(error >= 0)
		return 0;
	DBG_ALIGNMENT_DISABLE();
	error_code = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
	/* Handle errors. */
	if (error_code == EDESTADDRREQ) {
		DeeNet_ThrowErrorf(&DeeError_NotBound, error_code,
		                   "Socket %k is not bound and protocol %K does not allow listening on an unbound socket",
		                   self, sock_getprotonameorid(self->s_proto));
	} else if (error_code == EOPNOTSUPP) {
		DeeNet_ThrowErrorf(&DeeError_NoSupport, error_code,
		                   "The socket protocol %K does not allow listening",
		                   sock_getprotonameorid(self->s_proto));
	} else if (error_code == EBADF || error_code == ENOTSOCK) {
		goto err_closed;
	} else if (error_code == EINVAL) {
		if (!(state & SOCKET_FOPENED) ||
		    (state & SOCKET_FSHUTDOWN_R)) {
err_closed:
			err_socket_closed(error_code, self);
		} else {
			DeeNet_ThrowErrorf(&DeeError_IsConnected, error_code,
			                   "Cannot start listening on socket %k that is already connected",
			                   self);
		}
	} else {
		DeeNet_ThrowErrorf(&DeeError_NetError, error_code,
		                   "Failed to start listening with socket %k", self);
	}
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeSocket_Accept(DeeSocketObject *__restrict self,
                 uint64_t timeout_nanoseconds,
                 sock_t *__restrict sock_fd,
                 SockAddr *__restrict addr) {
	int error;
	socklen_t socklen;
	sock_t client_socket;
	uint64_t timeout_microseconds;
	timeout_microseconds = timeout_nanoseconds / 1000;
restart:
	socklen = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family, self->s_proto);
	if (timeout_nanoseconds == (uint64_t)-1) {
		/* Accept. */
restart_no_timeout:
		if (DeeThread_CheckInterrupt())
			goto err;
		socket_read(self);
		if (!(self->s_state & SOCKET_FOPENED) ||
		    (self->s_state & SOCKET_FSHUTDOWN_R)) {
			socket_endread(self);
			error = EBADF;
			goto socket_was_closed;
		}
#ifdef CONFIG_HOST_WINDOWS
		error = select_interruptible(self->s_socket,
		                             FD_ACCEPT | FD_CLOSE,
		                             SELECT_TIMEOUT_MICROSECONDS / 1000);
		if (error != WSA_WAIT_EVENT_0) {
			socket_endread(self);
			if (error == WSA_WAIT_IO_COMPLETION ||
			    error == WSA_WAIT_TIMEOUT)
				goto restart_no_timeout;
			goto handle_accept_error_neterror;
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set rfds;
			timeout.tv_sec = (long)(SELECT_TIMEOUT_MICROSECONDS / 1000000);
			timeout.tv_usec = (long)(SELECT_TIMEOUT_MICROSECONDS % 1000000);
			FD_ZERO(&rfds);
			FD_SET(self->s_socket, &rfds);
			error = select(self->s_socket + 1, &rfds, NULL, NULL, &timeout);
		}
		if (error <= 0) { /* Timeout or error */
			socket_endread(self);
			DBG_ALIGNMENT_DISABLE();
			if (error < 0
#ifdef EINTR
			    && GET_NET_ERROR() != EINTR
#endif /* EINTR */
			    ) {
				DBG_ALIGNMENT_ENABLE();
				goto handle_accept_error_neterror;
			}
			DBG_ALIGNMENT_ENABLE();
			goto restart_no_timeout;
		}
#endif /* !CONFIG_HOST_WINDOWS */
		DBG_ALIGNMENT_DISABLE();
		client_socket = accept(self->s_socket, &addr->sa, &socklen);
		DBG_ALIGNMENT_ENABLE();
		socket_endread(self);
	} else if (timeout_microseconds == 0) {
		/* Try-accept. */
do_try_select:
		socket_read(self);
		if (!(self->s_state & SOCKET_FOPENED) ||
		    (self->s_state & SOCKET_FSHUTDOWN_R)) {
			socket_endread(self);
			error = EBADF;
			goto socket_was_closed;
		}
#ifdef CONFIG_HOST_WINDOWS
		error = select_interruptible(self->s_socket, FD_ACCEPT | FD_CLOSE, 0);
		if (error != WSA_WAIT_EVENT_0) {
			socket_endread(self);
			if (error == WSA_WAIT_IO_COMPLETION ||
			    error == WSA_WAIT_TIMEOUT)
				return 1; /* Timeout */
			goto handle_accept_error_neterror;
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set rfds;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1; /* Prevent the OS from discarding this request. */
			FD_ZERO(&rfds);
			FD_SET(self->s_socket, &rfds);
			DBG_ALIGNMENT_DISABLE();
			error = select(self->s_socket + 1, &rfds, NULL, NULL, &timeout);
			DBG_ALIGNMENT_ENABLE();
			if (error <= 0) { /* Timeout or error */
				socket_endread(self);
				if (error < 0) {
#ifdef EINTR
					DBG_ALIGNMENT_DISABLE();
					if (GET_NET_ERROR() == EINTR) {
						DBG_ALIGNMENT_ENABLE();
#if 1 /* Handle EINTR the same way we would handle a timeout. */
						return 1;
#else
						goto do_try_select;
#endif
					}
					DBG_ALIGNMENT_ENABLE();
#endif /* EINTR */
					goto handle_accept_error_neterror;
				}
				return 1;
			}
		}
#endif /* !CONFIG_HOST_WINDOWS */
		/* Acquire an exclusive lock to prevent anyone else from stealing our client. */
		if (!socket_tryupgrade(self)) {
			socket_endread(self);
			DeeThread_SleepNoInt(1000);
			goto do_try_select;
		}
		DBG_ALIGNMENT_DISABLE();
		client_socket = accept(self->s_socket, &addr->sa, &socklen);
		DBG_ALIGNMENT_ENABLE();
		socket_endwrite(self);
	} else {
		uint64_t end_time;
		end_time = DeeThread_GetTimeMicroSeconds();
		end_time += timeout_microseconds;
		/* Timed-accept. */
do_timed_select:
		if (DeeThread_CheckInterrupt())
			goto err;
		ASSERT(timeout_microseconds != 0);
		socket_read(self);
		if (!(self->s_state & SOCKET_FOPENED) ||
		    (self->s_state & SOCKET_FSHUTDOWN_R)) {
			socket_endread(self);
			error = EBADF;
			goto socket_was_closed;
		}
#ifdef CONFIG_HOST_WINDOWS
		{
			DWORD timeout = SELECT_TIMEOUT_MICROSECONDS;
			if (timeout > timeout_microseconds)
				timeout = (DWORD)timeout_microseconds;
			error = select_interruptible(self->s_socket, FD_ACCEPT | FD_CLOSE,
			                             timeout / 1000);
		}
		if (error != WSA_WAIT_EVENT_0) {
			uint64_t now;
			socket_endread(self);
			if (error != WSA_WAIT_IO_COMPLETION &&
			    error != WSA_WAIT_TIMEOUT)
				goto handle_accept_error_neterror; /* Network error. */
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= end_time)
				return 1; /* Timeout */
			/* Update the remaining time.
			 * NOTE: Never ZERO because `end_time > now' right now. */
			timeout_microseconds = end_time - now;
			goto do_timed_select;
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set rfds;
			if (timeout_microseconds < SELECT_TIMEOUT_MICROSECONDS) {
				timeout.tv_sec = (long)(timeout_microseconds / 1000000);
				timeout.tv_usec = (long)(timeout_microseconds % 1000000);
			} else {
				timeout.tv_sec = (long)(SELECT_TIMEOUT_MICROSECONDS / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT_MICROSECONDS % 1000000);
			}
			FD_ZERO(&rfds);
			FD_SET(self->s_socket, &rfds);
			DBG_ALIGNMENT_DISABLE();
			error = select(self->s_socket + 1, &rfds, NULL, NULL, &timeout);
			DBG_ALIGNMENT_ENABLE();
		}
		if (error <= 0) { /* Timeout or error */
			uint64_t now;
			socket_endread(self);
			DBG_ALIGNMENT_DISABLE();
			if (error < 0
#ifdef EINTR
			    && GET_NET_ERROR() != EINTR
#endif /* EINTR */
			    ) {
				DBG_ALIGNMENT_ENABLE();
				goto handle_accept_error_neterror;
			}
			DBG_ALIGNMENT_ENABLE();
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= end_time)
				return 1; /* Timeout */
			/* Update the remaining time.
			 * NOTE: Never ZERO because `end_time > now' right now. */
			timeout_microseconds = end_time - now;
			goto do_timed_select;
		}
#endif /* !CONFIG_HOST_WINDOWS */
		/* Acquire an exclusive lock to prevent anyone else from stealing our client. */
		if (!socket_tryupgrade(self)) {
			uint64_t now;
			socket_endread(self);
			DeeThread_SleepNoInt(1000);
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= end_time)
				goto do_try_select;
			timeout_microseconds = end_time - now;
			goto do_timed_select;
		}
		/* Accept the client. */
		DBG_ALIGNMENT_DISABLE();
		client_socket = accept(self->s_socket, &addr->sa, &socklen);
		DBG_ALIGNMENT_ENABLE();
		socket_endwrite(self);
	}
	/* Check for errors and save the new socket in the caller-given pointer. */
	if (client_socket == INVALID_SOCKET)
		goto handle_accept_error_neterror;
	*sock_fd = client_socket;
	return 0;
handle_accept_error_neterror:
	DBG_ALIGNMENT_DISABLE();
	error = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
/*handle_accept_error:*/
	/* Start over on timeout. */
	if (error == EWOULDBLOCK)
		goto restart_after_timeout;
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	if (error == EAGAIN)
		goto restart_after_timeout;
#endif /* EAGAIN && EAGAIN != EWOULDBLOCK */
	/* Ignore this case (Can happen when the client changed their mind) */
	if (error == ECONNABORTED)
		goto restart;
#ifdef EINTR
	/* Same deal as `ECONNABORTED' -- Start over. */
	if (error == EINTR)
		goto restart;
#endif /* EINTR */
#ifdef ENOMEM
	if (error == ENOMEM) {
		if (Dee_CollectMemory(1))
			goto restart;
		goto err;
	}
#endif /* ENOMEM */
	if (error == EBADF || error == ENOTSOCK) {
socket_was_closed:
		err_socket_closed(error, self);
	} else if (error == EINVAL) {
		DeeNet_ThrowErrorf(&DeeError_NotListening, error,
		                   "Cannot accept connections from a socket %k that is not listening",
		                   self);
	} else if (error == EOPNOTSUPP) {
		DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
		                   "The type %K of socket %k does not support accepting connections",
		                   sock_gettypenameorid(self->s_type), self);
	} else {
		DeeNet_ThrowErrorf(&DeeError_NetError, error,
		                   "Failed to accept connections from %k",
		                   self);
	}
err:
	return -1;
restart_after_timeout:
	if (timeout_microseconds == 0)
		return 1;
	goto restart;
}

#ifdef CONFIG_HOST_WINDOWS
#define send(socket, buffer, length, flags) \
	((Dee_ssize_t)send(socket, (char const *)(buffer), (int)(length), flags))
#define sendto(socket, buffer, length, flags, target, targetlen) \
	((Dee_ssize_t)sendto(socket, (char const *)(buffer), (int)(length), flags, target, targetlen))
#define recv(socket, buffer, length, flags) \
	((Dee_ssize_t)recv(socket, (char *)(buffer), (int)(length), flags))
#define recvfrom(socket, buffer, length, flags, target, targetlen) \
	((Dee_ssize_t)recvfrom(socket, (char *)(buffer), (int)(length), flags, target, targetlen))
#endif /* CONFIG_HOST_WINDOWS */


PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_message_too_large(neterrno_t error, Socket *__restrict self, size_t bufsize) {
	return DeeNet_ThrowErrorf(&DeeError_MessageSize, error,
	                          "Message consisting of %" PRFuSIZ " bytes is too large for socket %k",
	                          bufsize, self);
}

PRIVATE char const transfer_context_send[] = "transfer";
PRIVATE char const transfer_context_recv[] = "receive";
PRIVATE ATTR_COLD NONNULL((2, 3)) int DCALL
err_invalid_transfer_mode(neterrno_t error, Socket *__restrict self,
                          char const *__restrict context, int mode) {
	return DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
	                          "Socket %k does not support %s mode %K",
	                          self, context, sock_getmsgflagsnameorid(mode));
}


#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
PRIVATE WUNUSED NONNULL((1)) int DCALL
socket_configure_recv(Socket *__restrict self) {
	(void)self;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
socket_configure_send(Socket *__restrict self) {
	(void)self;
	return 0;
}

PRIVATE ATTR_COLD int DCALL
err_configure_recv(Socket *__restrict self) {
	/* Handle errors that may have occurred during `socket_configure_recv()' */
	int error;
	DBG_ALIGNMENT_DISABLE();
	error = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
	return DeeNet_ThrowErrorf(&DeeError_NetError, error,
	                          "Failed to configure socket %k for receiving",
	                          self);
}

PRIVATE ATTR_COLD int DCALL
err_configure_send(Socket *__restrict self) {
	/* Handle errors that may have occurred during `socket_configure_send()' */
	int error;
	DBG_ALIGNMENT_DISABLE();
	error = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
	return DeeNet_ThrowErrorf(&DeeError_NetError, error,
	                          "Failed to configure socket %k for sending",
	                          self);
}
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */


/* Wait for data being ready to be sent. */
#ifdef CONFIG_HOST_WINDOWS
#define WAITFORDATA_RECV (FD_READ | FD_CLOSE)
#define WAITFORDATA_SEND (FD_WRITE | FD_CLOSE)
#else /* CONFIG_HOST_WINDOWS */
#define WAITFORDATA_RECV 0
#define WAITFORDATA_SEND 1
#endif /* !CONFIG_HOST_WINDOWS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
wait_for_data(Socket *__restrict self,
              uint64_t end_time, int mode) {
	int error;
#ifdef EINTR
restart:
#endif /* EINTR */
	if (end_time == (uint64_t)-1) {
		/* wait */
retry_infinite:
		socket_endread(self);
		if (DeeThread_CheckInterrupt())
			goto err_nounlock;
		socket_read(self);
#ifdef CONFIG_HOST_WINDOWS
		{
			error = select_interruptible(self->s_socket, mode,
			                             SELECT_TIMEOUT_MICROSECONDS / 1000);
			if (error != WSA_WAIT_EVENT_0) {
				if (error != WSA_WAIT_IO_COMPLETION &&
				    error != WSA_WAIT_TIMEOUT)
					goto err_select; /* Network error. */
				goto retry_infinite;
			}
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set fds;
			timeout.tv_sec = (long)(SELECT_TIMEOUT_MICROSECONDS / 1000000);
			timeout.tv_usec = (long)(SELECT_TIMEOUT_MICROSECONDS % 1000000);
			FD_ZERO(&fds);
			FD_SET(self->s_socket, &fds);
			DBG_ALIGNMENT_DISABLE();
			error = select(self->s_socket + 1,
			               mode == WAITFORDATA_RECV ? &fds : NULL,
			               mode == WAITFORDATA_SEND ? &fds : NULL,
			               NULL, &timeout);
			DBG_ALIGNMENT_ENABLE();
			if (error <= 0) { /* Timeout or error */
				if (error < 0)
					goto err_select;
				goto retry_infinite;
			}
		}
#endif /* !CONFIG_HOST_WINDOWS */
	} else if (end_time == 0) {
		/* Try-wait */
#ifdef CONFIG_HOST_WINDOWS
		error = select_interruptible(self->s_socket, mode, 0);
		if (error != WSA_WAIT_EVENT_0) {
			if (error != WSA_WAIT_IO_COMPLETION &&
			    error != WSA_WAIT_TIMEOUT)
				goto err_select; /* Network error. */
			goto send_timeout;
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set fds;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1;
			FD_ZERO(&fds);
			FD_SET(self->s_socket, &fds);
			DBG_ALIGNMENT_DISABLE();
			error = select(self->s_socket + 1,
			               mode == WAITFORDATA_RECV ? &fds : NULL,
			               mode == WAITFORDATA_SEND ? &fds : NULL,
			               NULL, &timeout);
			DBG_ALIGNMENT_ENABLE();
			if (error <= 0) { /* Timeout or error */
				if (error < 0)
					goto err_select;
				goto send_timeout;
			}
		}
#endif /* !CONFIG_HOST_WINDOWS */
	} else {
		/* timed-wait */
		uint64_t now;
retry_timeout:
		socket_endread(self);
		if (DeeThread_CheckInterrupt())
			goto err_nounlock;
		now = DeeThread_GetTimeMicroSeconds();
		if unlikely(now >= end_time)
			goto send_timeout_nounlock;
		socket_read(self);
#ifdef CONFIG_HOST_WINDOWS
		{
			DWORD timeout = (DWORD)((end_time - now) / 1000);
			if (timeout > SELECT_TIMEOUT_MICROSECONDS / 1000)
				timeout = SELECT_TIMEOUT_MICROSECONDS / 1000;
			error = select_interruptible(self->s_socket, mode, timeout);
			if (error != WSA_WAIT_EVENT_0) {
				if (error != WSA_WAIT_IO_COMPLETION &&
				    error != WSA_WAIT_TIMEOUT)
					goto err_select; /* Network error. */
				goto retry_timeout;
			}
		}
#else /* CONFIG_HOST_WINDOWS */
		{
			struct timeval timeout;
			fd_set fds;
			uint64_t remaining_time = end_time - now;
			if (remaining_time < SELECT_TIMEOUT_MICROSECONDS) {
				timeout.tv_sec = (long)(remaining_time / 1000000);
				timeout.tv_usec = (long)(remaining_time % 1000000);
			} else {
				timeout.tv_sec = (long)(SELECT_TIMEOUT_MICROSECONDS / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT_MICROSECONDS % 1000000);
			}
			FD_ZERO(&fds);
			FD_SET(self->s_socket, &fds);
			DBG_ALIGNMENT_DISABLE();
			error = select(self->s_socket + 1,
			               mode == WAITFORDATA_RECV ? &fds : NULL,
			               mode == WAITFORDATA_SEND ? &fds : NULL,
			               NULL, &timeout);
			DBG_ALIGNMENT_ENABLE();
			if (error <= 0) { /* Timeout or error */
				if (error < 0)
					goto err_select;
				goto retry_timeout;
			}
		}
#endif /* !CONFIG_HOST_WINDOWS */
	}
	return 0;
err_select:
	DBG_ALIGNMENT_DISABLE();
	error = GET_NET_ERROR();
	DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
	if (error == EINTR)
		goto restart;
#endif /* EINTR */
	socket_endread(self);
	if (error == EBADF) {
		err_socket_closed(error, self);
	} else {
		DeeNet_ThrowErrorf(&DeeError_NetError, error,
		                   "Failed to wait for data to be %s socket %k",
		                   mode == WAITFORDATA_RECV ? "received from" : "send to",
		                   self);
	}
err_nounlock:
	return -1;
send_timeout:
	socket_endread(self);
send_timeout_nounlock:
	return -2;
}

/* Wait for data being ready to be sent or received. */
#define wait_for_send(self, end_time) wait_for_data(self, end_time, WAITFORDATA_SEND)
#define wait_for_recv(self, end_time) wait_for_data(self, end_time, WAITFORDATA_RECV)



PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_receive_not_connected(neterrno_t error, Socket *__restrict socket) {
	return DeeNet_ThrowErrorf(&DeeError_NotConnected, error,
	                          "Cannot receive data from socket %k that isn't connected",
	                          socket);
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_receive_timed_out(neterrno_t error, Socket *__restrict socket) {
	return DeeNet_ThrowErrorf(&DeeError_TimedOut, error,
	                          "Timed out while receiving data through socket %k",
	                          socket);
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_connect_reset(neterrno_t error, Socket *__restrict socket) {
	return DeeNet_ThrowErrorf(&DeeError_ConnectReset, error,
	                          "The connection of socket %k was reset by its peer",
	                          socket);
}


INTERN WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeSocket_Send(DeeSocketObject *__restrict self,
               uint64_t timeout_nanoseconds,
               void const *__restrict buf, size_t bufsize,
               int flags) {
	Dee_ssize_t result;
	uint64_t end_time, timeout_microseconds;
	/* TODO: Change this function to use nano-seconds internally! */
	timeout_microseconds = timeout_nanoseconds / 1000;
	if (timeout_nanoseconds == (uint64_t)-1)
		timeout_microseconds = (uint64_t)-1;
	end_time = timeout_microseconds;
	if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
		end_time = DeeThread_GetTimeMicroSeconds() + timeout_microseconds;
again:
	if (timeout_microseconds && DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
	if (!(self->s_state & SOCKET_FSENDCONFOK)) {
		if (socket_upgrade(self) ||
		    !(self->s_state & SOCKET_FSENDCONFOK)) {
			/* Configure the socket for sending before the first send call.
			 * >> Unless overwritten by user-configurations, the socket
			 *    should not be blocking when attempting to send data. */
			result = socket_configure_send(self);
			if unlikely(result) {
				socket_endwrite(self);
				err_configure_send(self);
				goto err;
			}
			atomic_or(&self->s_state, SOCKET_FSENDCONFOK);
			socket_downgrade(self);
		}
	}
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
	result = wait_for_send(self, end_time);
	/* NOTE: in the event of a timeout or error,
	 *      `wait_for_send()' will have unlocked the socket. */
	if unlikely(result)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	result = send(self->s_socket, buf, bufsize, flags);
	DBG_ALIGNMENT_ENABLE();
	socket_endread(self);
	if unlikely(result < 0) {
		neterrno_t error;
		DBG_ALIGNMENT_DISABLE();
		error = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (error == EWOULDBLOCK
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		    || error == EAGAIN
#endif /* EAGAIN && EAGAIN != EWOULDBLOCK */
#ifdef EINTR
		    || error == EINTR
#endif /* EINTR */
		    ) {
			if (timeout_microseconds != (uint64_t)-1) {
				if (!timeout_microseconds ||
				    DeeThread_GetTimeMicroSeconds() >= end_time)
					return -2; /* Timeout */
			}
			goto again;
		}
		if (error == EBADF || error == ENOTSOCK) {
			err_socket_closed(error, self);
		} else if (error == EOPNOTSUPP) {
			err_invalid_transfer_mode(error, self, transfer_context_send, flags);
		} else if (error == ENOTCONN
#ifdef EPIPE
		           || (error == EPIPE && !(self->s_state & SOCKET_FSHUTDOWN_W))
#endif /* EPIPE */
		           ) {
			DeeNet_ThrowErrorf(&DeeError_NotConnected, error,
			                   "Cannot send data through unconnected socket %k",
			                   self);
#ifdef EPIPE
		} else if (error == EPIPE) {
			err_socket_closed(error, self);
#endif /* EPIPE */
		} else if (error == EMSGSIZE) {
			err_message_too_large(error, self, bufsize);
		} else if (error == ECONNRESET) {
			err_connect_reset(error, self);
		} else if (error == EDESTADDRREQ) {
			DeeNet_ThrowErrorf(&DeeError_NotBound, error,
			                   "Socket %k isn't connection-oriented and has no peer address set",
			                   self);
		} else if (error == ENETDOWN || error == ENETUNREACH) {
			DeeNet_ThrowErrorf(&DeeError_NetUnreachable, error,
			                   "No route to network of connected to socket %k can be established",
			                   socket);
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, error,
			                   "Failed to send %" PRFuSIZ " bytes of data through socket %k",
			                   bufsize, self);
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeSocket_Recv(DeeSocketObject *__restrict self,
               uint64_t timeout_nanoseconds,
               void *__restrict buf, size_t bufsize,
               int flags) {
	Dee_ssize_t result;
	uint64_t end_time, timeout_microseconds;
	/* TODO: Change this function to use nano-seconds internally! */
	timeout_microseconds = timeout_nanoseconds / 1000;
	if (timeout_nanoseconds == (uint64_t)-1)
		timeout_microseconds = (uint64_t)-1;
	end_time = timeout_microseconds;
	if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
		end_time = DeeThread_GetTimeMicroSeconds() + timeout_microseconds;
again:
	if (timeout_microseconds && DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
	if (!(self->s_state & SOCKET_FRECVCONFOK)) {
		if (socket_upgrade(self) ||
		    !(self->s_state & SOCKET_FRECVCONFOK)) {
			/* Configure the socket for receiving before the first recv call.
			 * >> Unless overwritten by user-configurations, the socket
			 *    should not be blocking when attempting to receive data. */
			result = socket_configure_recv(self);
			if unlikely(result) {
				socket_endwrite(self);
				err_configure_recv(self);
				goto err;
			}
			atomic_or(&self->s_state, SOCKET_FRECVCONFOK);
			socket_downgrade(self);
		}
	}
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
	result = wait_for_recv(self, end_time);
	/* NOTE: in the event of a timeout or error,
	 *      `wait_for_recv()' will have unlocked the socket. */
	if unlikely(result)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	result = recv(self->s_socket, buf, bufsize, flags);
	DBG_ALIGNMENT_ENABLE();
	socket_endread(self);
	if unlikely(result < 0) {
		neterrno_t error;
		DBG_ALIGNMENT_DISABLE();
		error = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (error == EWOULDBLOCK
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		    || error == EAGAIN
#endif /* EAGAIN&& EAGAIN != EWOULDBLOCK */
#ifdef EINTR
		    || error == EINTR
#endif /* EINTR */
		    ) {
			if (timeout_microseconds != (uint64_t)-1) {
				if (!timeout_microseconds ||
				    DeeThread_GetTimeMicroSeconds() >= end_time)
					return -2; /* Timeout */
			}
			goto again;
		}
#ifdef ENOMEM
		if (error == ENOMEM) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
#endif /* ENOMEM */
		if (error == EBADF || error == ENOTSOCK) {
			err_socket_closed(error, self);
#ifdef MSG_OOB
		} else if (error == EINVAL && (flags & MSG_OOB)) {
			/* Indicate that nothing was read by returning 0. */
			result = 0;
			goto done;
#endif /* MSG_OOB */
		} else if (error == ECONNRESET) {
			err_connect_reset(error, self);
		} else if (error == ENOTCONN) {
			err_receive_not_connected(error, self);
		} else if (error == EOPNOTSUPP) {
			err_invalid_transfer_mode(error, self, transfer_context_recv, flags);
		} else if (error == ETIMEDOUT) {
			/* Different kind of timeout: The connection timed out, not the data transfer! */
			err_receive_timed_out(error, self);
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, error,
			                   "Failed to receive data through socket %k",
			                   self);
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}


PRIVATE ATTR_COLD NONNULL((2, 3)) int DCALL
err_host_unreachable(neterrno_t error, Socket *__restrict socket,
                     SockAddr const *__restrict target) {
	return DeeNet_ThrowErrorf(&DeeError_HostUnreachable, error,
	                          "The host specified by %K cannot be reached",
	                          SockAddr_ToString(target, socket->s_proto,
	                                            SOCKADDR_STR_FNOFAIL |
	                                            SOCKADDR_STR_FNODNS));
}

INTERN WUNUSED NONNULL((1, 3, 6)) Dee_ssize_t DCALL
DeeSocket_SendTo(DeeSocketObject *__restrict self,
                 uint64_t timeout_nanoseconds,
                 void const *__restrict buf, size_t bufsize,
                 int flags, SockAddr const *__restrict target) {
	Dee_ssize_t result;
	uint64_t end_time, timeout_microseconds;
	/* TODO: Change this function to use nano-seconds internally! */
	timeout_microseconds = timeout_nanoseconds / 1000;
	if (timeout_nanoseconds == (uint64_t)-1)
		timeout_microseconds = (uint64_t)-1;
	end_time = timeout_microseconds;
	if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
		end_time = DeeThread_GetTimeMicroSeconds() + timeout_microseconds;
again:
	if (timeout_microseconds && DeeThread_CheckInterrupt())
		goto err;
	socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
	if (!(self->s_state & SOCKET_FSENDCONFOK)) {
		if (socket_upgrade(self) ||
		    !(self->s_state & SOCKET_FSENDCONFOK)) {
			/* Configure the socket for sending before the first send call.
			 * >> Unless overwritten by user-configurations, the socket
			 *    should not be blocking when attempting to send data. */
			result = socket_configure_send(self);
			if unlikely(result) {
				socket_endwrite(self);
				err_configure_send(self);
				goto err;
			}
			atomic_or(&self->s_state, SOCKET_FSENDCONFOK);
			socket_downgrade(self);
		}
	}
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
	result = wait_for_send(self, end_time);
	/* NOTE: in the event of a timeout or error,
	 *      `wait_for_send()' will have unlocked the socket. */
	if unlikely(result)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	result = sendto(self->s_socket, buf, bufsize, flags, &target->sa,
	                SockAddr_Sizeof(target->sa.sa_family, self->s_proto));
	DBG_ALIGNMENT_ENABLE();
	socket_endread(self);
	if unlikely(result < 0) {
		neterrno_t error;
		DBG_ALIGNMENT_DISABLE();
		error = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (error == EWOULDBLOCK
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		    || error == EAGAIN
#endif /* EAGAIN && EAGAIN != EWOULDBLOCK */
#ifdef EINTR
		    || error == EINTR
#endif /* EINTR */
		    ) {
			if (timeout_microseconds != (uint64_t)-1) {
				if (!timeout_microseconds ||
				    DeeThread_GetTimeMicroSeconds() >= end_time)
					return -2; /* Timeout */
			}
			goto again;
		}
#ifdef ENOMEM
		if (error == ENOMEM) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
#endif /* ENOMEM */
		if (error == EBADF || error == ENOTSOCK) {
			err_socket_closed(error, self);
		} else if (error == EAFNOSUPPORT) {
			DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
			                   "Target address family %K cannot be used with socket %k",
			                   sock_getafnameorid(target->sa.sa_family), self);
		} else if (error == ECONNRESET) {
			DeeNet_ThrowErrorf(&DeeError_ConnectReset, error,
			                   "The peer %K has reset the connection",
			                   SockAddr_ToString(target, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
#if 0
		} else if (error == ENOTCONN
#ifdef EPIPE
		           || (error == EPIPE && !(self->s_state & SOCKET_FSHUTDOWN_W))
#endif /* EPIPE */
		           ) {
			DeeNet_ThrowErrorf(&DeeError_NotConnected, error,
			                   "Socket %k is connection-oriented, but not connected",
			                   self);
#endif
#ifdef EPIPE
		} else if (error == EPIPE) {
			err_socket_closed(error, self);
#endif /* EPIPE */
		} else if (error == EMSGSIZE) {
			err_message_too_large(error, self, bufsize);
		} else if (error == EOPNOTSUPP) {
			err_invalid_transfer_mode(error, self, transfer_context_send, flags);
		} else if (error == EHOSTUNREACH) {
			err_host_unreachable(error, self, target);
		} else if (error == EISCONN) {
			DeeNet_ThrowErrorf(&DeeError_IsConnected, error,
			                   "A target address %K was specified when socket %k is already connected",
			                   SockAddr_ToString(target, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS),
			                   self);
		} else if (error == ENETDOWN || error == ENETUNREACH) {
			err_network_down(error, self, target);
		} else if (error == EINVAL) {
			DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
			                   "The specified target address %K is not supported by this implementation",
			                   SockAddr_ToString(target, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, error,
			                   "Failed to send %" PRFuSIZ " bytes of data through socket %k to address %K",
			                   bufsize, self, SockAddr_ToString(target, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 6)) Dee_ssize_t DCALL
DeeSocket_RecvFrom(DeeSocketObject *__restrict self,
                   uint64_t timeout_nanoseconds,
                   void *__restrict buf, size_t bufsize,
                   int flags, SockAddr *__restrict source) {
	Dee_ssize_t result;
	socklen_t length;
	uint64_t end_time, timeout_microseconds;
	/* TODO: Change this function to use nano-seconds internally! */
	timeout_microseconds = timeout_nanoseconds / 1000;
	if (timeout_nanoseconds == (uint64_t)-1)
		timeout_microseconds = (uint64_t)-1;
	end_time = timeout_microseconds;
	if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
		end_time = DeeThread_GetTimeMicroSeconds() + timeout_microseconds;
again:
	if (timeout_microseconds && DeeThread_CheckInterrupt())
		goto err;
	length = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,
	                         self->s_proto);
	socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
	if (!(self->s_state & SOCKET_FRECVCONFOK)) {
		if (socket_upgrade(self) ||
		    !(self->s_state & SOCKET_FRECVCONFOK)) {
			/* Configure the socket for receiving before the first recv call.
			 * >> Unless overwritten by user-configurations, the socket
			 *    should not be blocking when attempting to receive data. */
			result = socket_configure_recv(self);
			if unlikely(result) {
				socket_endwrite(self);
				err_configure_recv(self);
				goto err;
			}
			atomic_or(&self->s_state, SOCKET_FRECVCONFOK);
			socket_downgrade(self);
		}
	}
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
	result = wait_for_recv(self, end_time);
	/* NOTE: in the event of a timeout or error,
	 *      `wait_for_recv()' will have unlocked the socket. */
	if unlikely(result)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	result = recvfrom(self->s_socket, buf, bufsize, flags, &source->sa, &length);
	DBG_ALIGNMENT_ENABLE();
	socket_endread(self);
	if unlikely(result < 0) {
		neterrno_t error;
		DBG_ALIGNMENT_DISABLE();
		error = GET_NET_ERROR();
		DBG_ALIGNMENT_ENABLE();
		if (error == EWOULDBLOCK
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		    || error == EAGAIN
#endif /* EAGAIN && EAGAIN != EWOULDBLOCK */
#ifdef EINTR
		    || error == EINTR
#endif /* EINTR */
		) {
			if (timeout_microseconds != (uint64_t)-1) {
				if (!timeout_microseconds ||
				    DeeThread_GetTimeMicroSeconds() >= end_time)
					return -2; /* Timeout */
			}
			goto again;
		}
#ifdef ENOMEM
		if (error == ENOMEM) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
#endif /* ENOMEM */
		if (error == EBADF || error == ENOTSOCK) {
			err_socket_closed(error, self);
#ifdef MSG_OOB
		} else if (error == EINVAL && (flags & MSG_OOB)) {
			/* Indicate that nothing was read by returning 0. */
			result = 0;
			goto done;
#endif /* MSG_OOB */
#if 0
		} else if (error == ENOTCONN) {
			err_receive_not_connected(error, self);
#endif
		} else if (error == EOPNOTSUPP) {
			err_invalid_transfer_mode(error, self, transfer_context_recv, flags);
		} else if (error == ECONNRESET) {
			err_connect_reset(error, self);
		} else if (error == ETIMEDOUT) {
			/* Different kind of timeout: The connection timed out, not the data transfer! */
			err_receive_timed_out(error, self);
		} else {
			DeeNet_ThrowErrorf(&DeeError_NetError, error,
			                   "Failed to receive data through socket %k",
			                   self);
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED size_t DCALL get_recv_chunksize(void) {
	/* XXX: Consult an environment variable? */
#if 0
	return 1;
#else
	return 2048;
#endif
}

PRIVATE WUNUSED size_t DCALL get_recv_burstsize(void) {
	/* XXX: Consult an environment variable? */
	/* The max size of a single Ethernet frame
	 * (although we can't really assume Ethernet connections...) */
	return 1542;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSocket_RecvData(DeeSocketObject *__restrict self,
                   uint64_t timeout_nanoseconds,
                   size_t max_bufsize, int flags,
                   SockAddr *source) {
	DREF DeeObject *result;
	Dee_ssize_t recv_length;
	if (max_bufsize == (size_t)-1) {
		if (!source) {
			struct bytes_printer printer = BYTES_PRINTER_INIT;
			size_t chunksize             = get_recv_chunksize();
			/* Variable-length buffer. */
			for (;;) {
				uint8_t *part = bytes_printer_alloc(&printer, chunksize);
				if unlikely(!part)
					goto err_printer;
				recv_length = DeeSocket_Recv(self, timeout_nanoseconds, part, chunksize, flags);
				if (recv_length == -2) {
					/* A timeout during the first pass must cause ITER_DONE to be returned. */
					if (BYTES_PRINTER_SIZE(&printer) == chunksize) {
						bytes_printer_fini(&printer);
						return ITER_DONE;
					}
					/* Handle timeout as end-of-data. */
					recv_length = 0;
				}
				if unlikely(recv_length < 0)
					goto err_printer;
				/* Release unused data. */
				bytes_printer_release(&printer, chunksize - (size_t)recv_length);
				/* Stop trying when no more data can be read. */
				if (!recv_length)
					break;
				/* Once we've managed to read ~something~, drop the timeout to not  */
				timeout_nanoseconds = 0;
			}
			/* Pack together the generated string. */
			return bytes_printer_pack(&printer);
err_printer:
			bytes_printer_fini(&printer);
			return NULL;
		}
		/* With a specific source address, we must receive data in a single burst! */
		max_bufsize = get_recv_burstsize();
	}
	/* Fixed-length buffer. */
	result = DeeBytes_NewBufferUninitialized(max_bufsize);
	if unlikely(!result)
		goto err;
	recv_length = source ? DeeSocket_RecvFrom(self, timeout_nanoseconds,
	                                          DeeBytes_DATA(result),
	                                          max_bufsize, flags, source)
	                     : DeeSocket_Recv(self, timeout_nanoseconds,
	                                      DeeBytes_DATA(result),
	                                      max_bufsize, flags);
	if unlikely(recv_length < 0) {
		Dee_DecrefDokill(result);
		if (recv_length == -2)
			return ITER_DONE; /* Timeout. */
		goto err;
	}
	if ((size_t)recv_length != max_bufsize)
		result = DeeBytes_TruncateBuffer(result, (size_t)recv_length);
	return result;
err:
	return NULL;
}





PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_bind(Socket *self, size_t argc, DeeObject *const *argv) {
	SockAddr addr;
	if unlikely(SockAddr_FromArgv(&addr,
		                           self->s_sockaddr.sa.sa_family,
		                           self->s_proto,
		                           self->s_type,
		                           argc,
		                           argv))
	goto err;
	DBG_ALIGNMENT_ENABLE();
	if unlikely(DeeSocket_Bind(self, &addr))
		goto err;
	return_none;
err:
	DBG_ALIGNMENT_ENABLE();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_connect(Socket *self, size_t argc, DeeObject *const *argv) {
	SockAddr addr;
	if unlikely(SockAddr_FromArgv(&addr,
	                              self->s_sockaddr.sa.sa_family,
	                              self->s_proto,
	                              self->s_type,
	                              argc,
	                              argv))
		goto err;
	DBG_ALIGNMENT_ENABLE();
	if unlikely(DeeSocket_Connect(self, &addr))
		goto err;
	return_none;
err:
	DBG_ALIGNMENT_ENABLE();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_listen(Socket *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("listen", params: """
	int max_backlog = -1;
""", docStringPrefix: "socket");]]]*/
#define socket_listen_params "max_backlog=!-1"
	struct {
		int max_backlog;
	} args;
	args.max_backlog = -1;
	DeeArg_Unpack0Or1X(err, argc, argv, "listen", &args.max_backlog, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if unlikely(DeeSocket_Listen(self, args.max_backlog))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_doaccept(Socket *__restrict self, uint64_t timeout_nanoseconds) {
	DREF Socket *result;
	int error;
	result = DeeObject_MALLOC(Socket);
	if unlikely(!result)
		goto err;
	error = DeeSocket_Accept(self, timeout_nanoseconds,
	                         &result->s_socket,
	                         &result->s_sockaddr);
	if unlikely(error < 0)
		goto err_r;
	if (error > 0) {
		DeeObject_FREE(result);
		return_none; /* Timeout */
	}
	/* Fill in the remaining members of the new socket. */
	Dee_atomic_rwlock_init(&result->s_lock);
	result->s_peeraddr.sa.sa_family = result->s_sockaddr.sa.sa_family;
	result->s_state                 = (SOCKET_FOPENED | SOCKET_FHASSOCKADDR | SOCKET_FCONNECTED);
	result->s_type                  = self->s_type;
	result->s_proto                 = self->s_proto;
	DeeObject_Init(result, &DeeSocket_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_accept(Socket *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("accept", params: """
	uint64_t timeout_nanoseconds = (uint64_t)-1;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	args.timeout_nanoseconds = (uint64_t)-1;
	DeeArg_Unpack0Or1X(err, argc, argv, "accept", &args.timeout_nanoseconds, UNPx64, DeeObject_AsUInt64M1);
/*[[[end]]]*/
	return socket_doaccept(self, args.timeout_nanoseconds);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_tryaccept(Socket *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryaccept");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tryaccept");
/*[[[end]]]*/
	return socket_doaccept(self, 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_recv(Socket *self, size_t argc, DeeObject *const *argv) {
	size_t max_size;
	uint64_t timeout;
	int flags;
	DREF DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("recv", params: """
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
	DeeObject *arg3 = NULL;
""");]]]*/
	struct {
		DeeObject *arg1;
		DeeObject *arg2;
		DeeObject *arg3;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	args.arg3 = NULL;
	DeeArg_UnpackStruct0Or1Or2Or3(err, argc, argv, "recv", &args, &args.arg1, &args.arg2, &args.arg3);
/*[[[end]]]*/
	if (!args.arg2) {
		max_size = (size_t)-1;
		timeout  = (uint64_t)-1;
		flags    = 0;
		if (args.arg1) {
			if (DeeString_Check(args.arg1)) {
				/* "(flags:?Dstring)->?Dstring\n" */
				if (sock_getmsgflagsof(args.arg1, &flags))
					goto err;
			} else {
				/* "(max_size=!-1,int flags=!P{})->?Dstring\n" */
				/* "(max_size=!-1,timeout_nanoseconds=!-1)->?Dstring\n" */
				if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
					goto err;
			}
		}
	} else if (!args.arg3) {
		/* "(max_size=!-1,int flags=!P{})->?Dstring\n" */
		/* "(max_size=!-1,timeout_nanoseconds=!-1)->?Dstring\n" */
		if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
			goto err;
		if (DeeString_Check(args.arg2)) {
			if (sock_getmsgflagsof(args.arg2, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			flags = 0;
			if (DeeObject_AsInt64(args.arg2, (int64_t *)&timeout))
				goto err;
		}
	} else {
		/* "(max_size=!-1,timeout_nanoseconds=!-1,flags=!P{})->?Dstring\n" */
		/* "(max_size=!-1,timeout_nanoseconds=!-1,flags=!0)->?Dstring\n" */
		if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
			goto err;
		if (DeeObject_AsInt64(args.arg2, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg3, &flags))
			goto err;
	}
	result = DeeSocket_RecvData(self, timeout, max_size, flags, NULL);
	/* Return an empty string when the timeout has expired. */
	if (result == ITER_DONE) {
		result = Dee_EmptyString;
		Dee_Incref(result);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_recvinto(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeBuffer buffer;
	uint64_t timeout;
	int flags;
	Dee_ssize_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("recvinto", params: """
	DeeObject *data;
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
""");]]]*/
	struct {
		DeeObject *data;
		DeeObject *arg1;
		DeeObject *arg2;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	DeeArg_UnpackStruct1Or2Or3(err, argc, argv, "recvinto", &args, &args.data, &args.arg1, &args.arg2);
/*[[[end]]]*/
	if (!args.arg1) {
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!args.arg2) {
		//"(dst:?DBytes,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		if (DeeString_Check(args.arg1)) {
			if (sock_getmsgflagsof(args.arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg2, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = DeeSocket_Recv(self,
	                        timeout,
	                        buffer.bb_base,
	                        buffer.bb_size,
	                        flags);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result < 0)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
socket_recvfrom(Socket *self, size_t argc, DeeObject *const *argv) {
	DREF DeeSockAddrObject *result_addr;
	DREF DeeTupleObject *result;
	DREF DeeObject *result_text;
	size_t max_size;
	uint64_t timeout;
	int flags;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("recvfrom", params: """
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
	DeeObject *arg3 = NULL;
""");]]]*/
	struct {
		DeeObject *arg1;
		DeeObject *arg2;
		DeeObject *arg3;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	args.arg3 = NULL;
	DeeArg_UnpackStruct0Or1Or2Or3(err, argc, argv, "recvfrom", &args, &args.arg1, &args.arg2, &args.arg3);
/*[[[end]]]*/
	if (!args.arg2) {
		max_size = (size_t)-1;
		timeout  = (uint64_t)-1;
		flags    = 0;
		if (args.arg1) {
			if (DeeString_Check(args.arg1)) {
				/* "(flags:?Dstring)->(sockaddr,string)\n" */
				if (sock_getmsgflagsof(args.arg1, &flags))
					goto err;
			} else {
				/* "(max_size=!-1,int flags=!P{})->(sockaddr,string)\n" */
				/* "(max_size=!-1,timeout_nanoseconds=!-1)->(sockaddr,string)\n" */
				if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
					goto err;
			}
		}
	} else if (!args.arg3) {
		/* "(max_size=!-1,int flags=!P{})->(sockaddr,string)\n" */
		/* "(max_size=!-1,timeout_nanoseconds=!-1)->(sockaddr,string)\n" */
		if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
			goto err;
		if (DeeString_Check(args.arg2)) {
			if (sock_getmsgflagsof(args.arg2, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			flags = 0;
			if (DeeObject_AsInt64(args.arg2, (int64_t *)&timeout))
				goto err;
		}
	} else {
		/* "(max_size=!-1,timeout_nanoseconds=!-1,flags=!P{})->(sockaddr,string)\n" */
		/* "(max_size=!-1,timeout_nanoseconds=!-1,flags=!0)->(sockaddr,string)\n" */
		if (DeeObject_AsSSize(args.arg1, (Dee_ssize_t *)&max_size))
			goto err;
		if (DeeObject_AsInt64(args.arg2, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg3, &flags))
			goto err;
	}

	/* Create the socket address object that's going to be returned. */
	result_addr = DeeObject_MALLOC(DeeSockAddrObject);
	if unlikely(!result_addr)
		goto err;

	/* Actually receive the data. */
	result_text = DeeSocket_RecvData(self, timeout, max_size, flags,
	                                 &result_addr->sa_addr);
	if unlikely(!result_text)
		goto err_addr;

	/* Create a new tuple to package the 2 objects. */
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_text;
	if (result_text == ITER_DONE) {
		/* A somewhat different story: must return (none, "") */
		DeeObject_FREE(result_addr);
		result->t_elem[0] = DeeNone_NewRef();
		result->t_elem[1] = DeeString_NewEmpty();
	} else {
		DeeObject_Init(result_addr, &DeeSockAddr_Type);
		/* Fill the result tuple with the socket address and text. */
		result->t_elem[0] = (DeeObject *)result_addr; /* Inherit */
		result->t_elem[1] = (DeeObject *)result_text; /* Inherit */
	}
	return result;
err_text:
	if (result_text != ITER_DONE)
		Dee_Decref(result_text);
err_addr:
	DeeObject_FREE(result_addr);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
socket_recvfrominto(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeBuffer buffer;
	uint64_t timeout;
	int flags;
	Dee_ssize_t result_size;
	DREF DeeSockAddrObject *result_addr;
	DREF DeeTupleObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("recvfrominto", params: """
	DeeObject *data;
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
""");]]]*/
	struct {
		DeeObject *data;
		DeeObject *arg1;
		DeeObject *arg2;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	DeeArg_UnpackStruct1Or2Or3(err, argc, argv, "recvfrominto", &args, &args.data, &args.arg1, &args.arg2);
/*[[[end]]]*/
	if (!args.arg1) {
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!args.arg2) {
		//"(dst:?DBytes,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		if (DeeString_Check(args.arg1)) {
			if (sock_getmsgflagsof(args.arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
		if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg2, &flags))
			goto err;
	}

	/* Create the socket address object that's going to be returned. */
	result_addr = DeeObject_MALLOC(DeeSockAddrObject);
	if unlikely(!result_addr)
		goto err;
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FWRITABLE))
		goto err_addr;
	result_size = DeeSocket_RecvFrom(self,
	                                 timeout,
	                                 buffer.bb_base,
	                                 buffer.bb_size,
	                                 flags,
	                                 &result_addr->sa_addr);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result_size < 0)
		goto err_addr;

	/* Create a new tuple to package the 2 objects. */
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_addr;
	if (result_size == 0) {
		/* A somewhat different story: must return (none, "") */
		DeeObject_FREE(result_addr);
		result->t_elem[0] = DeeNone_NewRef();
		result->t_elem[1] = DeeInt_NewZero();
	} else {
		DREF DeeObject *result_size_ob;
		result_size_ob = DeeInt_NewSize((size_t)result_size);
		if unlikely(!result_size_ob) {
			DeeTuple_FreeUninitializedPair(result);
			goto err_addr;
		}
		DeeObject_Init(result_addr, &DeeSockAddr_Type);
		/* Fill the result tuple with the socket address and result-size. */
		DeeTuple_SET(result, 0, (DeeObject *)result_addr); /* Inherit */
		DeeTuple_SET(result, 1, result_size_ob);           /* Inherit */
	}
	return result;
err_addr:
	DeeObject_FREE(result_addr);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_send(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeBuffer buffer;
	uint64_t timeout;
	int flags;
	Dee_ssize_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("send", params: """
	DeeObject *data;
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
""");]]]*/
	struct {
		DeeObject *data;
		DeeObject *arg1;
		DeeObject *arg2;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	DeeArg_UnpackStruct1Or2Or3(err, argc, argv, "send", &args, &args.data, &args.arg1, &args.arg2);
/*[[[end]]]*/
	if (!args.arg1) {
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!args.arg2) {
		/* "(data:?DBytes,flags=!P{})->?Dint\n" */
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		if (DeeString_Check(args.arg1)) {
			if (sock_getmsgflagsof(args.arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n" */
		if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg2, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeSocket_Send(self,
	                        timeout,
	                        buffer.bb_base,
	                        buffer.bb_size,
	                        flags);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result < 0) {
		if (result != -2)
			goto err;
		result = 0;
	}
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_sendto(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeBuffer buffer;
	SockAddr target_addr;
	uint64_t timeout;
	int flags;
	Dee_ssize_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("sendto", params: """
	DeeObject *target;
	DeeObject *data;
	DeeObject *arg1 = NULL;
	DeeObject *arg2 = NULL;
""");]]]*/
	struct {
		DeeObject *target;
		DeeObject *data;
		DeeObject *arg1;
		DeeObject *arg2;
	} args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	if (DeeArg_UnpackStruct(argc, argv, "oo|oo:sendto", &args))
		goto err;
/*[[[end]]]*/
	/* Construct the target address. */
	if (DeeTuple_Check(args.target)) {
		if (SockAddr_FromArgv(&target_addr,
		                      self->s_sockaddr.sa.sa_family,
		                      self->s_proto,
		                      self->s_type,
		                      DeeTuple_SIZE(args.target),
		                      DeeTuple_ELEM(args.target)))
			goto err;
	} else {
		if (SockAddr_FromArgv(&target_addr,
		                      self->s_sockaddr.sa.sa_family,
		                      self->s_proto,
		                      self->s_type,
		                      1,
		                      &args.target))
			goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	if (!args.arg1) {
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!args.arg2) {
		/* "(data:?DBytes,flags=!P{})->?Dint\n" */
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		if (DeeString_Check(args.arg1)) {
			if (sock_getmsgflagsof(args.arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n" */
		/* "(data:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n" */
		if (DeeObject_AsInt64(args.arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(args.arg2, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeSocket_SendTo(self,
	                          timeout,
	                          buffer.bb_base,
	                          buffer.bb_size,
	                          flags,
	                          &target_addr);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result < 0) {
		if (result != -2)
			goto err;
		result = 0;
	}
	return DeeInt_NewSize((size_t)result);
err:
	DBG_ALIGNMENT_ENABLE();
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_wasshutdown(Socket *self, size_t argc, DeeObject *const *argv) {
	int mode;
	uint16_t state = self->s_state;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("wasshutdown", params: """
	DeeObject *shutdown_mode=!Prw = (DeeObject *)&shutdown_all;
""");]]]*/
	struct {
		DeeObject *shutdown_mode;
	} args;
	args.shutdown_mode = (DeeObject *)&shutdown_all;
	DeeArg_Unpack0Or1(err, argc, argv, "wasshutdown", &args.shutdown_mode);
/*[[[end]]]*/
	if (DeeString_Check(args.shutdown_mode) &&
	    DeeString_IsEmpty(args.shutdown_mode))
		return_bool(!(state & SOCKET_FOPENED));
	if unlikely(get_shutdown_modeof(args.shutdown_mode, &mode))
		goto err;
	if (mode == SHUT_RD) {
		mode = state & SOCKET_FSHUTDOWN_R;
	} else if (mode == SHUT_WR) {
		mode = state & SOCKET_FSHUTDOWN_W;
	} else if (mode == SHUT_RDWR) {
		mode = (state & (SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W)) ==
		       (SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W);
	}
	/* If the socket is't open any more, then it was shut down. */
	if (!(state & SOCKET_FOPENED))
		mode = true;
	return_bool(mode);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_osfhandle(Socket *__restrict self) {
#ifdef CONFIG_HOST_WINDOWS
	return DeeInt_NewUIntptr((uintptr_t)self->s_socket);
#else /* CONFIG_HOST_WINDOWS */
	return DeeInt_NewInt((int)self->s_socket);
#endif /* !CONFIG_HOST_WINDOWS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_wasclosed(Socket *__restrict self) {
	return_bool(!(self->s_state & SOCKET_FOPENED));
}


PRIVATE struct type_method tpconst socket_methods[] = {
	TYPE_METHOD_F("close", &socket_close, METHOD_FNOREFESCAPE,
	              "(shutdown_mode:?Dint)\n"
	              "(shutdown_mode=!Prw)\n"
	              "#t{:Interrupt}"
	              "#tValueError{Invalid shutdown mode}"
	              "#tNetError{Failed to shutdown @this socket}"
	              "#tFileClosed{@this socket has already been closed}"
	              "Closes the socket's file descriptor. When @shutdown_socket is a non-empty :string, "
	              "?#shutdown will automatically be invoked on @this socket if it hasn't before\n"
	              "Note that in the event that ?#shutdown has already been called, "),
	TYPE_METHOD_F("shutdown", &socket_shutdown, METHOD_FNOREFESCAPE,
	              "(how:?Dint)\n"
	              "(how=!Prw)\n"
	              "#t{:Interrupt}"
	              "#tValueError{Invalid shutdown mode}"
	              "#tNetError{Failed to shutdown @this socket}"
	              "#tFileClosed{@this socket has already been closed}"
	              "Shuts down @this socket either for reading, for writing or for both"),
	TYPE_METHOD_F("bind", &socket_bind, METHOD_FNOREFESCAPE,
	              "(args!)\n"
	              "#t{:Interrupt}"
	              "#t{?AAddrInUse?GNetError}{The address specified for binding is already in use}"
	              "#t{?ANoSupport?GNetError}{The protocol of @this socket does not support binding}"
	              "#t{?AAddrNotAvail?GNetError}{The speficied address is not reachable from this machine}"
	              "#t{?AIsConnected?GNetError}{@this socket has already been bound is is already connected}"
	              "#tNetError{@this socket has already been bound and its address family does not allow rebinding}"
	              "#tNetError{Failed to bind @this socket for some unknown reason}"
	              "#tFileClosed{@this socket has already been closed}"
	              "Binds @this socket to a given address.\n"
	              "Accepted arguments are the same as ${sockaddr(this.sock_af, args...)} when creating ?Gsockaddr"),
	TYPE_METHOD_F("connect", &socket_connect, METHOD_FNOREFESCAPE,
	              "(args!)\n"
	              "#t{:Interrupt}"
	              "#t{?AAddrNotAvail?GNetError}{The speficied address is not reachable from this machine}"
	              "#t{?ANoSupport?GNetError}{@this socket is currently listening and cannot be connected}"
	              "#t{?ANoSupport?GNetError}{The specified address family is not supported}"
	              "#t{?ANoSupport?GNetError}{@this socket uses an incompatible prototype to the specified address}"
	              "#t{?AIsConnected?GNetError}{@this socket is already connected or has been bound}"
	              "#t{?AConnectReset?GNetError}{The target reset the connection before it could be completed}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{Timed out while attempting to establish a connection}"
	              "#t{?AConnectRefused?GNetError}{The target isn't listening to connections or refused to connect}"
	              "#t{?ANetUnreachable?GNetError}{No route to the network of the given address can be established}"
	              "#t{?AHostUnreachable?ANetUnreachable?GNetError}{The host of the target address cannot be reached}"
	              "#tNetError{Failed to connect @this socket for some unknown reason}"
	              "#tFileClosed{@this socket has already been closed}"
	              "Connect @this socket with a given address.\n"
	              "Accepted arguments are the same as ${sockaddr(this.sock_af, args...)} when creating ?Gsockaddr"),
	TYPE_METHOD_F("listen", &socket_listen, METHOD_FNOREFESCAPE,
	              "(" socket_listen_params ")\n"
	              "#t{:Interrupt}"
	              "#t{?ANotBound?GNetError}{@this socket has not been bound and the protocol does not allow listening on an unbound address}"
	              "#t{?ANoSupport?GNetError}{The protocol of @this socket does not allow listening}"
	              "#t{?AIsConnected?GNetError}{The socket is already connected}"
	              "#tNetError{Failed to start listening for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pmax_backlog{The max number of connections to queue before ?#accept must be called to acknowledge them. "
	              "When negative, use a default backlog that can be configured with the environment variable "
	              "${(int from deemon)((environ from fs)[\"DEEMON_MAXBACKLOG\"])}}"
	              "Start listening for incoming connections on @this socket, preferrable after it has been ?#bound\n"
	              "Note that calling this function may require the user to whitelist deemon in their firewall"),
	TYPE_METHOD_F("accept", &socket_accept, METHOD_FNOREFESCAPE,
	              "(timeout_nanoseconds=!-1)->?Gsocket\n"
	              "(timeout_nanoseconds=!-1)->?N\n"
	              "#t{:Interrupt}"
	              "#t{?ANotListening?ANotBound?GNetError}{@this socket is not listening for incoming connections}"
	              "#t{?ANoSupport?GNetError}{The type of @this socket does not allow accepting of incoming connections}"
	              "#tNetError{Failed to start accept a connection for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#ptimeout_nanoseconds{The timeout describing for how long ?#accept should wait before returning ?N. "
	              "You may pass ${-1} for an infinite timeout or $0 to fail immediately.}"
	              "#r{A new socket object describing the connection to the new client, or ?N when @timeout_nanoseconds has passed}"
	              "Accept new incoming connections on a listening socket"),
	TYPE_METHOD_F("tryaccept", &socket_tryaccept, METHOD_FNOREFESCAPE,
	              "->?Gsocket\n"
	              "->?N\n"
	              "#t{:Interrupt}"
	              "#t{?ANotListening?ANotBound?GNetError}{@this socket is not listening for incoming connections}"
	              "#t{?ANoSupport?GNetError}{The type of @this socket does not allow accepting of incoming connections}"
	              "#tNetError{Failed to start accept a connection for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "Same as calling ?#accept with a timeout_nanoseconds argument of ${0}"),
	TYPE_METHOD_F("recv", &socket_recv, METHOD_FNOREFESCAPE,
	              "(flags:?Dstring)->?DBytes\n"
	              "(max_size=!-1,flags=!P{})->?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1)->?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1,flags=!P{})->?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1,flags=!0)->?DBytes\n"
	              "#t{:Interrupt}"
	              "#t{?ANotConnected?GNetError}{@this socket is not connected}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out}"
	              "#tNetError{Failed to receive data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. The integer value expects the same "
	              "values as the host system library whereas the string version can be used "
	              "to independently encode flags as $\",\" or $\"|\" separated case-insensitive "
	              "names with an optional $\"MSG\" and/or $\"_\" prefix. (e.g. $\"OOB|PEEK\")}"
	              "Receive data from a connection-oriented socket that has been connected\n"
	              "Note that passing ${-1} for @max_size, will cause the function to try and receive "
	              "all incoming data, potentially invoking the recv system call multiple times. "
	              "In this situation, @timeout_nanoseconds is used as the initial timeout for the first "
	              "chunk, with all following then read with a timeout of $0 (aka. try-read)\n"
	              "When @timeout_nanoseconds expires before any data is received, an empty string is returned\n"
	              "Some protocols may also cause this function to return an empty string to indicate a graceful "
	              "termination of the connection"),
	TYPE_METHOD_F("recvinto", &socket_recvinto, METHOD_FNOREFESCAPE,
	              "(dst:?DBytes,flags=!P{})->?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1)->?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
	              "#t{:Interrupt}"
	              "#t{?ANotConnected?GNetError}{@this socket is not connected}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out}"
	              "#tNetError{Failed to receive data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. See ?#recv for information on the string-encoded version}"
	              "Same as #recv, but received data is written into the given buffer @dst"),
	TYPE_METHOD_F("recvfrom", &socket_recvfrom, METHOD_FNOREFESCAPE,
	              "(flags:?Dstring)->?T2?Gsockaddr?DBytes\n"
	              "(max_size=!-1,flags=!P{})->?T2?Gsockaddr?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1)->?T2?Gsockaddr?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1,flags=!P{})->?T2?Gsockaddr?DBytes\n"
	              "(max_size=!-1,timeout_nanoseconds=!-1,flags=!0)->?T2?Gsockaddr?DBytes\n"
	              "#t{:Interrupt}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out (Note to be confused with @timeout_nanoseconds expiring; see below)}"
	              "#tNetError{Failed to receive data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. See ?#recv for information on the string-encoded version}"
	              "Same as ?#recv, but uses the recvfrom system call to read data, also returning "
	              "the socket address from which the data originates as the first of 2 :Tuple "
	              "arguments, the second being the text regularly returned ?#recv\n"
	              "The given @timeout_nanoseconds can be passed as either $0 to try-receive pending packages, "
	              "as ${-1} (default) to wait for incoming data indefinitely or until the socket is ?#{close}ed, or "
	              "as any other integer value to specify how long to wait before returning ${(none, \"\")}"),
	TYPE_METHOD_F("recvfrominto", &socket_recvfrominto, METHOD_FNOREFESCAPE,
	              "(dst:?DBytes,flags=!P{})->?T2?Gsockaddr?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1)->?T2?Gsockaddr?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?T2?Gsockaddr?Dint\n"
	              "(dst:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?T2?Gsockaddr?Dint\n"
	              "#t{:Interrupt}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out (Note to be confused with @timeout_nanoseconds expiring; see below)}"
	              "#tNetError{Failed to receive data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. See ?#recv for information on the string-encoded version}"
	              "Same as ?#recvfrom, buf read received data into the given buffer @dst"),
	TYPE_METHOD_F("send", &socket_send, METHOD_FNOREFESCAPE,
	              "(data:?DBytes,flags=!P{})->?Dint\n"
	              "(data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
	              "(data:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
	              "#t{:Interrupt}"
	              "#t{?ANotConnected?GNetError}{@this socket is not connected}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out (Note to be confused with @timeout_nanoseconds expiring; see below)}"
	              "#t{?AMessageSize?GNetError}{The socket is not connection-mode and no peer address is set}"
	              "#t{?ANotBound?GNetError}{The socket is not connection-mode and no peer address is set}"
	              "#t{?ANetUnreachable?GNetError}{No route to the connected peer could be established}"
	              "#tNetError{Failed to send data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. See ?#recv for information on the string-encoded version}"
	              "#r{The total number of bytes that was sent}"
	              "Send @data over the network to the peer of a connected socket"),
	TYPE_METHOD_F("sendto", &socket_sendto, METHOD_FNOREFESCAPE,
	              "(target:?DBytes,data:?DBytes,flags=!P{})->?Dint\n"
	              "(target:?DBytes,data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
	              "(target:?DBytes,data:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
	              "(target:?O,data:?DBytes,flags=!P{})->?Dint\n"
	              "(target:?O,data:?DBytes,timeout_nanoseconds=!-1,flags=!0)->?Dint\n"
	              "(target:?O,data:?DBytes,timeout_nanoseconds=!-1,flags=!P{})->?Dint\n"
	              "#t{:Interrupt}"
	              "#t{?ANotConnected?GNetError}{@this socket is not connected}"
	              "#t{?ANoSupport?GNetError}{The specified @flags are not supported by @this socket}"
	              "#t{?AConnectReset?GNetError}{The peer has reset the connection}"
	              "#t{?ATimedOut?AConnectReset?GNetError}{The connection to the peer timed out (Note to be confused with @timeout_nanoseconds expiring; see below)}"
	              "#t{?ANotBound?GNetError}{The socket is not connection-mode and no peer address is set}"
	              "#t{?ANetUnreachable?GNetError}{No route to the connected peer could be established}"
	              "#tNetError{Failed to send data for some reason}"
	              "#tFileClosed{@this socket has already been closed or was shut down}"
	              "#pflags{A set of flags used during delivery. See ?#recv for information on the string-encoded version}"
	              "#ptarget{A tuple consisting of arguments which can be used to construct a ?Gsockaddr object, or a single argument used for "
	              "the same purpose in ${target = target is Tuple ? sockaddr(this.sock_af, target...) : sockaddr(this.sock_af, target)}.}"
	              "#r{The total number of bytes that was sent}"
	              "Same as ?#send, but used to transmit data to a specific network target, rather than one that is already connected."),
	TYPE_METHOD_F("wasshutdown", &socket_wasshutdown, METHOD_FNOREFESCAPE,
	              "(how:?Dint)->?Dbool\n"
	              "(how=!?rw)->?Dbool\n"
	              "Returns ?t if @this socket has been ?#shutdown according to @how (inclusive when multiple modes are specified)\n"
	              "See ?#shutdown for possible values that may be passed to @how"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst socket_getsets[] = {
	TYPE_GETTER_F("sockname", &socket_sockname_get, METHOD_FNOREFESCAPE,
	              "->?Gsockaddr\n"
	              "#tFileClosed{@this socket has been closed}"
	              "#t{?ANotConnected?GNetError}{@this socket is neither connected, nor bound}"
	              "Returns the socket name (local address) of @this socket"),
	TYPE_GETTER_F("peeraddr", &socket_peeraddr_get, METHOD_FNOREFESCAPE,
	              "->?Gsockaddr\n"
	              "#t{:Interrupt}"
	              "#tFileClosed{@this socket has been closed}"
	              "#t{?ANotConnected?GNetError}{@this socket is not connected}"
	              "#t{?ANoSupport?GNetError}{@this socket's protocol does not allow for peer addresses}"
	              "#tNetError{Failed to query the peer address for some unknown reason}"
	              "Returns the peer (remote) address of @this socket"),
	TYPE_GETTER_F("wasclosed", &socket_wasclosed, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this socket has been ?#{close}ed"),
	TYPE_GETTER_F(Dee_fd_osfhandle_GETSET, &socket_osfhandle, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns the underlying file descriptor/handle associated @this socket"),
	TYPE_GETSET_END
};


PRIVATE struct type_member tpconst socket_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("isbound", STRUCT_CONST, Socket, s_state, SOCKET_FBOUND,
	                         "Returns ?t if @this socket has been bound (s.a. ?#bind)"),
	TYPE_MEMBER_BITFIELD_DOC("isconnected", STRUCT_CONST, Socket, s_state, SOCKET_FCONNECTED,
	                         "Returns ?t if @this socket has been ?#{connect}ed"),
	TYPE_MEMBER_BITFIELD_DOC("islistening", STRUCT_CONST, Socket, s_state, SOCKET_FLISTENING,
	                         "Returns ?t if @this socket is ?#{listen}ing for incoming connections"),
	TYPE_MEMBER_FIELD_DOC("sock_af", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeSocketObject, s_sockaddr.sa.sa_family),
	                      "The socket's address family as a system-specific integer id\n"
	                      "Usually one of AF_*, the name of which can be determined using ?Ggetafname"),
	TYPE_MEMBER_FIELD_DOC("sock_type", STRUCT_CONST | STRUCT_INT, offsetof(DeeSocketObject, s_type),
	                      "The socket's type as a system-specific integer id\n"
	                      "Usually one of SOCK_*, the name of which can be determined using ?Ggettypename"),
	TYPE_MEMBER_FIELD_DOC("sock_proto", STRUCT_CONST | STRUCT_INT, offsetof(DeeSocketObject, s_type),
	                      "The socket's protocol as a system-specific integer id\n"
	                      "Usually one of *PROTO_*, the name of which can be determined using ?Ggetprotoname"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_str(DeeSocketObject *__restrict self) {
	bool has_sock, has_peer;
	uint16_t state = atomic_read(&self->s_state);
	SockAddr sock, peer;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (ascii_printer_printf(&printer, "<socket %K, %K, %K: ",
	                         sock_getafnameorid(self->s_sockaddr.sa.sa_family),
	                         sock_gettypenameorid(self->s_type),
	                         sock_getprotonameorid(self->s_proto)) < 0)
		goto err;
	if (!(state & SOCKET_FOPENED)) {
		if (ASCII_PRINTER_PRINT(&printer, " Closed") < 0)
			goto err;
	} else {
		has_sock = !DeeSocket_GetSockName(self, &sock, false);
		has_peer = !DeeSocket_GetPeerAddr(self, &peer, false);
		if (has_sock && has_peer) {
			if (ascii_printer_printf(&printer, "%K -> %K",
			                         SockAddr_ToString(&sock, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS),
			                         SockAddr_ToString(&peer, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS)) < 0)
				goto err;
		} else if (has_sock) {
			if (ascii_printer_printf(&printer, "%K",
			                         SockAddr_ToString(&sock, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS)) < 0)
				goto err;
		} else if (has_peer) {
			if (ascii_printer_printf(&printer, "local -> %K",
			                         SockAddr_ToString(&peer, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS)) < 0)
				goto err;
		}
		if (state & SOCKET_FBOUND && ASCII_PRINTER_PRINT(&printer, " Bound") < 0)
			goto err;
		if (state & SOCKET_FCONNECTED && ASCII_PRINTER_PRINT(&printer, " Connected") < 0)
			goto err;
		if (state & SOCKET_FLISTENING && ASCII_PRINTER_PRINT(&printer, " Listening") < 0)
			goto err;
		if (state & (SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W) &&
		    ascii_printer_printf(&printer, " Shutdown(%s%s)",
		                         state & SOCKET_FSHUTDOWN_R ? "r" : "",
		                         state & SOCKET_FSHUTDOWN_W ? "w" : "") < 0)
			goto err;
	}
	if (ASCII_PRINTER_PRINT(&printer, ">") < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}

#if 0
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_repr(DeeSocketObject *__restrict self) {
	bool has_sock, has_peer;
	uint16_t state = atomic_read(&self->s_state);
	SockAddr sock, peer;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (ascii_printer_printf(&printer, "<socket(%R, %R, %R): ",
	                         sock_getafnameorid(self->s_sockaddr.sa.sa_family),
	                         sock_gettypenameorid(self->s_type),
	                         sock_getprotonameorid(self->s_proto)) < 0)
		goto err;
	if (!(state & SOCKET_FOPENED)) {
		if (ASCII_PRINTER_PRINT(&printer, " Closed") < 0)
			goto err;
	} else {
		has_sock = !DeeSocket_GetSockName(self, &sock, false);
		has_peer = !DeeSocket_GetPeerAddr(self, &peer, false);
		if (has_sock && has_peer) {
			if (ascii_printer_printf(&printer, "%K -> %K",
			                         SockAddr_ToString(&sock, self->s_proto, SOCKADDR_STR_FNOFAIL),
			                         SockAddr_ToString(&peer, self->s_proto, SOCKADDR_STR_FNOFAIL)) < 0)
				goto err;
		} else if (has_sock) {
			if (ascii_printer_printf(&printer, "%K",
			                         SockAddr_ToString(&sock, self->s_proto, SOCKADDR_STR_FNOFAIL)) < 0)
				goto err;
		} else if (has_peer) {
			if (ascii_printer_printf(&printer, "local -> %K",
			                         SockAddr_ToString(&peer, self->s_proto, SOCKADDR_STR_FNOFAIL)) < 0)
				goto err;
		}
		if (state & SOCKET_FBOUND && ASCII_PRINTER_PRINT(&printer, " Bound") < 0)
			goto err;
		if (state & SOCKET_FCONNECTED && ASCII_PRINTER_PRINT(&printer, " Connected") < 0)
			goto err;
		if (state & SOCKET_FLISTENING && ASCII_PRINTER_PRINT(&printer, " Listening") < 0)
			goto err;
		if (state & (SOCKET_FSHUTDOWN_R | SOCKET_FSHUTDOWN_W) &&
		    ascii_printer_printf(&printer, " Shutdown(%s%s)",
		                         state & SOCKET_FSHUTDOWN_R ? "r" : "",
		                         state & SOCKET_FSHUTDOWN_W ? "w" : "") < 0)
			goto err;
	}
	if (ASCII_PRINTER_PRINT(&printer, ">") < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}
#endif

INTERN DeeTypeObject DeeSocket_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "socket",
	/* .tp_doc      = */ DOC("(" socket_socket_params ")\n"
	                         "#pproto{The protocol to use for the socket, or ?N or $0 to use the default}"
	                         "#ptype{The socket type, or none to use $\"SOCK_STREAM\"}"
	                         "#paf{The socket address family (e.g.: $\"AF_INET\" or $\"AF_INET6\").}"
	                         /*     */ "NOTE: When possible, deemon will automatically configure $\"AF_INET6\" sockets to be "
	                         /*           */ "able to accept clients in dualstack mode (that is: allowing connections made using both IPv4 and IPv6)."
	                         /*           */ "To simplify this further, you may use ?#tcp to easily create an IPv6-ready server or client\n"
	                         "#t{?ANoSupport?GNetError}{The given protocol @proto cannot be used with the given address family @af}"
	                         "#t{?ANoSupport?GNetError}{The given socket type @type cannot be used with protocol @proto}"
	                         "#tNetError{Failed to create a new socket descriptor}"
	                         "#tValueError{$\"AF_AUTO\" cannot be used as address family in the socket constructor}"
	                         "Constructs and allocates a new socket descriptor that has yet to be ?#bound or be ?#{connect}ed\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t indicative of the socket not having been closed (s.a. ?#wasclosed)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeSocketObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&socket_ctor,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&socket_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_str,
		/* .tp_repr = */ NULL, /* (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_repr, */
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&socket_bool
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ socket_methods,
	/* .tp_getsets       = */ socket_getsets,
	/* .tp_members       = */ socket_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_SOCKET_SOCKET_C */
