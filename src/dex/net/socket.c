/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_SOCKET_C
#define GUARD_DEX_SOCKET_SOCKET_C 1

#include "libnet.h"

#include <deemon/api.h>
#include <deemon/file.h>
#include <deemon/dex.h>
#include <deemon/tuple.h>
#include <deemon/arg.h>
#include <deemon/none.h>
#include <deemon/format.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/bytes.h>
#include <deemon/string.h>

DECL_BEGIN

typedef DeeSocketObject Socket;

PRIVATE ATTR_COLD int DCALL
err_no_af_support(neterrno_t error, sa_family_t af) {
 return DeeError_SysThrowf(&DeeError_NoSupport,error,
                           "Address family %R is not supported",
                           sock_getafnameorid(af));
}

PRIVATE int DCALL
socket_ctor(Socket *__restrict self,
            size_t argc, DeeObject **__restrict argv,
            DeeObject *kw) {
 int af,type,proto;
 DeeObject *arg_af,*arg_type = Dee_None,*arg_proto = Dee_None;
 struct keyword kwlist[] = { K(af), K(type), K(proto), KEND };
 /* Parse and translate arguments. */
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"o|oo:socket",&arg_af,&arg_type,&arg_proto) ||
     sock_getafof(arg_af,&af) ||
     sock_gettypeof(arg_type,&type) ||
     sock_getprotoof(arg_proto,&proto))
     goto err;
 if (af == AF_AUTO) {
  DeeError_Throwf(&DeeError_ValueError,
                  "AF_AUTO cannot be used during socket construction");
  goto err;
 }
 rwlock_init(&self->s_lock);
 self->s_state                 = SOCKET_FOPENED;
 self->s_sockaddr.sa.sa_family = (sa_family_t)af;
 self->s_type                  = type;
 self->s_proto                 = proto;
 /* Create the socket descriptor. */
 DBG_ALIGNMENT_DISABLE();
 self->s_socket = socket(af,type,proto);
 DBG_ALIGNMENT_ENABLE();
 if (self->s_socket == INVALID_SOCKET) {
  /* Check for errors. */
  neterrno_t err;
  DBG_ALIGNMENT_DISABLE();
  err = GET_NET_ERROR();
  DBG_ALIGNMENT_ENABLE();
  if (err == EAFNOSUPPORT) {
   err_no_af_support(err,(sa_family_t)af);
  } else if (err == EPROTONOSUPPORT) {
   DeeError_SysThrowf(&DeeError_NoSupport,err,
                      "Protocol %R is not supported or cannot be used with address family %R",
                      sock_getprotonameorid(proto),
                      sock_getafnameorid((sa_family_t)af));
  } else if (err == EPROTOTYPE) {
   DeeError_SysThrowf(&DeeError_NoSupport,err,
                      "Socket type %R cannot be used with protocol %R",
                      sock_gettypenameorid(type),
                      sock_getprotonameorid(proto));
  } else {
   DeeError_SysThrowf(&DeeError_NetError,err,
                      "Failed to create `socket(%R,%R,%R)'",
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
#endif
 return 0;
err:
 return -1;
}

PRIVATE void DCALL
socket_fini(Socket *__restrict self) {
 if (self->s_state & SOCKET_FOPENED) {
  DBG_ALIGNMENT_DISABLE();
  closesocket(self->s_socket);
  DBG_ALIGNMENT_ENABLE();
 }
}

PRIVATE int DCALL
socket_bool(Socket *__restrict self) {
 return !!(self->s_state & SOCKET_FOPENED);
}


INTERN int DCALL
DeeSocket_GetSockName(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result,
                      bool throw_error) {
 uint16_t state;
again:
 socket_read(self);
 state = self->s_state;
 if unlikely(!(state&(SOCKET_FBOUND|SOCKET_FCONNECTED|SOCKET_FOPENED))) {
  socket_endread(self);
  if (state&(SOCKET_FBINDING|SOCKET_FCONNECTING)) {
   /* Socket is currently binding/connecting (wait a bit more) */
   DeeThread_SleepNoInterrupt(1000);
   goto again;
  }
  if (throw_error) {
   if (!(state&SOCKET_FOPENED)) {
    err_socket_closed(EINVAL,self);
   } else {
    DeeError_SysThrowf(&DeeError_NotConnected,ENOTCONN,
                       "Socket %k is neither bound nor connected",
                       self);
   }
  }
  return -1;
 }
 if (!(state&SOCKET_FHASSOCKADDR)) {
  /* Load the socket's name on first access. */
  socklen_t addrlen; int error;
  addrlen = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,self->s_proto);
  DBG_ALIGNMENT_DISABLE();
  error = getsockname(self->s_socket,&result->sa,&addrlen);
  DBG_ALIGNMENT_ENABLE();
  if unlikely(error < 0) {
   socket_endread(self);
   if (throw_error) {
    DBG_ALIGNMENT_DISABLE();
    error = GET_NET_ERROR();
    DBG_ALIGNMENT_ENABLE();
    if (error == EBADF || error == ENOTSOCK || error == EINVAL) {
     err_socket_closed(error,self);
    } else if (error == EOPNOTSUPP) {
     DeeError_SysThrowf(&DeeError_NoSupport,error,
                        "The socket's protocol %K does not support socket names",
                        sock_getprotonameorid(self->s_proto));
    } else {
     DeeError_SysThrowf(&DeeError_NetError,error,
                        "Failed to get name of socket %k",
                        self);
    }
   }
   return -1;
  }
  memcpy(&self->s_sockaddr,result,sizeof(SockAddr));
  ATOMIC_FETCHOR(self->s_state,SOCKET_FHASSOCKADDR);
  socket_endread(self);
 } else {
  socket_endread(self);
  memcpy(result,&self->s_sockaddr,sizeof(SockAddr));
 }
 return 0;
}
INTERN int DCALL
DeeSocket_GetPeerAddr(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result,
                      bool throw_error) {
 socklen_t socklen; int ok = 0;
 socket_read(self);
 if (self->s_state&SOCKET_FHASPEERADDR) {
  memcpy(result,&self->s_peeraddr,sizeof(SockAddr));
  socket_endread(self);
  return 0;
 }
 socket_endread(self);

 socklen = (socklen_t)SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,
                                      self->s_proto);
 socket_read(self);
 DBG_ALIGNMENT_DISABLE();
 if unlikely(getpeername(self->s_socket,&result->sa,&socklen) < 0)
    ok = -1;
 else if (!(self->s_state&SOCKET_FHASPEERADDR)) {
  memcpy(&self->s_peeraddr,result,sizeof(SockAddr));
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
   DeeError_SysThrowf(&DeeError_NotConnected,err,
                      "Socket %k is not connected",self);
  } else if (err == EOPNOTSUPP) {
   DeeError_SysThrowf(&DeeError_NoSupport,err,
                      "Cannot query peer address for socket protocol %R",
                      sock_getprotonameorid(self->s_proto));
  } else if (err == EINVAL) {
   err_socket_closed(err,self);
  } else {
   DeeError_SysThrowf(&DeeError_NetError,err,
                      "Failed to query peer address of socket %k",
                      self);
  }
 }
 return ok;
}


PRIVATE DREF DeeSockAddrObject *DCALL
socket_sockname_get(Socket *__restrict self) {
 DREF DeeSockAddrObject *result;
 result = DeeObject_MALLOC(DeeSockAddrObject);
 if unlikely(!result) goto done;
 if unlikely(DeeSocket_GetSockName(self,&result->sa_addr,true)) {
  DeeObject_Free(result);
  result = NULL;
  goto done;
 }
 DeeObject_Init(result,&DeeSockAddr_Type);
done:
 return result;
}
PRIVATE DREF DeeSockAddrObject *DCALL
socket_peeraddr_get(Socket *__restrict self) {
 DREF DeeSockAddrObject *result;
 if (DeeThread_CheckInterrupt())
     return NULL;
 result = DeeObject_MALLOC(DeeSockAddrObject);
 if unlikely(!result) goto done;
 if unlikely(DeeSocket_GetPeerAddr(self,&result->sa_addr,true)) {
  DeeObject_Free(result);
  result = NULL;
  goto done;
 }
 DeeObject_Init(result,&DeeSockAddr_Type);
done:
 return result;
}

PRIVATE DEFINE_STRING(shutdown_all,"rw");

PRIVATE int DCALL
socket_do_shutdown(Socket *__restrict self, int how) {
 int error;
 DBG_ALIGNMENT_DISABLE();
 error = shutdown(self->s_socket,how);
 if (error < 0 && GET_NET_ERROR() == ENOTCONN)
     error = 0;
 DBG_ALIGNMENT_ENABLE();
 return error;
}


PRIVATE ATTR_COLD int DCALL
err_invalid_shutdown_how(int how) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Invalid shutdown mode %x",how);
}
PRIVATE ATTR_COLD int DCALL
err_shutdown_failed(Socket *__restrict self, int error) {
 return DeeError_SysThrowf(&DeeError_NetError,error,
                           "Failed to shutdown socket %k",
                           self);
}

