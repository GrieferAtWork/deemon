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
#ifndef GUARD_DEX_JSON_LIBJSON_H
#define GUARD_DEX_JSON_LIBJSON_H 1

#include <deemon/api.h>

#include <deemon/float.h>       /* CONFIG_HAVE_FPU */
#include <deemon/hashset.h>
#include <deemon/object.h>
#include <deemon/util/atomic.h> /* atomic_read */
#include <deemon/util/lock.h>   /* Dee_atomic_rwlock_* */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

/* =======================================================================================
 * === JSON SUPPORT                                                                    ===
 * =======================================================================================
 *
 * The JSON dex provides the following functions:
 * >> #define JSONObject (float | int | string | bool | none | Sequence | Mapping)
 * >> function parse(data: File | Bytes | string | Mapping): JSONObject;
 * >> function parse(data: File | Bytes | string | Mapping, into: Object): Object;
 * >> function parse(data: File | Bytes | string | Mapping, into: Type): Object;
 * >> function write(data: Object | JSONObject, pretty: bool = false): string;
 * >> function write(data: Object | JSONObject, into: File, pretty: bool = false): File;
 *
 * Notes:
 * - When `parse(data)' returns a Sequence of Mapping, JSON input data may not be
 *   fully validated, as the returned sequences are allowed to be abstract sequence
 *   types that only parse JSON input as it is requested via sequence operations.
 *   In order to force evaluation of JSON input, you can cast the result to another
 *   type of sequence which will then force evaluation (just like with all other
 *   sequence proxy types)
 * - When given a `Mapping', `parse' behaves the same as though `json.write(data)'
 *   was given instead (iow: it will treat the contents of the mapping as a parsed
 *   JSON object and either re-return the mapping, or write its elements to `into')
 * - `parse' will decode JSON blobs into JSONObject-types as would be expected,
 *   but if given an `into' object, it will parse a JSON-object and fill in all
 *   of that object's attributes as attributes of `into'. Also note that when
 *   members of `into' have type annotations, decode will parse those annotations
 *   and construct instances of the referenced objects, as well as assert that the
 *   proper types are provided for simple JSON types. (Note that where there are no
 *   type annotations, or `Object' is annotated, *any* type of JSON attribute can
 *   be set)
 *   >> import * from deemon;
 *   >> import json;
 *   >> class Inner {
 *   >>     public member x: int;
 *   >>     public member y: int;
 *   >> }
 *   >> class Outer {
 *   >>     public member inner: Inner;
 *   >>     public member z: string;
 *   >> }
 *   >> local x = json.parse('{"inner":{"x":10,"y":20},"z":"Foo"}', Outer);
 *   >> print repr x; // Outer(inner: Inner(x: 10, y: 20), z: "Foo")
 *   >> json.parse('{z:42}', Outer); // Throws a TypeError
 *   - Note that for this purpose, Tuple type descriptions expected JSON input
 *     to offer arrays of matching length, and matching order. Also note that in
 *     case of multiple possible types, all provided types are tried in order,
 *     and the first one that doesn't result in a parser error is then used.
 * - `parse(File)' is the same as `parse(File.readall())', as in: there is no
 *   way to only parse a single JSON object from a file and leave the file's
 *   pointer such that it points directly after the JSON object.
 * - `parse(data, Type)' is the same as `parse(data, Type())',
 * - Only `public' members of DTOs will be initialized by `parse'
 * - `write' will encode JSONObject-types as would be expected, and all other
 *   objects are treated as DTOs and have their attributes enumerated and then
 *   written to the output file.
 * - Under the hood, the JSON dex uses a port of KOS's libjson system library
 * - Fun fact: with the exception of needing to write `none' instead of `null',
 *   deemon is actually syntactically compatible with JSON, and if you were to
 *   write `global final null = none;', it would be fully compatible!
 *   As such, technically speaking all functionality provided by this dex is
 *   also already available by `_strexec' (and `deemon.exec()'), though it should
 *   be obvious that *executing as code* would be a security risk in case you
 *   received the JSON-blob from an untrusted source (or really *any source*
 *   for that matter).
 */



