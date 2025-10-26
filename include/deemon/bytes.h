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
#ifndef GUARD_DEEMON_BYTES_H
#define GUARD_DEEMON_BYTES_H 1

#include "api.h"
/**/

#ifndef __INTELLISENSE__
#include "object.h"
#endif /* !__INTELLISENSE__ */
#include "types.h"
/**/

#include <hybrid/typecore.h>
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_bytes_object  bytes_object
#define Dee_bytes_printer bytes_printer
#define DEFINE_BYTES      Dee_DEFINE_BYTES
#define DEFINE_BYTES_EX   Dee_DEFINE_BYTES_EX
#endif /* DEE_SOURCE */


typedef struct Dee_bytes_object DeeBytesObject;
struct Dee_bytes_object {
	Dee_OBJECT_HEAD
	__BYTE_TYPE__                         *b_base;    /* [0..b_size][in(b_buffer.bb_base)][const] Base address of the used portion of the buffer. */
#ifdef CONFIG_EXPERIMENTAL_BYTES_INUSE
	size_t                                 b_size;    /* [<= b_buffer.bb_size][const_if(*b_inuse_p > 0)] Size of the used portion of the buffer */
#else /* CONFIG_EXPERIMENTAL_BYTES_INUSE */
	size_t                                 b_size;    /* [<= b_buffer.bb_size][const] Size of the used portion of the buffer */
#endif /* !CONFIG_EXPERIMENTAL_BYTES_INUSE */
	DREF DeeObject                        *b_orig;    /* [1..1][const][ref_if(!= self)] The object for which this is the buffer view. */
	DeeBuffer                              b_buffer;  /* [const] The buffer being accessed. */
#ifdef CONFIG_EXPERIMENTAL_BYTES_INUSE
#ifndef CONFIG_NO_THREADS
#define Dee_BYTES_HAVE_b_inuse_p
	Dee_refcnt_t                          *b_inuse_p; /* [lock(ATOMIC)][1..1][const] Bytes are currently in-use.
	                                                   * Used as a wait-for barrier by `DeeBytes_ReleaseRef()' to
	                                                   * ensure that anyone still using the bytes has gone away
	                                                   * before returning.
	                                                   *
	                                                   * This needs to be a pointer because sub-Bytes-views of 1
	                                                   * Bytes object need to reference the in-use counter of the
	                                                   * underlying (owner) Bytes. */
#endif /* !CONFIG_NO_THREADS */
#endif /* CONFIG_EXPERIMENTAL_BYTES_INUSE */
	unsigned int                           b_flags;   /* [const] Buffer access flags (Set of `Dee_BUFFER_F*') */
	COMPILER_FLEXIBLE_ARRAY(__BYTE_TYPE__, b_data);   /* ... Inline buffer data (Pointed to by `b_buffer.bb_base' if the Bytes object owns its own data) */
};


/* Define a statically initialized Bytes object `name' */
#define Dee_DEFINE_BYTES(name, flags, num_bytes, ...) \
	Dee_DEFINE_BYTES_EX(name, flags, __BYTE_TYPE__, num_bytes, __VA_ARGS__)
#ifdef Dee_BYTES_HAVE_b_inuse_p
#define Dee_DEFINE_BYTES_EX(name, flags, Titem, num_items, ...) \
	struct {                                                    \
		Dee_OBJECT_HEAD                                         \
		__BYTE_TYPE__ *b_base;                                  \
		size_t b_size;                                          \
		DREF DeeObject *b_orig;                                 \
		DeeBuffer b_buffer;                                     \
		Dee_refcnt_t *b_inuse_p;                                \
		unsigned int b_flags;                                   \
		Titem b_data[num_items];                                \
		Dee_refcnt_t _b_inuse;                                  \
	} name = {                                                  \
		Dee_OBJECT_HEAD_INIT(&DeeBytes_Type),                   \
		(__BYTE_TYPE__ *)name.b_data,                           \
		(num_items) * sizeof(Titem),                            \
		(DeeObject *)&name,                                     \
		DeeBuffer_INIT((__BYTE_TYPE__ *)name.b_data,            \
		               (num_items) * sizeof(Titem)),            \
		&name._b_inuse,                                         \
		flags,                                                  \
		__VA_ARGS__,                                            \
		0                                                       \
	}
