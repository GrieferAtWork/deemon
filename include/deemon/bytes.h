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
#ifndef GUARD_DEEMON_BYTES_H
#define GUARD_DEEMON_BYTES_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    uint8_t        *b_base;    /* [0..b_size][in(b_buffer.bb_base)][const]
                                * Base address of the used portion of the buffer. */
    size_t          b_size;    /* [<= b_buffer.bb_size][const]
                                * Size of the used portion of the buffer */
    DREF DeeObject *b_orig;    /* [1..1][const][ref_if(!= self)]
                                * The object for which this is the buffer view. */
    DeeBuffer       b_buffer;  /* [const] The buffer being accessed. */
    unsigned int    b_flags;   /* [const] Buffer access flags (Set of `DEE_BUFFER_F*') */
    uint8_t         b_data[1]; /* ... Inline buffer data (Pointed to by `b_buffer.bb_base' if the bytes object owns its own data) */
} DeeBytesObject;

#define DeeBytes_DATA(x)        ((DeeBytesObject *)REQUIRES_OBJECT(x))->b_base
#define DeeBytes_SIZE(x)        ((DeeBytesObject *)REQUIRES_OBJECT(x))->b_size
#define DeeBytes_TERM(x)        (DeeBytes_DATA(x)+DeeBytes_SIZE(x))
#define DeeBytes_WRITABLE(x)    (((DeeBytesObject *)REQUIRES_OBJECT(x))->b_flags & DEE_BUFFER_FWRITABLE)
#define DeeBytes_Check(x)       DeeObject_InstanceOfExact(x,&DeeBytes_Type) /* `bytes' is final. */
#define DeeBytes_CheckExact(x)  DeeObject_InstanceOfExact(x,&DeeBytes_Type)
#define DeeBytes_IsEmpty(x)     (((DeeBytesObject *)REQUIRES_OBJECT(x))->b_size==0)

/* The builtin `bytes' data type.
 * This type offers functionality identical to what can also be found in
 * the string API, however in addition, functions to inplace-modify the
 * data of a buffer are provided as well (such as `tolower()')
 * The bytes data type is a proxy-object designed to provide an extensive
 * API through which user-code can operate with bytes objects. */
DDATDEF DeeTypeObject DeeBytes_Type;

/* A singleton representing an empty bytes object.
 * NOTE: This object is not required to be used for empty bytes.
 *       Other instances may be used as well!
 * NOTE: The empty bytes object is writable (though there is no memory to write to)
 */
#ifdef GUARD_DEEMON_OBJECTS_BYTES_C
DDATDEF DeeBytesObject                DeeBytes_Empty;
#define Dee_EmptyBytes ((DeeObject *)&DeeBytes_Empty)
#else
DDATDEF DeeObject                     DeeBytes_Empty;
#define Dee_EmptyBytes              (&DeeBytes_Empty)
#endif
#define return_empty_bytes            return_reference_(Dee_EmptyBytes)



/* Construct a bytes-buffer from `self', using the generic object-buffer interface. */
DFUNDEF DREF DeeObject *DCALL DeeObject_Bytes(DeeObject *__restrict self,
                                              unsigned int flags,
                                              size_t start, size_t end);
/* Construct a writable bytes-buffer, consisting of a total of `num_bytes' bytes. */
DFUNDEF DREF DeeObject *DCALL DeeBytes_NewBuffer(size_t num_bytes, uint8_t init);
DFUNDEF DREF DeeObject *DCALL DeeBytes_NewBufferUninitialized(size_t num_bytes);
DFUNDEF DREF DeeObject *DCALL DeeBytes_NewBufferData(void const *__restrict data, size_t num_bytes);
DFUNDEF DREF DeeObject *DCALL DeeBytes_ResizeBuffer(DREF DeeObject *__restrict self, size_t num_bytes);
DFUNDEF ATTR_RETNONNULL DREF DeeObject *DCALL DeeBytes_TruncateBuffer(DREF DeeObject *__restrict self, size_t num_bytes);

