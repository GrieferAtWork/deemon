/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_SOCKADDR_C
#define GUARD_DEX_SOCKET_SOCKADDR_C 1
#define DEE_SOURCE 1

#include "libnet.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcasecmp(), bzero(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

DECL_BEGIN

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */


struct msg_desc {
	char md_name[16];
	int  md_flag;
};

PRIVATE struct msg_desc const sock_msg_names[] = {
#ifdef MSG_OOB
	{ "OOB", MSG_OOB },
#endif /* MSG_OOB */
#ifdef MSG_PEEK
	{ "PEEK", MSG_PEEK },
#endif /* MSG_PEEK */
#ifdef MSG_DONTROUTE
	{ "DONTROUTE", MSG_DONTROUTE },
#endif /* MSG_DONTROUTE */
#ifdef MSG_WAITALL
	{ "WAITALL", MSG_WAITALL },
#endif /* MSG_WAITALL */
#ifdef MSG_PUSH_IMMEDIATE
	{ "PUSH_IMMEDIATE", MSG_PUSH_IMMEDIATE },
#endif /* MSG_PUSH_IMMEDIATE */
#ifdef MSG_PARTIAL
	{ "PARTIAL", MSG_PARTIAL },
#endif /* MSG_PARTIAL */
#ifdef MSG_INTERRUPT
	{ "INTERRUPT", MSG_INTERRUPT },
#endif /* MSG_INTERRUPT */
};


/* Return a human-readable representation of
 * send/recv flags, or the flags as an integer object. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
sock_getmsgflagsof(DeeObject *__restrict name,
                   int *__restrict presult) {
	if (DeeString_Check(name)) {
		int result = 0;
		char *iter = DeeString_STR(name);
		while (*iter) {
			size_t part_length;
			struct msg_desc const *desc;
			for (;;) {
				if (MEMCASEEQ(iter, "MSG", 3 * sizeof(char))) {
					iter += 3;
					continue;
				}
				if (*iter == '_') {
					++iter;
					continue;
				}
				break;
			}
			/* Search for the end of this flag. */
			{
				char *temp = strchr(iter, '|');
				if (!temp)
					temp = strchr(iter, ',');
				if (!temp)
					temp = iter + strlen(iter);
				part_length = (size_t)(temp - iter);
			}
			if (part_length < COMPILER_LENOF(sock_msg_names[0].md_name)) {
				for (desc = sock_msg_names;
				     desc != COMPILER_ENDOF(sock_msg_names); ++desc) {
					if (!MEMCASEEQ(desc->md_name, iter, part_length))
						continue;
					/* Found it! */
					result |= desc->md_flag;
					goto next_part;
				}
			}
			/* Throw a NoSupport error, just like we do for the API
			 * functions when they don't recognize the passed flags. */
			return DeeError_Throwf(&DeeError_NoSupport,
			                       "MSG flag %$q is not recognized by this host",
			                       part_length, iter);
next_part:
			iter += part_length;
			if (*iter)
				++iter;
		}
		*presult = result;
		return 0;
	}
	return DeeObject_AsInt(name, presult);
}

INTERN WUNUSED DREF DeeObject *DCALL sock_getmsgflagsnameorid(int flags) {
	struct msg_desc const *desc;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	int match_mask               = 0;
	for (desc = sock_msg_names;
	     desc != COMPILER_ENDOF(sock_msg_names); ++desc) {
		if (flags & desc->md_flag) {
			/* Print the string representation of this flag. */
			if (ascii_printer_printf(&printer, "%sMSG_%s",
			                         ASCII_PRINTER_LEN(&printer) ? "|" : "",
			                         desc->md_name) < 0)
				goto err;

			match_mask |= desc->md_flag;
		}
	}
	/* If there are flags that we didn't recognize, return them as an integer. */
	if (flags & ~match_mask)
		goto fallback;
	return ascii_printer_pack(&printer);
fallback:
	ascii_printer_fini(&printer);
	return DeeInt_NewInt(flags);
err:
	ascii_printer_fini(&printer);
	return NULL;
}



struct af_desc {
	char ad_name[12]; /* Family name. */
	int  ad_value;    /* Family value. */
};
struct type_desc {
	char td_name[12]; /* Type name. */
	int  td_value;    /* Type value. */
};


PRIVATE struct af_desc const sock_af_names[] = {
#ifdef AF_UNSPEC
	{ "UNSPEC", AF_UNSPEC },
#endif /* AF_UNSPEC */
#ifdef AF_UNIX
	{ "UNIX", AF_UNIX },
#endif /* AF_UNIX */
#ifdef AF_INET
	{ "INET", AF_INET },
#endif /* AF_INET */
#ifdef AF_IMPLINK
	{ "IMPLINK", AF_IMPLINK },
#endif /* AF_IMPLINK */
#ifdef AF_PUP
	{ "PUP", AF_PUP },
#endif /* AF_PUP */
#ifdef AF_CHAOS
	{ "CHAOS", AF_CHAOS },
#endif /* AF_CHAOS */
#ifdef AF_NS
	{ "NS", AF_NS },
#endif /* AF_NS */
#ifdef AF_IPX
	{ "IPX", AF_IPX },
#endif /* AF_IPX */
#ifdef AF_ISO
	{ "ISO", AF_ISO },
#endif /* AF_ISO */
#ifdef AF_OSI
	{ "OSI", AF_OSI },
#endif /* AF_OSI */
#ifdef AF_ECMA
	{ "ECMA", AF_ECMA },
#endif /* AF_ECMA */
#ifdef AF_DATAKIT
	{ "DATAKIT", AF_DATAKIT },
#endif /* AF_DATAKIT */
#ifdef AF_CCITT
	{ "CCITT", AF_CCITT },
#endif /* AF_CCITT */
#ifdef AF_SNA
	{ "SNA", AF_SNA },
#endif /* AF_SNA */
#ifdef AF_DECnet
	{ "DECNET", AF_DECnet },
#endif /* AF_DECnet */
#ifdef AF_DLI
	{ "DLI", AF_DLI },
#endif /* AF_DLI */
#ifdef AF_LAT
	{ "LAT", AF_LAT },
#endif /* AF_LAT */
#ifdef AF_HYLINK
	{ "HYLINK", AF_HYLINK },
#endif /* AF_HYLINK */
#ifdef AF_APPLETALK
	{ "APPLETALK", AF_APPLETALK },
#endif /* AF_APPLETALK */
#ifdef AF_NETBIOS
	{ "NETBIOS", AF_NETBIOS },
#endif /* AF_NETBIOS */
#ifdef AF_VOICEVIEW
	{ "VOICEVIEW", AF_VOICEVIEW },
#endif /* AF_VOICEVIEW */
#ifdef AF_FIREFOX
	{ "FIREFOX", AF_FIREFOX },
#endif /* AF_FIREFOX */
#ifdef AF_UNKNOWN1
	{ "UNKNOWN1", AF_UNKNOWN1 },
#endif /* AF_UNKNOWN1 */
#ifdef AF_BAN
	{ "BAN", AF_BAN },
#endif /* AF_BAN */
#ifdef AF_ATM
	{ "ATM", AF_ATM },
#endif /* AF_ATM */
#ifdef AF_INET6
	{ "INET6", AF_INET6 },
#endif /* AF_INET6 */
#ifdef AF_CLUSTER
	{ "CLUSTER", AF_CLUSTER },
#endif /* AF_CLUSTER */
#ifdef AF_12844
	{ "12844", AF_12844 },
#endif /* AF_12844 */
#ifdef AF_IRDA
	{ "IRDA", AF_IRDA },
#endif /* AF_IRDA */
#ifdef AF_NETDES
	{ "NETDES", AF_NETDES },
#endif /* AF_NETDES */
#ifdef AF_TCNPROCESS
	{ "TCNPROCESS", AF_TCNPROCESS },
#endif /* AF_TCNPROCESS */
#ifdef AF_TCNMESSAGE
	{ "TCNMESSAGE", AF_TCNMESSAGE },
#endif /* AF_TCNMESSAGE */
#ifdef AF_ICLFXBM
	{ "ICLFXBM", AF_ICLFXBM },
#endif /* AF_ICLFXBM */
#ifdef AF_BTH
	{ "BTH", AF_BTH },
#endif /* AF_BTH */
#ifdef AF_LINK
	{ "LINK", AF_LINK },
#endif /* AF_LINK */
#ifdef AF_BLUETOOTH
	{ "NETLINK", AF_BLUETOOTH },
#endif /* AF_BLUETOOTH */
#ifdef AF_NETLINK
	{ "NETLINK", AF_NETLINK },
#endif /* AF_NETLINK */
#ifdef AF_PACKET
	{ "PACKET", AF_PACKET },
#endif /* AF_PACKET */
#ifdef AF_TIPC
	{ "TIPC", AF_TIPC },
#endif /* AF_TIPC */
#ifdef AF_CAN
	{ "CAN", AF_CAN },
#endif /* AF_CAN */
#ifdef PF_SYSTEM
	{ "SYSTEM", PF_SYSTEM },
#endif /* PF_SYSTEM */
	{ "AUTO", AF_AUTO }
};

