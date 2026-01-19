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
#ifndef GUARD_DEX_SOCKET_LIBNET_H
#define GUARD_DEX_SOCKET_LIBNET_H 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <deemon/api.h>

#ifdef CONFIG_HOST_WINDOWS

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif /* _MSC_VER */
#endif /* CONFIG_HOST_WINDOWS */

#include <deemon/system-features.h>
#include <deemon/system.h>

#ifndef CONFIG_HOST_WINDOWS
#ifdef CONFIG_HOST_UNIX
#ifdef CONFIG_HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* CONFIG_HAVE_SYS_SELECT_H */

#ifdef CONFIG_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* CONFIG_HAVE_SYS_SOCKET_H */

#ifdef CONFIG_HAVE_NETDB_H
#include <netdb.h>
#endif /* CONFIG_HAVE_NETDB_H */
#endif /* CONFIG_HOST_UNIX */
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#if (defined(CONFIG_HAVE_BLUETOOTH_BLUETOOTH_H) && \
     defined(CONFIG_HAVE_BLUETOOTH_RFCOMM_H) && \
     defined(CONFIG_HAVE_BLUETOOTH_L2CAP_H) && \
     defined(CONFIG_HAVE_BLUETOOTH_SCO_H) && \
     defined(CONFIG_HAVE_BLUETOOTH_HCI_H))
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sco.h>
#include <bluetooth/hci.h>
#else /* ... */
#undef AF_BLUETOOTH
#endif /* !... */

#ifdef CONFIG_HAVE_BLUETOOTH_H
#include <bluetooth.h>
#endif /* CONFIG_HAVE_BLUETOOTH_H */

#ifdef CONFIG_HAVE_LINUX_NETLINK_H
#ifdef CONFIG_HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif /* CONFIG_HAVE_ASM_TYPES_H */
#include <linux/netlink.h>
#else /* CONFIG_HAVE_LINUX_NETLINK_H */
#undef AF_NETLINK
#endif /* !CONFIG_HAVE_LINUX_NETLINK_H */

#ifdef CONFIG_HAVE_SYS_UN_H
#ifdef CONFIG_HOST_WINDOWS
/* Under certain cygwin configurations, stuff from <sys/un.h>
 * breaks due to prior definitions from <WinSock2.h> */
#define sockaddr         unix_sockaddr
#define sockaddr_storage unix_sockaddr_storage
#define linger           unix_linger
#include <sys/un.h>
#undef sockaddr
#undef sockaddr_storage
#undef linger
#else /* CONFIG_HOST_WINDOWS */
#include <sys/un.h>
#endif /* !CONFIG_HOST_WINDOWS */
#else /* CONFIG_HAVE_SYS_UN_H */
#ifndef CONFIG_HOST_WINDOWS
#undef AF_UNIX
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_HAVE_SYS_UN_H */

#include <deemon/format.h>
#include <deemon/types.h>
#include <deemon/util/lock.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>

#include <stdint.h> /* uint16_t, uint64_t, uintptr_t */