/* Configure libjson such that it uses deemon APIs rather than KOS APIs. */
#define LIBJSON_NO_SYSTEM_INCLUDES

/* Tell libjson that it doesn't need to do state-validation in json_writer calls. */
#define LIBJSON_NO_WRITER_CALLS_IN_BAD_STATE
#define LIBJSON_NO_WRITER_BADUSAGE

/* Disable some functions that we don't need */
#define LIBJSON_NO_WRITER_PUTNUMBER
#define LIBJSON_NO_WRITER_PUTBOOL
#define LIBJSON_NO_WRITER_PUTNULL
#define LIBJSON_NO_PARSER_GETSTRING
#define LIBJSON_NO_PARSER_GETNUMBER
#define LIBJSON_NO_PARSER_GETBOOL
#define LIBJSON_NO_PARSER_GETNULL
#define LIBJSON_NO_PARSER_FINDINDEX
#define LIBJSON_NO_PARSER_ENTER_LEAVE
#ifndef CONFIG_HAVE_FPU
#define LIBJSON_NO_PARSER_GETFLOAT
#endif /* !CONFIG_HAVE_FPU */

/* Enable KOS compatibility emulation */
#include <deemon/util/kos-compat.h> /* Needed for libjson */

/* Declare everything with PRIVATE scoping. */
#undef INTDEF
#undef INTERN
#define INTDEF PRIVATE
#define INTERN PRIVATE

/* clang-format off */
#include "../../libjson/include/api.h"    /* 1 */
#include "../../libjson/include/parser.h" /* 2 */
#include "../../libjson/include/writer.h" /* 3 */
#include "../../libjson/api.h"            /* 4 */
#include "../../libjson/parser.h"         /* 5 */
#include "../../libjson/writer.h"         /* 6 */
/* clang-format on */

#undef INTDEF
#undef INTERN
#define INTDEF __INTDEF
#define INTERN __INTERN

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *js_owner;  /* [1..1][const] Object that owns the memory that underlies `js_parser'. */
	struct json_parser  js_parser; /* [lock(js_lock)] Parser that points to the start of the `js_index'th element of the
	                                * JSON sequence. When at the last element, it points to the closing `]' of the sequence.
	                                * Note that all fields but `jp_pos' are [const] here! */
	size_t              js_index;  /* [lock(js_lock)] Index of the currently selected sequence element. */
	size_t              js_size;   /* [lock(js_lock && WRITE_ONCE)] Sequence size, or `0' if not yet calculated (or if empty). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t js_lock;   /* Lock for this JSON-sequence. */
#endif /* !CONFIG_NO_THREADS */
} DeeJsonSequenceObject;

#define DeeJsonSequence_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->js_lock)
#define DeeJsonSequence_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->js_lock)
#define DeeJsonSequence_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->js_lock)
#define DeeJsonSequence_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->js_lock)
#define DeeJsonSequence_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->js_lock)
#define DeeJsonSequence_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->js_lock)
#define DeeJsonSequence_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->js_lock)
#define DeeJsonSequence_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->js_lock)
#define DeeJsonSequence_LockRead(self)       Dee_atomic_rwlock_read(&(self)->js_lock)
#define DeeJsonSequence_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->js_lock)
#define DeeJsonSequence_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->js_lock)
#define DeeJsonSequence_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->js_lock)
#define DeeJsonSequence_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->js_lock)
#define DeeJsonSequence_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->js_lock)
#define DeeJsonSequence_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->js_lock)
#define DeeJsonSequence_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->js_lock)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *jm_owner;  /* [1..1][const] Object that owns the memory that underlies `jm_parser'. */
	struct json_parser  jm_parser; /* [lock(jm_lock)] Parser that points to the name-token of a random object.
	                                * member, or the closing `}'. Which of these, and what member exactly (if
	                                * any) is undefined and depends on the field that was last accessed.
	                                * Note that all fields but `jp_pos' are [const] here! */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t jm_lock;   /* Lock for this JSON-mapping. */
#endif /* !CONFIG_NO_THREADS */
} DeeJsonMappingObject;