#else /* Dee_BYTES_HAVE_b_inuse_p */
#define Dee_DEFINE_BYTES_EX(name, flags, Titem, num_items, ...) \
	struct {                                                    \
		Dee_OBJECT_HEAD                                         \
		__BYTE_TYPE__ *b_base;                                  \
		size_t b_size;                                          \
		DREF DeeObject *b_orig;                                 \
		DeeBuffer b_buffer;                                     \
		unsigned int b_flags;                                   \
		Titem b_data[num_items];                                \
	} name = {                                                  \
		Dee_OBJECT_HEAD_INIT(&DeeBytes_Type),                   \
		(__BYTE_TYPE__ *)name.b_data,                           \
		(num_items) * sizeof(Titem),                            \
		(DeeObject *)&name,                                     \
		DeeBuffer_INIT((__BYTE_TYPE__ *)name.b_data,            \
		               (num_items) * sizeof(Titem)),            \
		flags,                                                  \
		__VA_ARGS__                                             \
	}
#endif /* !Dee_BYTES_HAVE_b_inuse_p */



/* Data accessor helper macros for bytes objects */
#ifdef Dee_BYTES_HAVE_b_inuse_p
#define DeeBytes_IncUse(self) _DeeRefcnt_Inc(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_inuse_p)
#define DeeBytes_DecUse(self) _DeeRefcnt_Dec(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_inuse_p)
#else /* Dee_BYTES_HAVE_b_inuse_p */
#define DeeBytes_IncUse(self) (void)0
#define DeeBytes_DecUse(self) (void)0
#endif /* !Dee_BYTES_HAVE_b_inuse_p */

/* Access to the effect data-blob of some given "Bytes".
 * These function should only be used after `DeeBytes_IncUse()' has been called. */
#define DeeBytes_DATA(x) ((DeeBytesObject *)Dee_REQUIRES_OBJECT(x))->b_base
#define DeeBytes_SIZE(x) ((DeeBytesObject *)Dee_REQUIRES_OBJECT(x))->b_size
#define DeeBytes_END(x)  (DeeBytes_DATA(x) + DeeBytes_SIZE(x))

#define DeeBytes_WRITABLE(x)   (((DeeBytesObject *)Dee_REQUIRES_OBJECT(x))->b_flags & Dee_BUFFER_FWRITABLE)
#define DeeBytes_Check(x)      DeeObject_InstanceOfExact(x, &DeeBytes_Type) /* `Bytes' is final. */
#define DeeBytes_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBytes_Type)
#define DeeBytes_IsEmpty(x)    (((DeeBytesObject *)Dee_REQUIRES_OBJECT(x))->b_size == 0)

/* The builtin `Bytes' data type.
 * This type offers functionality identical to what can also be found in
 * the string API, however in addition, functions to inplace-modify the
 * data of a buffer are provided as well (such as `tolower()')
 * The bytes data type is a proxy-object designed to provide an extensive
 * API through which user-code can operate with bytes objects. */
DDATDEF DeeTypeObject DeeBytes_Type;

/* A singleton representing an empty Bytes object.
 * NOTE: This object is not required to be used for empty bytes.
 *       Other instances may be used as well!
 * NOTE: The empty Bytes object is writable (though there is no memory to write to)
 */
struct Dee_empty_bytes_struct {
	Dee_OBJECT_HEAD
	__BYTE_TYPE__  *b_base;
	size_t          b_size;
	DREF DeeObject *b_orig;
	DeeBuffer       b_buffer;
	unsigned int    b_flags;
	__BYTE_TYPE__   b_data[1];
};
DDATDEF struct Dee_empty_bytes_struct DeeBytes_Empty;
#define Dee_EmptyBytes ((DeeObject *)&DeeBytes_Empty)
#ifdef __INTELLISENSE__
#define DeeBytes_NewEmpty() ((DeeObject *)&DeeBytes_Empty)
#else /* __INTELLISENSE__ */
#define DeeBytes_NewEmpty() (Dee_Incref(&DeeBytes_Empty), (DeeObject *)&DeeBytes_Empty)
#endif /* !__INTELLISENSE__ */



/* Construct a bytes-buffer from `self', using the generic object-buffer interface.
 * @param: flags: Set of `Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Bytes(DeeObject *__restrict self,
                unsigned int flags,
                size_t start, size_t end);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TBytes(DeeTypeObject *tp_self,
                 DeeObject *__restrict self,
                 unsigned int flags,
                 size_t start, size_t end);

/* Construct a writable bytes-buffer, consisting of a total of `num_bytes' bytes. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_NewBuffer(size_t num_bytes, __BYTE_TYPE__ init);

DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_NewBufferUninitialized(size_t num_bytes);
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_TryNewBufferUninitialized(size_t num_bytes);
#define DeeBytes_Destroy(self) Dee_DecrefDokill(self)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_NewBufferData(void const *__restrict data, size_t num_bytes);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_TryNewBufferData(void const *__restrict data, size_t num_bytes);

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_ResizeBuffer(/*inherit(on_success)*/ DREF DeeObject *__restrict self, size_t num_bytes);

DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_TruncateBuffer(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t num_bytes);

/* Constructs a byte-view for data in `base...+=num_bytes' held by `owner'.
 * The given `flags' determines if the view is read-only, or can be modified.
 * @param: flags: Set of `Dee_BUFFER_F*' */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_NewView(DeeObject *owner, void *base,
                 size_t num_bytes, unsigned int flags);
#ifdef __INTELLISENSE__
WUNUSED ATTR_IN(2, 3) NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_NewViewRo(DeeObject *owner, void const *base, size_t num_bytes);
#else /* __INTELLISENSE__ */
#define DeeBytes_NewViewRo(owner, base, num_bytes) \
	DeeBytes_NewView(owner, (void *)(base), num_bytes, Dee_BUFFER_FREADONLY)
#endif /* !__INTELLISENSE__ */


#ifdef CONFIG_EXPERIMENTAL_BYTES_INUSE
/* Acquire/release a `Bytes' object referencing the given region of memory.
 *
 * For this purpose, if the returned `Bytes' are still in-use by the time
 * a call to `DeeBytes_ReleaseRef()' is made (iow: `DeeObject_IsShared()'),
 * then "b_size" is forceably set to `0', and the function waits until the
 * bytes's `*b_inuse_p' counter becomes `0'.
 *
 * The given `DeeBytes_ReleaseRef()' should only be called from the same
 * method that originally called `DeeBytes_AcquireRef()'. If this is not
 * strictly adhered to, deadlocks might happen (as a result of user-code
 * being able to trigger `DeeBytes_ReleaseRef()' in a context where the
 * same thread has previously called `DeeBytes_IncUse()') */
WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_AcquireRef(void *base, size_t num_bytes, unsigned int flags);
WUNUSED NONNULL((1)) void DCALL
DeeBytes_ReleaseRef(DREF DeeObject *self);
#endif /* CONFIG_EXPERIMENTAL_BYTES_INUSE */

#ifdef __INTELLISENSE__
#define DeeBytes_NewSubView(self, base, num_bytes)                                           \
	DeeBytes_NewView(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_orig, base, num_bytes, \
	                 ((DeeBytesObject *)(self))->b_flags)
#define DeeBytes_NewSubViewRo(self, base, num_bytes)                                                   \
	DeeBytes_NewView(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_orig, (void *)(base), num_bytes, \
	                 ((DeeBytesObject *)(self))->b_flags & ~Dee_BUFFER_FWRITABLE)
#else /* __INTELLISENSE__ */
#define DeeBytes_NewSubView(self, base, num_bytes)                                  \
	DeeBytes_NewView(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_buffer.bb_put \
	                 ? (DeeObject *)(self)                                          \
	                 : ((DeeBytesObject *)(self))->b_orig,                          \
	                 base, num_bytes, ((DeeBytesObject *)(self))->b_flags)
#if Dee_BUFFER_FMASK == Dee_BUFFER_FWRITABLE
#define DeeBytes_NewSubViewRo(self, base, num_bytes)                                \
	DeeBytes_NewView(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_buffer.bb_put \
	                 ? (DeeObject *)(self)                                          \
	                 : ((DeeBytesObject *)(self))->b_orig,                          \
	                 (void *)(base), num_bytes, Dee_BUFFER_FREADONLY)
#else /* Dee_BUFFER_FMASK == Dee_BUFFER_FWRITABLE */
#define DeeBytes_NewSubViewRo(self, base, num_bytes)                                \
	DeeBytes_NewView(((DeeBytesObject *)Dee_REQUIRES_OBJECT(self))->b_buffer.bb_put \
	                 ? (DeeObject *)(self)                                          \
	                 : ((DeeBytesObject *)(self))->b_orig,                          \
	                 (void *)(base), num_bytes,                                     \
	                 ((DeeBytesObject *)(self))->b_flags & ~Dee_BUFFER_FWRITABLE)
#endif /* Dee_BUFFER_FMASK != Dee_BUFFER_FWRITABLE */
#endif /* !__INTELLISENSE__ */

/* Construct a writable bytes-object that is initialized from the
 * items of the given `seq' casted to integers in the range of 00-FF */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_FromSequence(DeeObject *__restrict seq);