#ifdef CONFIG_HOST_WINDOWS
#undef EWOULDBLOCK
#undef EINPROGRESS
#undef EALREADY
#undef ENOTSOCK
#undef EDESTADDRREQ
#undef EMSGSIZE
#undef EPROTOTYPE
#undef ENOPROTOOPT
#undef EPROTONOSUPPORT
#undef ESOCKTNOSUPPORT
#undef EOPNOTSUPP
#undef EPFNOSUPPORT
#undef EAFNOSUPPORT
#undef EADDRINUSE
#undef EADDRNOTAVAIL
#undef ENETDOWN
#undef ENETUNREACH
#undef ENETRESET
#undef ECONNABORTED
#undef ECONNRESET
#undef ENOBUFS
#undef EISCONN
#undef ENOTCONN
#undef ESHUTDOWN
#undef ETOOMANYREFS
#undef ETIMEDOUT
#undef ECONNREFUSED
#undef ELOOP
#undef ENAMETOOLONG
#undef EHOSTDOWN
#undef EHOSTUNREACH
#undef ENOTEMPTY
#undef EPROCLIM
#undef EUSERS
#undef EDQUOT
#undef ESTALE
#undef EREMOTE
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
/* Additional errors. */
#undef EINVAL
#undef EBADF
#define EINVAL                  WSAEINVAL
#define EBADF                   WSAEBADF
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_WINDOWS
#ifndef IPPROTO_HOPOPTS
#define IPPROTO_HOPOPTS       0
#endif /* !IPPROTO_HOPOPTS */
#ifndef IPPROTO_IGMP
#define IPPROTO_IGMP          2
#endif /* !IPPROTO_IGMP */
#ifndef IPPROTO_GGP
#define IPPROTO_GGP           3
#endif /* !IPPROTO_GGP */
#ifndef IPPROTO_IPV4
#define IPPROTO_IPV4          4
#endif /* !IPPROTO_IPV4 */
#ifndef IPPROTO_ST
#define IPPROTO_ST            5
#endif /* !IPPROTO_ST */
#ifndef IPPROTO_CBT
#define IPPROTO_CBT           7
#endif /* !IPPROTO_CBT */
#ifndef IPPROTO_EGP
#define IPPROTO_EGP           8
#endif /* !IPPROTO_EGP */
#ifndef IPPROTO_IGP
#define IPPROTO_IGP           9
#endif /* !IPPROTO_IGP */
#ifndef IPPROTO_PUP
#define IPPROTO_PUP           12
#endif /* !IPPROTO_PUP */
#ifndef IPPROTO_IDP
#define IPPROTO_IDP           22
#endif /* !IPPROTO_IDP */
#ifndef IPPROTO_RDP
#define IPPROTO_RDP           27
#endif /* !IPPROTO_RDP */
#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6          41
#endif /* !IPPROTO_IPV6 */
#ifndef IPPROTO_ROUTING
#define IPPROTO_ROUTING       43
#endif /* !IPPROTO_ROUTING */
#ifndef IPPROTO_FRAGMENT
#define IPPROTO_FRAGMENT      44
#endif /* !IPPROTO_FRAGMENT */
#ifndef IPPROTO_ESP
#define IPPROTO_ESP           50
#endif /* !IPPROTO_ESP */
#ifndef IPPROTO_AH
#define IPPROTO_AH            51
#endif /* !IPPROTO_AH */
#ifndef IPPROTO_ICMPV6
#define IPPROTO_ICMPV6        58
#endif /* !IPPROTO_ICMPV6 */
#ifndef IPPROTO_NONE
#define IPPROTO_NONE          59
#endif /* !IPPROTO_NONE */
#ifndef IPPROTO_DSTOPTS
#define IPPROTO_DSTOPTS       60
#endif /* !IPPROTO_DSTOPTS */
#ifndef IPPROTO_ND
#define IPPROTO_ND            77
#endif /* !IPPROTO_ND */
#ifndef IPPROTO_ICLFXBM
#define IPPROTO_ICLFXBM       78
#endif /* !IPPROTO_ICLFXBM */
#ifndef IPPROTO_PIM
#define IPPROTO_PIM           103
#endif /* !IPPROTO_PIM */
#ifndef IPPROTO_PGM
#define IPPROTO_PGM           113
#endif /* !IPPROTO_PGM */
#ifndef IPPROTO_L2TP
#define IPPROTO_L2TP          115
#endif /* !IPPROTO_L2TP */
#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP          132
#endif /* !IPPROTO_SCTP */

#ifndef MSG_OOB
#define MSG_OOB  0x1
#endif /* !MSG_OOB */
#ifndef MSG_PEEK
#define MSG_PEEK 0x2
#endif /* !MSG_PEEK */
#ifndef MSG_DONTROUTE
#define MSG_DONTROUTE 0x4
#endif /* !MSG_DONTROUTE */
#ifndef MSG_WAITALL
#define MSG_WAITALL 0x8
#endif /* !MSG_WAITALL */
#ifndef MSG_PUSH_IMMEDIATE
#define MSG_PUSH_IMMEDIATE 0x20
#endif /* !MSG_PUSH_IMMEDIATE */
#ifndef MSG_PARTIAL
#define MSG_PARTIAL 0x8000
#endif /* !MSG_PARTIAL */
#ifndef MSG_INTERRUPT
#define MSG_INTERRUPT 0x10
#endif /* !MSG_INTERRUPT */
#endif /* !CONFIG_HOST_WINDOWS */



