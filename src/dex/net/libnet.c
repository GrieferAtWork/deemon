/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_LIBNET_C
#define GUARD_DEX_SOCKET_LIBNET_C 1
#define DEE_SOURCE

#include "libnet.h"
/**/

#include <deemon/api.h>

#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

#include <hybrid/byteswap.h>
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include <stddef.h> /* NULL */
#include <stdint.h> /* uint16_t, uint32_t, uint64_t */

DECL_BEGIN


/*[[[deemon (print_CMethod from rt.gen.unpack)("getafname", "int af", isconst: true);]]]*/
#define libnet_getafname_params "af:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getafname_f_impl(int af);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getafname_f(DeeObject *__restrict arg0) {
	int af;
	if (DeeObject_AsInt(arg0, &af))
		goto err;
	return libnet_getafname_f_impl(af);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_getafname, &libnet_getafname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getafname_f_impl(int af)
/*[[[end]]]*/
{
	return sock_getafnameorid(af);
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("getafof", "DeeObject *af:?X3?Dstring?Dint?N", isconst: true);]]]*/
#define libnet_getafof_params "af:?X3?Dstring?Dint?N"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getafof_f_impl(DeeObject *af);
PRIVATE DEFINE_CMETHOD1(libnet_getafof, &libnet_getafof_f_impl, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getafof_f_impl(DeeObject *af)
/*[[[end]]]*/
{
	int result;
	if unlikely(sock_getafof(af, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("gettypename", "int typ", isconst: true);]]]*/
#define libnet_gettypename_params "typ:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_gettypename_f_impl(int typ);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_gettypename_f(DeeObject *__restrict arg0) {
	int typ;
	if (DeeObject_AsInt(arg0, &typ))
		goto err;
	return libnet_gettypename_f_impl(typ);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_gettypename, &libnet_gettypename_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_gettypename_f_impl(int typ)
/*[[[end]]]*/
{
	return sock_gettypenameorid(typ);
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("gettypeof", "DeeObject *typ:?X3?Dstring?Dint?N", isconst: true);]]]*/
#define libnet_gettypeof_params "typ:?X3?Dstring?Dint?N"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_gettypeof_f_impl(DeeObject *typ);
PRIVATE DEFINE_CMETHOD1(libnet_gettypeof, &libnet_gettypeof_f_impl, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_gettypeof_f_impl(DeeObject *typ)
/*[[[end]]]*/
{
	int result;
	if unlikely(sock_gettypeof(typ, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("getprotoname", "int proto", isconst: true);]]]*/
#define libnet_getprotoname_params "proto:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getprotoname_f_impl(int proto);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getprotoname_f(DeeObject *__restrict arg0) {
	int proto;
	if (DeeObject_AsInt(arg0, &proto))
		goto err;
	return libnet_getprotoname_f_impl(proto);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_getprotoname, &libnet_getprotoname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getprotoname_f_impl(int proto)
/*[[[end]]]*/
{
	return sock_getprotonameorid(proto);
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("getprotoof", "DeeObject *proto:?X2?Dstring?Dint", isconst: true);]]]*/
#define libnet_getprotoof_params "proto:?X2?Dstring?Dint"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getprotoof_f_impl(DeeObject *proto);
PRIVATE DEFINE_CMETHOD1(libnet_getprotoof, &libnet_getprotoof_f_impl, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getprotoof_f_impl(DeeObject *proto)
/*[[[end]]]*/
{
	int result;
	if unlikely(sock_getprotoof(proto, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("getmsgflagsname", "int msgflags", isconst: true);]]]*/
#define libnet_getmsgflagsname_params "msgflags:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getmsgflagsname_f_impl(int msgflags);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getmsgflagsname_f(DeeObject *__restrict arg0) {
	int msgflags;
	if (DeeObject_AsInt(arg0, &msgflags))
		goto err;
	return libnet_getmsgflagsname_f_impl(msgflags);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_getmsgflagsname, &libnet_getmsgflagsname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_getmsgflagsname_f_impl(int msgflags)
/*[[[end]]]*/
{
	return sock_getmsgflagsnameorid(msgflags);
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("getmsgflagsof", "DeeObject *msgflags:?X2?Dstring?Dint", isconst: true);]]]*/
#define libnet_getmsgflagsof_params "msgflags:?X2?Dstring?Dint"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getmsgflagsof_f_impl(DeeObject *msgflags);
PRIVATE DEFINE_CMETHOD1(libnet_getmsgflagsof, &libnet_getmsgflagsof_f_impl, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_getmsgflagsof_f_impl(DeeObject *msgflags)
/*[[[end]]]*/
{
	int result;
	if unlikely(sock_getmsgflagsof(msgflags, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}






/*[[[deemon (print_CMethod from rt.gen.unpack)("ntoh16", "uint16_t x", isconst: true);]]]*/
#define libnet_ntoh16_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh16_f_impl(uint16_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_ntoh16_f(DeeObject *__restrict arg0) {
	uint16_t x;
	if (DeeObject_AsUInt16(arg0, &x))
		goto err;
	return libnet_ntoh16_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_ntoh16, &libnet_ntoh16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh16_f_impl(uint16_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt16((uint16_t)BETOH16(x));
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("ntoh32", "uint32_t x", isconst: true);]]]*/
#define libnet_ntoh32_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh32_f_impl(uint32_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_ntoh32_f(DeeObject *__restrict arg0) {
	uint32_t x;
	if (DeeObject_AsUInt32(arg0, &x))
		goto err;
	return libnet_ntoh32_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_ntoh32, &libnet_ntoh32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh32_f_impl(uint32_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt32((uint32_t)BETOH32(x));
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("ntoh64", "uint64_t x", isconst: true);]]]*/
#define libnet_ntoh64_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh64_f_impl(uint64_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_ntoh64_f(DeeObject *__restrict arg0) {
	uint64_t x;
	if (DeeObject_AsUInt64(arg0, &x))
		goto err;
	return libnet_ntoh64_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_ntoh64, &libnet_ntoh64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_ntoh64_f_impl(uint64_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt64((uint64_t)BETOH64(x));
}