#ifdef CONFIG_BUILDING_DEEMON
/* Print all bytes from `self' encoded as UTF-8.
 * In other words, bytes that are non-ASCII (aka. 80-FF) are
 * encoded as 2-byte UTF-8 sequences (aka: as LATIN-1), allowing
 * them to be properly interpreted by the given `printer' */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeBytes_Print(DeeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeBytes_PrintRepr(DeeObject *__restrict self,
                   Dee_formatprinter_t printer, void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeBytes_Print(self, printer, arg) \
	DeeObject_Print(self, printer, arg)
#define DeeBytes_PrintRepr(self, printer, arg) \
	DeeObject_PrintRepr(self, printer, arg)
#endif /* !CONFIG_BUILDING_DEEMON */

/* Unpack the given sequence `seq' into `num_bytes', invoking the
 * `operator int' on each, converting their values into bytes, before
 * storing those bytes in the given `dst' vector.
 * If the length of `seq' doesn't match `num_bytes', an UnpackError is thrown.
 * If `seq' is the none-singleton, `dst...+=num_bytes' is zero-initialized. */
DFUNDEF WUNUSED NONNULL((1, 3)) int
(DCALL DeeSeq_ItemsToBytes)(__BYTE_TYPE__ *__restrict dst, size_t num_bytes,
                            DeeObject *__restrict seq);




/* ================================================================================= */
/*   BYTES PRINTER API                                                               */
/* ================================================================================= */
struct Dee_bytes_printer {
	/* A bytes printer is similar to `unicode_printer' found in <deemon/string.h>,
	 * however instead of constructing a unicode object, it creates a writable bytes
	 * object consisting of unicode characters within the range 00-FF.
	 * Attempting to print a unicode character outside that range will result in
	 * a `UnicodeEncodeError' being thrown, following the reasoning that the character
	 * could not be encoded as LATIN-1 (which is the unicode range 00-FF mapping onto
	 * a single byte)
	 * As far as API usage goes, a `Dee_bytes_printer' functions very much the same as a
	 * unicode printer, with its UTF-8-enabled & Dee_formatprinter_t-compatible function
	 * being `bytes_printer_print'
	 */
	size_t          bp_length;  /* The number of bytes already printed. */
	DeeBytesObject *bp_bytes;   /* [0..1][owned] The resulting Bytes object. */
	unsigned char   bp_numpend; /* The number of pending UTF-8 characters. */
	unsigned char   bp_pend[7]; /* Pending UTF-8 characters. */
};
#define Dee_BYTES_PRINTER_INIT       { 0, NULL, 0 }
#define Dee_BYTES_PRINTER_SIZE(x)    ((x)->bp_length)
#define Dee_bytes_printer_init(self) ((self)->bp_length = 0, (self)->bp_bytes = NULL, (self)->bp_numpend = 0)
#define Dee_bytes_printer_fini(self) DeeObject_Free((self)->bp_bytes)

#ifdef DEE_SOURCE
#define BYTES_PRINTER_INIT  Dee_BYTES_PRINTER_INIT
#define BYTES_PRINTER_SIZE  Dee_BYTES_PRINTER_SIZE
#define bytes_printer_init  Dee_bytes_printer_init
#define bytes_printer_fini  Dee_bytes_printer_fini
#ifdef __INTELLISENSE__
#define Dee_bytes_printer_pack            bytes_printer_pack
#define Dee_bytes_printer_print           bytes_printer_print
#define Dee_bytes_printer_putc            bytes_printer_putc
#define Dee_bytes_printer_putb            bytes_printer_putb
#define Dee_bytes_printer_repeat          bytes_printer_repeat
#define Dee_bytes_printer_append          bytes_printer_append
#define Dee_bytes_printer_alloc           bytes_printer_alloc
#define Dee_bytes_printer_release         bytes_printer_release
#define Dee_bytes_printer_printf          bytes_printer_printf
#define Dee_bytes_printer_vprintf         bytes_printer_vprintf
#define Dee_bytes_printer_printobject     bytes_printer_printobject
#define Dee_bytes_printer_printobjectrepr bytes_printer_printobjectrepr
#else /* __INTELLISENSE__ */
#define bytes_printer_printf          Dee_bytes_printer_printf
#define bytes_printer_vprintf         Dee_bytes_printer_vprintf
#define bytes_printer_printobject     Dee_bytes_printer_printobject
#define bytes_printer_printobjectrepr Dee_bytes_printer_printobjectrepr
#endif /* !__INTELLISENSE__ */
#endif /* DEE_SOURCE */

/* _Always_ inherit all byte data (even upon error) saved in
 * `self', and construct a new Bytes object from all that data, before
 * returning a reference to that object.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `bytes_printer_fini()'
 * @return: * :   A reference to the packed Bytes object.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_bytes_printer_pack)(/*inherit(always)*/ struct Dee_bytes_printer *__restrict self);

/* Append the given `text' to the end of the Bytes object.
 * This function is intended to be used as the general-purpose
 * Dee_formatprinter_t-compatible callback for generating data
 * to-be written into a Bytes object. */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DPRINTER_CC Dee_bytes_printer_print)(void *__restrict self,
                                      /*utf-8*/ char const *__restrict text,
                                      size_t textlen);