/* Constructs a byte-view for data in `base...+=num_bytes' held by `owner'.
 * The given `flags' determines if the view is read-only, or can be modified.
 * @param: flags: Set of `DEE_BUFFER_F*' */
DFUNDEF DREF DeeObject *DCALL
DeeBytes_NewView(DeeObject *__restrict owner, void *__restrict base,
                 size_t num_bytes, unsigned int flags);

#ifdef __INTELLISENSE__
#define DeeBytes_NewSubView(self,base,num_bytes) \
        DeeBytes_NewView((self)->b_orig,base,num_bytes,(self)->b_flags)
#define DeeBytes_NewSubViewRo(self,base,num_bytes) \
        DeeBytes_NewView((self)->b_orig,base,num_bytes,(self)->b_flags & ~DEE_BUFFER_FWRITABLE)
#else
#define DeeBytes_NewSubView(self,base,num_bytes) \
        DeeBytes_NewView((self)->b_buffer.bb_put ? (DeeObject *)(self) : (self)->b_orig, \
                          base,num_bytes,(self)->b_flags)
#if DEE_BUFFER_FMASK == DEE_BUFFER_FWRITABLE
#define DeeBytes_NewSubViewRo(self,base,num_bytes) \
        DeeBytes_NewView((self)->b_buffer.bb_put ? (DeeObject *)(self) : (self)->b_orig, \
                          base,num_bytes,DEE_BUFFER_FREADONLY)
#else
#define DeeBytes_NewSubViewRo(self,base,num_bytes) \
        DeeBytes_NewView((self)->b_buffer.bb_put ? (DeeObject *)(self) : (self)->b_orig, \
                          base,num_bytes,(self)->b_flags & ~DEE_BUFFER_FWRITABLE)
#endif
#endif

/* Construct a writable bytes-object that is initialized from the
 * items of the given `seq' casted to integers in the range of 00-FF */
DFUNDEF DREF DeeObject *DCALL
DeeBytes_FromSequence(DeeObject *__restrict seq);

#ifdef CONFIG_BUILDING_DEEMON
/* Print all bytes from `self' encoded as UTF-8.
 * In other words, bytes that are non-ASCII (aka. 80-FF) are
 * encoded as 2-byte UTF-8 sequences, allowing them to be
 * properly interpreted by the given `printer' */
INTDEF dssize_t DCALL
DeeBytes_PrintUtf8(DeeObject *__restrict self,
                   dformatprinter printer, void *arg);
#endif

/* Unpack the given sequence `seq' into `num_bytes', invoking the
 * `operator int' on each, converting their values into bytes, before
 * storing those bytes in the given `dst' vector.
 * If the length of `seq' doesn't match `num_bytes',
 * an UnpackError is thrown.
 * If `seq' is the none-singleton, `dst...+=num_bytes' is zero-initialized. */
DFUNDEF int
(DCALL DeeSeq_ItemsToBytes)(uint8_t *__restrict dst, size_t num_bytes,
                            DeeObject *__restrict seq);




/* ================================================================================= */
/*   BYTES PRINTER API                                                               */
/* ================================================================================= */
struct bytes_printer {
    /* A bytes printer is similar to `unicode_printer' found in <deemon/string.h>,
     * however instead of constructing a unicode object, it creates a writable bytes
     * object consisting of unicode characters within the range 00-FF.
     * Attempting to print a unicode character outside that range will result in
     * a `UnicodeEncodeError' being thrown, following the reasoning that the character
     * could not be encoded as LATIN-1 (which is the unicode range 00-FF mapping onto
     * a single byte)
     * As far as API usage goes, a `bytes_printer' functions very much the same as a
     * unicode printer, with its UTF-8-enabled & dformatprinter-compatible function
     * being `bytes_printer_print'
     */
    size_t          bp_length;  /* The number of bytes already printed. */
    DeeBytesObject *bp_bytes;   /* [0..1][owned] The resulting bytes object. */
    unsigned char   bp_numpend; /* The number of pending UTF-8 characters. */
    unsigned char   bp_pend[7]; /* Pending UTF-8 characters. */
};
#define BYTES_PRINTER_INIT       { 0, NULL, 0 }
#define BYTES_PRINTER_SIZE(x)   ((x)->bp_length)
#define bytes_printer_init(self) ((self)->bp_length = 0,(self)->bp_bytes = NULL,(self)->bp_numpend = 0)
#define bytes_printer_fini(self) DeeObject_Free((self)->bp_bytes)