#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif /* !INADDR_ANY */

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK NTOH32_C(LOCALHOST)
#endif /* !INADDR_LOOPBACK */

#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST 0xffffffff
#endif /* !INADDR_BROADCAST */

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* !INADDR_NONE */

#ifndef INADDR_UNSPEC_GROUP
#define INADDR_UNSPEC_GROUP 0xe0000000
#endif /* !INADDR_UNSPEC_GROUP */

#ifndef INADDR_ALLHOSTS_GROUP
#define INADDR_ALLHOSTS_GROUP 0xe0000001
#endif /* !INADDR_ALLHOSTS_GROUP */

#ifndef INADDR_MAX_LOCAL_GROUP
#define INADDR_MAX_LOCAL_GROUP 0xe00000ff
#endif /* !INADDR_MAX_LOCAL_GROUP */

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT { 0 }
#endif /* !IN6ADDR_ANY_INIT */

#ifndef IN6ADDR_LOOPBACK_INIT
#define IN6ADDR_LOOPBACK_INIT { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
#endif /* !IN6ADDR_LOOPBACK_INIT */



#ifndef IPPORT_RESERVED
#define IPPORT_RESERVED 1024
#endif /* !IPPORT_RESERVED */

#ifndef IPPORT_USERRESERVED
#define IPPORT_USERRESERVED 5000
#endif /* !IPPORT_USERRESERVED */

#ifndef IPPROTO_IP
#define IPPROTO_IP 0
#endif /* !IPPROTO_IP */

#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP 1
#endif /* !IPPROTO_ICMP */

#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif /* !IPPROTO_TCP */

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif /* !IPPROTO_UDP */

#ifndef IPPROTO_RAW
#define IPPROTO_RAW 255
#endif /* !IPPROTO_RAW */



#ifdef AF_BLUETOOTH
#if defined(__FreeBSD__)
#define BTPROTO_HCI                   BLUETOOTH_PROTO_HCI
#define BTPROTO_L2CAP                 BLUETOOTH_PROTO_L2CAP
#define BTPROTO_RFCOMM                BLUETOOTH_PROTO_RFCOMM
#define HCI_FILTER                    SO_HCI_RAW_FILTER
#define SOL_HCI                       SOL_HCI_RAW
#define hci_dev                       hci_node
// -- sockaddr_rc --
#define sockaddr_rc                   sockaddr_rfcomm
#define rc_family                     rfcomm_family
#define rc_bdaddr                     rfcomm_bdaddr
#define rc_channel                    rfcomm_channel
// -- sockaddr_l2 --
#define sockaddr_l2                   sockaddr_l2cap
#define l2_family                     l2cap_family
#define l2_psm                        l2cap_psm
#define l2_bdaddr                     l2cap_bdaddr
#define l2_cid                        l2cap_cid
#define l2_bdaddr_type                l2cap_bdaddr_type
#elif defined(__NetBSD__) || defined(__DragonFly__)
#define HCI_DATA_DIR                  SO_HCI_DIRECTION
#define SOL_HCI                       BTPROTO_HCI
// -- sockaddr_hci --
#define sockaddr_hci                  sockaddr_bt
#define hci_family                    bt_family
#define hci_dev                       bt_dev
#define hci_channel                   bt_channel
// -- sockaddr_sco --
#define sco_family                    bt_family
#define sco_bdaddr                    bt_bdaddr
// -- sockaddr_rc --
#define sockaddr_rc                   sockaddr_bt
#define rc_family                     bt_family
#define rc_bdaddr                     bt_bdaddr
#define rc_channel                    bt_channel
// -- sockaddr_l2 --
#define sockaddr_l2                   sockaddr_bt
#define l2_family                     bt_family
#define l2_psm                        bt_psm
#define l2_bdaddr                     bt_bdaddr
#define l2_cid                        bt_cid
#define l2_bdaddr_type                bt_bdaddr_type
#endif
#endif /* AF_BLUETOOTH */

