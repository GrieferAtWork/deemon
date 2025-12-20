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
#ifndef GUARD_DEX_SOCKET_LIBNET_C
#define GUARD_DEX_SOCKET_LIBNET_C 1
#define DEE_SOURCE

#include "libnet.h"
/**/

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

#include <hybrid/byteswap.h>
#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN


PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getafname_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getafname", params: "int af");]]]*/
	struct {
		int af;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "getafname", &args.af, "d", DeeObject_AsInt);
/*[[[end]]]*/
	return sock_getafnameorid(args.af);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getafof_f(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getafof", params: "DeeObject *af");]]]*/
	struct {
		DeeObject *af;
	} args;
	DeeArg_Unpack1(err, argc, argv, "getafof", &args.af);
/*[[[end]]]*/
	if unlikely(sock_getafof(args.af, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_gettypename_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("gettypename", params: "int typ");]]]*/
	struct {
		int typ;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "gettypename", &args.typ, "d", DeeObject_AsInt);
/*[[[end]]]*/
	return sock_gettypenameorid(args.typ);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_gettypeof_f(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("gettypeof", params: "DeeObject *typ");]]]*/
	struct {
		DeeObject *typ;
	} args;
	DeeArg_Unpack1(err, argc, argv, "gettypeof", &args.typ);
/*[[[end]]]*/
	if unlikely(sock_gettypeof(args.typ, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getprotoname_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getprotoname", params: "int proto");]]]*/
	struct {
		int proto;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "getprotoname", &args.proto, "d", DeeObject_AsInt);
/*[[[end]]]*/
	return sock_getprotonameorid(args.proto);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getprotoof_f(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("gettypeof", params: "DeeObject *proto");]]]*/
	struct {
		DeeObject *proto;
	} args;
	DeeArg_Unpack1(err, argc, argv, "gettypeof", &args.proto);
/*[[[end]]]*/
	if (sock_getprotoof(args.proto, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getmsgflagsname_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getmsgflagsname", params: "int msgflags");]]]*/
	struct {
		int msgflags;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "getmsgflagsname", &args.msgflags, "d", DeeObject_AsInt);
/*[[[end]]]*/
	return sock_getmsgflagsnameorid(args.msgflags);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_getmsgflagsof_f(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getmsgflags", params: "DeeObject *msgflags");]]]*/
	struct {
		DeeObject *msgflags;
	} args;
	DeeArg_Unpack1(err, argc, argv, "getmsgflags", &args.msgflags);
/*[[[end]]]*/
	if (sock_getmsgflagsof(args.msgflags, &result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(lib_getafname, &lib_getafname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_getafof, &lib_getafof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_gettypename, &lib_gettypename_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_gettypeof, &lib_gettypeof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_getprotoname, &lib_getprotoname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_getprotoof, &lib_getprotoof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_getmsgflagsname, &lib_getmsgflagsname_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_getmsgflagsof, &lib_getmsgflagsof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);


PRIVATE WUNUSED DREF DeeObject *DCALL
lib_ntoh16_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ntoh16", params: "uint16_t x");]]]*/
	struct {
		uint16_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "ntoh16", &args.x, UNPu16, DeeObject_AsUInt16);
/*[[[end]]]*/
	return DeeInt_NewUInt16((uint16_t)BETOH16(args.x));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_ntoh32_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ntoh32", params: "uint32_t x");]]]*/
	struct {
		uint32_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "ntoh32", &args.x, UNPu32, DeeObject_AsUInt32);
/*[[[end]]]*/
	return DeeInt_NewUInt32((uint32_t)BETOH32(args.x));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_ntoh64_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ntoh64", params: "uint64_t x");]]]*/
	struct {
		uint64_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "ntoh64", &args.x, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	return DeeInt_NewUInt64((uint64_t)BETOH64(args.x));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_hton16_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("hton16", params: "uint16_t x");]]]*/
	struct {
		uint16_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "hton16", &args.x, UNPu16, DeeObject_AsUInt16);
/*[[[end]]]*/
	return DeeInt_NewUInt16((uint16_t)HTOBE16(args.x));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_hton32_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("hton32", params: "uint32_t x");]]]*/
	struct {
		uint32_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "hton32", &args.x, UNPu32, DeeObject_AsUInt32);
/*[[[end]]]*/
	return DeeInt_NewUInt32((uint32_t)HTOBE32(args.x));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