#define DeeJsonMapping_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->jm_lock)
#define DeeJsonMapping_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->jm_lock)
#define DeeJsonMapping_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->jm_lock)
#define DeeJsonMapping_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->jm_lock)
#define DeeJsonMapping_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->jm_lock)
#define DeeJsonMapping_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->jm_lock)
#define DeeJsonMapping_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->jm_lock)
#define DeeJsonMapping_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->jm_lock)
#define DeeJsonMapping_LockRead(self)       Dee_atomic_rwlock_read(&(self)->jm_lock)
#define DeeJsonMapping_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->jm_lock)
#define DeeJsonMapping_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->jm_lock)
#define DeeJsonMapping_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->jm_lock)
#define DeeJsonMapping_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->jm_lock)
#define DeeJsonMapping_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->jm_lock)
#define DeeJsonMapping_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->jm_lock)
#define DeeJsonMapping_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->jm_lock)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject    *ji_owner;  /* [1..1][const] Object that owns the memory that underlies `ji_parser'. */
	struct json_parser ji_parser; /* Parser that points to (depending on this being a sequence- or mapping-iterator):
	                               * - the next not-yet-yielded sequence element, or the closing `]' token
	                               * - the name of the next not-yet-yielded mapping member, or the closing `}' token
	                               * All fields except for the `jp_pos' are [const], and `jp_pos' must be
	                               * accessed atomically. */
} DeeJsonIteratorObject;

/* Safely load the current parser of a given JSON iterator. */
#define DeeJsonIterator_GetParser(self, result)                            \
	(void)((result)->jp_start    = (self)->ji_parser.jp_start,             \
	       (result)->jp_end      = (self)->ji_parser.jp_end,               \
	       (result)->jp_pos      = atomic_read(&(self)->ji_parser.jp_pos), \
	       (result)->jp_encoding = (self)->ji_parser.jp_encoding)
#define DeeJsonIterator_GetParserEx(self, result)                  \
	(void)(DeeJsonIterator_GetParser(self, &(result)->djp_parser), \
	       (result)->djp_owner = (self)->ji_owner)



/* Internal types used for sequence proxies that perform JIT parsing of underlying JSON blobs. */
INTDEF DeeTypeObject DeeJsonSequence_Type;
INTDEF DeeTypeObject DeeJsonSequenceIterator_Type;
INTDEF DeeTypeObject DeeJsonMapping_Type;
INTDEF DeeTypeObject DeeJsonMappingIterator_Type;


typedef struct {
	struct json_parser djp_parser; /* Underlying parser */
	DeeObject         *djp_owner;  /* [1..1] Underlying object that owns `djp_parser' */
} DeeJsonParser;



/* Parse a single JSON element and convert it into an object.
 * Upon success, the given parser `self' will point at the first
 * token *after* the just-parsed JSON element (though only when
 * the `must_advance_parser' parameter is set to true).
 *
 * Json elements are converted to deemon objects as follows:
 * - `null'           ->  `DeeNone_Type'
 * - `true', `false'  ->  `DeeBool_Type'
 * - `123'            ->  `DeeInt_Type'
 * - `123.4'          ->  `DeeFloat_Type'
 * - `"foo"'          ->  `DeeString_Type'
 * - `[x, y, z]'      ->  `DeeJsonSequence_Type'
 * - `{ "foo": 42 }'  ->  `DeeJsonMapping_Type'
 *
 * Note that this covers all valid JSON constructs, and that those
 * constructs that allow for the recursive inclusion of other
 * constructs are parsed and returned as abstract sequence proxies
 * that will parse contained JSON constructs as they are accessed.
 *
 * @param: must_advance_parser: When set to `false', there is no
 *                              need to advance `self', though this
 *                              may still be done (meaning that when
 *                              this parameter is `false', `self'
 *                              will be left in an undefined state)
 * @return: * : The equivalent deemon object of the just-parsed JSON
 * @return: NULL: An error was thrown. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseObject(DeeJsonParser *__restrict self,
                    bool must_advance_parser);

/* Check what's the canonical type of whatever is about to be parsed by `self' */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeJson_PeekCanonicalObjectType(struct json_parser const *__restrict self);