/* Append a single UTF-8 character. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_bytes_printer_putc)(struct Dee_bytes_printer *__restrict self, char ch);

/* Append a single byte. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_bytes_printer_putb)(struct Dee_bytes_printer *__restrict self, __BYTE_TYPE__ byte);

/* Repeat the given `byte' a total of `count' times. */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL Dee_bytes_printer_repeat)(struct Dee_bytes_printer *__restrict self,
                                 __BYTE_TYPE__ byte, size_t count);


/* Append raw byte data to the given bytes-printer, without concern
 * about any kind of encoding. - Just copy over the raw bytes.
 * -> A far as unicode support goes, this function has _nothing_ to
 *    do with any kind of encoding. - It just blindly copies the given
 *    data into the buffer of the resulting Bytes object.
 * -> The equivalent unicode_printer function is `unicode_printer_print8' */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
(DPRINTER_CC Dee_bytes_printer_append)(struct Dee_bytes_printer *__restrict self,
                                       __BYTE_TYPE__ const *__restrict data,
                                       size_t datalen);

/* Allocate a buffer of `datalen' bytes at the end of the printer. */
DFUNDEF WUNUSED NONNULL((1)) __BYTE_TYPE__ *
(DCALL Dee_bytes_printer_alloc)(struct Dee_bytes_printer *__restrict self, size_t datalen);

/* Release the last `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
DFUNDEF NONNULL((1)) void
(DCALL Dee_bytes_printer_release)(struct Dee_bytes_printer *__restrict self, size_t datalen);

#ifdef __INTELLISENSE__
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_bytes_printer_printf)(struct Dee_bytes_printer *__restrict self, char const *__restrict format, ...);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_bytes_printer_vprintf)(struct Dee_bytes_printer *__restrict self, char const *__restrict format, va_list args);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_bytes_printer_printobject)(struct Dee_bytes_printer *__restrict self, DeeObject *__restrict ob);
WUNUSED NONNULL((1, 2)) Dee_ssize_t (Dee_bytes_printer_printobjectrepr)(struct Dee_bytes_printer *__restrict self, DeeObject *__restrict ob);
#else /* __INTELLISENSE__ */
#define Dee_bytes_printer_printf(self, ...)           DeeFormat_Printf(&Dee_bytes_printer_print, self, __VA_ARGS__)
#define Dee_bytes_printer_vprintf(self, format, args) DeeFormat_VPrintf(&Dee_bytes_printer_print, self, format, args)
#define Dee_bytes_printer_printobject(self, ob)       DeeObject_Print(ob, &Dee_bytes_printer_print, self)
#define Dee_bytes_printer_printobjectrepr(self, ob)   DeeObject_PrintRepr(ob, &Dee_bytes_printer_print, self)
#endif /* !__INTELLISENSE__ */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeSeq_ItemsToBytes(dst, num_bytes, seq) __builtin_expect(DeeSeq_ItemsToBytes(dst, num_bytes, seq), 0)
#define Dee_bytes_printer_putc(self, ch)         __builtin_expect(Dee_bytes_printer_putc(self, ch), 0)
#define Dee_bytes_printer_putb(self, byte)       __builtin_expect(Dee_bytes_printer_putb(self, byte), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#ifdef DEE_SOURCE
#ifndef __INTELLISENSE__
#define bytes_printer_pack            Dee_bytes_printer_pack
#define bytes_printer_print           Dee_bytes_printer_print
#define bytes_printer_putc            Dee_bytes_printer_putc
#define bytes_printer_putb            Dee_bytes_printer_putb
#define bytes_printer_repeat          Dee_bytes_printer_repeat
#define bytes_printer_append          Dee_bytes_printer_append
#define bytes_printer_alloc           Dee_bytes_printer_alloc
#define bytes_printer_release         Dee_bytes_printer_release
#define bytes_printer_printf          Dee_bytes_printer_printf
#define bytes_printer_vprintf         Dee_bytes_printer_vprintf
#define bytes_printer_printobject     Dee_bytes_printer_printobject
#define bytes_printer_printobjectrepr Dee_bytes_printer_printobjectrepr
#endif /* !__INTELLISENSE__ */
#endif /* DEE_SOURCE */


DECL_END

#endif /* !GUARD_DEEMON_BYTES_H */