/*[[[deemon (print_CMethod from rt.gen.unpack)("hton16", "uint16_t x", isconst: true);]]]*/
#define libnet_hton16_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton16_f_impl(uint16_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_hton16_f(DeeObject *__restrict arg0) {
	uint16_t x;
	if (DeeObject_AsUInt16(arg0, &x))
		goto err;
	return libnet_hton16_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_hton16, &libnet_hton16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton16_f_impl(uint16_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt16((uint16_t)HTOBE16(x));
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("hton32", "uint32_t x", isconst: true);]]]*/
#define libnet_hton32_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton32_f_impl(uint32_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_hton32_f(DeeObject *__restrict arg0) {
	uint32_t x;
	if (DeeObject_AsUInt32(arg0, &x))
		goto err;
	return libnet_hton32_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_hton32, &libnet_hton32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton32_f_impl(uint32_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt32((uint32_t)HTOBE32(x));
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("hton64", "uint64_t x", isconst: true);]]]*/
#define libnet_hton64_params "x:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton64_f_impl(uint64_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libnet_hton64_f(DeeObject *__restrict arg0) {
	uint64_t x;
	if (DeeObject_AsUInt64(arg0, &x))
		goto err;
	return libnet_hton64_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libnet_hton64, &libnet_hton64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libnet_hton64_f_impl(uint64_t x)
/*[[[end]]]*/
{
	return DeeInt_NewUInt64((uint64_t)HTOBE64(x));
}


#ifdef CONFIG_HOST_WINDOWS
#define PTR_libnet_init &libnet_init
PRIVATE WUNUSED int DCALL libnet_init(void) {
	/* Start up the the windows networking subsystem. */
	neterrno_t error;
	WSADATA wsaData;
	/* Use 1.1, for maximum backwards compatibility. */
	DBG_ALIGNMENT_DISABLE();
	error = (neterrno_t)WSAStartup(MAKEWORD(1, 1), &wsaData);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(error != 0)
		return DeeError_Throwf(&DeeError_NetError, "WSAStartup() : %lu", error);
	return 0;
}
#elif defined(CONFIG_HAVE_signal) && defined(SIGPIPE)
#define PTR_libnet_init &libnet_init
PRIVATE WUNUSED int DCALL libnet_init(void) {
	/* SIGPIPE is generated when a remote socket is closed. */
	void (*handler)(int);
	DBG_ALIGNMENT_DISABLE();
	/* Try to ignore this signal. */
	handler = signal(SIGPIPE, SIG_IGN);
	/* Make sure we don't override a custom handler */
	if (handler != SIG_DFL)
		signal(SIGPIPE, handler);
	DBG_ALIGNMENT_ENABLE();
	return 0;
}
#endif /* ... */

#ifdef CONFIG_HOST_WINDOWS
#define PTR_libnet_fini &libnet_fini
PRIVATE void DCALL libnet_fini(void) {
	DBG_ALIGNMENT_DISABLE();
	WSACleanup();
	DBG_ALIGNMENT_ENABLE();
}
#endif /* CONFIG_HOST_WINDOWS */






DEX_BEGIN