PRIVATE struct type_desc const sock_type_names[] = {
#ifdef SOCK_STREAM
	{ "STREAM",    SOCK_STREAM },
#endif /* SOCK_STREAM */
#ifdef SOCK_DGRAM
	{ "DGRAM",     SOCK_DGRAM },
#endif /* SOCK_DGRAM */
#ifdef SOCK_RAW
	{ "RAW",       SOCK_RAW },
#endif /* SOCK_RAW */
#ifdef SOCK_RDM
	{ "RDM",       SOCK_RDM },
#endif /* SOCK_RDM */
#ifdef SOCK_SEQPACKET
	{ "SEQPACKET", SOCK_SEQPACKET },
#endif /* SOCK_SEQPACKET */
};

INTERN WUNUSED DREF DeeObject *DCALL
sock_getafname(int value) {
	struct af_desc const *iter = sock_af_names;
	for (; iter != COMPILER_ENDOF(sock_af_names); ++iter) {
		if (iter->ad_value != value)
			continue;
		return DeeString_Newf("AF_%s", iter->ad_name);
	}
	return ITER_DONE;
}

INTERN WUNUSED DREF DeeObject *DCALL
sock_gettypename(int value) {
	struct type_desc const *iter = sock_type_names;
	for (; iter != COMPILER_ENDOF(sock_type_names); ++iter) {
		if (iter->td_value != value)
			continue;
		return DeeString_Newf("SOCK_%s", iter->td_name);
	}
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
sock_getafvalue(char const *__restrict name, int *__restrict presult) {
	struct af_desc const *iter = sock_af_names;
	size_t name_len;
	if (MEMCASEEQ(name, "AF_", 3 * sizeof(char)))
		name += 3;
	name_len = strlen(name);
	for (; iter != COMPILER_ENDOF(sock_af_names); ++iter) {
		if (!MEMCASEEQ(iter->ad_name, name, name_len * sizeof(char)))
			continue;
		*presult = iter->ad_value;
		return true;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
sock_gettypevalue(char const *__restrict name, int *__restrict presult) {
	struct type_desc const *iter = sock_type_names;
	size_t name_len;
	if (MEMCASEEQ(name, "SOCK_", 5 * sizeof(char)))
		name += 5;
	name_len = strlen(name);
	for (; iter != COMPILER_ENDOF(sock_type_names); ++iter) {
		if (!MEMCASEEQ(iter->td_name, name, name_len * sizeof(char)))
			continue;
		*presult = iter->td_value;
		return true;
	}
	return false;
}



/* Lock used to access the system's database functions. */
PRIVATE DEFINE_RWLOCK(sysdb_lock);

INTERN WUNUSED DREF DeeObject *DCALL
sock_getprotoname(int value) {
	struct protoent *ent;
	DREF DeeStringObject *result;
	char const *name;
	size_t name_length;
again:
	rwlock_write(&sysdb_lock);
	DBG_ALIGNMENT_DISABLE();
	ent = getprotobynumber(value);
	DBG_ALIGNMENT_ENABLE();
	if (ent && (name = ent->p_name) != NULL) {
		name_length = strlen(name);
		result = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                     (name_length + 1) * sizeof(char));
		if unlikely(!result) {
			rwlock_endwrite(&sysdb_lock);
			if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
			                      (name_length + 1) * sizeof(char)))
				goto again;
			return NULL;
		}
		memcpy(result->s_str, name, (name_length + 1) * sizeof(char));
		rwlock_endwrite(&sysdb_lock);
		result->s_hash = (dhash_t)-1;
		result->s_data = NULL;
		result->s_len  = name_length;
		DeeObject_Init(result, &DeeString_Type);
		return (DREF DeeObject *)result;
	}
	rwlock_endwrite(&sysdb_lock);
	if (!ent)
		return ITER_DONE;
	return_empty_string;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
sock_getprotovalue(char const *__restrict name,
                   int *__restrict presult) {
	struct protoent *ent;
again:
	rwlock_write(&sysdb_lock);
	ent = getprotobyname(name);
	if (ent) {
		*presult = ent->p_proto;
		rwlock_endwrite(&sysdb_lock);
		return true;
	}
	rwlock_endwrite(&sysdb_lock);
	/* Search for case-insensitive `PROTO' in the name and re-try with
	 * the following string after stripping leading underscores.
	 * >> `sock_getprotovalue("IPPROTO_TCP")' should still work,
	 *     but the system's database usually only accepts `"TCP"' */
	for (; *name; ++name) {
		if (!MEMCASEEQ(name, "PROTO", 5 * sizeof(char)))
			continue;
		name += 5;
		while (*name == '_')
			++name;
		goto again;
	}
	return false;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
sock_getafof(DeeObject *__restrict name, int *__restrict presult) {
	if (DeeNone_Check(name)) {
		*presult = AF_AUTO;
		return 0;
	}
	if (DeeString_Check(name)) {
		if unlikely(!sock_getafvalue(DeeString_STR(name), presult)) {
			return DeeError_Throwf(&DeeError_NoSupport,
			                       "Unknown address family %r",
			                       name);
		}
		return 0;
	}
	return DeeObject_AsInt(name, presult);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
sock_gettypeof(DeeObject *__restrict name, int *__restrict presult) {
	if (DeeNone_Check(name)) {
		*presult = SOCK_STREAM;
		return 0;
	}
	if (DeeString_Check(name)) {
		if unlikely(!sock_gettypevalue(DeeString_STR(name), presult)) {
			return DeeError_Throwf(&DeeError_NoSupport,
			                       "Unknown socket type %r",
			                       name);
		}
		return 0;
	}
	return DeeObject_AsInt(name, presult);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
sock_getprotoof(DeeObject *__restrict name, int *__restrict presult) {
#if 0 /* None already evaluates to int(0) */
	if (DeeNone_Check(name)) {
		*presult = 0;
		return 0; /* Default/unused protocol */
	}
#endif
	if (DeeString_Check(name)) {
		if unlikely(!sock_getprotovalue(DeeString_STR(name), presult)) {
			return DeeError_Throwf(&DeeError_NoSupport,
			                       "Unknown protocol name %r",
			                       name);
		}
		return 0;
	}
	return DeeObject_AsInt(name, presult);
}

INTERN WUNUSED DREF DeeObject *DCALL
sock_getafnameorid(int value) {
	DREF DeeObject *result;
	result = sock_getafname(value);
	if (result == ITER_DONE)
		result = DeeInt_NewS32((int32_t)value);
	return result;
}

INTERN WUNUSED DREF DeeObject *DCALL
sock_gettypenameorid(int value) {
	DREF DeeObject *result;
	result = sock_gettypename(value);
	if (result == ITER_DONE)
		result = DeeInt_NewInt(value);
	return result;
}

INTERN WUNUSED DREF DeeObject *DCALL
sock_getprotonameorid(int value) {
	DREF DeeObject *result;
	result = sock_getprotoname(value);
	if (result == ITER_DONE)
		result = DeeInt_NewInt(value);
	return result;
}


struct shutdown_option {
	char so_nam[12];
	int  so_opt;
};

PRIVATE struct shutdown_option const shutdown_options[] = {
	{ "R", SHUT_RD },
	{ "W", SHUT_WR },
	{ "RW", SHUT_RDWR },
	{ "WR", SHUT_RDWR },
	{ "RD", SHUT_RD },
	{ "WR", SHUT_WR },
	{ "RDRW", SHUT_RDWR },
	{ "WRRD", SHUT_RDWR },
	{ "READ", SHUT_RD },
	{ "WRITE", SHUT_WR },
	{ "READWRITE", SHUT_RDWR },
	{ "WRITEREAD", SHUT_RDWR }
};


INTERN WUNUSED NONNULL((1, 2)) int DCALL
get_shutdown_mode(char const *__restrict mode,
                  int *__restrict presult) {
	char const *used_mode = mode;
	size_t mode_length;
	struct shutdown_option const *iter;
	for (;;) {
		if (MEMCASEEQ(used_mode, "SHUT", 4 * sizeof(char))) {
			used_mode += 4;
			continue;
		}
		if (*used_mode == '_') {
			++used_mode;
			continue;
		}
		break;
	}
	iter = shutdown_options, mode_length = strlen(used_mode);
	for (; iter != COMPILER_ENDOF(shutdown_options); ++iter) {
		if (!MEMCASEEQ(iter->so_nam, mode, mode_length * sizeof(char)))
			continue;
		*presult = iter->so_opt;
		return 0;
	}
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid shutdown mode `%s'",
	                       mode);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
get_shutdown_modeof(DeeObject *__restrict mode,
                    int *__restrict presult) {
	if (DeeString_Check(mode))
		return get_shutdown_mode(DeeString_STR(mode), presult);
	return DeeObject_AsInt(mode, presult);
}





INTERN socklen_t DCALL
SockAddr_Sizeof(sa_family_t family, int protocol) {
	socklen_t result;
	(void)protocol;
	switch (family) {
#ifdef AF_INET
	case AF_INET:
		result = sizeof(struct sockaddr_in);
		break;
#endif /* AF_INET */

#ifdef AF_INET6
	case AF_INET6:
		result = sizeof(struct sockaddr_in6);
		break;
#endif /* AF_INET6 */

#ifdef AF_UNIX
	case AF_UNIX:
		result = sizeof(struct sockaddr_un);
		break;
#endif /* AF_UNIX */

#ifdef AF_NETLINK
	case AF_NETLINK:
		result = sizeof(struct sockaddr_nl);
		break;
#endif /* AF_NETLINK */

#ifdef AF_BLUETOOTH
	case AF_BLUETOOTH:
		switch (protocol) {

		case BTPROTO_L2CAP:
			result = sizeof(struct sockaddr_l2);
			break;

		case BTPROTO_RFCOMM:
			result = sizeof(struct sockaddr_rc);
			break;

		case BTPROTO_HCI:
			result = sizeof(struct sockaddr_hci);
			break;

#ifndef __FreeBSD__
		case BTPROTO_SCO:
			result = sizeof(struct sockaddr_sco);
			break;
#endif /* __FreeBSD__ */

		default:
			/* Try our luck with the actual size... */
			result = sizeof(SockAddr);
			break;
		}
#endif /* AF_BLUETOOTH */

	default:
		/* Try our luck with the actual size... */
		result = sizeof(SockAddr);
		break;
	}
	return result;
}

PRIVATE ATTR_COLD int DCALL
err_no_host(char const *__restrict host,
            char const *port, int error) {
	(void)error; /* XXX: New error code class? */
	if (port) {
		return DeeError_Throwf(&DeeError_HostNotFound,
		                       "Host %q with port %q could not be found",
		                       host, port);
	} else {
		return DeeError_Throwf(&DeeError_HostNotFound,
		                       "Host %q could not be found",
		                       host);
	}
}

PRIVATE ATTR_COLD int DCALL
err_no_host_data(char const *__restrict host,
                 char const *port, int family, int error) {
	(void)error; /* XXX: New error code class? */
	if (port) {
		return DeeError_Throwf(&DeeError_NoHostAddress,
		                       "Host %q with port %q is valid but has no addresses for family %K associated with it",
		                       host, port, sock_getafnameorid(family));
	} else {
		return DeeError_Throwf(&DeeError_NoHostAddress,
		                       "Host %q is valid but has no addresses for family %K associated with it",
		                       host, sock_getafnameorid(family));
	}
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sock_gethostbyaddr(void const *__restrict data, socklen_t datalen,
                   sa_family_t family, int flags) {
	struct hostent *hp;
	DeeStringObject *result;
	neterrno_t error;
	size_t name_length;
#ifdef TRY_AGAIN
	int attempt_counter = 0;
#endif /* TRY_AGAIN */
restart:
	if (flags & SOCKADDR_STR_FNODNS)
		goto nodns;
	rwlock_write(&sysdb_lock);
	DBG_ALIGNMENT_DISABLE();
	hp = (struct hostent *)gethostbyaddr((char const *)data, datalen, family);
	DBG_ALIGNMENT_ENABLE();
	if (!hp) {
		DBG_ALIGNMENT_DISABLE();
		error = h_errno;
		DBG_ALIGNMENT_ENABLE();
		rwlock_endwrite(&sysdb_lock);
		if (flags & SOCKADDR_STR_FNOFAIL)
			goto nodns;
		if (error == HOST_NOT_FOUND) {
			(void)error; /* XXX: New error code class? */
			DeeError_Throwf(&DeeError_HostNotFound,
			                "Host %K with family %K could not be found",
			                sock_gethostbyaddr(data, datalen, family,
			                                   SOCKADDR_STR_FNODNS |
			                                   SOCKADDR_STR_FNOFAIL),
			                sock_getafnameorid(family));
		} else if (error == NO_ADDRESS
#if defined(NO_DATA) && NO_DATA != NO_ADDRESS
		           || error == NO_DATA
#endif /* NO_DATA && NO_DATA != NO_ADDRESS */
		           ) {
			(void)error; /* XXX: New error code class? */
			DeeError_Throwf(&DeeError_NoHostAddress,
			                "Host %K with family %K has no addresses associated",
			                sock_gethostbyaddr(data, datalen, family,
			                                   SOCKADDR_STR_FNODNS |
			                                   SOCKADDR_STR_FNOFAIL),
			                sock_getafnameorid(family));
#ifdef TRY_AGAIN
		} else if (error == TRY_AGAIN && attempt_counter < 3) {
			if (DeeThread_Sleep(10000))
				goto err;
			++attempt_counter;
			goto restart;
#endif /* TRY_AGAIN */
		} else {
			(void)error; /* XXX: New error code class? */
			DeeError_Throwf(&DeeError_NetError,
			                "Failed to get host address for %K",
			                sock_gethostbyaddr(data, datalen, family,
			                                   SOCKADDR_STR_FNODNS |
			                                   SOCKADDR_STR_FNOFAIL));
		}
		goto err;
	}
	if unlikely(!hp->h_name)
		goto nodns2;
	name_length = strlen(hp->h_name);
	/* Safely copy the host's name. */
	result = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
	                                                (name_length + 1) * sizeof(char));
	if unlikely(!result) {
		rwlock_endwrite(&sysdb_lock);
		if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
		                      (name_length + 1) * sizeof(char)))
			goto restart;
		goto err;
	}
	memcpy(result->s_str, hp->h_name, (name_length + 1) * sizeof(char));
	rwlock_endwrite(&sysdb_lock);
	DeeObject_Init(result, &DeeString_Type);
	result->s_data = NULL;
	result->s_hash = (dhash_t)-1;
	result->s_len  = name_length;
	return (DREF DeeObject *)result;
err:
	return NULL;
nodns2:
	rwlock_endwrite(&sysdb_lock);
nodns:
#ifdef AF_INET
	if (family == AF_INET) {
		uint32_t host = NTOH32(*(uint32_t *)data);
		return DeeString_Newf("%I8u.%I8u.%I8u.%I8u",
		                      (host & 0xff000000) >> 24, (host & 0xff0000) >> 16,
		                      (host & 0xff00) >> 8, (host & 0xff));
	}
#endif /* AF_INET */
#ifdef AF_INET6
	if (family == AF_INET6) {
		uint16_t *words = (uint16_t *)data;
		return DeeString_Newf("%I16u:%I16u:%I16u:%I16u:%I16u:%I16u:%I16u:%I16u",
		                      NTOH16(words[0]), NTOH16(words[1]),
		                      NTOH16(words[2]), NTOH16(words[3]),
		                      NTOH16(words[4]), NTOH16(words[5]),
		                      NTOH16(words[6]), NTOH16(words[7]));
	}
#endif /* AF_INET6 */
	return DeeString_Newf("[%K-sockaddr]", sock_getafnameorid(family));
}



INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SockAddr_ToString(SockAddr const *__restrict self, int protocol, int flags) {
	DREF DeeObject *result;
	sa_family_t family = self->sa.sa_family;
	switch (family) {

#ifdef AF_INET
#ifdef AF_RDS
	case AF_RDS:
#endif /* AF_RDS */
	case AF_INET:
		result = sock_gethostbyaddr(&self->sa_inet.sin_addr, 4, family, flags);
		if (!(flags & SOCKADDR_STR_FNOPORT))
			return DeeString_Newf("%K:%I16u", result, NTOH16(self->sa_inet.sin_port));
		break;
#endif /* AF_INET */

#ifdef AF_INET6
	case AF_INET6:
		result = sock_gethostbyaddr(&self->sa_inet.sin_addr, 4, family, flags);
		if (!(flags & SOCKADDR_STR_FNOPORT))
			return DeeString_Newf("[%K]:%I16u", result, NTOH16(self->sa_inet6.sin6_port));
		break;
#endif /* AF_INET6 */

#ifdef AF_UNIX
	case AF_UNIX:
		return DeeString_Newf("AF_UNIX:%.*q",
		                      (unsigned int)(sizeof(self->sa_un.sun_path) / sizeof(char)),
		                      self->sa_un.sun_path);
#endif /* AF_UNIX */

	default:
		result = sock_gethostbyaddr(&self->sa_inet.sin_addr,
		                            SockAddr_Sizeof(family, protocol) -
		                            offsetof(SockAddr, sa_inet.sin_addr),
		                            family, flags);
		if unlikely(!result)
			goto err;
		break;
	}
	return result;
err:
	return NULL;
}

PRIVATE int DCALL
get_port_name(char const *__restrict port, size_t port_length,
              uint16_t *__restrict presult) {
	char const *iter = port;
	uint16_t new_result, result = 0;
	if unlikely(!port_length)
		goto invalid_port;
	do {
		char ch = *iter++;
		if unlikely(!(ch >= '0' && ch <= '9'))
			goto invalid_port;
		new_result = result * 10 + (ch - '0');
		if unlikely(new_result < result)
			goto invalid_port;
		result = new_result;
	} while (--port_length);
	*presult = result;
	return 0;
invalid_port:
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid port string %q",
	                       port);
}


#define MAX_SPECIAL_NAME_LENGTH 16

#ifdef AF_INET
struct inet_hostname {
	char     name[MAX_SPECIAL_NAME_LENGTH];
	uint32_t addr; /* Address (in net endian) */
};

PRIVATE struct inet_hostname const speical_inet_names[] = {
	{ "ANY", HTON32_C(INADDR_ANY) },
	{ "LOCALHOST", HTON32_C(INADDR_LOOPBACK) },
	{ "LOOPBACK", HTON32_C(INADDR_LOOPBACK) },
	{ "BROADCAST", HTON32_C(INADDR_BROADCAST) },
	{ "NONE", HTON32_C(INADDR_NONE) },
	{ "UNSPEC_GROUP", HTON32_C(INADDR_UNSPEC_GROUP) },
	{ "ALLHOSTS_GROUP", HTON32_C(INADDR_ALLHOSTS_GROUP) },
	{ "MAX_LOCAL_GROUP", HTON32_C(INADDR_MAX_LOCAL_GROUP) }
};
#endif /* AF_INET */


#ifdef AF_INET6
struct inet6_hostname {
	char            name[MAX_SPECIAL_NAME_LENGTH];
	struct in6_addr addr;
};

PRIVATE struct inet6_hostname const speical_inet6_names[] = {
	{ "ANY", IN6ADDR_ANY_INIT },
	{ "LOCALHOST", IN6ADDR_LOOPBACK_INIT },
	{ "LOOPBACK", IN6ADDR_LOOPBACK_INIT },
};
#endif /* AF_INET */





PRIVATE int DCALL
SockAddr_FromStringPort(SockAddr *__restrict self, int family, int protocol, int type,
                        char const *__restrict host, size_t host_length,
                        char const *__restrict port, size_t port_length) {
	int error         = 0;
	char *port_buffer = NULL;
	char *host_buffer = NULL;
#if defined(EAI_AGAIN) || defined(TRY_AGAIN)
	int attempt_counter = 0;
#endif /* EAI_AGAIN || TRY_AGAIN */
	/* Strip whitespace before and after the host and port names. */
	while (host_length && DeeUni_IsSpace(*host))
		++host, --host_length;
	while (host_length && DeeUni_IsSpace(host[host_length - 1]))
		--host_length;
	while (port_length && DeeUni_IsSpace(*port))
		++port, --port_length;
	while (port_length && DeeUni_IsSpace(port[port_length - 1]))
		--port_length;
	if (host_length == 1 && host[0] == '*') {
		/* Automatic, any-host address:
		 * >> IPv4: 0.0.0.0
		 * >> IPv6: ::
		 * Used to allow the binding of a server socket on the local network,
		 * whilst automatically selecting the used proper socket address format. */
		switch (family) {

#ifdef AF_INET
		case AF_INET:
#endif /* AF_INET */
#ifdef AF_INET6
		case AF_INET6:
#endif /* AF_INET6 */
			bzero(self, sizeof(SockAddr));
			if (get_port_name(port, port_length, &self->sa_inet.sin_port))
				goto err;
			/* Convert network endian. */
			self->sa_inet.sin_port = HTON16(self->sa_inet.sin_port);
			self->sa.sa_family     = (sa_family_t)family;
			return 0;

		default: break;
		}
	}

	/* Make sure that we've got a NUL-terminated input string. */
	if (host[host_length]) {
		host_buffer = (char *)Dee_AMalloc((host_length + 1) * sizeof(char));
		if unlikely(!host_buffer)
			goto err;
		memcpy(host_buffer, host, host_length * sizeof(char));
		host_buffer[host_length] = '\0';
		host                     = host_buffer;
	}
	if (port[port_length]) {
		port_buffer = (char *)Dee_AMalloc((port_length + 1) * sizeof(char));
		if unlikely(!port_buffer)
			goto err;
		memcpy(port_buffer, port, port_length * sizeof(char));
		port_buffer[port_length] = '\0';
		port                     = port_buffer;
	}
#ifdef AF_INET
	if (family == AF_INET)
		goto do_gethostbyname;
#endif /* AF_INET */
#ifdef AF_RDS
	if (family == AF_RDS)
		goto do_gethostbyname;
#endif /* AF_RDS */

	{
		struct addrinfo *info;
		struct addrinfo hints;
retry_addrinfo:
		if (family != AF_AUTO) {
			bzero(&hints, sizeof(hints));
			hints.ai_family   = (sa_family_t)family;
			hints.ai_protocol = protocol;
			hints.ai_socktype = type;
		}
		rwlock_read(&sysdb_lock);
		DBG_ALIGNMENT_DISABLE();
		error = getaddrinfo(host, port, family == AF_AUTO ? NULL : &hints, &info);
		DBG_ALIGNMENT_ENABLE();
		if (error != 0) {
			rwlock_endread(&sysdb_lock);
#ifdef EAI_AGAIN
			if (error == EAI_AGAIN && attempt_counter < 3) {
				if (DeeThread_Sleep(10000))
					goto err;
				++attempt_counter;
				goto retry_addrinfo;
			}
#endif /* EAI_AGAIN */
#ifdef EAI_MEMORY
			if (error == EAI_MEMORY) {
				if (Dee_CollectMemory(1))
					goto retry_addrinfo;
				goto err;
			}
#endif /* EAI_MEMORY */
#ifdef EAI_NODATA
			if (error == EAI_NODATA) {
				err_no_host_data(host, port, family, error);
				goto err;
			}
#endif /* EAI_NODATA */
			/* For auto-address-family, retry using the INET family. */
			if (family == AF_AUTO
#ifdef AF_INET6
			    || family == AF_INET6
#endif /* AF_INET6 */
#ifdef AF_INET
			    || family == AF_INET
#endif /* AF_INET */
#ifdef AF_RDS
			    || family == AF_RDS
#endif /* AF_RDS */
			) {
				DBG_ALIGNMENT_DISABLE();
				freeaddrinfo(info);
				DBG_ALIGNMENT_ENABLE();
				goto do_gethostbyname;
			}
#ifdef EAI_NONAME
			if (error == EAI_NONAME) {
				err_no_host(host, port, error);
				goto err;
			}
#endif /* EAI_NONAME */
#ifdef EAI_ADDRFAMILY
			if (error == EAI_ADDRFAMILY) {
				err_no_host_data(host, port, family, error);
				goto err;
			}
#endif /* EAI_ADDRFAMILY */
#ifdef EAI_SYSTEM
			if (error == EAI_SYSTEM) {
#ifdef CONFIG_HOST_WINDOWS
				error = (int)GetLastError();
#else /* CONFIG_HOST_WINDOWS */
				error = (int)errno;
#endif /* !CONFIG_HOST_WINDOWS */
			}
#endif /* EAI_SYSTEM */
			(void)error; /* XXX: New error code class? */
#ifdef CONFIG_HOST_WINDOWS
			DeeError_Throwf(&DeeError_NetError,
			                "Failed to query getaddrinfo(%q,%q)",
			                host, port);
#else /* CONFIG_HOST_WINDOWS */
			DeeError_Throwf(&DeeError_NetError,
			                "Failed to query getaddrinfo(%q,%q): %s",
			                host, port, gai_strerror(error));
#endif /* !CONFIG_HOST_WINDOWS */
			goto err;
		}
#if 1 /* Is this correct, or even necessary? */
		while (!info->ai_addr && info->ai_next)
			info = info->ai_next;
#endif
		if unlikely(!info->ai_addr) {
			DBG_ALIGNMENT_DISABLE();
			freeaddrinfo(info);
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&sysdb_lock);
			DeeError_Throwf(&DeeError_NetError,
			                "Count not find any address for %q using port %q",
			                host, port);
			error = -1;
		} else if (protocol != 0 &&
		         protocol != info->ai_protocol) {
			/* If an explicit protocol was specified, ensure that it is being used. */
			int real_proto = info->ai_protocol;
			DBG_ALIGNMENT_DISABLE();
			freeaddrinfo(info);
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&sysdb_lock);
			DeeError_Throwf(&DeeError_NotImplemented,
			                "Host %q on port %q uses a different protocol %K than %K",
			                host, port,
			                sock_getprotonameorid(real_proto),
			                sock_getprotonameorid(protocol));
		} else if unlikely(info->ai_addrlen > sizeof(SockAddr)) {
			sa_family_t info_family = info->ai_addr->sa_family;
			socklen_t info_len      = (socklen_t)info->ai_addrlen;
			COMPILER_READ_BARRIER();
			DBG_ALIGNMENT_DISABLE();
			freeaddrinfo(info);
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&sysdb_lock);
			DeeError_Throwf(&DeeError_NotImplemented,
			                "Address family %K for %q on port %q is too "
			                "big (its size %Iu exceeds the limit of %Iu)",
			                sock_getafnameorid(info_family),
			                host, port, info_len, sizeof(SockAddr));
			error = -1;
		} else {
			bzero(self, sizeof(SockAddr));
			memcpy(self, info->ai_addr, info->ai_addrlen);
			DBG_ALIGNMENT_DISABLE();
			freeaddrinfo(info);
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&sysdb_lock);
			error = 0;
		}
	}
done:
	Dee_XAFree(port_buffer);
	Dee_XAFree(host_buffer);
	return error;
err:
	error = -1;
	goto done;

	/* Special fallback handling for INET. */
#if defined(AF_INET) || defined(AF_RDS)
	{
		struct hostent *hp;
#ifdef TRY_AGAIN
		int attempt_counter;
#endif /* TRY_AGAIN */
do_gethostbyname:
		bzero(self, sizeof(SockAddr));
		/* Quick check: If the host starts with a digit, or with `:',
		 * then it isn't a special name, but an absolute address. */
		if (host_length && (!DeeUni_IsDecimal(*host) && *host != ':')) {
			char const *special_name = host;
			size_t special_size      = host_length;
			int special_family       = AF_AUTO;
			for (;;) {
#ifdef AF_INET
				if (MEMCASEEQ(special_name, "INADDR", 6 * sizeof(char))) {
					if (special_family != AF_AUTO)
						goto no_special_hostname;
					special_family = AF_INET;
					special_name += 6;
					special_size -= 6;
					continue;
				}
#endif /* AF_INET */
#ifdef AF_INET6
				if (MEMCASEEQ(special_name, "IN6ADDR", 7 * sizeof(char))) {
					if (special_family != AF_AUTO)
						goto no_special_hostname;
					special_family = AF_INET6;
					special_name += 7;
					special_size -= 7;
					continue;
				}
#endif /* AF_INET6 */
				if (*special_name == '_') {
					++special_name;
					--special_size;
					continue;
				}
				break;
			}
			if (special_size > MAX_SPECIAL_NAME_LENGTH)
				goto no_special_hostname;
			if (special_family == AF_AUTO)
				special_family = family;
#ifdef AF_INET6
			/* Search the special-name databases for IPv6 names */
			if (special_family == AF_INET6 ||
			    special_family == AF_AUTO) {
				struct inet6_hostname const *iter;
				for (iter = speical_inet6_names;
				     iter != COMPILER_ENDOF(speical_inet6_names); ++iter) {
					if (!MEMCASEEQ(iter->name, special_name, special_size))
						continue;
					memcpy(&self->sa_inet6.sin6_addr, &iter->addr, 16);
					goto do_port_inet6;
				}
			}
#endif /* AF_INET6 */
#ifdef AF_INET
			/* Search the special-name databases for IPv4 names */
			if (special_family == AF_INET ||
			    special_family == AF_AUTO) {
				struct inet_hostname const *iter;
				for (iter = speical_inet_names;
				     iter != COMPILER_ENDOF(speical_inet_names); ++iter) {
					if (!MEMCASEEQ(iter->name, special_name, special_size))
						continue;
					self->sa_inet.sin_addr.s_addr = iter->addr;
					goto do_port_inet;
				}
			}
#endif /* AF_INET */
		}
no_special_hostname:
#ifdef TRY_AGAIN
		attempt_counter = 0;
do_gethostbyname_again:
#endif /* TRY_AGAIN */
		rwlock_read(&sysdb_lock);
		DBG_ALIGNMENT_DISABLE();
		hp = (struct hostent *)gethostbyname(host);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(!hp) {
			DBG_ALIGNMENT_DISABLE();
			error = h_errno;
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&sysdb_lock);
			if (error == HOST_NOT_FOUND) {
				err_no_host(host, NULL, error);
			} else if (error == NO_ADDRESS
#if defined(NO_DATA) && NO_DATA != NO_ADDRESS
			           || error == NO_DATA
#endif /* NO_DATA && NO_DATA != NO_ADDRESS */
			) {
				err_no_host_data(host, NULL, family, error);
#ifdef TRY_AGAIN
			} else if (error == TRY_AGAIN && attempt_counter < 3) {
				if (DeeThread_Sleep(10000))
					goto err;
				++attempt_counter;
				goto do_gethostbyname_again;
#endif /* TRY_AGAIN */
			} else {
				(void)error; /* XXX: New error code class? */
				DeeError_Throwf(&DeeError_NetError,
				                "Failed to query host name %q",
				                host);
			}
			goto err;
		}
#ifdef AF_INET
		if (hp->h_addrtype == AF_INET) {
			self->sa_inet.sin_addr.s_addr = *(uint32_t *)hp->h_addr;
			rwlock_endread(&sysdb_lock);
do_port_inet:
			self->sa_inet.sin_family = AF_INET;
			if (get_port_name(port, port_length, &self->sa_inet.sin_port))
				goto err;
			/* Convert network endian. */
			self->sa_inet.sin_port = HTON16(self->sa_inet.sin_port);
		} else
#endif /* AF_INET */
#ifdef AF_INET6
		if (hp->h_addrtype == AF_INET6) {
			memcpy(&self->sa_inet6.sin6_addr, hp->h_addr, 16);
			rwlock_endread(&sysdb_lock);
do_port_inet6:
			self->sa_inet6.sin6_family = AF_INET6;
			if (get_port_name(port, port_length, &self->sa_inet6.sin6_port))
				goto err;
			/* Convert network endian. */
			self->sa_inet6.sin6_port = HTON16(self->sa_inet6.sin6_port);
		} else
#endif /* AF_INET6 */
		{
			sa_family_t fam = (sa_family_t)hp->h_addrtype;
			rwlock_endread(&sysdb_lock);
			(void)error; /* XXX: New error code class? */
			DeeError_Throwf(&DeeError_NetError,
			                "Unsupported address family %K for host name %q",
			                sock_getafnameorid(fam), host);
			goto err;
		}
		goto done;
	}
#endif /* AF_INET */
}



PRIVATE int DCALL
SockAddr_FromString(SockAddr *__restrict self,
                    int family, int protocol, int type,
                    char const *__restrict string,
                    size_t string_length) {
	char chr;
	char const *port_begin, *port_end;
	int bracket_recursion;
	port_end          = (port_begin = string) + string_length;
	bracket_recursion = 0;
	while (port_begin != port_end) {
		chr = *port_begin++;
		if (chr == '[') {
			++bracket_recursion;
		} else if (chr == ']') {
			--bracket_recursion;
		} else if (chr == ':' && bracket_recursion == 0) {
			size_t host_length = ((size_t)(port_begin - string)) - 1;
			/* Remove surrounding brackets from the IP name. */
			if (*string == '[' && string[host_length - 1] == ']')
				++string, host_length -= 2;
			return SockAddr_FromStringPort(self, family, protocol, type, string, host_length,
			                               port_begin, (size_t)(port_end - port_begin));
		}
	}
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Address string %$q does not contain a port",
	                       string_length, string);
}


#ifdef AF_BLUETOOTH
PRIVATE int DCALL priv_stobdaddr(char *name, bdaddr_t *bdaddr) {
	unsigned int b0, b1, b2, b3, b4, b5;
	char ch;
	int n;
	n = sscanf(name, "%X:%X:%X:%X:%X:%X%c", &b5, &b4, &b3, &b2, &b1, &b0, &ch);
	if (n == 6 && (b0 | b1 | b2 | b3 | b4 | b5) < 256) {
		bdaddr->b[0] = b0;
		bdaddr->b[1] = b1;
		bdaddr->b[2] = b2;
		bdaddr->b[3] = b3;
		bdaddr->b[4] = b4;
		bdaddr->b[5] = b5;
		return 0;
	}
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid bluetooth address %q",
	                       name);
}
#endif /* AF_BLUETOOTH */