lib_hton64_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("hton64", params: "uint64_t x");]]]*/
	struct {
		uint64_t x;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "hton64", &args.x, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	return DeeInt_NewUInt64((uint64_t)HTOBE64(args.x));
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(lib_ntoh16, lib_ntoh16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_ntoh32, lib_ntoh32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_ntoh64, lib_ntoh64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_hton16, lib_hton16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_hton32, lib_hton32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(lib_hton64, lib_hton64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);


PRIVATE struct dex_symbol symbols[] = {
	{ "socket", (DeeObject *)&DeeSocket_Type, MODSYM_FREADONLY },
	{ "sockaddr", (DeeObject *)&DeeSockAddr_Type, MODSYM_FREADONLY },
	{ "NetError", (DeeObject *)&DeeError_NetError, MODSYM_FREADONLY },
	{ "getafname", (DeeObject *)&lib_getafname, MODSYM_FREADONLY,
	  DOC("(id:?Dint)->?Dstring\n"
	      "(id:?Dint)->?Dint\n"
	      "Return the name of a given address family, given its system-dependent ID\n"
	      "When not known, re-return the given @id") },
	{ "getafof", (DeeObject *)&lib_getafof, MODSYM_FREADONLY,
	  DOC("(id:?Dint)->?Dint\n"
	      "(name:?Dstring)->?Dint\n"
	      "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
	      "Return the system-dependent ID of a given address family @name, or re-return the given @id.\n"
	      "The given @name is encoded the same way as the first constructor argument for ?Gsocket") },
	{ "gettypename", (DeeObject *)&lib_gettypename, MODSYM_FREADONLY,
	  DOC("(typ:?Dint)->?Dstring\n"
	      "(typ:?Dint)->?Dint\n"
	      "Return the name of a given socket type @type, given its system-dependent ID\n"
	      "When not known, re-return the given @type") },
	{ "gettypeof", (DeeObject *)&lib_gettypeof, MODSYM_FREADONLY,
	  DOC("(typ:?Dint)->?Dint\n"
	      "(name:?Dstring)->?Dint\n"
	      "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
	      "Return the system-dependent ID of a given socket type @name, or re-return the given @type.\n"
	      "The given @name is encoded the same way as the second constructor argument for ?Gsocket") },
	{ "getprotoname", (DeeObject *)&lib_getprotoname, MODSYM_FREADONLY,
	  DOC("(proto:?Dint)->?Dstring\n"
	      "(proto:?Dint)->?Dint\n"
	      "Return the name of a given protocol @proto, given its system-dependent ID\n"
	      "When not known, re-return the given @proto") },
	{ "getprotoof", (DeeObject *)&lib_getprotoof, MODSYM_FREADONLY,
	  DOC("(proto:?Dint)->?Dint\n"
	      "(name:?Dstring)->?Dint\n"
	      "#t{?ANoSupport?GNetError}{The given @name is not recognized by this library}"
	      "Return the system-dependent ID of a given protocol name @name, or re-return the given @proto.\n"
	      "The given @name is encoded the same way as the third constructor argument for ?Gsocket") },
	{ "getmsgflagsname", (DeeObject *)&lib_getmsgflagsname, MODSYM_FREADONLY,
	  DOC("(msgflags:?Dint)->?Dstring\n"
	      "(msgflags:?Dint)->?Dint\n"
	      "Return the name of a given message flags @msgflags, given a system-dependent set of flags\n"
	      "When not known, re-return the given @msgflags") },
	{ "getmsgflagsof", (DeeObject *)&lib_getmsgflagsof, MODSYM_FREADONLY,
	  DOC("(msgflags:?Dint)->?Dint\n"
	      "(flags:?Dstring)->?Dint\n"
	      "#t{?ANoSupport?GNetError}{The given @flags contains at least one flag that is not recognized by this library}"
	      "Return a system-dependent set of flags for given message flags @flags, or re-return the given @flags.\n"
	      "The given @flags is encoded the same way as the flags argument passed to ?Arecv?Gsocket") },
	{ "ntoh16", (DeeObject *)&lib_ntoh16, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 16-bit integer @x from network-endian to host-endian") },
	{ "ntoh32", (DeeObject *)&lib_ntoh32, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 32-bit integer @x from network-endian to host-endian") },
	{ "ntoh64", (DeeObject *)&lib_ntoh64, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 64-bit integer @x from network-endian to host-endian") },
	{ "hton16", (DeeObject *)&lib_hton16, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 16-bit integer @x from host-endian to network-endian") },
	{ "hton32", (DeeObject *)&lib_hton32, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 32-bit integer @x from host-endian to network-endian") },
	{ "hton64", (DeeObject *)&lib_hton64, MODSYM_FREADONLY,
	  DOC("(x:?Dint)->?Dint\n"
	      "Convert a 64-bit integer @x from host-endian to network-endian") },
	{ NULL }
};


PRIVATE int DCALL
libnet_init(DeeDexObject *__restrict UNUSED(self)) {
#ifdef CONFIG_HOST_WINDOWS
	/* Start up the the windows networking subsystem. */
	neterrno_t error;
	WSADATA wsaData;
	/* Use 1.1, for maximum backwards compatibility. */
	DBG_ALIGNMENT_DISABLE();
	error = (neterrno_t)WSAStartup(MAKEWORD(1, 1), &wsaData);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(error != 0)
		return DeeError_Throwf(&DeeError_NetError, "WSAStartup() : %lu", error);
#elif defined(CONFIG_HAVE_signal) && defined(SIGPIPE)
	/* SIGPIPE is generated when a remote socket is closed. */
	void (*handler)(int);
	DBG_ALIGNMENT_DISABLE();
	/* Try to ignore this signal. */
	handler = signal(SIGPIPE, SIG_IGN);
	/* Make sure we don't override a custom handler */
	if (handler != SIG_DFL)
		signal(SIGPIPE, handler);
	DBG_ALIGNMENT_ENABLE();
#endif /* ... */
	return 0;
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE void DCALL
libnet_fini(DeeDexObject *__restrict UNUSED(self)) {
	DBG_ALIGNMENT_DISABLE();
	WSACleanup();
	DBG_ALIGNMENT_ENABLE();
}
#endif /* CONFIG_HOST_WINDOWS */


PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols,
	/* .d_init    = */ &libnet_init,
#ifdef CONFIG_HOST_WINDOWS
	/* .d_fini    = */ &libnet_fini
#else /* CONFIG_HOST_WINDOWS */
	/* .d_fini    = */ NULL
#endif /* !CONFIG_HOST_WINDOWS */
};

DECL_END


#endif /* !GUARD_DEX_SOCKET_LIBNET_C */