DEX_MEMBER_F_NODOC("socket", &DeeSocket_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("sockaddr", &DeeSockAddr_Type, MODSYM_FREADONLY),
DEX_MEMBER_F_NODOC("NetError", &DeeError_NetError, MODSYM_FREADONLY),
DEX_MEMBER_F("getafname", &libnet_getafname, MODSYM_FREADONLY,
             "(" libnet_getafname_params ")->?X2?Dstring?Dint\n"
             "Return the name of a given address family, given its system-dependent ID\n"
             "When not known, re-return the given @id"),
DEX_MEMBER_F("getafof", &libnet_getafof, MODSYM_FREADONLY,
             "(" libnet_getafof_params ")->?Dint\n"
             "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
             "Return the system-dependent ID of a given address family @name, or re-return the given @id.\n"
             "The given @name is encoded the same way as the first constructor argument for ?Gsocket"),
DEX_MEMBER_F("gettypename", &libnet_gettypename, MODSYM_FREADONLY,
             "(" libnet_gettypename_params ")->?X2?Dstring?Dint\n"
             "Return the name of a given socket type @type, given its system-dependent ID\n"
             "When not known, re-return the given @type"),
DEX_MEMBER_F("gettypeof", &libnet_gettypeof, MODSYM_FREADONLY,
             "(" libnet_gettypeof_params ")->?Dint\n"
             "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
             "Return the system-dependent ID of a given socket type @name, or re-return the given @type.\n"
             "The given @name is encoded the same way as the second constructor argument for ?Gsocket"),
DEX_MEMBER_F("getprotoname", &libnet_getprotoname, MODSYM_FREADONLY,
             "(" libnet_getprotoname_params ")->?X2?Dstring?Dint\n"
             "Return the name of a given protocol @proto, given its system-dependent ID\n"
             "When not known, re-return the given @proto"),
DEX_MEMBER_F("getprotoof", &libnet_getprotoof, MODSYM_FREADONLY,
             "(" libnet_getprotoof_params ")->?Dint\n"
             "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
             "Return the system-dependent ID of a given protocol name @name, or re-return the given @proto.\n"
             "The given @name is encoded the same way as the third constructor argument for ?Gsocket"),
DEX_MEMBER_F("getmsgflagsname", &libnet_getmsgflagsname, MODSYM_FREADONLY,
             "(" libnet_getmsgflagsname_params ")->?X2?Dstring?Dint\n"
             "Return the name of a given message flags @msgflags, given a system-dependent set of flags\n"
             "When not known, re-return the given @msgflags"),
DEX_MEMBER_F("getmsgflagsof", &libnet_getmsgflagsof, MODSYM_FREADONLY,
             "(" libnet_getmsgflagsof_params ")->?Dint\n"
             "#t{?ANoSupport?GNetError}{The given @flags contains at least one flag that is not recognized by this library}"
             "Return a system-dependent set of flags for given message flags @flags, or re-return the given @flags.\n"
             "The given @flags is encoded the same way as the flags argument passed to ?Arecv?Gsocket"),
DEX_MEMBER_F("ntoh16", &libnet_ntoh16, MODSYM_FREADONLY,
             "(" libnet_ntoh16_params ")->?Dint\n"
             "Convert a 16-bit integer @x from network-endian to host-endian"),
DEX_MEMBER_F("ntoh32", &libnet_ntoh32, MODSYM_FREADONLY,
             "(" libnet_ntoh32_params ")->?Dint\n"
             "Convert a 32-bit integer @x from network-endian to host-endian"),
DEX_MEMBER_F("ntoh64", &libnet_ntoh64, MODSYM_FREADONLY,
             "(" libnet_ntoh64_params ")->?Dint\n"
             "Convert a 64-bit integer @x from network-endian to host-endian"),
DEX_MEMBER_F("hton16", &libnet_hton16, MODSYM_FREADONLY,
             "(" libnet_hton16_params ")->?Dint\n"
             "Convert a 16-bit integer @x from host-endian to network-endian"),
DEX_MEMBER_F("hton32", &libnet_hton32, MODSYM_FREADONLY,
             "(" libnet_hton32_params ")->?Dint\n"
             "Convert a 32-bit integer @x from host-endian to network-endian"),
DEX_MEMBER_F("hton64", &libnet_hton64, MODSYM_FREADONLY,
             "(" libnet_hton64_params ")->?Dint\n"
             "Convert a 64-bit integer @x from host-endian to network-endian"),

#ifndef PTR_libnet_init
#define PTR_libnet_init NULL
#endif /* !PTR_libnet_init */
#ifndef PTR_libnet_fini
#define PTR_libnet_fini NULL
#endif /* !PTR_libnet_fini */

/* clang-format off */
DEX_END(
	/* init:  */ PTR_libnet_init,
	/* fini:  */ PTR_libnet_fini,
	/* clear: */ NULL
);
/* clang-format on */

DECL_END


#endif /* !GUARD_DEX_SOCKET_LIBNET_C */