INTERN int DCALL
SockAddr_FromArgv(SockAddr *__restrict self,
                  int family, int protocol, int type,
                  size_t argc, DeeObject *const *argv) {
	DeeObject *arg0;
	(void)protocol; /* Not used most of the time */

	/* Check for simple case: a socket address object was passed. */
	if (argc == 1 && DeeObject_InstanceOf(argv[0], &DeeSockAddr_Type)) {
		arg0 = argv[0];
		if unlikely(family != AF_AUTO &&
			         family != ((DeeSockAddrObject *)arg0)->sa_addr.sa.sa_family)
		{
			DeeError_Throwf(&DeeError_ValueError,
			                "Unexpected Address Family %K (wanted %K, but got %k)",
			                sock_getafnameorid(((DeeSockAddrObject *)arg0)->sa_addr.sa.sa_family),
			                sock_getafnameorid(family), arg0);
			goto err;
		}
		memcpy(self, &((DeeSockAddrObject *)arg0)->sa_addr, sizeof(SockAddr));
		goto done;
	}
	bzero(self, sizeof(SockAddr));
	self->sa.sa_family = (sa_family_t)family;
	switch (family) {

	case AF_AUTO:
#ifdef AF_RDS
	case AF_RDS:
#endif /* AF_RDS */
#ifdef AF_INET
	case AF_INET:
#endif /* AF_INET */
#ifdef AF_INET6
	case AF_INET6:
#endif /* AF_INET6 */
	{
		uint16_t port;
#ifdef AF_INET
		uint32_t host;
		uint8_t a, b, c, d;
#endif /* AF_INET */
		switch (argc) {

		case 1:
			arg0 = argv[0];
			/* (string address) */
			/* (?T2?Dint?Dint address) */
#ifdef AF_INET
			if (DeeTuple_Check(arg0) && DeeTuple_SIZE(arg0) == 2 &&
			    (family == AF_AUTO || family == AF_INET)) {
				if (DeeObject_AsUInt32(DeeTuple_GET(arg0, 0), &host) ||
				    DeeObject_AsUInt16(DeeTuple_GET(arg0, 1), &port))
					goto err;
do_init_inet_hostport:
				self->sa.sa_family            = AF_INET;
				self->sa_inet.sin_addr.s_addr = HTON32(host);
				self->sa_inet.sin_port        = HTON16(port);
				goto done;
			}
#endif /* AF_INET */
do_generic_string:
			arg0 = argv[0];
			if (DeeObject_AssertTypeExact(arg0, &DeeString_Type))
				goto err;
			if (SockAddr_FromString(self, family, protocol, type,
			                        DeeString_STR(arg0),
			                        DeeString_SIZE(arg0)))
				goto err;
			goto done;

		case 2:
do_generic_string_2:
			arg0 = argv[0];
			if (DeeString_Check(argv[1]) || DeeInt_Check(argv[1])) {
				DREF DeeObject *arg2_string;
				int error;
				if (DeeObject_AssertTypeExact(arg0, &DeeString_Type))
					goto err;
				/* Cast to string, so we can also allow integers as second argument */
				arg2_string = DeeObject_Str(argv[1]);
				if unlikely(!arg2_string)
					goto err;
				error = SockAddr_FromStringPort(self, family, protocol, type,
				                                DeeString_STR(arg0), DeeString_SIZE(arg0),
				                                DeeString_STR(arg2_string), DeeString_SIZE(arg2_string));
				Dee_Decref(arg2_string);
				if (error)
					goto err;
			}
			if (DeeObject_AsUInt16(argv[1], &port))
				goto err;
			arg0 = argv[0];
#ifdef AF_INET
			if ((family == AF_AUTO || family == AF_INET) &&
			    !DeeString_Check(arg0)) {
				if (DeeObject_AsUInt32(arg0, &host))
					goto err;
				goto do_init_inet_hostport;
			}
#endif /* AF_INET */
			if (DeeObject_AssertTypeExact(arg0, &DeeString_Type))
				goto err;
			{
				char port_buffer[32];
				Dee_sprintf(port_buffer, "%u", (unsigned)port);
				if (SockAddr_FromStringPort(self, family, protocol, type,
				                            DeeString_STR(arg0), DeeString_SIZE(arg0),
				                            port_buffer, strlen(port_buffer)))
					goto err;
			}
			goto done;

#ifdef AF_INET
		case 5:
			/* (uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port) */
			if (family == AF_AUTO || family == AF_INET) {
				if (DeeObject_AsUInt8(argv[0], &a) ||
				    DeeObject_AsUInt8(argv[1], &b) ||
				    DeeObject_AsUInt8(argv[2], &c) ||
				    DeeObject_AsUInt8(argv[3], &d) ||
				    DeeObject_AsUInt16(argv[4], &port))
					goto err;
				self->sa.sa_family            = AF_INET;
				self->sa_inet.sin_addr.s_addr = SOCKADDRINET(a, b, c, d);
				self->sa_inet.sin_port        = HTON16(port);
				goto done;
			}
			ATTR_FALLTHROUGH
#endif /* AF_INET */

		default:
			DeeError_Throwf(&DeeError_TypeError,
			                "Constructing address family %K requires "
#ifdef AF_INET
			                "1, 2 or 5 "
#elif defined(AF_INET6)
			                "1 or 2 "
#else
			                "1 or 2 "
#endif
			                "arguments, but %Iu were given",
			                sock_getafnameorid(family), argc);
			break;
		}
	}	break;

#ifdef AF_UNIX
	case AF_UNIX:
		if likely(argc != 1) {
			DeeError_Throwf(&DeeError_TypeError,
			                "Constructing address family AF_UNIX requires 1 argument, but %Iu were given",
			                argc);
			goto err;
		}
		arg0 = argv[0];
		if (DeeObject_AssertTypeExact(arg0, &DeeString_Type))
			goto err;
		/* v Note the '>='. That is on purpose as we need 'DeeString_SIZE(arg0) + 1'
		 *   bytes for the string and its terminating \0 character */
		if unlikely(DeeString_SIZE(arg0) >= sizeof(((struct sockaddr_un *)NULL)->sun_path) / sizeof(char)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Given path for 'AF_UNIX' is too long: %r", arg0);
			goto err;
		}
		/* NOTE: No need to copy the NUL-character. -  */
		memcpy(self->sa_un.sun_path,
		       DeeString_STR(arg0),
		       DeeString_SIZE(arg0));
		break;
#endif /* AF_UNIX */

#ifdef AF_NETLINK
	case AF_NETLINK: {
		switch (argc) {

		case 1:
			goto do_generic_string;

		case 2:
			bzero(&self->sa_nl, sizeof(struct sockaddr_nl));
			self->sa_nl.nl_family = AF_NETLINK;
			if (DeeObject_AsUINT(argv[0], &self->sa_nl.nl_pid) ||
			    DeeObject_AsUINT(argv[1], &self->sa_nl.nl_groups))
				goto err;
			break;

		default:
			DeeError_Throwf(&DeeError_TypeError,
			                "Construction address Family AF_NETLINK "
			                "requires 2 arguments, but %Iu were given",
			                argc);
			goto err;
		}
	}	break;
#endif /* AF_NETLINK */

#ifdef AF_BLUETOOTH
	case AF_BLUETOOTH: {
		switch (protocol) {

		case BTPROTO_L2CAP: {
			if unlikely(argc != 2) {
				if (argc == 1)
					goto do_generic_string;
				DeeError_Throwf(&DeeError_TypeError,
				                "Construction address Family AF_BLUETOOTH with protocol "
				                "BTPROTO_L2CAP requires 2 argument, but %Iu were given",
				                argc);
				goto err;
			}
			self->bt_l2.l2_family = AF_BLUETOOTH;
			if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
				goto err;
			if (DeeObject_AsUINT(argv[1], &self->bt_l2.l2_psm))
				goto err;
			if unlikely(priv_stobdaddr(DeeString_STR(argv[0]), &self->bt_l2.l2_bdaddr))
				goto err;
		}	break;

		case BTPROTO_RFCOMM: {
			if unlikely(argc != 2) {
				if (argc == 1)
					goto do_generic_string;
				DeeError_Throwf(&DeeError_TypeError,
				                "Construction address Family AF_BLUETOOTH with protocol "
				                "BTPROTO_RFCOMM requires 2 argument, but %Iu were given",
				                argc);
				goto err;
			}
			self->bt_rc.rc_family = AF_BLUETOOTH;
			if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
				goto err;
			if (DeeObject_AsUINT(argv[1], &self->bt_rc.rc_channel))
				goto err;
			if unlikely(priv_stobdaddr(DeeString_STR(argv[0]), &self->bt_rc.rc_bdaddr))
				goto err;
		}	break;

		case BTPROTO_HCI: {
			if unlikely(argc != 1) {
				if (argc == 2)
					goto do_generic_string_2;
				DeeError_Throwf(&DeeError_TypeError,
				                "Construction address Family AF_BLUETOOTH with protocol "
				                "BTPROTO_HCI requires 1 argument, but %Iu were given",
				                argc);
				goto err;
			}
			self->bt_hci.hci_family = AF_BLUETOOTH;
#if defined(__NetBSD__) || defined(__DragonFly__)
			if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
				goto err;
			if unlikely(priv_stobdaddr(DeeString_StR(argv[0]), &self->bt_hci.hci_bdaddr))
				goto err;
#else /* __NetBSD__ || __DragonFly__ */
			if (DeeObject_AsUINT(argv[0], &self->bt_hci.hci_dev))
				goto err;
#endif /* !__NetBSD__ && !__DragonFly__ */
		}	break;

#if !defined(__FreeBSD__)
		case BTPROTO_SCO: {
			if unlikely(argc != 1) {
				if (argc == 2)
					goto do_generic_string_2;
				DeeError_Throwf(&DeeError_TypeError,
				                "Construction address Family AF_BLUETOOTH with protocol "
				                "BTPROTO_SCO requires 1 argument, but %Iu were given",
				                argc);
				goto err;
			}
			self->bt_sco.sco_family = AF_BLUETOOTH;
			if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
				goto err;
			if unlikely(priv_stobdaddr(DeeString_STR(argv[0]), &self->bt_sco.sco_bdaddr))
				goto err;
		}	break;
#endif /* !__FreeBSD__ */

		default:
			if (argc == 1)
				goto do_generic_string;
			if (argc == 2)
				goto do_generic_string_2;
			DeeError_Throwf(&DeeError_TypeError,
			                "Invalid protocol %K for address family AF_BLUETOOTH",
			                sock_getprotonameorid(protocol));
			goto err;
		}
	}	break;
#endif /* AF_BLUETOOTH */

	default:
		if (argc == 1)
			goto do_generic_string;
		if (argc == 2)
			goto do_generic_string_2;
		DeeError_Throwf(&DeeError_NotImplemented,
		                "Address family %K is not supported",
		                sock_getafnameorid(family));
		goto err;
	}
done:
	DBG_ALIGNMENT_DISABLE();
	return 0;
err:
	DBG_ALIGNMENT_DISABLE();
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_str(DeeSockAddrObject *__restrict self) {
	return SockAddr_ToString(&self->sa_addr, 0, SOCKADDR_STR_FNOFAIL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_repr(DeeSockAddrObject *__restrict self) {
	return DeeString_Newf("sockaddr(%R)",
	                      SockAddr_ToString(&self->sa_addr, 0,
	                                        SOCKADDR_STR_FNOFAIL));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sockaddr_ctor(DeeSockAddrObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	int af_type, result;
	if (!argc) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Expected at least one argument for construction of `sockaddr'");
		goto err;
	}
	if (sock_getafof(argv[0], &af_type))
		goto err;
	result = SockAddr_FromArgv(&self->sa_addr, af_type, 0, 0, argc - 1, argv + 1);
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return -1;
}

PRIVATE struct type_member sockaddr_members[] = {
	TYPE_MEMBER_FIELD_DOC("family", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeSockAddrObject, sa_addr.sa.sa_family),
	                      "The address family of this :sockaddr"),
	TYPE_MEMBER_FIELD_DOC("sa_family", STRUCT_CONST | STRUCT_UINT16_T, offsetof(DeeSockAddrObject, sa_addr.sa.sa_family),
	                      "The address family of this :sockaddr (alias for #family)"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
err_no_such_attribute(DeeSockAddrObject *__restrict self,
                      char const *__restrict name) {
	DeeError_Throwf(&DeeError_AttributeError,
	                "Socket addresses of family %K have no attribute %s",
	                sock_getafnameorid(self->sa_addr.sa.sa_family), name);
	return NULL;
}


#ifdef AF_INET
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet_host(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET)
		return err_no_such_attribute(self, "inet_host");
	return DeeInt_NewU32(NTOH32(self->sa_addr.sa_inet.sin_addr.s_addr));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet_port(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET)
		return err_no_such_attribute(self, "inet_port");
	return DeeInt_NewU16(NTOH16(self->sa_addr.sa_inet.sin_port));
}
#endif /* AF_INET */

#ifdef AF_INET6
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet6_port(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET6)
		return err_no_such_attribute(self, "inet6_port");
	return DeeInt_NewU16(NTOH16(self->sa_addr.sa_inet6.sin6_port));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet6_host(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET6)
		return err_no_such_attribute(self, "inet6_host");
	return DeeInt_NewU128(*(duint128_t *)&self->sa_addr.sa_inet6.sin6_addr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet6_flowinfo(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET6)
		return err_no_such_attribute(self, "inet6_flowinfo");
	return DeeInt_NewU32(self->sa_addr.sa_inet6.sin6_flowinfo);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sockaddr_inet6_scope_id(DeeSockAddrObject *__restrict self) {
	if (self->sa_addr.sa.sa_family != AF_INET6)
		return err_no_such_attribute(self, "inet6_scope_id");
	return DeeInt_NewU32(self->sa_addr.sa_inet6.sin6_scope_id);
}
#endif /* AF_INET6 */

PRIVATE struct type_getset sockaddr_getsets[] = {
#ifdef AF_INET
	{ "inet_host", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet_host, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET\": The host address in host endian") },
	{ "inet_port", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet_port, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET\": The port number in host endian") },
#endif /* AF_INET */
#ifdef AF_INET6
	{ "inet6_port", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet6_port, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET6\": The port number in host endian") },
	{ "inet6_host", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet6_host, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET6\": The host address") },
	{ "inet6_flowinfo", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet6_flowinfo, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET6\": The IPv6 flow identifier") },
	{ "inet6_scope_id", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_inet6_scope_id, NULL, NULL,
	  DOC("->?Dint\nFor $\"AF_INET6\": The IPv6 scope id") },
#endif /* AF_INET6 */
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
sockaddr_hash(DeeSockAddrObject *__restrict self) {
	return Dee_HashPtr(&self->sa_addr, SockAddr_Sizeof(self->sa_addr.sa.sa_family, 0));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sockaddr_eq(DeeSockAddrObject *self,
            DeeSockAddrObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, &DeeSockAddr_Type))
		return NULL;
	return_bool(memcmp(&self->sa_addr, &other->sa_addr,
	                   SockAddr_Sizeof(self->sa_addr.sa.sa_family, 0)) == 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sockaddr_ne(DeeSockAddrObject *self,
            DeeSockAddrObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, &DeeSockAddr_Type))
		return NULL;
	return_bool(memcmp(&self->sa_addr, &other->sa_addr,
	                   SockAddr_Sizeof(self->sa_addr.sa.sa_family, 0)) != 0);
}

PRIVATE struct type_cmp sockaddr_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&sockaddr_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sockaddr_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sockaddr_ne
};

INTERN DeeTypeObject DeeSockAddr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "sockaddr",
#ifdef CONFIG_NO_DOC
	/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
	/* .tp_doc      = */ "(family:?X2?Dstring?Dint,args!)\n"
	                     "@param family The family of which this is a socket address. You may pass ?N, an empty "
	                                   "string, $\"AF_AUTO\", or ${-1} to determine the proper family automatically\n"
	                     "Create a new socket address. @args is documented below and depends on @family\n"
	                     "Note that many functions from :socket also accept @args as arguments to various functions\n"
	                     "\n"
	                     "(args!,other:?Dsockaddr)\n"
	                     "@throw ValueError The @other socket uses a ?#family incompatible to the previously specified family\n"
	                     "@param other The @other socket address that should be duplicated\n"
	                     "Always accepted is another socket address that is duplicated\n"
	                     "\n"
	                     "(args!,host_and_port:?Dstring)\n"
	                     "(args!,host:?Dstring,port:?Dstring)\n"
	                     "(args!,host:?Dstring,port:?Dint)\n"
	                     "@throw NetError.HostNotFound.NoHostAddress The @host exists, but offers no address matching the required family\n"
	                     "@throw NetError.HostNotFound Failed to find the host described by @host_and_port or @host and @port\n"
	                     "@throw NetError An unknown error caused the hostname lookup to fail\n"
	                     "@param host_and_port The host and port separated by a $\":\" character\n"
	                     "Always accepted is a host+port argument part encoded in strings\n"
#if defined(AF_INET) || defined(AF_INET6)
	                     "Note that this is the preferred way of encoding "
#ifdef AF_INET
	                     "IPv4 "
#ifdef AF_INET6
	                     "and IPv6 "
#endif /* AF_INET6 */
#else /* AF_INET */
	                     "IPv6 "
#endif /* !AF_INET */
	                     "addresses\n"
#ifdef AF_INET
	                     "The following special names are recognized as case-insensitive IPv4 host names, "
	                     "following an optional prefix `INADDR', as well as any number of underscores:\n"
	                     "#T{Name|Value~"
	                     "$\"ANY\"|$\"0.0.0.0\"&"
	                     "$\"LOCALHOST\"|$\"127.0.0.1\"&"
	                     "$\"LOOPBACK\"|$\"127.0.0.1\"&"
	                     "$\"BROADCAST\"|$\"255.255.255.255\"&"
	                     "$\"NONE\"|$\"255.255.255.255\"&"
	                     "$\"UNSPEC_GROUP\"|$\"14.0.0.0\"&"
	                     "$\"ALLHOSTS_GROUP\"|$\"14.0.0.1\"&"
	                     "$\"MAX_LOCAL_GROUP\"|$\"14.0.0.255\"}\n"
#endif /* AF_INET */
#ifdef AF_INET6
#ifdef AF_INET
	                     "Similarly, the following special IPv6 host names are recognized:\n"
#else /* AF_INET */
	                     "The following special names are recognized as case-insensitive IPv6 host names, "
	                     "following an optional prefix $\"IN6ADDR\", as well as any number of underscores:\n"
#endif /* !AF_INET */
	                     "#T{Name|Value~"
	                     "$\"ANY\"|$\"::0\"&"
	                     "$\"LOCALHOST\"|$\"::1\"&"
	                     "$\"LOOPBACK\"|$\"::1\"}\n"
#endif /* AF_INET6 */
#endif /* AF_INET || AF_INET6 */
	                     "\n"
#ifdef AF_INET
	                     "(family=!PAF_INET,host_and_port:?T2?Dint?Dint)\n"
	                     "(family=!PAF_AUTO,host_and_port:?T2?Dint?Dint)\n"
	                     "(family=!PAF_INET,host:?Dint,port:?Dint)\n"
	                     "(family=!PAF_AUTO,host:?Dint,port:?Dint)\n"
	                     "Create an IPv4 address using a @host and @port data pair\n"
	                     "\n"
	                     "(family=!PAF_INET,a:?Dint,b:?Dint,c:?Dint,d:?Dint,port:?Dint)\n"
	                     "(family=!PAF_AUTO,a:?Dint,b:?Dint,c:?Dint,d:?Dint,port:?Dint)\n"
	                     "Create an IPv4 address by combining the 4 single-byte integers @a, @b, @c and @d alongside @port\n"
	                     "\n"
#endif /* AF_INET */
#ifdef AF_UNIX
	                     "(family=!PAF_UNIX,path:?Dstring)\n"
	                     "@throw ValueError The given @path is too long\n"
	                     "Create a unix-pipe socket address using the given @path\n"
	                     "\n"
#endif /* AF_UNIX */
#ifdef AF_NETLINK
	                     "(family=!PAF_NETLINK,pid:?Dint,groups:?Dint)\n"
	                     "\n"
#endif /* AF_NETLINK */
	                     "str->\n"
	                     "@throw \n"
	                     "Generates and returns a string representation of @this sockaddr\n"
	                     ,
#endif /* !CONFIG_NO_DOC */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&sockaddr_ctor,
				TYPE_FIXED_ALLOCATOR(DeeSockAddrObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sockaddr_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sockaddr_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL, /* XXX: Buffer interface to access the raw sockaddr data? */
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sockaddr_getsets,
	/* .tp_members       = */ sockaddr_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END


#endif /* !GUARD_DEX_SOCKET_SOCKADDR_C */