PRIVATE DREF DeeObject *DCALL
socket_close(Socket *__restrict self, size_t argc,
             DeeObject **__restrict argv) {
 sock_t socket_handle;
 DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
 if (DeeArg_Unpack(argc,argv,"|o:close",&shutdown_mode))
     goto err;
 if (!DeeString_Check(shutdown_mode) ||
     !DeeString_IsEmpty(shutdown_mode)) {
  int error,mode; uint16_t new_state;
  if unlikely(get_shutdown_modeof(shutdown_mode,&mode))
     goto err;
  /* First of: acquire read-access and call shutdown(). */
  if (mode == SHUT_RD) {
   new_state = SOCKET_FSHUTDOWN_R;
  } else if (mode == SHUT_WR) {
   new_state = SOCKET_FSHUTDOWN_W;
  } else {
   new_state = SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W;
  }
again_shutdown:
  /* Test for interrupts. */
  if (DeeThread_CheckInterrupt())
      goto err;
  socket_read(self);
  if (ATOMIC_FETCHOR(self->s_state,SOCKET_FSHUTTINGDOWN) &
                                   SOCKET_FSHUTTINGDOWN) {
   /* Special case: Another shutdown operation is still in progress. */
   socket_endread(self);
   DeeThread_SleepNoInterrupt(1000);
   goto again_shutdown;
  }
  /* We'll have to do the shutdown. */
  error = socket_do_shutdown(self,mode);
  if unlikely(error < 0) {
   DBG_ALIGNMENT_DISABLE();
   error = GET_NET_ERROR();
   DBG_ALIGNMENT_ENABLE();
   ATOMIC_FETCHAND(self->s_state,~SOCKET_FSHUTTINGDOWN);
   socket_endread(self);
   if (error == EINVAL)
    err_invalid_shutdown_how(mode);
   else {
    err_shutdown_failed(self,error);
   }
   goto err;
  }
  /* Indicate that shutdown has been completed. */
  ATOMIC_FETCHOR(self->s_state,new_state);
  ATOMIC_FETCHAND(self->s_state,~SOCKET_FSHUTTINGDOWN);
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

PRIVATE DREF DeeObject *DCALL
socket_shutdown(Socket *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
 DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
 if (DeeArg_Unpack(argc,argv,"|o:shutdown",&shutdown_mode))
     goto err;
 if (!DeeString_Check(shutdown_mode) ||
     !DeeString_IsEmpty(shutdown_mode)) {
  int error,mode; uint16_t new_state;
  if unlikely(get_shutdown_modeof(shutdown_mode,&mode))
     goto err;
  /* First of: acquire read-access and call shutdown(). */
  if (mode == SHUT_RD) {
   new_state = SOCKET_FSHUTDOWN_R;
  } else if (mode == SHUT_WR) {
   new_state = SOCKET_FSHUTDOWN_W;
  } else {
   new_state = SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W;
  }
again_shutdown:
  /* Test for interrupts. */
  if (DeeThread_CheckInterrupt())
      goto err;
  socket_read(self);
  if (ATOMIC_FETCHOR(self->s_state,SOCKET_FSHUTTINGDOWN) &
                                   SOCKET_FSHUTTINGDOWN) {
   /* Special case: The socket is already being shut down. */
   socket_endread(self);
   DeeThread_SleepNoInterrupt(1000);
   goto again_shutdown;
  }
  /* Actually do the shutdown. */
  error = socket_do_shutdown(self,mode);
  if unlikely(error < 0) {
   DBG_ALIGNMENT_DISABLE();
   error = GET_NET_ERROR();
   DBG_ALIGNMENT_ENABLE();
   if (error == ENOTCONN) {
    /* Ignore not-connected errors. */
   } else {
    ATOMIC_FETCHAND(self->s_state,~SOCKET_FSHUTTINGDOWN);
    socket_endread(self);
    if (error == EINVAL)
     err_invalid_shutdown_how(mode);
    else if (error == EBADF || error == ENOTSOCK)
     err_socket_closed(error,self);
    else {
     err_shutdown_failed(self,error);
    }
    goto err;
   }
  }
  /* Indicate that shutdown has been completed. */
  ATOMIC_FETCHOR(self->s_state,new_state);
  ATOMIC_FETCHAND(self->s_state,~SOCKET_FSHUTTINGDOWN);
  socket_endread(self);
 }
 return_none;
err:
 return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_addr_not_available(neterrno_t error, SockAddr const *__restrict addr,
                       int prototype) {
 return DeeError_SysThrowf(&DeeError_AddrNotAvail,error,
                           "The specified address %K is not available from the local machine",
                           SockAddr_ToString(addr,prototype,SOCKADDR_STR_FNOFAIL | SOCKADDR_STR_FNODNS));
}

INTERN int DCALL
DeeSocket_Bind(DeeSocketObject *__restrict self,
               SockAddr const *__restrict addr) {
 int error; uint16_t state;
 neterrno_t error_code;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 socket_read(self);
 /* Enter the binding-state. */
 if (ATOMIC_FETCHOR(self->s_state,SOCKET_FBINDING) &
                                  SOCKET_FBINDING) {

  socket_endread(self);
  /* The socket is already in the middle of a binding-call. */
  DeeThread_SleepNoInterrupt(1000);
  goto again;
 }
 DBG_ALIGNMENT_DISABLE();
#if defined(SOL_SOCKET) && defined(SO_REUSEADDR) /* Enable local address reuse */
 { int yes = 1; setsockopt(self->s_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes)); }
#endif
 /* Do the bind system call. */
 error = bind(self->s_socket,(struct sockaddr *)addr,(socklen_t)
              SockAddr_Sizeof(addr->sa.sa_family,self->s_proto));
 DBG_ALIGNMENT_ENABLE();
 if likely(error >= 0) {
  /* Save the (now) active socket address. */
  memcpy(&self->s_sockaddr,addr,sizeof(SockAddr));
  COMPILER_WRITE_BARRIER();
  ATOMIC_FETCHOR(self->s_state,SOCKET_FBOUND|SOCKET_FHASSOCKADDR);
 }
 /* Unset the binding-flag. */
 state = ATOMIC_FETCHAND(self->s_state,~SOCKET_FBINDING);
 socket_endread(self);
 if likely(error >= 0)
    return 0;
 DBG_ALIGNMENT_DISABLE();
 error_code = GET_NET_ERROR();
 DBG_ALIGNMENT_ENABLE();
 /* Handle errors. */
 if (error_code == EADDRINUSE) {
  DeeError_SysThrowf(&DeeError_AddrInUse,error_code,
                     "The specified address %K is already in use",
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 } else if (error_code == EADDRNOTAVAIL) {
  err_addr_not_available(error_code,addr,self->s_proto);
 } else if (error_code == EBADF || error_code == ENOTSOCK) {
  goto err_closed;
 } else if (error_code == EINVAL) {
  if (!(state&SOCKET_FOPENED) ||
       (state&SOCKET_FSHUTDOWN_R)) {
err_closed:
   err_socket_closed(error_code,self);
  } else {
   DeeError_SysThrowf(&DeeError_NetError,error_code,
                      "Cannot rebind socket %k of address family %K to address %K",self,
                      sock_getafnameorid(self->s_proto),
                      SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
  }
 } else if (error_code == EOPNOTSUPP) {
  DeeError_SysThrowf(&DeeError_NoSupport,error_code,
                     "The socket protocol %K does allow address binding",
                     sock_getafnameorid(self->s_proto));
 } else if (error_code == EISCONN) {
  DeeError_SysThrowf(&DeeError_IsConnected,error_code,
                     "socket %k is already connected",self);
 } else {
  DeeError_SysThrowf(&DeeError_NetError,error_code,
                     "Failed to bind socket %k to address %K",self,
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 }
err:
 return -1;
}


#ifdef CONFIG_HOST_WINDOWS
/* On windows, accept() isn't interruptible (and neither is `select()')
 * But to still ensure that this blocking call can be interrupted by
 * `thread.interrupt()', we need to use this system call right here: */
PRIVATE DWORD DCALL
select_interruptible(SOCKET sock, DWORD lNetworkEvents, DWORD timeout) {
 DWORD result; HANDLE sockevent;
 /* NOTE: No need to do further error checking.
  *       These API functions will propagate errors for us automatically. */
 DBG_ALIGNMENT_DISABLE();
 sockevent = WSACreateEvent();
 WSAEventSelect(sock,sockevent,lNetworkEvents);
 result = WSAWaitForMultipleEvents(1,&sockevent,FALSE,timeout,TRUE);
 /* Apparently this is required to prevent the event also closing the socket??? */
 WSAEventSelect(sock,sockevent,0);
 WSACloseEvent(sockevent);
 DBG_ALIGNMENT_ENABLE();
 return result;
}
#endif

#define SELECT_TIMEOUT  100000 /* In microseconds. */


PRIVATE ATTR_COLD int DCALL
err_network_down(int error, Socket *__restrict socket, SockAddr const *__restrict addr) {
 return DeeError_SysThrowf(&DeeError_NetUnreachable,error,
                           "No route to network of %K can be established",
                           SockAddr_ToString(addr,socket->s_proto,
                                             SOCKADDR_STR_FNOFAIL |
                                             SOCKADDR_STR_FNODNS));
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4548)
#endif

INTERN int DCALL
DeeSocket_Connect(DeeSocketObject *__restrict self,
                  SockAddr const *__restrict addr) {
 int error;
 socklen_t addrlen;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 socket_read(self);
 /* Enter the connecting-state. */
 if (ATOMIC_FETCHOR(self->s_state,SOCKET_FCONNECTING) &
                                  SOCKET_FCONNECTING) {

  socket_endread(self);
  /* The socket is already in the middle of a connecting-call.
   * Wait for that other connect() call to complete, then try again. */
  DeeThread_SleepNoInterrupt(1000);
  goto again;
 }
 /* Do the connect system call. */
 addrlen = SockAddr_Sizeof(addr->sa.sa_family,self->s_proto);
 DBG_ALIGNMENT_DISABLE();
 error = connect(self->s_socket,(struct sockaddr *)addr,addrlen);
 DBG_ALIGNMENT_ENABLE();
 if (error < 0) {
  DBG_ALIGNMENT_DISABLE();
  error = (int)GET_NET_ERROR();
  DBG_ALIGNMENT_ENABLE();
  if (error == EINPROGRESS
#ifdef EINTR
   || error == EINTR
#endif
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
                                FD_CONNECT|FD_CLOSE,
                                SELECT_TIMEOUT/1000);
   if (error != WSA_WAIT_EVENT_0) {
    if (error == WSA_WAIT_IO_COMPLETION ||
        error == WSA_WAIT_TIMEOUT)
        goto restart_select; /* Timeout */
    goto err_connect_failure;
   }
#else
   {
    struct timeval timeout; fd_set wfds;
    timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
    timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
    FD_ZERO(&wfds);
    FD_SET(self->s_socket,&wfds);
    DBG_ALIGNMENT_DISABLE();
    error = select(self->s_socket+1,NULL,&wfds,NULL,&timeout);
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
#endif
     goto err_connect_failure;
    }
   }
#endif
   errlen = sizeof(error);
   /* Check for an error in the connect operation */
   DBG_ALIGNMENT_DISABLE();
   if (getsockopt(self->s_socket,SOL_SOCKET,SO_ERROR,
                 (char *)&error,&errlen))
       error = 0;
   DBG_ALIGNMENT_ENABLE();
   if unlikely(error)
      goto err_connect_failure;
  }
 }
 ATOMIC_FETCHOR(self->s_state,SOCKET_FCONNECTED|SOCKET_FHASSOCKADDR);
 /* Unset the connecting-flag. */
 ATOMIC_FETCHAND(self->s_state,~SOCKET_FCONNECTING);
 socket_endread(self);
 return 0;
err_connect_failed:
 ATOMIC_FETCHAND(self->s_state,~SOCKET_FCONNECTING);
 goto err;
err_connect_failure:
 ATOMIC_FETCHAND(self->s_state,~SOCKET_FCONNECTING);
 socket_endread(self);
 /* Handle errors. */
 if (error == EADDRNOTAVAIL) {
  err_addr_not_available((neterrno_t)error,addr,self->s_proto);
 } else if (error == EAFNOSUPPORT) {
  err_no_af_support((neterrno_t)error,addr->sa.sa_family);
 } else if (error == EBADF || error == ENOTSOCK) {
  err_socket_closed(error,self);
 } else if (error == EOPNOTSUPP) {
  DeeError_Throwf(&DeeError_NoSupport,
                  "The socket %k is currently listening and cannot connect",
                  self);
 } else if (error == EALREADY || error == EISCONN) {
  DeeError_SysThrowf(&DeeError_IsConnected,error,
                     "Socket %k is already connected or connecting",self);
 } else if (error == ECONNRESET) {
  DeeError_SysThrowf(&DeeError_ConnectReset,error,
                     "The target %K reset the connection request before it could complete",
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 } else if (error == ECONNREFUSED) {
  DeeError_SysThrowf(&DeeError_ConnectRefused,error,
                     "Target %K is not listening or has refused to connect",
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 } else if (error == ENETUNREACH) {
  err_network_down(error,self,addr);
 } else if (error == EHOSTUNREACH) {
  DeeError_SysThrowf(&DeeError_HostUnreachable,error,
                     "The target host %K cannot be reached",
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 } else if (error == EPROTOTYPE) {
  DeeError_Throwf(&DeeError_NoSupport,
                  "The address %K uses a different type than socket %k",
                  SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS),self);
 } else if (error == ETIMEDOUT) {
  DeeError_Throwf(&DeeError_TimedOut,
                  "Timed out while attempting to connect to %K",
                  SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 } else {
  DeeError_SysThrowf(&DeeError_NetError,error,
                     "Failed to connect socket %k with address %K",self,
                     SockAddr_ToString(addr,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
 }
err:
 return -1;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

PRIVATE int DCALL get_default_backlog(void) {
 /* TODO: getenv("DEEMON_MAXBACKLOG") */
 return 5;
}

INTERN int DCALL
DeeSocket_Listen(DeeSocketObject *__restrict self, int max_backlog) {
 int error; uint16_t state;
 neterrno_t error_code;
 if (max_backlog < 0)
     max_backlog = get_default_backlog();
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 socket_read(self);
 /* Enter the listening-state. */
 if (ATOMIC_FETCHOR(self->s_state,SOCKET_FSTARTLISTENING) &
                                  SOCKET_FSTARTLISTENING) {

  socket_endread(self);
  /* The socket is already in the middle of a listening-call. */
  DeeThread_SleepNoInterrupt(1000);
  goto again;
 }
 /* Do the listen system call. */
 DBG_ALIGNMENT_DISABLE();
 error = listen(self->s_socket,max_backlog);
 DBG_ALIGNMENT_ENABLE();
 if likely(error >= 0)
    ATOMIC_FETCHOR(self->s_state,SOCKET_FLISTENING);
 /* Unset the binding-flag. */
 state = ATOMIC_FETCHAND(self->s_state,~SOCKET_FSTARTLISTENING);
 socket_endread(self);
 if likely(error >= 0)
    return 0;
 DBG_ALIGNMENT_DISABLE();
 error_code = GET_NET_ERROR();
 DBG_ALIGNMENT_ENABLE();
 /* Handle errors. */
 if (error_code == EDESTADDRREQ) {
  DeeError_SysThrowf(&DeeError_NotBound,error_code,
                     "Socket %k is not bound and protocol %K does not allow listening on an unbound socket",
                     self,sock_getprotonameorid(self->s_proto));
 } else if (error_code == EOPNOTSUPP) {
  DeeError_SysThrowf(&DeeError_NoSupport,error_code,
                     "The socket protocol %K does not allow listening",
                     sock_getprotonameorid(self->s_proto));
 } else if (error_code == EBADF || error_code == ENOTSOCK) {
  goto err_closed;
 } else if (error_code == EINVAL) {
  if (!(state&SOCKET_FOPENED) ||
       (state&SOCKET_FSHUTDOWN_R)) {
err_closed:
   err_socket_closed(error_code,self);
  } else {
   DeeError_SysThrowf(&DeeError_IsConnected,error_code,
                      "Cannot start listening on socket %k that is already connected",
                      self);
  }
 } else {
  DeeError_SysThrowf(&DeeError_NetError,error_code,
                     "Failed to start listening with socket %k",self);
 }
err:
 return -1;
}


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4548)
#endif
INTERN int DCALL
DeeSocket_Accept(DeeSocketObject *__restrict self,
                 uint64_t timeout_microseconds,
                 sock_t *__restrict sock_fd,
                 SockAddr *__restrict addr) {
 int error; socklen_t socklen;
 sock_t client_socket;
restart:
 socklen = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,self->s_proto);
 if (timeout_microseconds == (uint64_t)-1) {
  /* Accept. */
  if (DeeThread_CheckInterrupt())
      goto err;
  socket_read(self);
  if (!(self->s_state&SOCKET_FOPENED) ||
       (self->s_state&SOCKET_FSHUTDOWN_R)) {
   socket_endread(self);
   error = EBADF;
   goto socket_was_closed;
  }
#ifdef CONFIG_HOST_WINDOWS
  error = select_interruptible(self->s_socket,
                               FD_ACCEPT|FD_CLOSE,
                               SELECT_TIMEOUT/1000);
  if (error != WSA_WAIT_EVENT_0) {
   socket_endread(self);
   if (error == WSA_WAIT_IO_COMPLETION ||
       error == WSA_WAIT_TIMEOUT)
       goto restart;
   goto handle_accept_error_neterror;
  }
#else
  {
   struct timeval timeout; fd_set rfds;
   timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
   timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
   FD_ZERO(&rfds);
   FD_SET(self->s_socket,&rfds);
   error = select(self->s_socket+1,&rfds,NULL,NULL,&timeout);
  }
  if (error <= 0) { /* Timeout or error */
   socket_endread(self);
   DBG_ALIGNMENT_DISABLE();
   if (error < 0
#ifdef EINTR
    && GET_NET_ERROR() != EINTR
#endif
       ) {
    DBG_ALIGNMENT_ENABLE();
    goto handle_accept_error_neterror;
   }
   DBG_ALIGNMENT_ENABLE();
   goto do_timed_select;
  }
#endif
  DBG_ALIGNMENT_DISABLE();
  client_socket = accept(self->s_socket,&addr->sa,&socklen);
  DBG_ALIGNMENT_ENABLE();
  socket_endread(self);
 } else if (timeout_microseconds == 0) {
  /* Try-accept. */
do_try_select:
  socket_read(self);
  if (!(self->s_state&SOCKET_FOPENED) ||
       (self->s_state&SOCKET_FSHUTDOWN_R)) {
   socket_endread(self);
   error = EBADF;
   goto socket_was_closed;
  }
#ifdef CONFIG_HOST_WINDOWS
  error = select_interruptible(self->s_socket,FD_ACCEPT|FD_CLOSE,0);
  if (error != WSA_WAIT_EVENT_0) {
   socket_endread(self);
   if (error == WSA_WAIT_IO_COMPLETION ||
       error == WSA_WAIT_TIMEOUT)
       return 1; /* Timeout */
   goto handle_accept_error_neterror;
  }
#else
  {
   struct timeval timeout; fd_set rfds;
   timeout.tv_sec  = 0;
   timeout.tv_usec = 1; /* Prevent the OS from discarding this request. */
   FD_ZERO(&rfds);
   FD_SET(self->s_socket,&rfds);
   DBG_ALIGNMENT_DISABLE();
   error = select(self->s_socket+1,&rfds,NULL,NULL,&timeout);
   DBG_ALIGNMENT_ENABLE();
   if (error <= 0) { /* Timeout or error */
    socket_endread(self);
    if (error < 0) {
#ifdef EINTR
     DBG_ALIGNMENT_DISABLE();
     if (GET_NET_ERROR() == EINTR) {
      DBG_ALIGNMENT_ENABLE();
      goto do_try_select;
     }
     DBG_ALIGNMENT_ENABLE();
#endif
     goto handle_accept_error_neterror;
    }
    return 1;
   }
  }
#endif
  /* Acquire an exclusive lock to prevent anyone else from stealing our client. */
  if (!socket_tryupgrade(self)) {
   socket_endread(self);
   DeeThread_SleepNoInterrupt(1000);
   goto do_try_select;
  }
  DBG_ALIGNMENT_DISABLE();
  client_socket = accept(self->s_socket,&addr->sa,&socklen);
  DBG_ALIGNMENT_ENABLE();
  socket_endwrite(self);
 } else {
  uint64_t end_time;
  end_time  = DeeThread_GetTimeMicroSeconds();
  end_time += timeout_microseconds;
  /* Timed-accept. */
do_timed_select:
  if (DeeThread_CheckInterrupt())
      goto err;
  ASSERT(timeout_microseconds);
  socket_read(self);
  if (!(self->s_state&SOCKET_FOPENED) ||
       (self->s_state&SOCKET_FSHUTDOWN_R)) {
   socket_endread(self);
   error = EBADF;
   goto socket_was_closed;
  }
#ifdef CONFIG_HOST_WINDOWS
  {
   DWORD timeout = SELECT_TIMEOUT;
   if (timeout > timeout_microseconds)
       timeout = (DWORD)timeout_microseconds;
   error = select_interruptible(self->s_socket,FD_ACCEPT|FD_CLOSE,
                                timeout/1000);
  }
  if (error != WSA_WAIT_EVENT_0) {
   uint64_t now;
   socket_endread(self);
   if (error != WSA_WAIT_IO_COMPLETION &&
       error != WSA_WAIT_TIMEOUT)
       goto handle_accept_error_neterror; /* Network error. */
   now = DeeThread_GetTimeMicroSeconds();
   if (now >= end_time) return 1; /* Timeout */
   /* Update the remaining time.
    * NOTE: Never ZERO because `end_time > now' right now. */
   timeout_microseconds = end_time-now;
   goto do_timed_select;
  }
#else
  {
   struct timeval timeout; fd_set rfds;
   if (timeout_microseconds < SELECT_TIMEOUT) {
    timeout.tv_sec  = (long)(timeout_microseconds / 1000000);
    timeout.tv_usec = (long)(timeout_microseconds % 1000000);
   } else {
    timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
    timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
   }
   FD_ZERO(&rfds);
   FD_SET(self->s_socket,&rfds);
   DBG_ALIGNMENT_DISABLE();
   error = select(self->s_socket+1,&rfds,NULL,NULL,&timeout);
   DBG_ALIGNMENT_ENABLE();
  }
  if (error <= 0) { /* Timeout or error */
   uint64_t now;
   socket_endread(self);
   DBG_ALIGNMENT_DISABLE();
   if (error < 0
#ifdef EINTR
    && GET_NET_ERROR() != EINTR
#endif
       ) {
    DBG_ALIGNMENT_ENABLE();
    goto handle_accept_error_neterror;
   }
   DBG_ALIGNMENT_ENABLE();
   now = DeeThread_GetTimeMicroSeconds();
   if (now >= end_time) return 1; /* Timeout */
   /* Update the remaining time.
    * NOTE: Never ZERO because `end_time > now' right now. */
   timeout_microseconds = end_time-now;
   goto do_timed_select;
  }
#endif
  /* Acquire an exclusive lock to prevent anyone else from stealing our client. */
  if (!socket_tryupgrade(self)) {
   uint64_t now;
   socket_endread(self);
   DeeThread_SleepNoInterrupt(1000);
   now = DeeThread_GetTimeMicroSeconds();
   if (now >= end_time) goto do_try_select;
   timeout_microseconds = end_time - now;
   goto do_timed_select;
  }
  /* Accept the client. */
  DBG_ALIGNMENT_DISABLE();
  client_socket = accept(self->s_socket,&addr->sa,&socklen);
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
 if (error == EWOULDBLOCK) goto restart_after_timeout;
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
 if (error == EAGAIN) goto restart_after_timeout;
#endif
 /* Ignore this case (Can happen when the client changed their mind) */
 if (error == ECONNABORTED) goto restart;
#ifdef EINTR
 /* Same deal as `ECONNABORTED' -- Start over. */
 if (error == EINTR) goto restart;
#endif
#ifdef ENOMEM
 if (error == ENOMEM) {
  if (Dee_CollectMemory(1))
      goto restart;
  goto err;
 }
#endif
 if (error == EBADF || error == ENOTSOCK) {
socket_was_closed:
  err_socket_closed(error,self);
 } else if (error == EINVAL) {
  DeeError_SysThrowf(&DeeError_NotListening,error,
                     "Cannot accept connections from a socket %k that is not listening",
                     self);
 } else if (error == EOPNOTSUPP) {
  DeeError_SysThrowf(&DeeError_NoSupport,error,
                     "The type %K of socket %k does not support accepting connections",
                     sock_gettypenameorid(self->s_type),self);
 } else {
  DeeError_SysThrowf(&DeeError_NetError,error,
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
#endif

#ifdef CONFIG_HOST_WINDOWS
#define send(socket,buffer,length,flags)                      (dssize_t)send(socket,(char const *)(buffer),(int)(length),flags)
#define sendto(socket,buffer,length,flags,target,targetlen)   (dssize_t)sendto(socket,(char const *)(buffer),(int)(length),flags,target,targetlen)
#define recv(socket,buffer,length,flags)                      (dssize_t)recv(socket,(char *)(buffer),(int)(length),flags)
#define recvfrom(socket,buffer,length,flags,target,targetlen) (dssize_t)recvfrom(socket,(char *)(buffer),(int)(length),flags,target,targetlen)
#endif


PRIVATE ATTR_COLD int DCALL
err_message_too_large(int error, Socket *__restrict self, size_t bufsize) {
 return DeeError_SysThrowf(&DeeError_MessageSize,error,
                           "Message consisting of %Iu bytes is too large for socket %k",
                           bufsize,self);
}

PRIVATE char const transfer_context_send[] = "transfer";
PRIVATE char const transfer_context_recv[] = "receive";
PRIVATE ATTR_COLD int DCALL
err_invalid_transfer_mode(int error, Socket *__restrict self,
                          char const *__restrict context, int mode) {
 return DeeError_SysThrowf(&DeeError_NoSupport,error,
                           "Socket %k does not support %s mode %K",
                           self,context,sock_getmsgflagsnameorid(mode));
}


#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
PRIVATE int DCALL
socket_configure_recv(Socket *__restrict self) {
 (void)self;
 return 0;
}
PRIVATE int DCALL
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
 return DeeError_SysThrowf(&DeeError_NetError,error,
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
 return DeeError_SysThrowf(&DeeError_NetError,error,
                           "Failed to configure socket %k for sending",
                           self);
}
#endif


/* Wait for data being ready to be sent. */
#ifdef CONFIG_HOST_WINDOWS
#define WAITFORDATA_RECV (FD_READ|FD_CLOSE)
#define WAITFORDATA_SEND (FD_WRITE|FD_CLOSE)
#else
#define WAITFORDATA_RECV  0
#define WAITFORDATA_SEND  1
#endif
PRIVATE int DCALL
wait_for_data(Socket *__restrict self,
              uint64_t end_time, int mode) {
 int error;
#ifdef EINTR
restart:
#endif
 if (end_time == (uint64_t)-1) {
  /* wait */
retry_infinite:
  socket_endread(self);
  if (DeeThread_CheckInterrupt())
      goto err_nounlock;
  socket_read(self);
#ifdef CONFIG_HOST_WINDOWS
  {
   error = select_interruptible(self->s_socket,mode,
                                SELECT_TIMEOUT/1000);
   if (error != WSA_WAIT_EVENT_0) {
    if (error != WSA_WAIT_IO_COMPLETION &&
        error != WSA_WAIT_TIMEOUT)
        goto err_select; /* Network error. */
    goto retry_infinite;
   }
  }
#else
  {
   struct timeval timeout; fd_set fds;
   timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
   timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
   FD_ZERO(&fds);
   FD_SET(self->s_socket,&fds);
   DBG_ALIGNMENT_DISABLE();
   error = select(self->s_socket+1,
                  mode == WAITFORDATA_RECV ? &fds : NULL,
                  mode == WAITFORDATA_SEND ? &fds : NULL,
                  NULL,&timeout);
   DBG_ALIGNMENT_ENABLE();
   if (error <= 0) { /* Timeout or error */
    if (error < 0) goto err_select;
    goto retry_infinite;
   }
  }
#endif
 } else if (end_time == 0) {
  /* Try-wait */
#ifdef CONFIG_HOST_WINDOWS
  error = select_interruptible(self->s_socket,mode,0);
  if (error != WSA_WAIT_EVENT_0) {
   if (error != WSA_WAIT_IO_COMPLETION &&
       error != WSA_WAIT_TIMEOUT)
       goto err_select; /* Network error. */
   goto send_timeout;
  }
#else
  {
   struct timeval timeout; fd_set fds;
   timeout.tv_sec  = 0;
   timeout.tv_usec = 1;
   FD_ZERO(&fds);
   FD_SET(self->s_socket,&fds);
   DBG_ALIGNMENT_DISABLE();
   error = select(self->s_socket+1,
                  mode == WAITFORDATA_RECV ? &fds : NULL,
                  mode == WAITFORDATA_SEND ? &fds : NULL,
                  NULL,&timeout);
   DBG_ALIGNMENT_ENABLE();
   if (error <= 0) { /* Timeout or error */
    if (error < 0) goto err_select;
    goto send_timeout;
   }
  }
#endif
 } else {
  /* timed-wait */
  uint64_t now;
retry_timeout:
  socket_endread(self);
  if (DeeThread_CheckInterrupt())
      goto err_nounlock;
  now = DeeThread_GetTimeMicroSeconds();
  if unlikely(now >= end_time) goto send_timeout_nounlock;
  socket_read(self);
#ifdef CONFIG_HOST_WINDOWS
  {
   DWORD timeout = (DWORD)((end_time-now)/1000);
   if (timeout > SELECT_TIMEOUT/1000)
       timeout = SELECT_TIMEOUT/1000;
   error = select_interruptible(self->s_socket,mode,timeout);
   if (error != WSA_WAIT_EVENT_0) {
    if (error != WSA_WAIT_IO_COMPLETION &&
        error != WSA_WAIT_TIMEOUT)
        goto err_select; /* Network error. */
    goto retry_timeout;
   }
  }
#else
  {
   struct timeval timeout; fd_set fds;
   uint64_t remaining_time = end_time-now;
   if (remaining_time < SELECT_TIMEOUT) {
    timeout.tv_sec  = (long)(remaining_time / 1000000);
    timeout.tv_usec = (long)(remaining_time % 1000000);
   } else {
    timeout.tv_sec  = (long)(SELECT_TIMEOUT / 1000000);
    timeout.tv_usec = (long)(SELECT_TIMEOUT % 1000000);
   }
   FD_ZERO(&fds);
   FD_SET(self->s_socket,&fds);
   DBG_ALIGNMENT_DISABLE();
   error = select(self->s_socket+1,
                  mode == WAITFORDATA_RECV ? &fds : NULL,
                  mode == WAITFORDATA_SEND ? &fds : NULL,
                  NULL,&timeout);
   DBG_ALIGNMENT_ENABLE();
   if (error <= 0) { /* Timeout or error */
    if (error < 0) goto err_select;
    goto retry_timeout;
   }
  }
#endif
 }
 return 0;
err_select:
 DBG_ALIGNMENT_DISABLE();
 error = GET_NET_ERROR();
 DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
 if (error == EINTR)
     goto restart;
#endif
 socket_endread(self);
 if (error == EBADF) {
  err_socket_closed(error,self);
 } else {
  DeeError_SysThrowf(&DeeError_NetError,error,
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
#define wait_for_send(self,end_time) wait_for_data(self,end_time,WAITFORDATA_SEND)
#define wait_for_recv(self,end_time) wait_for_data(self,end_time,WAITFORDATA_RECV)



PRIVATE ATTR_COLD int DCALL
err_receive_not_connected(int error, Socket *__restrict socket) {
 return DeeError_SysThrowf(&DeeError_NotConnected,error,
                           "Cannot receive data from socket %k that isn't connected",
                           socket);
}
PRIVATE ATTR_COLD int DCALL
err_receive_timed_out(int error, Socket *__restrict socket) {
 return DeeError_SysThrowf(&DeeError_TimedOut,error,
                           "Timed out while receiving data through socket %k",
                           socket);
}
PRIVATE ATTR_COLD int DCALL
err_connect_reset(int error, Socket *__restrict socket) {
 return DeeError_SysThrowf(&DeeError_ConnectReset,error,
                           "The connection of socket %k was reset by its peer",
                           socket);
}


INTERN dssize_t DCALL
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
 if (!(self->s_state&SOCKET_FSENDCONFOK)) {
  if (socket_upgrade(self) ||
    !(self->s_state&SOCKET_FSENDCONFOK)) {
   /* Configure the socket for sending before the first send call.
    * >> Unless overwritten by user-configurations, the socket
    *    should not be blocking when attempting to send data. */
   result = socket_configure_send(self);
   if unlikely(result) {
    socket_endwrite(self);
    err_configure_send(self);
    goto err;
   }
   ATOMIC_FETCHOR(self->s_state,SOCKET_FSENDCONFOK);
   socket_downgrade(self);
  }
 }
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
 result = wait_for_send(self,end_time);
 /* NOTE: in the event of a timeout or error,
  *      `wait_for_send()' will have unlocked the socket. */
 if unlikely(result)
    goto done;
 DBG_ALIGNMENT_DISABLE();
 result = send(self->s_socket,buf,bufsize,flags);
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
#endif
#ifdef EINTR
   || error == EINTR
#endif
      ) {
   if (timeout_microseconds != (uint64_t)-1) {
    if (!timeout_microseconds ||
         DeeThread_GetTimeMicroSeconds() >= end_time)
        return -2; /* Timeout */
   }
   goto again;
  }
  if (error == EBADF || error == ENOTSOCK) {
   err_socket_closed(error,self);
  } else if (error == EOPNOTSUPP) {
   err_invalid_transfer_mode(error,self,transfer_context_send,flags);
  } else if (error == ENOTCONN
#ifdef EPIPE
         || (error == EPIPE && !(self->s_state&SOCKET_FSHUTDOWN_W))
#endif
             ) {
   DeeError_SysThrowf(&DeeError_NotConnected,error,
                      "Cannot send data through unconnected socket %k",
                      self);
#ifdef EPIPE
  } else if (error == EPIPE) {
   err_socket_closed(error,self);
#endif
  } else if (error == EMSGSIZE) {
   err_message_too_large(error,self,bufsize);
  } else if (error == ECONNRESET) {
   err_connect_reset(error,self);
  } else if (error == EDESTADDRREQ) {
   DeeError_SysThrowf(&DeeError_NotBound,error,
                      "Socket %k isn't connection-oriented and has no peer address set",
                      self);
  } else if (error == ENETDOWN || error == ENETUNREACH) {
   DeeError_SysThrowf(&DeeError_NetUnreachable,error,
                      "No route to network of connected to socket %k can be established",
                      socket);
  } else {
   DeeError_SysThrowf(&DeeError_NetError,error,
                      "Failed to send %Iu bytes of data through socket %k",
                      bufsize,self);
  }
  goto err;
 }
done:
 return result;
err:
 return -1;
}

INTERN dssize_t DCALL
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
 if (!(self->s_state&SOCKET_FRECVCONFOK)) {
  if (socket_upgrade(self) ||
    !(self->s_state&SOCKET_FRECVCONFOK)) {
   /* Configure the socket for receiving before the first recv call.
    * >> Unless overwritten by user-configurations, the socket
    *    should not be blocking when attempting to receive data. */
   result = socket_configure_recv(self);
   if unlikely(result) {
    socket_endwrite(self);
    err_configure_recv(self);
    goto err;
   }
   ATOMIC_FETCHOR(self->s_state,SOCKET_FRECVCONFOK);
   socket_downgrade(self);
  }
 }
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
 result = wait_for_recv(self,end_time);
 /* NOTE: in the event of a timeout or error,
  *      `wait_for_recv()' will have unlocked the socket. */
 if unlikely(result)
    goto done;
 DBG_ALIGNMENT_DISABLE();
 result = recv(self->s_socket,buf,bufsize,flags);
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
#endif
#ifdef EINTR
   || error == EINTR
#endif
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
   if (Dee_CollectMemory(1)) goto again;
   goto err;
  }
#endif
  if (error == EBADF || error == ENOTSOCK) {
   err_socket_closed(error,self);
#ifdef MSG_OOB
  } else if (error == EINVAL && (flags&MSG_OOB)) {
   /* Indicate that nothing was read by returning 0. */
   result = 0;
   goto done;
#endif
  } else if (error == ECONNRESET) {
   err_connect_reset(error,self);
  } else if (error == ENOTCONN) {
   err_receive_not_connected(error,self);
  } else if (error == EOPNOTSUPP) {
   err_invalid_transfer_mode(error,self,transfer_context_recv,flags);
  } else if (error == ETIMEDOUT) {
   /* Different kind of timeout: The connection timed out, not the data transfer! */
   err_receive_timed_out(error,self);
  } else {
   DeeError_SysThrowf(&DeeError_NetError,error,
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


PRIVATE ATTR_COLD int DCALL
err_host_unreachable(int error, Socket *__restrict socket,
                     SockAddr const *__restrict target) {
 return DeeError_SysThrowf(&DeeError_HostUnreachable,error,
                           "The host specified by %K cannot be reached",
                           SockAddr_ToString(target,socket->s_proto,
                                             SOCKADDR_STR_FNOFAIL |
                                             SOCKADDR_STR_FNODNS));
}

INTERN dssize_t DCALL
DeeSocket_SendTo(DeeSocketObject *__restrict self,
                 uint64_t timeout_microseconds,
                 void const *__restrict buf, size_t bufsize,
                 int flags, SockAddr const *__restrict target) {
 dssize_t result;
 uint64_t end_time = timeout_microseconds;
 ASSERT(target);
 if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
     end_time = DeeThread_GetTimeMicroSeconds()+timeout_microseconds;
again:
 if (timeout_microseconds && DeeThread_CheckInterrupt())
     goto err;
 socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
 if (!(self->s_state&SOCKET_FSENDCONFOK)) {
  if (socket_upgrade(self) ||
    !(self->s_state&SOCKET_FSENDCONFOK)) {
   /* Configure the socket for sending before the first send call.
    * >> Unless overwritten by user-configurations, the socket
    *    should not be blocking when attempting to send data. */
   result = socket_configure_send(self);
   if unlikely(result) {
    socket_endwrite(self);
    err_configure_send(self);
    goto err;
   }
   ATOMIC_FETCHOR(self->s_state,SOCKET_FSENDCONFOK);
   socket_downgrade(self);
  }
 }
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
 result = wait_for_send(self,end_time);
 /* NOTE: in the event of a timeout or error,
  *      `wait_for_send()' will have unlocked the socket. */
 if unlikely(result)
    goto done;
 DBG_ALIGNMENT_DISABLE();
 result = sendto(self->s_socket,buf,bufsize,flags,&target->sa,
                 SockAddr_Sizeof(target->sa.sa_family,self->s_proto));
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
#endif
#ifdef EINTR
   || error == EINTR
#endif
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
#endif
  if (error == EBADF || error == ENOTSOCK) {
   err_socket_closed(error,self);
  } else if (error == EAFNOSUPPORT) {
   DeeError_SysThrowf(&DeeError_NoSupport,error,
                      "Target address family %K cannot be used with socket %k",
                      sock_getafnameorid(target->sa.sa_family),self);
  } else if (error == ECONNRESET) {
   DeeError_SysThrowf(&DeeError_ConnectReset,error,
                      "The peer %K has reset the connection",
                      SockAddr_ToString(target,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
#if 0
  } else if (error == ENOTCONN
#ifdef EPIPE
         || (error == EPIPE && !(self->s_state&SOCKET_FSHUTDOWN_W))
#endif
             ) {
   DeeError_SysThrowf(&DeeError_NotConnected,error,
                      "Socket %k is connection-oriented, but not connected",
                      self);
#endif
#ifdef EPIPE
  } else if (error == EPIPE) {
   err_socket_closed(error,self);
#endif
  } else if (error == EMSGSIZE) {
   err_message_too_large(error,self,bufsize);
  } else if (error == EOPNOTSUPP) {
   err_invalid_transfer_mode(error,self,transfer_context_send,flags);
  } else if (error == EHOSTUNREACH) {
   err_host_unreachable(error,self,target);
  } else if (error == EISCONN) {
   DeeError_SysThrowf(&DeeError_IsConnected,error,
                      "A target address %K was specified when socket %k is already connected",
                      SockAddr_ToString(target,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS),
                      self);
  } else if (error == ENETDOWN || error == ENETUNREACH) {
   err_network_down(error,self,target);
  } else if (error == EINVAL) {
   DeeError_SysThrowf(&DeeError_NoSupport,error,
                      "The specified target address %K is not supported by this implementation",
                      SockAddr_ToString(target,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
  } else {
   DeeError_SysThrowf(&DeeError_NetError,error,
                      "Failed to send %Iu bytes of data through socket %k to address %K",
                      bufsize,self,SockAddr_ToString(target,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS));
  }
  goto err;
 }
done:
 return result;
err:
 return -1;
}

INTERN dssize_t DCALL
DeeSocket_RecvFrom(DeeSocketObject *__restrict self,
                   uint64_t timeout_microseconds,
                   void *__restrict buf, size_t bufsize,
                   int flags, SockAddr *__restrict source) {
 dssize_t result; socklen_t length;
 uint64_t end_time = timeout_microseconds;
 ASSERT(source);
 if (timeout_microseconds && timeout_microseconds != (uint64_t)-1)
     end_time = DeeThread_GetTimeMicroSeconds() + timeout_microseconds;
again:
 if (timeout_microseconds && DeeThread_CheckInterrupt())
     goto err;
 length = SockAddr_Sizeof(self->s_sockaddr.sa.sa_family,
                          self->s_proto);
 socket_read(self);
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
 if (!(self->s_state&SOCKET_FRECVCONFOK)) {
  if (socket_upgrade(self) ||
    !(self->s_state&SOCKET_FRECVCONFOK)) {
   /* Configure the socket for receiving before the first recv call.
    * >> Unless overwritten by user-configurations, the socket
    *    should not be blocking when attempting to receive data. */
   result = socket_configure_recv(self);
   if unlikely(result) {
    socket_endwrite(self);
    err_configure_recv(self);
    goto err;
   }
   ATOMIC_FETCHOR(self->s_state,SOCKET_FRECVCONFOK);
   socket_downgrade(self);
  }
 }
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
 result = wait_for_recv(self,end_time);
 /* NOTE: in the event of a timeout or error,
  *      `wait_for_recv()' will have unlocked the socket. */
 if unlikely(result)
    goto done;
 DBG_ALIGNMENT_DISABLE();
 result = recvfrom(self->s_socket,buf,bufsize,flags,&source->sa,&length);
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
#endif
#ifdef EINTR
   || error == EINTR
#endif
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
#endif
  if (error == EBADF || error == ENOTSOCK) {
   err_socket_closed(error,self);
#ifdef MSG_OOB
  } else if (error == EINVAL && (flags&MSG_OOB)) {
   /* Indicate that nothing was read by returning 0. */
   result = 0;
   goto done;
#endif
#if 0
  } else if (error == ENOTCONN) {
   err_receive_not_connected(error,self);
#endif
  } else if (error == EOPNOTSUPP) {
   err_invalid_transfer_mode(error,self,transfer_context_recv,flags);
  } else if (error == ECONNRESET) {
   err_connect_reset(error,self);
  } else if (error == ETIMEDOUT) {
   /* Different kind of timeout: The connection timed out, not the data transfer! */
   err_receive_timed_out(error,self);
  } else {
   DeeError_SysThrowf(&DeeError_NetError,error,
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

PRIVATE size_t DCALL get_recv_chunksize(void) {
 /* XXX: Consult an environment variable? */
#if 0
 return 1;
#else
 return 2048;
#endif
}
PRIVATE size_t DCALL get_recv_burstsize(void) {
 /* XXX: Consult an environment variable? */
 /* The max size of a single ethernet frame
  * (although we can't really assume ethernet connections...) */
 return 1542;
}


INTERN DREF DeeObject *DCALL
DeeSocket_RecvData(DeeSocketObject *__restrict self,
                   uint64_t timeout_microseconds,
                   size_t max_bufsize, int flags,
                   SockAddr *source) {
 DREF DeeObject *result; dssize_t recv_length;
 if (max_bufsize == (size_t)-1) {
  if (!source) {
   struct bytes_printer printer = BYTES_PRINTER_INIT;
   size_t chunksize = get_recv_chunksize();
   /* Variable-length buffer. */
   for (;;) {
    uint8_t *part = bytes_printer_alloc(&printer,chunksize);
    if unlikely(!part) goto err_printer;
    recv_length = DeeSocket_Recv(self,timeout_microseconds,part,chunksize,flags);
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
    bytes_printer_release(&printer,chunksize-(size_t)recv_length);
    /* Stop trying when no more data can be read. */
    if (!recv_length) break;
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
 if unlikely(!result) goto err;
 recv_length = source ? DeeSocket_RecvFrom(self,timeout_microseconds,
                                           DeeBytes_DATA(result),
                                           max_bufsize,flags,source)
                      : DeeSocket_Recv(self,timeout_microseconds,
                                       DeeBytes_DATA(result),
                                       max_bufsize,flags);
 if unlikely(recv_length < 0) {
  Dee_DecrefDokill(result);
  if (recv_length == -2)
      return ITER_DONE; /* Timeout. */
  goto err;
 }
 if ((size_t)recv_length != max_bufsize)
     result = DeeBytes_TruncateBuffer(result,(size_t)recv_length);
 return result;
err:
 return NULL;
}





PRIVATE DREF DeeObject *DCALL
socket_bind(Socket *__restrict self, size_t argc,
            DeeObject **__restrict argv) {
 SockAddr addr;
 if unlikely(SockAddr_FromArgv(&addr,
                                self->s_sockaddr.sa.sa_family,
                                self->s_proto,
                                self->s_type,
                                argc,
                                argv))
    goto err;
 DBG_ALIGNMENT_ENABLE();
 if unlikely(DeeSocket_Bind(self,&addr))
    goto err;
 return_none;
err:
 DBG_ALIGNMENT_ENABLE();
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_connect(Socket *__restrict self, size_t argc,
               DeeObject **__restrict argv) {
 SockAddr addr;
 if unlikely(SockAddr_FromArgv(&addr,
                                self->s_sockaddr.sa.sa_family,
                                self->s_proto,
                                self->s_type,
                                argc,
                                argv))
    goto err;
 DBG_ALIGNMENT_ENABLE();
 if unlikely(DeeSocket_Connect(self,&addr))
    goto err;
 return_none;
err:
 DBG_ALIGNMENT_ENABLE();
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_listen(Socket *__restrict self, size_t argc,
              DeeObject **__restrict argv) {
 int max_backlog = -1;
 if (DeeArg_Unpack(argc,argv,"|d:listen",&max_backlog))
     goto err;
 if unlikely(DeeSocket_Listen(self,max_backlog))
    goto err;
 return_none;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_doaccept(Socket *__restrict self, uint64_t timeout) {
 DREF Socket *result; int error;
 result = DeeObject_MALLOC(Socket);
 if unlikely(!result) goto err;
 error = DeeSocket_Accept(self,timeout,
                         &result->s_socket,
                         &result->s_peeraddr);
 if unlikely(error < 0) goto err2;
 if (error > 0) {
  DeeObject_Free(result);
  return_none; /* Timeout */
 }
 /* Fill in the remaining members of the new socket. */
 rwlock_init(&result->s_lock);
 result->s_sockaddr.sa.sa_family = result->s_peeraddr.sa.sa_family;
 result->s_state = (SOCKET_FOPENED|SOCKET_FHASPEERADDR|SOCKET_FCONNECTED);
 result->s_type  = self->s_type;
 result->s_proto = self->s_proto;
 DeeObject_Init(result,&DeeSocket_Type);
 return (DREF DeeObject *)result;
err2:
 DeeObject_Free(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
socket_accept(Socket *__restrict self, size_t argc,
              DeeObject **__restrict argv) {
 uint64_t timeout = (uint64_t)-1;
 if (DeeArg_Unpack(argc,argv,"|I64d:accept",&timeout))
     return NULL;
 return socket_doaccept(self,timeout);
}
PRIVATE DREF DeeObject *DCALL
socket_tryaccept(Socket *__restrict self, size_t argc,
                 DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":tryaccept"))
     return NULL;
 return socket_doaccept(self,0);
}
PRIVATE DREF DeeObject *DCALL
socket_recv(Socket *__restrict self, size_t argc,
            DeeObject **__restrict argv) {
 size_t max_size; uint64_t timeout; int flags;
 DeeObject *arg_0 = NULL,*arg_1 = NULL,*arg_2 = NULL;
 DREF DeeObject *result;
 if (DeeArg_Unpack(argc,argv,"|ooo:recv",&arg_0,&arg_1,&arg_2))
     goto err;
 if (!arg_1) {
  max_size = (size_t)-1;
  timeout  = (uint64_t)-1;
  flags    = 0;
  if (arg_0) {
   if (DeeString_Check(arg_0)) {
    /* "(string flags)->string\n" */
    if (sock_getmsgflagsof(arg_0,&flags))
        goto err;
   } else {
    /* "(int max_size=-1,int string flags=\"\")->string\n" */
    /* "(int max_size=-1,int timeout_microseconds=-1)->string\n" */
    if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size))
        goto err;
   }
  }
 } else if (!arg_2) {
  /* "(int max_size=-1,int string flags=\"\")->string\n" */
  /* "(int max_size=-1,int timeout_microseconds=-1)->string\n" */
  if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size))
      goto err;
  if (DeeString_Check(arg_1)) {
   if (sock_getmsgflagsof(arg_1,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   flags = 0;
   if (DeeObject_AsInt64(arg_1,(int64_t *)&timeout))
       goto err;
  }
 } else {
  /* "(int max_size=-1,int timeout_microseconds=-1,string flags=\"\")->string\n" */
  /* "(int max_size=-1,int timeout_microseconds=-1,int flags=0)->string\n" */
  if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size) ||
      DeeObject_AsInt64(arg_1,(int64_t *)&timeout) ||
      sock_getmsgflagsof(arg_2,&flags))
      goto err;
 }
 result = DeeSocket_RecvData(self,timeout,max_size,flags,NULL);
 /* Return an empty string when the timeout has expired. */
 if (result == ITER_DONE) { result = Dee_EmptyString; Dee_Incref(result); }
 return result;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
socket_recvinto(Socket *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
 DeeBuffer buffer; DeeObject *data;
 DeeObject *arg1 = NULL,*arg2 = NULL;
 uint64_t timeout; int flags; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|oo:recvinto",&data,&arg1,&arg2))
     goto err;
 if (!arg1) {
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  timeout = (uint64_t)-1;
  flags   = 0;
 } else if (!arg2) {
  //"(buffer dst,string flags=\"\")->int\n"
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  if (DeeString_Check(arg1)) {
   if (sock_getmsgflagsof(arg1,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   if (DeeObject_AsInt64(arg1,(int64_t *)&timeout))
       goto err;
   flags = 0;
  }
 } else {
  //"(buffer dst,int timeout_microseconds=-1,string flags=\"\")->int\n"
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  if (DeeObject_AsInt64(arg1,(int64_t *)&timeout))
      goto err;
  if (sock_getmsgflagsof(arg2,&flags))
      goto err;
 }
 if (DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FWRITABLE))
     goto err;
 result = DeeSocket_Recv(self,
                         timeout,
                         buffer.bb_base,
                         buffer.bb_size,
                         flags);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FWRITABLE);
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_recvfrom(Socket *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
 DREF DeeSockAddrObject *result_addr;
 DREF DeeObject *result_text,*result;
 size_t max_size; uint64_t timeout; int flags;
 DeeObject *arg_0 = NULL,*arg_1 = NULL,*arg_2 = NULL;
 if (DeeArg_Unpack(argc,argv,"|ooo:recvfrom",&arg_0,&arg_1,&arg_2))
     goto err;
 if (!arg_1) {
  max_size = (size_t)-1;
  timeout  = (uint64_t)-1;
  flags    = 0;
  if (arg_0) {
   if (DeeString_Check(arg_0)) {
    /* "(string flags)->(sockaddr,string)\n" */
    if (sock_getmsgflagsof(arg_0,&flags))
        goto err;
   } else {
    /* "(int max_size=-1,int string flags=\"\")->(sockaddr,string)\n" */
    /* "(int max_size=-1,int timeout_microseconds=-1)->(sockaddr,string)\n" */
    if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size))
        goto err;
   }
  }
 } else if (!arg_2) {
  /* "(int max_size=-1,int string flags=\"\")->(sockaddr,string)\n" */
  /* "(int max_size=-1,int timeout_microseconds=-1)->(sockaddr,string)\n" */
  if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size))
      goto err;
  if (DeeString_Check(arg_1)) {
   if (sock_getmsgflagsof(arg_1,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   flags = 0;
   if (DeeObject_AsInt64(arg_1,(int64_t *)&timeout))
       goto err;
  }
 } else {
  /* "(int max_size=-1,int timeout_microseconds=-1,string flags=\"\")->(sockaddr,string)\n" */
  /* "(int max_size=-1,int timeout_microseconds=-1,int flags=0)->(sockaddr,string)\n" */
  if (DeeObject_AsSSize(arg_0,(dssize_t *)&max_size) ||
      DeeObject_AsInt64(arg_1,(int64_t *)&timeout) ||
      sock_getmsgflagsof(arg_2,&flags))
      goto err;
 }
 /* Create the socket address object that's going to be returned. */
 result_addr = DeeObject_MALLOC(DeeSockAddrObject);
 if unlikely(!result_addr) goto err;
 /* Actually receive the data. */
 result_text = DeeSocket_RecvData(self,timeout,max_size,flags,
                                 &result_addr->sa_addr);
 if unlikely(!result_text) goto err_addr;
 /* Create a new tuple to package the 2 objects. */
 result = DeeTuple_NewUninitialized(2);
 if unlikely(!result) goto err_text;
 if (result_text == ITER_DONE) {
  /* A somewhat different story: must return (none,"") */
  DeeObject_Free(result_addr);
  DeeTuple_SET(result,0,Dee_None);
  DeeTuple_SET(result,1,Dee_EmptyString);
  Dee_Incref(Dee_None);
  Dee_Incref(Dee_EmptyString);
 } else {
  DeeObject_Init(result_addr,&DeeSockAddr_Type);
  /* Fill the result tuple with the socket address and text. */
  DeeTuple_SET(result,0,(DeeObject *)result_addr); /* Inherit */
  DeeTuple_SET(result,1,(DeeObject *)result_text); /* Inherit */
 }
 return result;
err_text: if (result_text != ITER_DONE) Dee_Decref(result_text);
err_addr: DeeObject_Free(result_addr);
err:      return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_recvfrominto(Socket *__restrict self, size_t argc,
                    DeeObject **__restrict argv) {
 DeeBuffer buffer; DeeObject *data;
 DeeObject *arg1 = NULL,*arg2 = NULL;
 uint64_t timeout; int flags;
 dssize_t result_size;
 DREF DeeSockAddrObject *result_addr;
 DREF DeeObject *result;
 if (DeeArg_Unpack(argc,argv,"o|oo:recvfrominto",&data,&arg1,&arg2))
     goto err;
 if (!arg1) {
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  timeout = (uint64_t)-1;
  flags   = 0;
 } else if (!arg2) {
  //"(buffer dst,string flags=\"\")->int\n"
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  if (DeeString_Check(arg1)) {
   if (sock_getmsgflagsof(arg1,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   if (DeeObject_AsInt64(arg1,(int64_t *)&timeout))
       goto err;
   flags = 0;
  }
 } else {
  //"(buffer dst,int timeout_microseconds=-1,string flags=\"\")->int\n"
  //"(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
  if (DeeObject_AsInt64(arg1,(int64_t *)&timeout))
      goto err;
  if (sock_getmsgflagsof(arg2,&flags))
      goto err;
 }
 /* Create the socket address object that's going to be returned. */
 result_addr = DeeObject_MALLOC(DeeSockAddrObject);
 if unlikely(!result_addr) goto err;
 if (DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FWRITABLE))
     goto err_addr;
 result_size = DeeSocket_RecvFrom(self,
                                  timeout,
                                  buffer.bb_base,
                                  buffer.bb_size,
                                  flags,
                                 &result_addr->sa_addr);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FWRITABLE);
 if unlikely(result_size < 0) goto err_addr;
 /* Create a new tuple to package the 2 objects. */
 result = DeeTuple_NewUninitialized(2);
 if unlikely(!result) goto err_addr;
 if (result_size == 0) {
  /* A somewhat different story: must return (none,"") */
  DeeObject_Free(result_addr);
  DeeTuple_SET(result,0,Dee_None);
  DeeTuple_SET(result,1,(DeeObject *)&DeeInt_Zero);
  Dee_Incref(Dee_None);
  Dee_Incref(&DeeInt_Zero);
 } else {
  DREF DeeObject *result_size_ob;
  result_size_ob = DeeInt_NewSize((size_t)result_size);
  if unlikely(!result_size_ob) {
   DeeTuple_FreeUninitialized(result);
   goto err_addr;
  }
  DeeObject_Init(result_addr,&DeeSockAddr_Type);
  /* Fill the result tuple with the socket address and result-size. */
  DeeTuple_SET(result,0,(DeeObject *)result_addr); /* Inherit */
  DeeTuple_SET(result,1,result_size_ob);           /* Inherit */
 }
 return result;
err_addr:
 DeeObject_Free(result_addr);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
socket_send(Socket *__restrict self, size_t argc,
            DeeObject **__restrict argv) {
 DeeBuffer buffer;
 DeeObject *data,*arg_0 = NULL,*arg_1 = NULL;
 uint64_t timeout; int flags; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|oo:send",&data,&arg_0,&arg_1))
     goto err;
 if (!arg_0) {
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  timeout = (uint64_t)-1;
  flags   = 0;
 } else if (!arg_1) {
  /* "(buffer data,string flags=\"\")->int\n" */
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  if (DeeString_Check(arg_0)) {
   if (sock_getmsgflagsof(arg_0,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   if (DeeObject_AsInt64(arg_0,(int64_t *)&timeout))
       goto err;
   flags = 0;
  }
 } else {
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  /* "(buffer data,int timeout_microseconds=-1,string flags=\"\")->int\n" */
  if (DeeObject_AsInt64(arg_0,(int64_t *)&timeout) ||
      sock_getmsgflagsof(arg_1,&flags))
      goto err;
 }
 if (DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FREADONLY))
     goto err;
 result = DeeSocket_Send(self,
                         timeout,
                         buffer.bb_base,
                         buffer.bb_size,
                         flags);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FREADONLY);
 if unlikely(result < 0) {
  if (result != -2)
      goto err;
  result = 0;
 }
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
socket_sendto(Socket *__restrict self, size_t argc,
              DeeObject **__restrict argv) {
 DeeBuffer buffer; SockAddr target_addr;
 DeeObject *target,*data,*arg_0 = NULL,*arg_1 = NULL;
 uint64_t timeout; int flags; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oo|oo:sendto",&target,&data,&arg_0,&arg_1))
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
                        &target)) goto err;
 }
 DBG_ALIGNMENT_ENABLE();
 if (!arg_0) {
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  timeout = (uint64_t)-1;
  flags   = 0;
 } else if (!arg_1) {
  /* "(buffer data,string flags=\"\")->int\n" */
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  if (DeeString_Check(arg_0)) {
   if (sock_getmsgflagsof(arg_0,&flags))
       goto err;
   timeout = (uint64_t)-1;
  } else {
   if (DeeObject_AsInt64(arg_0,(int64_t *)&timeout))
       goto err;
   flags = 0;
  }
 } else {
  /* "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n" */
  /* "(buffer data,int timeout_microseconds=-1,string flags=\"\")->int\n" */
  if (DeeObject_AsInt64(arg_0,(int64_t *)&timeout) ||
      sock_getmsgflagsof(arg_1,&flags))
      goto err;
 }
 if (DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FREADONLY))
     goto err;
 result = DeeSocket_SendTo(self,
                           timeout,
                           buffer.bb_base,
                           buffer.bb_size,
                           flags,
                          &target_addr);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FREADONLY);
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


PRIVATE DREF DeeObject *DCALL
socket_isbound(Socket *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isbound"))
     return NULL;
 return_bool(self->s_state & SOCKET_FBOUND);
}
PRIVATE DREF DeeObject *DCALL
socket_isconnected(Socket *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":isconnected"))
     return NULL;
 return_bool(self->s_state & SOCKET_FCONNECTED);
}
PRIVATE DREF DeeObject *DCALL
socket_islistening(Socket *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":islistening"))
     return NULL;
 return_bool(self->s_state & SOCKET_FLISTENING);
}
PRIVATE DREF DeeObject *DCALL
socket_wasshutdown(Socket *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *shutdown_mode = (DeeObject *)&shutdown_all;
 int mode; uint16_t state = self->s_state;
 if (DeeArg_Unpack(argc,argv,"|o:wasshutdown",&shutdown_mode))
     goto err;
 if (DeeString_Check(shutdown_mode) &&
     DeeString_IsEmpty(shutdown_mode))
     return_bool(!(state & SOCKET_FOPENED));
 if unlikely(get_shutdown_modeof(shutdown_mode,&mode))
    goto err;
 if (mode == SHUT_RD) {
  mode = state & SOCKET_FSHUTDOWN_R;
 } else if (mode == SHUT_WR) {
  mode = state & SOCKET_FSHUTDOWN_W;
 } else if (mode == SHUT_RDWR) {
  mode = (state & (SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W)) ==
                  (SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W);
 }
 /* If the socket is't open any more, then it was shut down. */
 mode |= !(state & SOCKET_FOPENED);
 return_bool_(mode);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
socket_wasclosed(Socket *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":wasclosed"))
     return NULL;
 return_bool(!(self->s_state & SOCKET_FOPENED));
}

PRIVATE DREF DeeObject *DCALL
socket_fileno(Socket *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":fileno"))
     return NULL;
#ifdef CONFIG_HOST_WINDOWS
 return DeeInt_NewUIntptr((uintptr_t)self->s_socket);
#else
 return DeeInt_NewInt((int)self->s_socket);
#endif
}


PRIVATE struct type_method socket_methods[] = {
    { "close", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_close,
      DOC("(int shutdown_mode)\n"
          "(string shutdown_mode=\"rw\")\n"
          "@interrupt\n"
          "@throw ValueError Invalid shutdown mode\n"
          "@throw NetError Failed to shutdown @this socket\n"
          "@throw HandleClosed @this socket has already been closed\n"
          "Closes the socket's file descriptor. When @shutdown_socket is a non-empty :string, "
          "#shutdown will automatically be invoked on @this socket if it hasn't before\n"
          "Note that in the event that #shutdown has already been called, ") },
    { "shutdown", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_shutdown,
      DOC("(int how)\n"
          "(string how=\"rw\")\n"
          "@interrupt\n"
          "@throw ValueError Invalid shutdown mode\n"
          "@throw NetError Failed to shutdown @this socket\n"
          "@throw HandleClosed @this socket has already been closed\n"
          "Shuts down @this socket either for reading, for writing or for both") },
    { "bind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_bind,
      DOC("(...)\n"
          "@interrupt\n"
          "@throw NetError.AddrInUse The address specified for binding is already in use\n"
          "@throw NetError.NoSupport The protocol of @this socket does not support binding\n"
          "@throw NetError.AddrNotAvail The speficied address is not reachable from this machine\n"
          "@throw NetError.IsConnected @this socket has already been bound is is already connected\n"
          "@throw NetError @this socket has already been bound and its address family does not allow rebinding\n"
          "@throw NetError Failed to bind @this socket for some unknown reason\n"
          "@throw HandleClosed @this socket has already been closed\n"
          "Binds @this socket to a given address.\n"
          "Accepted arguments are the same as ${sockaddr(this.sock_af,...)} when creating :sockaddr") },
    { "connect", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_connect,
      DOC("(...)\n"
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
          "@throw HandleClosed @this socket has already been closed\n"
          "Connect @this socket with a given address.\n"
          "Accepted arguments are the same as ${sockaddr(this.sock_af,...)} when creating :sockaddr") },
    { "listen", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_listen,
      DOC("(int max_backlog=-1)\n"
          "@interrupt\n"
          "@throw NetError.NotBound @this socket has not been bound and the protocol does not allow listening on an unbound address\n"
          "@throw NetError.NoSupport The protocol of @this socket does not allow listening\n"
          "@throw NetError.IsConnected The socket is already connected\n"
          "@throw NetError Failed to start listening for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param max_backlog The max number of connections to queue before #accept must be called to acknowledge them. "
                             "When negative, use a default backlog that can be configured with the environment variable ${\"DEEMON_MAXBACKLOG\"}\n"
          "Start listening for incoming connections on @this socket, preferrable after it has been #bound\n"
          "Note that calling this function may require the user to whitelist deemon in their firewall") },
    { "accept", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_accept,
      DOC("(int timeout_microseconds=-1)->socket\n"
          "(int timeout_microseconds=-1)\n"
          "@interrupt\n"
          "@throw NetError.NotBound.NotListening @this socket is not listening for incoming connections\n"
          "@throw NetError.NoSupport The type of @this socket does not allow accepting of incoming connections\n"
          "@throw NetError Failed to start accept a connection for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param timeout_microseconds The timeout describing for how long #accept should wait before returning :none. "
                                      "You may pass ${-1} for an infinite timeout or $0 to fail immediately.\n"
          "@return A new socket object describing the connection to the new client, or :none when @timeout_microseconds has passed\n"
          "Accept new incoming connections on a listening socket") },
    { "tryaccept", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_tryaccept,
      DOC("->socket\n"
          "->none\n"
          "@interrupt\n"
          "@throw NetError.NotBound.NotListening @this socket is not listening for incoming connections\n"
          "@throw NetError.NoSupport The type of @this socket does not allow accepting of incoming connections\n"
          "@throw NetError Failed to start accept a connection for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "Same as calling #accept with a timeout_microseconds argument of ${0}") },
    { "recv", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_recv,
      DOC("(string flags)->bytes\n"
          "(int max_size=-1,string flags=\"\")->bytes\n"
          "(int max_size=-1,int timeout_microseconds=-1)->bytes\n"
          "(int max_size=-1,int timeout_microseconds=-1,string flags=\"\")->bytes\n"
          "(int max_size=-1,int timeout_microseconds=-1,int flags=0)->bytes\n"
          "@interrupt\n"
          "@throw NetError.NotConnected @this socket is not connected\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out\n"
          "@throw NetError Failed to receive data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
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
    { "recvinto", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_recvinto,
      DOC("(buffer dst,string flags=\"\")->int\n"
          "(buffer dst,int timeout_microseconds=-1)->int\n"
          "(buffer dst,int timeout_microseconds=-1,string flags=\"\")->int\n"
          "(buffer dst,int timeout_microseconds=-1,int flags=0)->int\n"
          "@interrupt\n"
          "@throw NetError.NotConnected @this socket is not connected\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out\n"
          "@throw NetError Failed to receive data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param flags A set of flags used during delivery. See #recv for information on the string-encoded version\n"
          "Same as #recv, but received data is written into the given buffer @dst") },
    { "recvfrom", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_recvfrom,
      DOC("(string flags)->(sockaddr,bytes)\n"
          "(int max_size=-1,int string flags=\"\")->(sockaddr,bytes)\n"
          "(int max_size=-1,int timeout_microseconds=-1)->(sockaddr,bytes)\n"
          "(int max_size=-1,int timeout_microseconds=-1,string flags=\"\")->(sockaddr,bytes)\n"
          "(int max_size=-1,int timeout_microseconds=-1,int flags=0)->(sockaddr,bytes)\n"
          "@interrupt\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
          "@throw NetError Failed to receive data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param flags A set of flags used during delivery. See #recv for information on the string-encoded version\n"
          "Same as #recv, but uses the recvfrom system call to read data, also returning "
          "the socket address from which the data originates as the first of 2 :tuple "
          "arguments, the second being the text regularly returned #recv\n"
          "The given @timeout_microseconds can be passed as either $0 to try-receive pending packages, "
          "as ${-1} (default) to wait for incoming data indefinitely or until the socket is #{close}ed, or "
          "as any other integer value to specify how long to wait before returning ${(none,\"\")}") },
    { "recvfrominto", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_recvfrominto,
      DOC("(buffer dst,string flags=\"\")->(sockaddr,int)\n"
          "(buffer dst,int timeout_microseconds=-1)->(sockaddr,int)\n"
          "(buffer dst,int timeout_microseconds=-1,string flags=\"\")->(sockaddr,int)\n"
          "(buffer dst,int timeout_microseconds=-1,int flags=0)->(sockaddr,int)\n"
          "@interrupt\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
          "@throw NetError Failed to receive data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param flags A set of flags used during delivery. See #recv for information on the string-encoded version\n"
          "Same as #recvfrom, buf read received data into the given buffer @dst") },
    { "send", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_send,
      DOC("(buffer data,string flags=\"\")->int\n"
          "(buffer data,int timeout_microseconds=-1,int flags=0)->int\n"
          "(buffer data,int timeout_microseconds=-1,string flags=\"\")->int\n"
          "@interrupt\n"
          "@throw NetError.NotConnected @this socket is not connected\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
          "@throw NetError.MessageSize The socket is not connection-mode and no peer address is set\n"
          "@throw NetError.NotBound The socket is not connection-mode and no peer address is set\n"
          "@throw NetError.NetUnreachable No route to the connected peer could be established\n"
          "@throw NetError Failed to send data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param flags A set of flags used during delivery. See #recv for information on the string-encoded version\n"
          "@return The total number of bytes that was sent\n"
          "Send @data over the network to the peer of a connected socket") },
    { "sendto", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_sendto,
      DOC("((...) target,buffer data,string flags=\"\")->int\n"
          "((...) target,buffer data,int timeout_microseconds=-1,int flags=0)->int\n"
          "((...) target,buffer data,int timeout_microseconds=-1,string flags=\"\")->int\n"
          "(object target,buffer data,string flags=\"\")->int\n"
          "(object target,buffer data,int timeout_microseconds=-1,int flags=0)->int\n"
          "(object target,buffer data,int timeout_microseconds=-1,string flags=\"\")->int\n"
          "@interrupt\n"
          "@throw NetError.NotConnected @this socket is not connected\n"
          "@throw NetError.NoSupport The specified @flags are not supported by @this socket\n"
          "@throw NetError.ConnectReset The peer has reset the connection\n"
          "@throw NetError.ConnectReset.TimedOut The connection to the peer timed out (Note to be confused with @timeout_microseconds expiring; see below)\n"
          "@throw NetError.NotBound The socket is not connection-mode and no peer address is set\n"
          "@throw NetError.NetUnreachable No route to the connected peer could be established\n"
          "@throw NetError Failed to send data for some reason\n"
          "@throw HandleClosed @this socket has already been closed or was shut down\n"
          "@param flags A set of flags used during delivery. See #recv for information on the string-encoded version\n"
          "@param target A tuple consisting of arguments which can be used to construct a :sockaddr object, or a single argument used for "
                        "the same purpose in ${target = target is tuple ? sockaddr(this.sock_af,target...) : sockaddr(this.sock_af,target)}.\n"
          "@return The total number of bytes that was sent\n"
          "Same as #send, but used to transmit data to a specific network target, rather than one that is already connected.") },
    { "isbound", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_isbound,
      DOC("->bool\n"
          "Returns :true if @this socket has been bound (s.a. #bind)") },
    { "isconnected", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_isconnected,
      DOC("->bool\n"
          "Returns :true if @this socket has been #{connect}ed") },
    { "islistening", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_islistening,
      DOC("->bool\n"
          "Returns :true if @this socket is #{listen}ing for incoming connections") },
    { "wasshutdown", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_wasshutdown,
      DOC("(int how)->bool\n"
          "(string how=\"rw\")->bool\n"
          "Returns :true if @this socket has been #shutdown according to @how (inclusive when multiple modes are specified)\n"
          "See #shutdown for possible values that may be passed to @how") },
    { "wasclosed", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_wasclosed,
      DOC("->bool\n"
          "Returns :true if @this socket has been #{close}ed") },
    { "fileno", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&socket_fileno,
      DOC("->int\n"
          "Returns the underlying file descriptor/handle associated @this socket") },
    { NULL }
};

PRIVATE struct type_getset socket_getsets[] = {
    { "sockname", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_sockname_get, NULL, NULL,
      DOC("->sockaddr\n"
          "@throw HandleClosed @this socket has been closed\n"
          "@throw NetError.NotConnected @this socket is neither connected, nor bound\n"
          "Returns the socket name (local address) of @this socket") },
    { "peeraddr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_peeraddr_get, NULL, NULL,
      DOC("->sockaddr\n"
          "@interrupt\n"
          "@throw HandleClosed @this socket has been closed\n"
          "@throw NetError.NotConnected @this socket is not connected\n"
          "@throw NetError.NoSupport @this socket's protocol does not allow for peer addresses\n"
          "@throw NetError Failed to query the peer address for some unknown reason\n"
          "Returns the peer (remote) address of @this socket") },
    { NULL }
};

PRIVATE struct type_member socket_members[] = {
    TYPE_MEMBER_FIELD_DOC("sock_af",STRUCT_CONST|STRUCT_UINT16_T,
                          offsetof(DeeSocketObject,s_sockaddr.sa.sa_family),
                          "The socket's address family as a system-specific integer id\n"
                          "Usually one of AF_*, the name of which can be determined using :getafname"),
    TYPE_MEMBER_FIELD_DOC("sock_type",STRUCT_CONST|STRUCT_INT,
                          offsetof(DeeSocketObject,s_type),
                          "The socket's type as a system-specific integer id\n"
                          "Usually one of SOCK_*, the name of which can be determined using :gettypename"),
    TYPE_MEMBER_FIELD_DOC("sock_proto",STRUCT_CONST|STRUCT_INT,
                          offsetof(DeeSocketObject,s_type),
                          "The socket's protocol as a system-specific integer id\n"
                          "Usually one of *PROTO_*, the name of which can be determined using :getprotoname"),
    TYPE_MEMBER_END
};


PRIVATE DREF DeeObject *DCALL
socket_str(DeeSocketObject *__restrict self) {
 bool has_sock,has_peer; uint16_t state; SockAddr sock,peer;
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 state = ATOMIC_READ(self->s_state);
 if (ascii_printer_printf(&printer,"<socket %K, %K, %K: ",
                           sock_getafnameorid(self->s_sockaddr.sa.sa_family),
                           sock_gettypenameorid(self->s_type),
                           sock_getprotonameorid(self->s_proto)) < 0)
     goto err;
 if (!(state&SOCKET_FOPENED)) {
  if (ASCII_PRINTER_PRINT(&printer," Closed") < 0) goto err;
 } else {
  has_sock = !DeeSocket_GetSockName(self,&sock,false);
  has_peer = !DeeSocket_GetPeerAddr(self,&peer,false);
  if (has_sock && has_peer) {
   if (ascii_printer_printf(&printer,"%K -> %K",
                             SockAddr_ToString(&sock,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS),
                             SockAddr_ToString(&peer,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS)) < 0)
       goto err;
  } else if (has_sock) {
   if (ascii_printer_printf(&printer,"%K",
                             SockAddr_ToString(&sock,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS)) < 0)
       goto err;
  } else if (has_peer) {
   if (ascii_printer_printf(&printer,"local -> %K",
                             SockAddr_ToString(&peer,self->s_proto,SOCKADDR_STR_FNOFAIL|SOCKADDR_STR_FNODNS)) < 0)
       goto err;
  }
  if (state&SOCKET_FBOUND && ASCII_PRINTER_PRINT(&printer," Bound") < 0) goto err;
  if (state&SOCKET_FCONNECTED && ASCII_PRINTER_PRINT(&printer," Connected") < 0) goto err;
  if (state&SOCKET_FLISTENING && ASCII_PRINTER_PRINT(&printer," Listening") < 0) goto err;
  if (state&(SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W) &&
      ascii_printer_printf(&printer," Shutdown(%s%s)",
                            state&SOCKET_FSHUTDOWN_R ? "r" : "",
                            state&SOCKET_FSHUTDOWN_W ? "w" : "") < 0) goto err;
 }
 if (ASCII_PRINTER_PRINT(&printer,">") < 0) goto err;
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}

#if 0
PRIVATE DREF DeeObject *DCALL
socket_repr(DeeSocketObject *__restrict self) {
 bool has_sock,has_peer; uint16_t state; SockAddr sock,peer;
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 state = ATOMIC_READ(self->s_state);
 if (ascii_printer_printf(&printer,"<socket(%R,%R,%R): ",
                            sock_getafnameorid(self->s_sockaddr.sa.sa_family),
                            sock_gettypenameorid(self->s_type),
                            sock_getprotonameorid(self->s_proto)) < 0)
     goto err;
 if (!(state&SOCKET_FOPENED)) {
  if (ASCII_PRINTER_PRINT(&printer," Closed") < 0) goto err;
 } else {
  has_sock = !DeeSocket_GetSockName(self,&sock,false);
  has_peer = !DeeSocket_GetPeerAddr(self,&peer,false);
  if (has_sock && has_peer) {
   if (ascii_printer_printf(&printer,"%K -> %K",
                              SockAddr_ToString(&sock,self->s_proto,SOCKADDR_STR_FNOFAIL),
                              SockAddr_ToString(&peer,self->s_proto,SOCKADDR_STR_FNOFAIL)) < 0)
       goto err;
  } else if (has_sock) {
   if (ascii_printer_printf(&printer,"%K",
                              SockAddr_ToString(&sock,self->s_proto,SOCKADDR_STR_FNOFAIL)) < 0)
       goto err;
  } else if (has_peer) {
   if (ascii_printer_printf(&printer,"local -> %K",
                              SockAddr_ToString(&peer,self->s_proto,SOCKADDR_STR_FNOFAIL)) < 0)
       goto err;
  }
  if (state&SOCKET_FBOUND && ASCII_PRINTER_PRINT(&printer," Bound") < 0) goto err;
  if (state&SOCKET_FCONNECTED && ASCII_PRINTER_PRINT(&printer," Connected") < 0) goto err;
  if (state&SOCKET_FLISTENING && ASCII_PRINTER_PRINT(&printer," Listening") < 0) goto err;
  if (state&(SOCKET_FSHUTDOWN_R|SOCKET_FSHUTDOWN_W) &&
      ascii_printer_printf(&printer," Shutdown(%s%s)",
                             state&SOCKET_FSHUTDOWN_R ? "r" : "",
                             state&SOCKET_FSHUTDOWN_W ? "w" : "") < 0) goto err;
 }
 if (ASCII_PRINTER_PRINT(&printer,">") < 0) goto err;
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}
#endif

INTERN DeeTypeObject DeeSocket_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"socket",
    /* .tp_doc      = */DOC("(int af, int type = ..., int proto = ...)\n"
                            "(string af, string type = ..., string proto = ...)\n"
                            "@param proto The protocol to use for the socket, or :none or $0 to use the default\n"
                            "@param type The socket type, or none to use ${\"SOCK_STREAM\"}\n"
                            "@param af The socket address family (e.g.: ${\"AF_INET\"} or ${\"AF_INET6\"}).\n"
                            "          NOTE: When possible, deemon will automatically configure ${\"AF_INET6\"} sockets to be "
                                            "able to accept clients in dualstack mode (that is: allowing connections made using both IPv4 and IPv6)\n"
                            "                To simplify this further, you may use #tcp to easily create an IPv6-ready server or client\n"
                            "@throw NetError.NoSupport The given protocol @proto cannot be used with the given address family @af\n"
                            "@throw NetError.NoSupport The given socket type @type cannot be used with protocol @proto\n"
                            "@throw NetError Failed to create a new socket descriptor\n"
                            "@throw ValueError ${\"AF_AUTO\"} cannot be used as address family in the socket constructor\n"
                            "Constructs and allocates a new socket descriptor that has yet to be #bound or be #{connect}ed\n"
                            "\n"
                            "bool()\n"
                            "Returns :true indicative of the socket not having been closed (s.a. #wasclosed)"),
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeSocketObject)
                },
                /* .tp_any_ctor_kw = */(void *)&socket_ctor,
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&socket_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_str,
        /* .tp_repr = */NULL, /* (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&socket_repr, */
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&socket_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */socket_methods,
    /* .tp_getsets       = */socket_getsets,
    /* .tp_members       = */socket_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

DECL_END

#endif /* !GUARD_DEX_SOCKET_SOCKET_C */