DECL_BEGIN


#ifndef NET_ENDIAN
#define NET_ENDIAN 4321 /* Big endian */
#endif /* !NET_ENDIAN */

#if __BYTE_ORDER__ == NET_ENDIAN
#define SOCKADDRINET(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))
#define INET_HOSTNAME_A(x) (((x)&0xff000000) >> 24)
#define INET_HOSTNAME_B(x) (((x)&0x00ff0000) >> 16)
#define INET_HOSTNAME_C(x) (((x)&0x0000ff00) >> 8)
#define INET_HOSTNAME_D(x)  ((x)&0x000000ff)
#define LOCALHOST DEE_UINT32_C(0x7f000001) // 127.0.0.1 (in network endian)
#define HTON16   /* nothing */
#define HTON16_C /* nothing */
#define HTON32   /* nothing */
#define HTON32_C /* nothing */
#define HTON64   /* nothing */
#define HTON64_C /* nothing */
#else /* __BYTE_ORDER__ == NET_ENDIAN */
#define SOCKADDRINET(a, b, c, d) ((a) | (b) << 8 | (c) << 16 | (d) << 24)
#define INET_HOSTNAME_A(x)  ((x)&0x000000ff)
#define INET_HOSTNAME_B(x) (((x)&0x0000ff00) >> 8)
#define INET_HOSTNAME_C(x) (((x)&0x00ff0000) >> 16)
#define INET_HOSTNAME_D(x) (((x)&0xff000000) >> 24)
#define LOCALHOST 0x0100007f // 127.0.0.1 (in network endian)
#define HTON16    BSWAP16
#define HTON16_C  BSWAP16_C
#define HTON32    BSWAP32
#define HTON32_C  BSWAP32_C
#define HTON64    BSWAP64
#define HTON64_C  BSWAP64_C
#endif /* __BYTE_ORDER__ != NET_ENDIAN */

#define NTOH16    HTON16
#define NTOH16_C  HTON16_C
#define NTOH32    HTON32
#define NTOH32_C  HTON32_C
#define NTOH64    HTON64
#define NTOH64_C  HTON64_C

STATIC_ASSERT(LOCALHOST == SOCKADDRINET(127, 0, 0, 1));

#ifndef CONFIG_HOST_WINDOWS
#define closesocket    close
#define INVALID_SOCKET (-1)
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_WINDOWS
#define SOCK_FMT  "%" PRFuSIZ
typedef uintptr_t SocketHandleType;
#else /* CONFIG_HOST_WINDOWS */
#define SOCK_FMT  "%d"
typedef int       SocketHandleType;
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_WINDOWS
typedef u_short  sa_family_t;
typedef uint16_t in_port_t;
#endif /* CONFIG_HOST_WINDOWS */

#ifdef __INTELLISENSE__
typedef SocketHandleType sock_t;
#else /* __INTELLISENSE__ */
#undef sock_t
#define sock_t  SocketHandleType
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_HOST_WINDOWS
/* Dunno why the windows headers didn't already do this... */
#undef SHUT_RD
#undef SHUT_WR
#undef SHUT_RDWR
#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH
#if defined(AF_UNIX) && !defined(CONFIG_HOST_UNIX)
struct sockaddr_un {
	sa_family_t sun_family;    /* AF_UNIX */
	char        sun_path[108]; /* pathname */
};
#endif /* AF_UNIX && !CONFIG_HOST_UNIX */
#endif /* CONFIG_HOST_WINDOWS */

#if (defined(AF_INET6) &&     \
     defined(IPPROTO_IPV6) && \
     defined(IPV6_V6ONLY))