/* Helper to specifically parse strings. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseString(struct json_parser *__restrict self);

/* Helper to specifically parse integers / floats. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeJson_ParseNumber(struct json_parser *__restrict self);

/* Implement the functionality of parsing JSON *into* the attributes
 * of a given object `into'. Note that for this purpose, it is OK if
 * the object specifies more fields than are provided by `self', but
 * it is not OK if `self' tries to set a non-existent field, or (when
 * the field being set has type annotation) tries to set a field with
 * a type annotation that disallows the type of object given by `self'
 *
 * Additionally, when type annotation specifies a type other than the
 * standard set of literals supported by JSON (bool, none, string, int,
 * float, Sequence or Mapping), such as a user-defined class, then an
 * instance of that class will be created and recursively parsed into.
 *
 * NOTE: This function also supports user-defined struct types defined
 *       by the `ctypes' dex.
 *
 * @return: 1 : `throw_error_if_typing_fails' is `false' and typing failed
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeJson_ParseInto(DeeJsonParser *__restrict self, DeeObject *into,
                  bool must_advance_parser,
                  bool throw_error_if_typing_fails);

/* Similar to `DeeJson_ParseInto()', but construct an instance of `into_type' and populate it.
 * @return: *   : Success
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: `throw_error_if_typing_fails' is `false' and typing failed */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeJson_ParseIntoType(DeeJsonParser  *__restrict self,
                      DeeTypeObject *into_type,
                      bool must_advance_parser,
                      bool throw_error_if_typing_fails);

/* Same as `DeeJson_ParseInto()', but don't actually do any JSON-parsing,
 * but simply read out the elements of `mapping' and assign them to the
 * attributes of `self', whilst doing the same special handling that is
 * also done by `DeeJson_ParseInto()' for type annotations.
 *
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeObject_PopulateFromMapping(DeeObject *self, DeeObject *mapping);


typedef struct {
	struct json_writer djw_writer;    /* Underlying writer */
	DeeObject         *djw_recursion; /* [1..1] Recursion handler, or `Dee_None' */
	DeeObject         *djw_obj;       /* [?..1] The object whose attributes are currently being enumerated */
	DeeHashSetObject   djw_active;    /* Set of object that are currently being enumerated */
} DeeJsonWriter;

#define DeeJsonWriter_Init(self, printer, arg, format)               \
	(json_writer_init(&(self)->djw_writer, printer, arg, format),    \
	 DeeObject_InitInherited(&(self)->djw_active, &DeeHashSet_Type), \
	 (*DeeHashSet_Type.tp_init.tp_alloc.tp_ctor)(Dee_AsObject(&(self)->djw_active)))
#define DeeJsonWriter_Fini(self)                                            \
	((*DeeHashSet_Type.tp_init.tp_dtor)(Dee_AsObject(&(self)->djw_active)), \
	 json_writer_fini(&(self)->djw_writer))
#define DeeJsonWriter_IsActive(self, obj)     DeeHashSet_Contains(Dee_AsObject(&(self)->djw_active), obj)
#define DeeJsonWriter_InsertActive(self, obj) DeeHashSet_Insert(Dee_AsObject(&(self)->djw_active), obj)
#define DeeJsonWriter_RemoveActive(self, obj) DeeHashSet_Remove(Dee_AsObject(&(self)->djw_active), obj)

/* Convert an object `obj' to JSON and write said JSON to `self'.
 * This function supports the same set of object types as are supported
 * by the parsing set of functions above, and you can configure `self'
 * to either produce JSON in a compact (`JSON_WRITER_FORMAT_COMPACT')
 * or pretty (`JSON_WRITER_FORMAT_PRETTY') format.
 *
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE NONNULL((1, 2)) int DCALL
DeeJson_WriteObject(DeeJsonWriter *__restrict self,
                    DeeObject *__restrict obj);

DECL_END

#endif /* !GUARD_DEX_JSON_LIBJSON_H */
