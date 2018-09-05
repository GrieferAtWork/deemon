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
#ifndef GUARD_DEX_SOCKET_LIBNET_C
#define GUARD_DEX_SOCKET_LIBNET_C 1

#include "libnet.h"

#include <deemon/api.h>
#include <deemon/file.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/objmethod.h>
#include <hybrid/byteswap.h>

DECL_BEGIN


PRIVATE DREF DeeObject *DCALL
lib_getafname_f(size_t argc, DeeObject **__restrict argv) {
 int af_id;
 if (DeeArg_Unpack(argc,argv,"d:getafname",&af_id))
     return NULL;
 return sock_getafnameorid(af_id);
}
PRIVATE DREF DeeObject *DCALL
lib_getafof_f(size_t argc, DeeObject **__restrict argv) {
 DeeObject *afob; int afid;
 if (DeeArg_Unpack(argc,argv,"o:getafof",&afob) ||
     sock_getafof(afob,&afid))
     return NULL;
 return DeeInt_NewInt(afid);
}

PRIVATE DREF DeeObject *DCALL
lib_gettypename_f(size_t argc, DeeObject **__restrict argv) {
 int af_id;
 if (DeeArg_Unpack(argc,argv,"d:gettypename",&af_id))
     return NULL;
 return sock_gettypenameorid(af_id);
}
PRIVATE DREF DeeObject *DCALL
lib_gettypeof_f(size_t argc, DeeObject **__restrict argv) {
 DeeObject *afob; int afid;
 if (DeeArg_Unpack(argc,argv,"o:gettypeof",&afob) ||
     sock_gettypeof(afob,&afid))
     return NULL;
 return DeeInt_NewInt(afid);
}

PRIVATE DREF DeeObject *DCALL
lib_getprotoname_f(size_t argc, DeeObject **__restrict argv) {
 int af_id;
 if (DeeArg_Unpack(argc,argv,"d:getprotoname",&af_id))
     return NULL;
 return sock_getprotonameorid(af_id);
}
PRIVATE DREF DeeObject *DCALL
lib_getprotoof_f(size_t argc, DeeObject **__restrict argv) {
 DeeObject *afob; int afid;
 if (DeeArg_Unpack(argc,argv,"o:getprotoof",&afob) ||
     sock_getprotoof(afob,&afid))
     return NULL;
 return DeeInt_NewInt(afid);
}

PRIVATE DREF DeeObject *DCALL
lib_getmsgflagsname_f(size_t argc, DeeObject **__restrict argv) {
 int af_id;
 if (DeeArg_Unpack(argc,argv,"d:getmsgflagsname",&af_id))
     return NULL;
 return sock_getmsgflagsnameorid(af_id);
}
PRIVATE DREF DeeObject *DCALL
lib_getmsgflagsof_f(size_t argc, DeeObject **__restrict argv) {
 DeeObject *afob; int afid;
 if (DeeArg_Unpack(argc,argv,"o:getmsgflags",&afob) ||
     sock_getmsgflagsof(afob,&afid))
     return NULL;
 return DeeInt_NewInt(afid);
}

PRIVATE DEFINE_CMETHOD(lib_getafname,&lib_getafname_f);
PRIVATE DEFINE_CMETHOD(lib_getafof,&lib_getafof_f);
PRIVATE DEFINE_CMETHOD(lib_gettypename,&lib_gettypename_f);
PRIVATE DEFINE_CMETHOD(lib_gettypeof,&lib_gettypeof_f);
PRIVATE DEFINE_CMETHOD(lib_getprotoname,&lib_getprotoname_f);
PRIVATE DEFINE_CMETHOD(lib_getprotoof,&lib_getprotoof_f);
PRIVATE DEFINE_CMETHOD(lib_getmsgflagsname,&lib_getmsgflagsname_f);
PRIVATE DEFINE_CMETHOD(lib_getmsgflagsof,&lib_getmsgflagsof_f);