#define DEE_HAVE_IPv6_DUALSTACK 1
#else /* ... */
#define DEE_HAVE_IPv6_DUALSTACK 0
#endif /* !... */

#ifndef SHUT_RD
#define SHUT_RD   0
#endif /* !SHUT_RD */
#ifndef SHUT_WR
#define SHUT_WR   1
#endif /* !SHUT_WR */
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif /* !SHUT_RDWR */

#ifdef CONFIG_HOST_WINDOWS
typedef DWORD              neterrno_t;
#define GET_NET_ERROR()    (neterrno_t)WSAGetLastError()
#define SET_NET_ERROR(x)   WSASetLastError((int)(x))
#define DeeNet_ThrowErrorf DeeNTSystem_ThrowErrorf
#endif /* CONFIG_HOST_WINDOWS */

#ifndef GET_NET_ERROR
typedef int                neterrno_t;
#define GET_NET_ERROR()    DeeSystem_GetErrno()
#define SET_NET_ERROR(x)   DeeSystem_SetErrno(x)
#define DeeNet_ThrowErrorf DeeUnixSystem_ThrowErrorf
#endif /* !GET_NET_ERROR */


/* Accepted by libnet: Automatically determine
 * the address family from other arguments. */
#define AF_AUTO (-1)


typedef union {
	struct sockaddr         sa;
#ifdef AF_INET
	struct sockaddr_in      sa_inet;
#endif /* !AF_INET */
#ifdef AF_INET6
	struct sockaddr_in6     sa_inet6;
	struct sockaddr_storage storage; 
#endif/* !AF_INET6 */
#ifdef AF_UNIX
	struct sockaddr_un      sa_un;
#endif /* !AF_UNIX */
#ifdef AF_NETLINK
	struct sockaddr_nl      sa_nl;
#endif /* !AF_NETLINK */
#ifdef AF_BLUETOOTH
	struct sockaddr_l2      bt_l2;
	struct sockaddr_rc      bt_rc;
	struct sockaddr_sco     bt_sco;
	struct sockaddr_hci     bt_hci;
#endif /* !AF_BLUETOOTH */
} SockAddr;

#define SockAddr_FAMILY(ob) ((ob)->sa.sa_family)
#ifdef AF_INET
#define SockAddr_INET_HOST(ob) ((ob)->sa_inet.sin_addr.s_addr)
#define SockAddr_INET_PORT(ob) ((ob)->sa_inet.sin_port)
#endif /* AF_INET */
#ifdef AF_INET6
#define SockAddr_INET6_HOST(ob) ((DeeINet6Host *)(ob)->sa_inet6.sin6_addr.s6_addr)
#define SockAddr_INET6_PORT(ob) ((ob)->sa_inet6.sin6_port)
#endif /* AF_INET6 */
#ifdef AF_UNIX
#define SockAddr_UNIX_PATH(ob) ((ob)->sa_un.sun_path)
#endif /* AF_UNIX */
#ifdef AF_NETLINK
#define SockAddr_NETLINK_PID(ob)    ((ob)->sa_nl.nl_pid)
#define SockAddr_NETLINK_GROUPS(ob) ((ob)->sa_nl.nl_groups)
#endif /* AF_NETLINK */

/* Wrapper around `gethostbyaddr()'
 * @param flags: Set of `SOCKADDR_STR_F*' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sock_gethostbyaddr(void const *__restrict data, socklen_t datalen,
                   sa_family_t family, int flags);

/* @param flags: Set of `SOCKADDR_STR_F*' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SockAddr_ToString(SockAddr const *__restrict self, int protocol, int flags);
#define SOCKADDR_STR_FNORMAL 0x0000 /* Normal flags:?Dstring. */
#define SOCKADDR_STR_FNOFAIL 0x0001 /* Don't return with an error if DNS lookup failed. */
#define SOCKADDR_STR_FNODNS  0x0002 /* Don't use DNS names in the string (use raw IPs instead). */
#define SOCKADDR_STR_FNOPORT 0x0004 /* Don't include port numbers in address strings. */


