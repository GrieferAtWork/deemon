/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_SOCKET_C
#define GUARD_DEX_SOCKET_SOCKET_C 1
#define DEE_SOURCE 1

#include "libnet.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

DECL_BEGIN

typedef DeeSocketObject Socket;

PRIVATE ATTR_COLD int DCALL
err_no_af_support(neterrno_t error, sa_family_t af) {
	return DeeNet_ThrowErrorf(&DeeError_NoSupport, error,
	                          "Address family %R is not supported",
	                          sock_getafnameorid(af));
}

PRIVATE NONNULL((1)) int DCALL
socket_ctor(Socket *__restrict self,
            size_t argc, DeeObject *const *argv,
            DeeObject *kw) {
	int af, type, proto;
	DeeObject *arg_af, *arg_type = Dee_None, *arg_proto = Dee_None;
	PRIVATE struct keyword kwlist[] = { K(af), K(type), K(proto), KEND };
	/* Parse and translate arguments. */
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oo:socket", &arg_af, &arg_type, &arg_proto))
		goto err;
	if (sock_getafof(arg_af, &af))
		goto err;
	if (sock_gettypeof(arg_type, &type))
		goto err;
	if (sock_getprotoof(arg_proto, &proto))
		goto err;
	if (af == AF_AUTO) {
		DeeError_Throwf(&DeeError_ValueError,
		                "AF_AUTO cannot be used during socket construction");
		goto err;
	}
	atomic_rwlock_init(&self->s_lock);
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
		closesocket(self->s_socket);
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
			DeeThread_SleepNoInterrupt(1000);
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
		ATOMIC_OR(self->s_state, SOCKET_FHASSOCKADDR);
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
	if unlikely(ok && throw_error) {
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
	if (DeeArg_Unpack(argc, argv, "|o:close", &shutdown_mode))
		goto err;
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
		if (ATOMIC_FETCHOR(self->s_state, SOCKET_FSHUTTINGDOWN) &
		    SOCKET_FSHUTTINGDOWN) {
			/* Special case: Another shutdown operation is still in progress. */
			socket_endread(self);
			DeeThread_SleepNoInterrupt(1000);
			goto again_shutdown;
		}
		/* We'll have to do the shutdown. */
		error = socket_do_shutdown(self, mode);
		if unlikely(error < 0) {
			DBG_ALIGNMENT_DISABLE();
			error = GET_NET_ERROR();
			DBG_ALIGNMENT_ENABLE();
			ATOMIC_AND(self->s_state, ~SOCKET_FSHUTTINGDOWN);
			socket_endread(self);
			if (error == EINVAL)
				err_invalid_shutdown_how(error, mode);
			else {
				err_shutdown_failed(self, error);
			}
			goto err;
		}
		/* Indicate that shutdown has been completed. */
		ATOMIC_OR(self->s_state, new_state);
		ATOMIC_AND(self->s_state, ~SOCKET_FSHUTTINGDOWN);
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
	closesocket(socket_handle);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_shutdown(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
	if (DeeArg_Unpack(argc, argv, "|o:shutdown", &shutdown_mode))
		goto err;
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
		if (ATOMIC_FETCHOR(self->s_state, SOCKET_FSHUTTINGDOWN) &
		    SOCKET_FSHUTTINGDOWN) {
			/* Special case: The socket is already being shut down. */
			socket_endread(self);
			DeeThread_SleepNoInterrupt(1000);
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
				ATOMIC_AND(self->s_state, ~SOCKET_FSHUTTINGDOWN);
				socket_endread(self);
				if (error == EINVAL)
					err_invalid_shutdown_how(error, mode);
				else if (error == EBADF || error == ENOTSOCK)
					err_socket_closed(error, self);
				else {
					err_shutdown_failed(self, error);
				}
				goto err;
			}
		}
		/* Indicate that shutdown has been completed. */
		ATOMIC_OR(self->s_state, new_state);
		ATOMIC_AND(self->s_state, ~SOCKET_FSHUTTINGDOWN);
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
	if (ATOMIC_FETCHOR(self->s_state, SOCKET_FBINDING) & SOCKET_FBINDING) {

		socket_endread(self);
		/* The socket is already in the middle of a binding-call. */
		DeeThread_SleepNoInterrupt(1000);
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
		ATOMIC_OR(self->s_state, SOCKET_FBOUND | SOCKET_FHASSOCKADDR);
	}
	/* Unset the binding-flag. */
	state = ATOMIC_FETCHAND(self->s_state, ~SOCKET_FBINDING);
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
	WSAEventSelect(hSocket, hSockEvent, lNetworkEvents);
	dwResult = WSAWaitForMultipleEvents(1, &hSockEvent, FALSE, dwTimeout, TRUE);
	/* Apparently this is required to prevent the event also closing the socket??? */
	WSAEventSelect(hSocket, hSockEvent, 0);
	WSACloseEvent(hSockEvent);
	DBG_ALIGNMENT_ENABLE();
	return dwResult;
}
#endif /* CONFIG_HOST_WINDOWS */

#define SELECT_TIMEOUT 100000 /* In microseconds. */


PRIVATE ATTR_COLD NONNULL((2, 3)) int DCALL
err_network_down(neterrno_t error, Socket *__restrict socket,
                 SockAddr const *__restrict addr) {
	return DeeNet_ThrowErrorf(&DeeError_NetUnreachable, error,
	                          "No route to network of %K can be established",
	                          SockAddr_ToString(addr, socket->s_proto,
	                                            SOCKADDR_STR_FNOFAIL |
	                                            SOCKADDR_STR_FNODNS));
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4548)
#endif /* _MSC_VER */

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
	if (ATOMIC_FETCHOR(self->s_state, SOCKET_FCONNECTING) & SOCKET_FCONNECTING) {
		socket_endread(self);
		/* The socket is already in the middle of a connecting-call.
		 * Wait for that other connect() call to complete, then try again. */
		DeeThread_SleepNoInterrupt(1000);
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
			                             SELECT_TIMEOUT / 1000);
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
				timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
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
	ATOMIC_OR(self->s_state, SOCKET_FCONNECTED | SOCKET_FHASSOCKADDR);
	/* Unset the connecting-flag. */
	ATOMIC_AND(self->s_state, ~SOCKET_FCONNECTING);
	socket_endread(self);
	return 0;
err_connect_failed:
	ATOMIC_AND(self->s_state, ~SOCKET_FCONNECTING);
	goto err;
err_connect_failure:
	ATOMIC_AND(self->s_state, ~SOCKET_FCONNECTING);
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */


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
	if (ATOMIC_FETCHOR(self->s_state, SOCKET_FSTARTLISTENING) &
	    SOCKET_FSTARTLISTENING) {

		socket_endread(self);
		/* The socket is already in the middle of a listening-call. */
		DeeThread_SleepNoInterrupt(1000);
		goto again;
	}
	/* Do the listen system call. */
	DBG_ALIGNMENT_DISABLE();
	error = listen(self->s_socket, max_backlog);
	DBG_ALIGNMENT_ENABLE();
	if likely(error >= 0)
		ATOMIC_OR(self->s_state, SOCKET_FLISTENING);
	/* Unset the binding-flag. */
	state = ATOMIC_FETCHAND(self->s_state, ~SOCKET_FSTARTLISTENING);
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


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4548)
#endif /* _MSC_VER */

INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeSocket_Accept(DeeSocketObject *__restrict self,
                 uint64_t timeout_microseconds,
                 sock_t *__restrict sock_fd,
                 SockAddr *__restrict addr) {
	int error;
	socklen_t socklen;
	sock_t client_socket;
restart:
	socklen = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family, self->s_proto);
	if (timeout_microseconds == (uint64_t)-1) {
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
		                             SELECT_TIMEOUT / 1000);
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
			timeout.tv_sec = (long)(SELECT_TIMEOUT / 1000000);
			timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
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
			DeeThread_SleepNoInterrupt(1000);
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
			DWORD timeout = SELECT_TIMEOUT;
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
			if (timeout_microseconds < SELECT_TIMEOUT) {
				timeout.tv_sec = (long)(timeout_microseconds / 1000000);
				timeout.tv_usec = (long)(timeout_microseconds % 1000000);
			} else {
				timeout.tv_sec = (long)(SELECT_TIMEOUT / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
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
			DeeThread_SleepNoInterrupt(1000);
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
#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */

#ifdef CONFIG_HOST_WINDOWS
#define send(socket, buffer, length, flags) \
	((dssize_t)send(socket, (char const *)(buffer), (int)(length), flags))
#define sendto(socket, buffer, length, flags, target, targetlen) \
	((dssize_t)sendto(socket, (char const *)(buffer), (int)(length), flags, target, targetlen))
#define recv(socket, buffer, length, flags) \
	((dssize_t)recv(socket, (char *)(buffer), (int)(length), flags))
#define recvfrom(socket, buffer, length, flags, target, targetlen) \
	((dssize_t)recvfrom(socket, (char *)(buffer), (int)(length), flags, target, targetlen))
#endif /* CONFIG_HOST_WINDOWS */


PRIVATE ATTR_COLD NONNULL((2)) int DCALL
err_message_too_large(neterrno_t error, Socket *__restrict self, size_t bufsize) {
	return DeeNet_ThrowErrorf(&DeeError_MessageSize, error,
	                          "Message consisting of %Iu bytes is too large for socket %k",
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
			                             SELECT_TIMEOUT / 1000);
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
			timeout.tv_sec = (long)(SELECT_TIMEOUT / 1000000);
			timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
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
			if (timeout > SELECT_TIMEOUT / 1000)
				timeout = SELECT_TIMEOUT / 1000;
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
			if (remaining_time < SELECT_TIMEOUT) {
				timeout.tv_sec = (long)(remaining_time / 1000000);
				timeout.tv_usec = (long)(remaining_time % 1000000);
			} else {
				timeout.tv_sec = (long)(SELECT_TIMEOUT / 1000000);
				timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
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


INTERN WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeSocket_Send(DeeSocketObject *__restrict self,
               uint64_t timeout_microseconds,
               void const *__restrict buf, size_t bufsize,
               int flags) {
	dssize_t result;
	uint64_t end_time = timeout_microseconds;
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
			ATOMIC_OR(self->s_state, SOCKET_FSENDCONFOK);
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
			                   "Failed to send %Iu bytes of data through socket %k",
			                   bufsize, self);
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeSocket_Recv(DeeSocketObject *__restrict self,
               uint64_t timeout_microseconds,
               void *__restrict buf, size_t bufsize,
               int flags) {
	dssize_t result;
	uint64_t end_time = timeout_microseconds;
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
			ATOMIC_OR(self->s_state, SOCKET_FRECVCONFOK);
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

INTERN WUNUSED NONNULL((1, 3, 6)) dssize_t DCALL
DeeSocket_SendTo(DeeSocketObject *__restrict self,
                 uint64_t timeout_microseconds,
                 void const *__restrict buf, size_t bufsize,
                 int flags, SockAddr const *__restrict target) {
	dssize_t result;
	uint64_t end_time = timeout_microseconds;
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
			ATOMIC_OR(self->s_state, SOCKET_FSENDCONFOK);
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
			                   "Failed to send %Iu bytes of data through socket %k to address %K",
			                   bufsize, self, SockAddr_ToString(target, self->s_proto, SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
		}
		goto err;
	}
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 6)) dssize_t DCALL
DeeSocket_RecvFrom(DeeSocketObject *__restrict self,
                   uint64_t timeout_microseconds,
                   void *__restrict buf, size_t bufsize,
                   int flags, SockAddr *__restrict source) {
	dssize_t result;
	socklen_t length;
	uint64_t end_time = timeout_microseconds;
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
			ATOMIC_OR(self->s_state, SOCKET_FRECVCONFOK);
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
                   uint64_t timeout_microseconds,
                   size_t max_bufsize, int flags,
                   SockAddr *source) {
	DREF DeeObject *result;
	dssize_t recv_length;
	if (max_bufsize == (size_t)-1) {
		if (!source) {
			struct bytes_printer printer = BYTES_PRINTER_INIT;
			size_t chunksize             = get_recv_chunksize();
			/* Variable-length buffer. */
			for (;;) {
				uint8_t *part = bytes_printer_alloc(&printer, chunksize);
				if unlikely(!part)
					goto err_printer;
				recv_length = DeeSocket_Recv(self, timeout_microseconds, part, chunksize, flags);
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
				timeout_microseconds = 0;
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
	recv_length = source ? DeeSocket_RecvFrom(self, timeout_microseconds,
	                                          DeeBytes_DATA(result),
	                                          max_bufsize, flags, source)
	                     : DeeSocket_Recv(self, timeout_microseconds,
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
	int max_backlog = -1;
	if (DeeArg_Unpack(argc, argv, "|d:listen", &max_backlog))
		goto err;
	if unlikely(DeeSocket_Listen(self, max_backlog))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_doaccept(Socket *__restrict self, uint64_t timeout) {
	DREF Socket *result;
	int error;
	result = DeeObject_MALLOC(Socket);
	if unlikely(!result)
		goto err;
	error = DeeSocket_Accept(self, timeout,
	                         &result->s_socket,
	                         &result->s_peeraddr);
	if unlikely(error < 0)
		goto err_r;
	if (error > 0) {
		DeeObject_FREE(result);
		return_none; /* Timeout */
	}
	/* Fill in the remaining members of the new socket. */
	atomic_rwlock_init(&result->s_lock);
	result->s_sockaddr.sa.sa_family = result->s_peeraddr.sa.sa_family;
	result->s_state                 = (SOCKET_FOPENED | SOCKET_FHASPEERADDR | SOCKET_FCONNECTED);
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
	uint64_t timeout = (uint64_t)-1;
	if (DeeArg_Unpack(argc, argv, "|" UNPd64 ":accept", &timeout))
		goto err;
	return socket_doaccept(self, timeout);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_tryaccept(Socket *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":tryaccept"))
		goto err;
	return socket_doaccept(self, 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_recv(Socket *self, size_t argc, DeeObject *const *argv) {
	size_t max_size;
	uint64_t timeout;
	int flags;
	DeeObject *arg_0 = NULL, *arg_1 = NULL, *arg_2 = NULL;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "|ooo:recv", &arg_0, &arg_1, &arg_2))
		goto err;
	if (!arg_1) {
		max_size = (size_t)-1;
		timeout  = (uint64_t)-1;
		flags    = 0;
		if (arg_0) {
			if (DeeString_Check(arg_0)) {
				/* "(flags:?Dstring)->?Dstring\n" */
				if (sock_getmsgflagsof(arg_0, &flags))
					goto err;
			} else {
				/* "(max_size=!-1,int flags=!P{})->?Dstring\n" */
				/* "(max_size=!-1,timeout_microseconds=!-1)->?Dstring\n" */
				if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
					goto err;
			}
		}
	} else if (!arg_2) {
		/* "(max_size=!-1,int flags=!P{})->?Dstring\n" */
		/* "(max_size=!-1,timeout_microseconds=!-1)->?Dstring\n" */
		if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
			goto err;
		if (DeeString_Check(arg_1)) {
			if (sock_getmsgflagsof(arg_1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			flags = 0;
			if (DeeObject_AsInt64(arg_1, (int64_t *)&timeout))
				goto err;
		}
	} else {
		/* "(max_size=!-1,timeout_microseconds=!-1,flags=!P{})->?Dstring\n" */
		/* "(max_size=!-1,timeout_microseconds=!-1,flags=!0)->?Dstring\n" */
		if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
			goto err;
		if (DeeObject_AsInt64(arg_1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg_2, &flags))
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
	DeeObject *data;
	DeeObject *arg1 = NULL, *arg2 = NULL;
	uint64_t timeout;
	int flags;
	dssize_t result;
	if (DeeArg_Unpack(argc, argv, "o|oo:recvinto", &data, &arg1, &arg2))
		goto err;
	if (!arg1) {
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!arg2) {
		//"(dst:?DBytes,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		if (DeeString_Check(arg1)) {
			if (sock_getmsgflagsof(arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		if (DeeObject_AsInt64(arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg2, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = DeeSocket_Recv(self,
	                        timeout,
	                        buffer.bb_base,
	                        buffer.bb_size,
	                        flags);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result < 0)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_recvfrom(Socket *self, size_t argc, DeeObject *const *argv) {
	DREF DeeSockAddrObject *result_addr;
	DREF DeeObject *result_text, *result;
	size_t max_size;
	uint64_t timeout;
	int flags;
	DeeObject *arg_0 = NULL, *arg_1 = NULL, *arg_2 = NULL;
	if (DeeArg_Unpack(argc, argv, "|ooo:recvfrom", &arg_0, &arg_1, &arg_2))
		goto err;
	if (!arg_1) {
		max_size = (size_t)-1;
		timeout  = (uint64_t)-1;
		flags    = 0;
		if (arg_0) {
			if (DeeString_Check(arg_0)) {
				/* "(flags:?Dstring)->(sockaddr,string)\n" */
				if (sock_getmsgflagsof(arg_0, &flags))
					goto err;
			} else {
				/* "(max_size=!-1,int flags=!P{})->(sockaddr,string)\n" */
				/* "(max_size=!-1,timeout_microseconds=!-1)->(sockaddr,string)\n" */
				if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
					goto err;
			}
		}
	} else if (!arg_2) {
		/* "(max_size=!-1,int flags=!P{})->(sockaddr,string)\n" */
		/* "(max_size=!-1,timeout_microseconds=!-1)->(sockaddr,string)\n" */
		if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
			goto err;
		if (DeeString_Check(arg_1)) {
			if (sock_getmsgflagsof(arg_1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			flags = 0;
			if (DeeObject_AsInt64(arg_1, (int64_t *)&timeout))
				goto err;
		}
	} else {
		/* "(max_size=!-1,timeout_microseconds=!-1,flags=!P{})->(sockaddr,string)\n" */
		/* "(max_size=!-1,timeout_microseconds=!-1,flags=!0)->(sockaddr,string)\n" */
		if (DeeObject_AsSSize(arg_0, (dssize_t *)&max_size))
			goto err;
		if (DeeObject_AsInt64(arg_1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg_2, &flags))
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
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_text;
	if (result_text == ITER_DONE) {
		/* A somewhat different story: must return (none, "") */
		DeeObject_FREE(result_addr);
		DeeTuple_SET(result, 0, Dee_None);
		DeeTuple_SET(result, 1, Dee_EmptyString);
		Dee_Incref(Dee_None);
		Dee_Incref(Dee_EmptyString);
	} else {
		DeeObject_Init(result_addr, &DeeSockAddr_Type);
		/* Fill the result tuple with the socket address and text. */
		DeeTuple_SET(result, 0, (DeeObject *)result_addr); /* Inherit */
		DeeTuple_SET(result, 1, (DeeObject *)result_text); /* Inherit */
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_recvfrominto(Socket *self, size_t argc, DeeObject *const *argv) {
	DeeBuffer buffer;
	DeeObject *data;
	DeeObject *arg1 = NULL, *arg2 = NULL;
	uint64_t timeout;
	int flags;
	dssize_t result_size;
	DREF DeeSockAddrObject *result_addr;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o|oo:recvfrominto", &data, &arg1, &arg2))
		goto err;
	if (!arg1) {
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!arg2) {
		//"(dst:?DBytes,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		if (DeeString_Check(arg1)) {
			if (sock_getmsgflagsof(arg1, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(arg1, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
		//"(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
		if (DeeObject_AsInt64(arg1, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg2, &flags))
			goto err;
	}
	/* Create the socket address object that's going to be returned. */
	result_addr = DeeObject_MALLOC(DeeSockAddrObject);
	if unlikely(!result_addr)
		goto err;
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FWRITABLE))
		goto err_addr;
	result_size = DeeSocket_RecvFrom(self,
	                                 timeout,
	                                 buffer.bb_base,
	                                 buffer.bb_size,
	                                 flags,
	                                 &result_addr->sa_addr);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result_size < 0)
		goto err_addr;
	/* Create a new tuple to package the 2 objects. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_addr;
	if (result_size == 0) {
		/* A somewhat different story: must return (none, "") */
		DeeObject_FREE(result_addr);
		DeeTuple_SET(result, 0, Dee_None);
		DeeTuple_SET(result, 1, (DeeObject *)&DeeInt_Zero);
		Dee_Incref(Dee_None);
		Dee_Incref(&DeeInt_Zero);
	} else {
		DREF DeeObject *result_size_ob;
		result_size_ob = DeeInt_NewSize((size_t)result_size);
		if unlikely(!result_size_ob) {
			DeeTuple_FreeUninitialized(result);
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
	DeeObject *data, *arg_0 = NULL, *arg_1 = NULL;
	uint64_t timeout;
	int flags;
	dssize_t result;
	if (DeeArg_Unpack(argc, argv, "o|oo:send", &data, &arg_0, &arg_1))
		goto err;
	if (!arg_0) {
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!arg_1) {
		/* "(data:?DBytes,flags=!P{})->?Dint\n" */
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		if (DeeString_Check(arg_0)) {
			if (sock_getmsgflagsof(arg_0, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(arg_0, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n" */
		if (DeeObject_AsInt64(arg_0, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg_1, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeSocket_Send(self,
	                        timeout,
	                        buffer.bb_base,
	                        buffer.bb_size,
	                        flags);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FREADONLY);
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
	DeeObject *target, *data, *arg_0 = NULL, *arg_1 = NULL;
	uint64_t timeout;
	int flags;
	dssize_t result;
	if (DeeArg_Unpack(argc, argv, "oo|oo:sendto", &target, &data, &arg_0, &arg_1))
		goto err;
	/* Construct the target address. */
	if (DeeTuple_Check(target)) {
		if (SockAddr_FromArgv(&target_addr,
		                      self->s_sockaddr.sa.sa_family,
		                      self->s_proto,
		                      self->s_type,
		                      DeeTuple_SIZE(target),
		                      DeeTuple_ELEM(target)))
			goto err;
	} else {
		if (SockAddr_FromArgv(&target_addr,
		                      self->s_sockaddr.sa.sa_family,
		                      self->s_proto,
		                      self->s_type,
		                      1,
		                      &target))
			goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	if (!arg_0) {
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		timeout = (uint64_t)-1;
		flags   = 0;
	} else if (!arg_1) {
		/* "(data:?DBytes,flags=!P{})->?Dint\n" */
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		if (DeeString_Check(arg_0)) {
			if (sock_getmsgflagsof(arg_0, &flags))
				goto err;
			timeout = (uint64_t)-1;
		} else {
			if (DeeObject_AsInt64(arg_0, (int64_t *)&timeout))
				goto err;
			flags = 0;
		}
	} else {
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n" */
		/* "(data:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n" */
		if (DeeObject_AsInt64(arg_0, (int64_t *)&timeout))
			goto err;
		if (sock_getmsgflagsof(arg_1, &flags))
			goto err;
	}
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeSocket_SendTo(self,
	                          timeout,
	                          buffer.bb_base,
	                          buffer.bb_size,
	                          flags,
	                          &target_addr);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FREADONLY);
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
	DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
	int mode;
	uint16_t state = self->s_state;
	if (DeeArg_Unpack(argc, argv, "|o:wasshutdown", &shutdown_mode))
		goto err;
	if (DeeString_Check(shutdown_mode) &&
	    DeeString_IsEmpty(shutdown_mode))
		return_bool(!(state & SOCKET_FOPENED));
	if unlikely(get_shutdown_modeof(shutdown_mode, &mode))
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
	mode |= !(state & SOCKET_FOPENED);
	return_bool_(mode);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_fileno(Socket *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":fileno"))
		goto err;
#ifdef CONFIG_HOST_WINDOWS
	return DeeInt_NewUIntptr((uintptr_t)self->s_socket);
#else /* CONFIG_HOST_WINDOWS */
	return DeeInt_NewInt((int)self->s_socket);
#endif /* !CONFIG_HOST_WINDOWS */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
socket_wasclosed(Socket *__restrict self) {
	return_bool(!(self->s_state & SOCKET_FOPENED));
}


PRIVATE struct type_method tpconst socket_methods[] = {
	{ "close",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_close,
	  DOC("(shutdown_mode:?Dint)\n"
	      "(shutdown_mode=!Prw)\n"
	      "@interrupt\n"
	      "@throw ValueError Invalid shutdown mode\n"
	      "@throw NetError Failed to shutdown @this socket\n"
	      "@throw FileClosed @this socket has already been closed\n"
	      "Closes the socket's file descriptor. When @shutdown_socket is a non-empty :string, "
	      "?#shutdown will automatically be invoked on @this socket if it hasn't before\n"
	      "Note that in the event that ?#shutdown has already been called, ") },
	{ "shutdown",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_shutdown,
	  DOC("(how:?Dint)\n"
	      "(how=!Prw)\n"
	      "@interrupt\n"
	      "@throw ValueError Invalid shutdown mode\n"
	      "@throw NetError Failed to shutdown @this socket\n"
	      "@throw FileClosed @this socket has already been closed\n"
	      "Shuts down @this socket either for reading, for writing or for both") },
	{ "bind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_bind,
	  DOC("(args!)\n"
	      "@interrupt\n"
	      "@throw NetError.AddrInUse The address specified for binding is already in use\n"
	      "@throw NetError.NoSupport The protocol of @this socket does not support binding\n"
	      "@throw NetError.AddrNotAvail The speficied address is not reachable from this machine\n"
	      "@throw NetError.IsConnected @this socket has already been bound is is already connected\n"
	      "@throw NetError @this socket has already been bound and its address family does not allow rebinding\n"
	      "@throw NetError Failed to bind @this socket for some unknown reason\n"
	      "@throw FileClosed @this socket has already been closed\n"
	      "Binds @this socket to a given address.\n"
	      "Accepted arguments are the same as ${sockaddr(this.sock_af, args...)} when creating ?Gsockaddr") },
	{ "connect",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_connect,
	  DOC("(args!)\n"
	      "@interrupt\n"
	      "@throw NetError.AddrNotAvail The speficied address is not reachable from this machine\n"
	      "@throw NetError.NoSupport @this socket is currently listening and cannot be connected\n"
	      "@throw NetError.NoSupport The specified address family is not supported\n"
	      "@throw NetError.NoSupport @this socket uses an incompatible prototype to the specified address\n"
	      "@throw NetError.IsConnected @this socket is already connected or has been bound\n"
	      "@throw NetError.ConnectReset The target reset the connection before it could be completed\n"
	      "@throw NetError.ConnectReset.TimedOut Timed out while attempting to establish a connection\n"
	      "@throw NetError.ConnectRefused The target isn't listening to connections or refused to connect\n"
	      "@throw NetError.NetUnreachable No route to the network of the given address can be established\n"
	      "@throw NetError.NetUnreachable.HostUnreachable The host of the target address cannot be reached\n"
	      "@throw NetError Failed to connect @this socket for some unknown reason\n"
	      "@throw FileClosed @this socket has already been closed\n"
	      "Connect @this socket with a given address.\n"
	      "Accepted arguments are the same as ${sockaddr(this.sock_af, args...)} when creating ?Gsockaddr") },
	{ "listen",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_listen,
	  DOC("(max_backlog=!-1)\n"
	      "@interrupt\n"
	      "@throw NetError.NotBound @this socket has not been bound and the protocol does not allow listening on an unbound address\n"
	      "@throw NetError.NoSupport The protocol of @this socket does not allow listening\n"
	      "@throw NetError.IsConnected The socket is already connected\n"
	      "@throw NetError Failed to start listening for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param max_backlog The max number of connections to queue before ?#accept must be called to acknowledge them. "
	      "When negative, use a default backlog that can be configured with the environment variable "
	      "${(int from deemon)((environ from fs)[\"DEEMON_MAXBACKLOG\"])}\n"
	      "Start listening for incoming connections on @this socket, preferrable after it has been ?#bound\n"
	      "Note that calling this function may require the user to whitelist deemon in their firewall") },
	{ "accept",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_accept,
	  DOC("(timeout_microseconds=!-1)->?Gsocket\n"
	      "(timeout_microseconds=!-1)->?N\n"
	      "@interrupt\n"
	      "@throw NetError.NotBound.NotListening @this socket is not listening for incoming connections\n"
	      "@throw NetError.NoSupport The type of @this socket does not allow accepting of incoming connections\n"
	      "@throw NetError Failed to start accept a connection for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param timeout_microseconds The timeout describing for how long ?#accept should wait before returning ?N. "
	      "You may pass ${-1} for an infinite timeout or $0 to fail immediately.\n"
	      "@return A new socket object describing the connection to the new client, or ?N when @timeout_microseconds has passed\n"
	      "Accept new incoming connections on a listening socket") },
	{ "tryaccept",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_tryaccept,
	  DOC("->?Gsocket\n"
	      "->?N\n"
	      "@interrupt\n"
	      "@throw NetError.NotBound.NotListening @this socket is not listening for incoming connections\n"
	      "@throw NetError.NoSupport The type of @this socket does not allow accepting of incoming connections\n"
	      "@throw NetError Failed to start accept a connection for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "Same as calling ?#accept with a timeout_microseconds argument of ${0}") },
	{ "recv",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_recv,
	  DOC("(flags:?Dstring)->?DBytes\n"
	      "(max_size=!-1,flags=!P{})->?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1)->?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1,flags=!P{})->?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1,flags=!0)->?DBytes\n"
	      "@interrupt\n"
	      "@throw NetError.NotConnected @this socket is not connected\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out\n"
	      "@throw NetError Failed to receive data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. The integer value expects the same "
	      "values as the host system library whereas the string version can be used "
	      "to independently encode flags as $\",\" or $\"|\" separated case-insensitive "
	      "names with an optional $\"MSG\" and/or $\"_\" prefix. (e.g. $\"OOB|PEEK\")\n"
	      "Receive data from a connection-oriented socket that has been connected\n"
	      "Note that passing ${-1} for @max_size, will cause the function to try and receive "
	      "all incoming data, potentially invoking the recv system call multiple times. "
	      "In this situation, @timeout_microseconds is used as the initial timeout for the first "
	      "chunk, with all following then read with a timeout of $0 (aka. try-read)\n"
	      "When @timeout_microseconds expires before any data is received, an empty string is returned\n"
	      "Some protocols may also cause this function to return an empty string to indicate a graceful "
	      "termination of the connection") },
	{ "recvinto",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_recvinto,
	  DOC("(dst:?DBytes,flags=!P{})->?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1)->?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
	      "@interrupt\n"
	      "@throw NetError.NotConnected @this socket is not connected\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out\n"
	      "@throw NetError Failed to receive data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. See ?#recv for information on the string-encoded version\n"
	      "Same as #recv, but received data is written into the given buffer @dst") },
	{ "recvfrom",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_recvfrom,
	  DOC("(flags:?Dstring)->?T2?Gsockaddr?DBytes\n"
	      "(max_size=!-1,flags=!P{})->?T2?Gsockaddr?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1)->?T2?Gsockaddr?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1,flags=!P{})->?T2?Gsockaddr?DBytes\n"
	      "(max_size=!-1,timeout_microseconds=!-1,flags=!0)->?T2?Gsockaddr?DBytes\n"
	      "@interrupt\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
	      "@throw NetError Failed to receive data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. See ?#recv for information on the string-encoded version\n"
	      "Same as ?#recv, but uses the recvfrom system call to read data, also returning "
	      "the socket address from which the data originates as the first of 2 :Tuple "
	      "arguments, the second being the text regularly returned ?#recv\n"
	      "The given @timeout_microseconds can be passed as either $0 to try-receive pending packages, "
	      "as ${-1} (default) to wait for incoming data indefinitely or until the socket is ?#{close}ed, or "
	      "as any other integer value to specify how long to wait before returning ${(none, \"\")}") },
	{ "recvfrominto",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_recvfrominto,
	  DOC("(dst:?DBytes,flags=!P{})->?T2?Gsockaddr?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1)->?T2?Gsockaddr?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1,flags=!P{})->?T2?Gsockaddr?Dint\n"
	      "(dst:?DBytes,timeout_microseconds=!-1,flags=!0)->?T2?Gsockaddr?Dint\n"
	      "@interrupt\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
	      "@throw NetError Failed to receive data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. See ?#recv for information on the string-encoded version\n"
	      "Same as ?#recvfrom, buf read received data into the given buffer @dst") },
	{ "send",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_send,
	  DOC("(data:?DBytes,flags=!P{})->?Dint\n"
	      "(data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
	      "(data:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
	      "@interrupt\n"
	      "@throw NetError.NotConnected @this socket is not connected\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
	      "@throw NetError.MessageSize The socket is not connection-mode and no peer address is set\n"
	      "@throw NetError.NotBound The socket is not connection-mode and no peer address is set\n"
	      "@throw NetError.NetUnreachable No route to the connected peer could be established\n"
	      "@throw NetError Failed to send data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. See ?#recv for information on the string-encoded version\n"
	      "@return The total number of bytes that was sent\n"
	      "Send @data over the network to the peer of a connected socket") },
	{ "sendto",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_sendto,
	  DOC("(target:?DBytes,data:?DBytes,flags=!P{})->?Dint\n"
	      "(target:?DBytes,data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
	      "(target:?DBytes,data:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
	      "(target:?O,data:?DBytes,flags=!P{})->?Dint\n"
	      "(target:?O,data:?DBytes,timeout_microseconds=!-1,flags=!0)->?Dint\n"
	      "(target:?O,data:?DBytes,timeout_microseconds=!-1,flags=!P{})->?Dint\n"
	      "@interrupt\n"
	      "@throw NetError.NotConnected @this socket is not connected\n"
	      "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
	      "@throw NetError.ConnectReset The peer has reset the connection\n"
	      "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
	      "@throw NetError.NotBound The socket is not connection-mode and no peer address is set\n"
	      "@throw NetError.NetUnreachable No route to the connected peer could be established\n"
	      "@throw NetError Failed to send data for some reason\n"
	      "@throw FileClosed @this socket has already been closed or was shut down\n"
	      "@param flags A set of flags used during delivery. See ?#recv for information on the string-encoded version\n"
	      "@param target A tuple consisting of arguments which can be used to construct a ?Gsockaddr object, or a single argument used for "
	      "the same purpose in ${target = target is Tuple ? sockaddr(this.sock_af, target...) : sockaddr(this.sock_af, target)}.\n"
	      "@return The total number of bytes that was sent\n"
	      "Same as ?#send, but used to transmit data to a specific network target, rather than one that is already connected.") },
	{ "wasshutdown",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_wasshutdown,
	  DOC("(how:?Dint)->?Dbool\n"
	      "(how=!?rw)->?Dbool\n"
	      "Returns ?t if @this socket has been ?#shutdown according to @how (inclusive when multiple modes are specified)\n"
	      "See ?#shutdown for possible values that may be passed to @how") },
	{ "fileno", /* TODO: Use DeeSysFD_INT_GETSET / DeeSysFD_HANDLE_GETSET for this! */
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&socket_fileno,
	  DOC("->?Dint\n"
	      "Returns the underlying file descriptor/handle associated @this socket") },
	{ NULL }
};

PRIVATE struct type_getset tpconst socket_getsets[] = {
	{ "sockname",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_sockname_get, NULL, NULL,
	  DOC("->?Gsockaddr\n"
	      "@throw FileClosed @this socket has been closed\n"
	      "@throw NetError.NotConnected @this socket is neither connected, nor bound\n"
	      "Returns the socket name (local address) of @this socket") },
	{ "peeraddr",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_peeraddr_get, NULL, NULL,
	  DOC("->?Gsockaddr\n"
	      "@interrupt\n"
	      "@throw FileClosed @this socket has been closed\n"
	      "@throw NetError.NotConnected @this socket is not connected\n"
	      "@throw NetError.NoSupport @this socket's protocol does not allow for peer addresses\n"
	      "@throw NetError Failed to query the peer address for some unknown reason\n"
	      "Returns the peer (remote) address of @this socket") },
	{ "wasclosed",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_wasclosed, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this socket has been ?#{close}ed") },
	{ NULL }
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
	uint16_t state;
	SockAddr sock, peer;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	state                        = ATOMIC_READ(self->s_state);
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
	uint16_t state;
	SockAddr sock, peer;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	state                        = ATOMIC_READ(self->s_state);
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
	/* .tp_doc      = */ DOC("(af:?X2?Dint?Dstring,type:?X2?Dint?Dstring=!N,proto:?X2?Dint?Dstring=!N)\n"
	                         "@param proto The protocol to use for the socket, or ?N or $0 to use the default\n"
	                         "@param type The socket type, or none to use $\"SOCK_STREAM\"\n"
	                         "@param af The socket address family (e.g.: $\"AF_INET\" or $\"AF_INET6\").\n"
	                         "          NOTE: When possible, deemon will automatically configure $\"AF_INET6\" sockets to be "
	                                         "able to accept clients in dualstack mode (that is: allowing connections made using both IPv4 and IPv6)\n"
	                         "                To simplify this further, you may use ?#tcp to easily create an IPv6-ready server or client\n"
	                         "@throw NetError.NoSupport The given protocol @proto cannot be used with the given address family @af\n"
	                         "@throw NetError.NoSupport The given socket type @type cannot be used with protocol @proto\n"
	                         "@throw NetError Failed to create a new socket descriptor\n"
	                         "@throw ValueError $\"AF_AUTO\" cannot be used as address family in the socket constructor\n"
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
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeSocketObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&socket_ctor,
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