PRIVATE DREF DeeObject *DCALL
lib_ntoh16_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t i;
 if (DeeArg_Unpack(argc,argv,"I16u:ntoh16",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU16(BSWAP16(i));
#else
 return DeeInt_NewU16(i);
#endif
}
PRIVATE DREF DeeObject *DCALL
lib_ntoh32_f(size_t argc, DeeObject **__restrict argv) {
 uint32_t i;
 if (DeeArg_Unpack(argc,argv,"I32u:ntoh32",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU32(BSWAP32(i));
#else
 return DeeInt_NewU32(i);
#endif
}
PRIVATE DREF DeeObject *DCALL
lib_ntoh64_f(size_t argc, DeeObject **__restrict argv) {
 uint64_t i;
 if (DeeArg_Unpack(argc,argv,"I64u:ntoh64",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU64(BSWAP64(i));
#else
 return DeeInt_NewU64(i);
#endif
}

PRIVATE DREF DeeObject *DCALL
lib_hton16_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t i;
 if (DeeArg_Unpack(argc,argv,"I16u:hton16",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU16(BSWAP16(i));
#else
 return DeeInt_NewU16(i);
#endif
}
PRIVATE DREF DeeObject *DCALL
lib_hton32_f(size_t argc, DeeObject **__restrict argv) {
 uint32_t i;
 if (DeeArg_Unpack(argc,argv,"I32u:hton32",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU32(BSWAP32(i));
#else
 return DeeInt_NewU32(i);
#endif
}
PRIVATE DREF DeeObject *DCALL
lib_hton64_f(size_t argc, DeeObject **__restrict argv) {
 uint64_t i;
 if (DeeArg_Unpack(argc,argv,"I64u:hton64",&i))
     return NULL;
#ifdef CONFIG_LITTLE_ENDIAN
 return DeeInt_NewU64(BSWAP64(i));
#else
 return DeeInt_NewU64(i);
#endif
}

PRIVATE DEFINE_CMETHOD(lib_ntoh16,lib_ntoh16_f);
PRIVATE DEFINE_CMETHOD(lib_ntoh32,lib_ntoh32_f);
PRIVATE DEFINE_CMETHOD(lib_ntoh64,lib_ntoh64_f);
PRIVATE DEFINE_CMETHOD(lib_hton16,lib_hton16_f);
PRIVATE DEFINE_CMETHOD(lib_hton32,lib_hton32_f);
PRIVATE DEFINE_CMETHOD(lib_hton64,lib_hton64_f);


PRIVATE struct dex_symbol symbols[] = {
    { "socket", (DeeObject *)&DeeSocket_Type, MODSYM_FREADONLY },
    { "sockaddr", (DeeObject *)&DeeSockAddr_Type, MODSYM_FREADONLY },
    { "NetError", (DeeObject *)&DeeError_NetError, MODSYM_FREADONLY },
    { "getafname", (DeeObject *)&lib_getafname, MODSYM_FREADONLY,
      DOC("(int id)->string\n"
          "(int id)->int\n"
          "Return the name of a given address family, given its system-dependent ID\n"
          "When not known, re-return the given @id") },
    { "getafof", (DeeObject *)&lib_getafof, MODSYM_FREADONLY,
      DOC("(int id)->int\n"
          "(string name)->int\n"
          "@throw NetError.NoSupport The given @name is not recognized by this library\n"
          "Return the system-dependent ID of a given address family @name, or re-return the given @id.\n"
          "The given @name is encoded the same way as the first constructor argument for :socket") },
    { "gettypename", (DeeObject *)&lib_gettypename, MODSYM_FREADONLY,
      DOC("(int type)->string\n"
          "(int type)->int\n"
          "Return the name of a given socket type @type, given its system-dependent ID\n"
          "When not known, re-return the given @type") },
    { "gettypeof", (DeeObject *)&lib_gettypeof, MODSYM_FREADONLY,
      DOC("(int type)->int\n"
          "(string name)->int\n"
          "@throw NetError.NoSupport The given @name is not recognized by this library\n"
          "Return the system-dependent ID of a given socket type @name, or re-return the given @type.\n"
          "The given @name is encoded the same way as the second constructor argument for :socket") },
    { "getprotoname", (DeeObject *)&lib_getprotoname, MODSYM_FREADONLY,
      DOC("(int proto)->string\n"
          "(int proto)->int\n"
          "Return the name of a given protocol @proto, given its system-dependent ID\n"
          "When not known, re-return the given @proto") },
    { "getprotoof", (DeeObject *)&lib_getprotoof, MODSYM_FREADONLY,
      DOC("(int proto)->int\n"
          "(string name)->int\n"
          "@throw NetError.NoSupport The given @name is not recognized by this library\n"
          "Return the system-dependent ID of a given protocol name @name, or re-return the given @proto.\n"
          "The given @name is encoded the same way as the third constructor argument for :socket") },
    { "getmsgflagsname", (DeeObject *)&lib_getmsgflagsname, MODSYM_FREADONLY,
      DOC("(int msgflags)->string\n"
          "(int msgflags)->int\n"
          "Return the name of a given message flags @msgflags, given a system-dependent set of flags\n"
          "When not known, re-return the given @msgflags") },
    { "getmsgflagsof", (DeeObject *)&lib_getmsgflagsof, MODSYM_FREADONLY,
      DOC("(int msgflags)->int\n"
          "(string flags)->int\n"
          "@throw NetError.NoSupport The given @flags contains at least one flag that is not recognized by this library\n"
          "Return a system-dependent set of flags for given message flags @flags, or re-return the given @flags.\n"
          "The given @flags is encoded the same way as the flags argument passed to :socket.recv") },
    { "ntoh16", (DeeObject *)&lib_ntoh16, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 16-bit integer @x from network-endian to host-endian") },
    { "ntoh32", (DeeObject *)&lib_ntoh32, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 32-bit integer @x from network-endian to host-endian") },
    { "ntoh64", (DeeObject *)&lib_ntoh64, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 64-bit integer @x from network-endian to host-endian") },
    { "hton16", (DeeObject *)&lib_hton16, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 16-bit integer @x from host-endian to network-endian") },
    { "hton32", (DeeObject *)&lib_hton32, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 32-bit integer @x from host-endian to network-endian") },
    { "hton64", (DeeObject *)&lib_hton64, MODSYM_FREADONLY,
      DOC("(int x)->int\n"
          "Convert a 64-bit integer @x from host-endian to network-endian") },
    { NULL }
};


PRIVATE int DCALL
libnet_init(DeeDexObject *__restrict UNUSED(self)) {
#ifdef CONFIG_HOST_WINDOWS
 /* Start up the the windows networking subsystem. */
 neterrno_t error; WSADATA wsaData;
 /* Use 1.1, for maximum backwards compatibility. */
 DBG_ALIGNMENT_DISABLE();
 error = (neterrno_t)WSAStartup(MAKEWORD(1,1),&wsaData);
 DBG_ALIGNMENT_ENABLE();
 if unlikely(error != 0) {
  DeeError_Throwf(&DeeError_NetError,"WSAStartup() : %lu",error);
  return -1;
 }
#else
 /* SIGPIPE is generated when a remote socket is closed. */
 void (*handler)(int);
 DBG_ALIGNMENT_DISABLE();
 /* Try to ignore this signal. */
 handler = signal(SIGPIPE,SIG_IGN);
 /* Make sure we don't override a custom handler */
 if (handler != SIG_DFL) signal(SIGPIPE,handler);
 DBG_ALIGNMENT_ENABLE();
#endif
 return 0;
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE void DCALL
libnet_fini(DeeDexObject *__restrict UNUSED(self)) {
 DBG_ALIGNMENT_DISABLE();
 WSACleanup();
 DBG_ALIGNMENT_ENABLE();
}
#endif


PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols,
    /* .d_init    = */&libnet_init
#ifdef CONFIG_HOST_WINDOWS
    ,
    /* .d_fini    = */&libnet_fini
#endif
};

DECL_END


#endif /* !GUARD_DEX_SOCKET_LIBNET_C */