/* Returns the effective size of the given socket address when used with `protocol'. */
INTDEF socklen_t DCALL SockAddr_Sizeof(sa_family_t family, int protocol);

/* Initialize a generic socket address from an argument vector.
 * HINT: `family' may be set to `AF_AUTO' for automatic deduction. */
INTDEF WUNUSED NONNULL((1)) int DCALL
SockAddr_FromArgv(SockAddr *__restrict self,
                  int family, int protocol, int type,
                  size_t argc, DeeObject *const *argv);

typedef struct socket_object DeeSocketObject;
typedef struct sockaddr_object DeeSockAddrObject;


/* Address family / socket type / socket protocol database access. */
INTDEF WUNUSED DREF DeeObject *DCALL sock_getafname(int value); /* @return: ITER_DONE: Not found. @return: NULL: Error. */
INTDEF WUNUSED DREF DeeObject *DCALL sock_gettypename(int value); /* @return: ITER_DONE: Not found. @return: NULL: Error. */
INTDEF WUNUSED DREF DeeObject *DCALL sock_getprotoname(int value); /* @return: ITER_DONE: Not found. @return: NULL: Error. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
sock_getafvalue(char const *__restrict name,
                int *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
sock_gettypevalue(char const *__restrict name,
                  int *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
sock_getprotovalue(char const *__restrict name,
                   int *__restrict p_result);

/* Same as the functions above, but return an int-object for
 * `value' when the database doesn't recognize the value. */
INTDEF WUNUSED DREF DeeObject *DCALL sock_getafnameorid(int value);
INTDEF WUNUSED DREF DeeObject *DCALL sock_gettypenameorid(int value);
INTDEF WUNUSED DREF DeeObject *DCALL sock_getprotonameorid(int value);