/* _Always_ inherit all byte data (even upon error) saved in
 * `self', and construct a new bytes object from all that data, before
 * returning a reference to that object.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `bytes_printer_fini()'
 * @return: * :   A reference to the packed bytes object.
 * @return: NULL: An error occurred. */
DFUNDEF DREF DeeObject *
(DCALL bytes_printer_pack)(/*inherit(always)*/struct bytes_printer *__restrict self);

/* Append the given `text' to the end of the bytes object.
 * This function is intended to be used as the general-purpose
 * dformatprinter-compatible callback for generating data to-be
 * written into a bytes object. */
DFUNDEF dssize_t
(DCALL bytes_printer_print)(struct bytes_printer *__restrict self,
                            /*utf-8*/char const *__restrict text,
                            size_t textlen);

/* Append a single UTF-8 character. */
DFUNDEF int (DCALL bytes_printer_putc)(struct bytes_printer *__restrict self, char ch);

/* Append a single byte. */
DFUNDEF int (DCALL bytes_printer_putb)(struct bytes_printer *__restrict self, uint8_t byte);

/* Repeat the given `byte' a total of `count' times. */
DFUNDEF dssize_t (DCALL bytes_printer_repeat)(struct bytes_printer *__restrict self, uint8_t byte, size_t count);


/* Append raw byte data to the given bytes-printer, without concern
 * about any kind of encoding. - Just copy over the raw bytes.
 * -> A far as unicode support goes, this function has _nothing_ to
 *    do with any kind of encoding. - It just blindly copies the given
 *    data into the buffer of the resulting bytes object.
 * -> The equivalent unicode_printer function is `unicode_printer_print8' */
DFUNDEF dssize_t
(DCALL bytes_printer_append)(struct bytes_printer *__restrict self,
                             uint8_t const *__restrict data, size_t datalen);

/* Allocate a buffer of `datalen' bytes at the end of the printer. */
DFUNDEF uint8_t *(DCALL bytes_printer_alloc)(struct bytes_printer *__restrict self, size_t datalen);

/* Release the last `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
DFUNDEF void (DCALL bytes_printer_release)(struct bytes_printer *__restrict self, size_t datalen);

#ifdef __INTELLISENSE__
dssize_t (bytes_printer_printf)(struct bytes_printer *__restrict self, char const *__restrict format, ...);
dssize_t (bytes_printer_vprintf)(struct bytes_printer *__restrict self, char const *__restrict format, va_list args);
dssize_t (bytes_printer_printobject)(struct bytes_printer *__restrict self, DeeObject *__restrict ob);
dssize_t (bytes_printer_printobjectrepr)(struct bytes_printer *__restrict self, DeeObject *__restrict ob);
#else
#define bytes_printer_printf(self,...)          Dee_FormatPrintf((dformatprinter)&bytes_printer_print,self,__VA_ARGS__)
#define bytes_printer_vprintf(self,format,args) Dee_VFormatPrintf((dformatprinter)&bytes_printer_print,self,format,args)
#define bytes_printer_printobject(self,ob)      DeeObject_Print(ob,(dformatprinter)&bytes_printer_print,self)
#define bytes_printer_printobjectrepr(self,ob)  DeeObject_PrintRepr(ob,(dformatprinter)&bytes_printer_print,self)
#endif


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeSeq_ItemsToBytes(dst,num_bytes,seq) __builtin_expect(DeeSeq_ItemsToBytes(dst,num_bytes,seq),0)
#define bytes_printer_putc(self,ch)            __builtin_expect(bytes_printer_putc(self,ch),0)
#define bytes_printer_putb(self,byte)          __builtin_expect(bytes_printer_putb(self,byte),0)
#endif
#endif


DECL_END

#endif /* !GUARD_DEEMON_BYTES_H */