/* Cast integer to direct numbers or search the database for strings.
 * When not found, throw an error and return -1.
 * NOTE: When `none' is passed, these functions return the following:
 *    - sock_getafof:    AF_AUTO
 *    - sock_gettypeof:  SOCK_STREAM
 *    - sock_getprotoof: 0 */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sock_getafof(DeeObject *__restrict name,
             int *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sock_gettypeof(DeeObject *__restrict name,
               int *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sock_getprotoof(DeeObject *__restrict name,
                int *__restrict p_result);


/* Translate the MSG_* flags used by send/recv functions. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sock_getmsgflagsof(DeeObject *__restrict name, int *__restrict p_result);
INTDEF WUNUSED DREF DeeObject *DCALL
sock_getmsgflagsnameorid(int flags);

#if 0
#define SOCKET_HAVE_CONFIGURE_SENDRECV
#endif

struct socket_object {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t s_lock;     /* Lock for this socket. */
#endif /* !CONFIG_NO_THREADS */
	sock_t              s_socket;   /* [lock(s_lock)] System-specific socket handle.
	                                 * NOTE: Set to `INVALID_SOCKET' when closed. */
	SockAddr            s_sockaddr; /* [const(.sa.sa_family)][lock(s_lock)]
	                                 * Local socket address (family is the `af' constructor argument) */
	SockAddr            s_peeraddr; /* [const(.sa.sa_family)][lock(s_lock)]
	                                 * Local socket address (family is the `af' constructor argument) */
	int                 s_type;     /* [const] Socket type (`type' constructor argument). */
	int                 s_proto;    /* [const] Socket protocol (`proto' constructor argument). */
	uint16_t            s_state;    /* [lock(s_lock) + ATOMIC] Socket state (Set of `SOCKET_F*') */
#define SOCKET_FNORMAL         0x0000 /* Normal flags. */
#define SOCKET_FBINDING        0x0001 /* Socket is current being bound. */
#define SOCKET_FBOUND          0x0002 /* Socket was bound. */
#define SOCKET_FCONNECTING     0x0004 /* Internally set while a socket is connecting. */
#define SOCKET_FCONNECTED      0x0008 /* Socket was connected. */
#define SOCKET_FSTARTLISTENING 0x0010 /* Socket is starting to listen. */
#define SOCKET_FLISTENING      0x0020 /* Socket is listening (SO_ACCEPTCONN). */
#define SOCKET_FSHUTDOWN_R     0x0040 /* Socket was shut down for reading. */
#define SOCKET_FSHUTDOWN_W     0x0080 /* Socket was shut down for writing. */
#ifdef SOCKET_HAVE_CONFIGURE_SENDRECV
#define SOCKET_FRECVCONFOK     0x0100 /* The recv timeout has been configured appropriately. */
#define SOCKET_FSENDCONFOK     0x0200 /* The send timeout has been configured appropriately. */
#endif /* SOCKET_HAVE_CONFIGURE_SENDRECV */
#define SOCKET_FHASSOCKADDR    0x1000 /* The socket's `s_sockaddr' field has been initialized. */
#define SOCKET_FHASPEERADDR    0x2000 /* The socket's `s_peeraddr' field has been initialized. */
#define SOCKET_FOPENED         0x4000 /* Socket hasn't been closed (yet). */
#define SOCKET_FSHUTTINGDOWN   0x8000 /* Socket is being shut down. */
};
INTDEF DeeTypeObject DeeSocket_Type;

#define socket_reading(x)       Dee_atomic_rwlock_reading(&(x)->s_lock)
#define socket_writing(x)       Dee_atomic_rwlock_writing(&(x)->s_lock)
#define socket_tryread(self)    Dee_atomic_rwlock_tryread(&(self)->s_lock)
#define socket_trywrite(self)   Dee_atomic_rwlock_trywrite(&(self)->s_lock)
#define socket_read(self)       Dee_atomic_rwlock_read(&(self)->s_lock)
#define socket_write(self)      Dee_atomic_rwlock_write(&(self)->s_lock)
#define socket_tryupgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->s_lock)
#define socket_upgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->s_lock)
#define socket_downgrade(self)  Dee_atomic_rwlock_downgrade(&(self)->s_lock)
#define socket_endwrite(self)   Dee_atomic_rwlock_endwrite(&(self)->s_lock)
#define socket_endread(self)    Dee_atomic_rwlock_endread(&(self)->s_lock)
#define socket_end(self)        Dee_atomic_rwlock_end(&(self)->s_lock)


/* Try to retrieve the local (sock) / remote (peer) address of a given socket.
 * @param: throw_error: When true, throw an error describing the reason before returning `-1'. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_GetSockName(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result, bool throw_error);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_GetPeerAddr(DeeSocketObject *__restrict self,
                      SockAddr *__restrict result, bool throw_error);

/* Bind/Connect commands. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_Bind(DeeSocketObject *__restrict self,
               SockAddr const *__restrict addr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSocket_Connect(DeeSocketObject *__restrict self,
                  SockAddr const *__restrict addr);
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeSocket_Listen(DeeSocketObject *__restrict self, int max_backlog);

/* Accept a new connection.
 * @return: -1: An error occurred.
 * @return:  0: Successfully accepted a new connection.
 * @return:  1: Timed out.
 * @param: timeout_nanoseconds: The timeout (in nanoseconds).
 *                              Set to `0' for try-accept; Set to (uint64_t)-1 to never time out. */
INTDEF NONNULL((1, 3, 4)) int DCALL
DeeSocket_Accept(DeeSocketObject *__restrict self,
                 uint64_t timeout_nanoseconds,
                 sock_t *__restrict sock_fd,
                 SockAddr *__restrict addr);

/* Send/Receive data. `timeout_nanoseconds' behaves the same as for `DeeSocket_Accept()'.
 * @return: * : The number of received bytes.
 * @return: -1: An error occurred.
 * @return: -2: The given `timeout_nanoseconds' has expired. */
INTDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeSocket_Recv(DeeSocketObject *__restrict self,
               uint64_t timeout_nanoseconds,
               void *__restrict buf, size_t bufsize,
               int flags);
INTDEF WUNUSED NONNULL((1, 3, 6)) Dee_ssize_t DCALL
DeeSocket_RecvFrom(DeeSocketObject *__restrict self,
                   uint64_t timeout_nanoseconds,
                   void *__restrict buf, size_t bufsize,
                   int flags, SockAddr *__restrict source);
INTDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeSocket_Send(DeeSocketObject *__restrict self,
               uint64_t timeout_nanoseconds,
               void const *__restrict buf, size_t bufsize,
               int flags);
INTDEF WUNUSED NONNULL((1, 3, 6)) Dee_ssize_t DCALL
DeeSocket_SendTo(DeeSocketObject *__restrict self,
                 uint64_t timeout_nanoseconds,
                 void const *__restrict buf, size_t bufsize,
                 int flags, SockAddr const *__restrict target);

/* Receive data from the given source, or the bound peer (when `source' is NULL)
 * NOTE: When the given `timeout_nanoseconds' has expired, `ITER_DONE' is returned. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSocket_RecvData(DeeSocketObject *__restrict self,
                   uint64_t timeout_nanoseconds,
                   size_t max_bufsize, int flags,
                   SockAddr *source);

/* Convert a given object to message flags (Set of `MSG_*') */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
sock_getmsgflagsof(DeeObject *__restrict name,
                  int *__restrict p_result);
INTDEF WUNUSED DREF DeeObject *DCALL
sock_getmsgflagsnameorid(int flags);



/* Translate the given string into an options mode for shutdown.
 * Accepted values are (all case-insensitive with an
 * optional `SHUT' prefix and any number of `_' removed):
 *   - "R":         SHUT_RD
 *   - "W":         SHUT_WR
 *   - "RW":        SHUT_RDWR
 *   - "WR":        SHUT_RDWR
 *   - "RD":        SHUT_RD
 *   - "WR":        SHUT_WR
 *   - "RDRW":      SHUT_RDWR
 *   - "WRRD":      SHUT_RDWR
 *   - "READ":      SHUT_RD
 *   - "WRITE":     SHUT_WR
 *   - "READWRITE": SHUT_RDWR
 *   - "WRITEREAD": SHUT_RDWR
 * When the given mode is not recognized, an
 * `Error.ValueError' is thrown and -1 is returned. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
get_shutdown_mode(char const *__restrict mode, int *__restrict p_result);

/* Same as `get_shutdown_mode', but interpret strings
 * and convert everything else to an integer. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
get_shutdown_modeof(DeeObject *__restrict mode, int *__restrict p_result);



struct sockaddr_object {
	OBJECT_HEAD
	SockAddr    sa_addr; /* [const] The associated socket address. */
};
INTDEF DeeTypeObject DeeSockAddr_Type;




/* New error classes added by the net subsystem. */
INTDEF DeeTypeObject DeeError_NetError; /* extends `Error.SystemError' */
INTDEF DeeTypeObject     DeeError_NoSupport;
INTDEF DeeTypeObject     DeeError_NotBound;
INTDEF DeeTypeObject         DeeError_NotListening;
INTDEF DeeTypeObject     DeeError_NotConnected;
INTDEF DeeTypeObject     DeeError_IsConnected;
INTDEF DeeTypeObject     DeeError_ConnectRefused;
INTDEF DeeTypeObject     DeeError_ConnectReset;
INTDEF DeeTypeObject         DeeError_TimedOut;
INTDEF DeeTypeObject     DeeError_NetUnreachable;
INTDEF DeeTypeObject         DeeError_HostUnreachable;
INTDEF DeeTypeObject     DeeError_MessageSize;
INTDEF DeeTypeObject     DeeError_AddrInUse;
INTDEF DeeTypeObject     DeeError_AddrNotAvail;
INTDEF DeeTypeObject     DeeError_HostNotFound; /* Host name does not exist. */
INTDEF DeeTypeObject         DeeError_NoHostAddress; /* Host has no addresses associated with it. */

/* Throws an `Error.SystemError.FSError.FileClosed' */
INTDEF NONNULL((2)) int DCALL
err_socket_closed(neterrno_t err, DeeSocketObject *__restrict self);


DECL_END

#endif /* !GUARD_DEX_SOCKET_LIBNET_H */
