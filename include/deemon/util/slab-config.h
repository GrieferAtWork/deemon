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
/*!export **/
/*!export Dee_SLAB_**/
#ifndef GUARD_DEEMON_UTIL_SLAB_CONFIG_H
#define GUARD_DEEMON_UTIL_SLAB_CONFIG_H 1 /*!export-*/

#include "../api.h"

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

/* Host-specific slab configuration:
 *
 * >> #define Dee_SLAB_CHUNKSIZE_MIN               <Smallest slab size from Dee_SLAB_CHUNKSIZE_FOREACH()>
 * >> #define Dee_SLAB_CHUNKSIZE_MAX               <Greatest slab size from Dee_SLAB_CHUNKSIZE_FOREACH()>
 * >> #define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    <Invoke 'cb(n, _)' for every supported slab size>
 * >> #define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) <Invoke 'cb(n, _)' for every supported GC slab size>
 *
 * Notes on GC slabs:
 * - GC slabs aren't actually distinct from regular slabs, they only need
 *   a different enumeration and name because they allocate slabs that have
 *   some extra space for a leading `struct Dee_gc_head'
 * - As such (with 32-bit pointers), `DeeGCSlab_Malloc16()' would just be a
 *   wrapper around `DeeSlab_Malloc24()' that adjusts the returned pointer
 *   by an offset of `8' bytes (on success).
 * - Also as a consequence, the macro `Dee_SLAB_CHUNKSIZE_GC_FOREACH()'
 *   doesn't actually add anything new (the set of supported GC slabs is
 *   already defined by `Dee_SLAB_CHUNKSIZE_FOREACH()'), only that another
 *   macro is needed because the preprocessor can't evaluate the logic:
 *   >> SLABS    = {<some set of integers>}
 *   >> GC_SLABS = SOME_SUBSET_OF((
 *   >>     for (local n: SLABS)
 *   >>         if (n > sizeof(struct Dee_gc_head))
 *   >>             n - sizeof(struct Dee_gc_head)
 *   >> ).asset)
 *   iow: every slab (can) have a GC sibling after subtracting
 *        `sizeof(struct Dee_gc_head)' from the slab's size.
 */

/* To tune GC slab sizes (and to figure out which types use slab allocators),
 * you can use the following deemon script:
 * ```deemon
import rt;
import * from deemon;
local typesBySize: {int: Type} = Dict();
for (local t: rt.__globals__) {
	if (t is rt.CMethod0)
		t = t();
	if (t is Type && !Type.__isvariable__(t)) {
		local s = try ( // Not completely right: assumes that "sizeof(DeeObject) == sizeof(Dee_gc_head)"
			Type.__instancesize__(t) + (Type.__isgc__(t) ? Object.__instancesize__ : 0)
		) catch (...) -1;
		typesBySize.setdefault(s, []).append(t);
	}
}
for (local s: typesBySize.keys.sorted())
	print(s, ": ", #typesBySize[s], " types");
for (local s: typesBySize.keys.sorted()) {
	local tps = typesBySize[s];
	print(s, ":");
	for (local t: tps)
		print("\t", Type.__hascustomallocator__(t), " ", repr t);
}
```
 * Currently, this slab size configuration covers pretty much all internals,
 * with a fairly even spread of types using the different allocators:
 * >> 12: 47 types
 * >> 16: 81 types
 * >> 20: 41 types
 * >> 24: 36 types
 * >> 32: 30 types
 * >> 40: 29 types
 * >> 52: 12 types
 *
 * The only types (where adding slab support might improve
 * performance) that don't fall into any slab allocator are:
 * - 88:  _FileBuffer              (as returned by `File.open()')
 * - 92:  _YieldFunctionIterator   (as produced by `operator iter()' on user-defined yield functions)
 *
 * Some more types where performance gains wouldn't
 * matter (due to the type only being used rarely):
 * - 100: Thread    (wouldn't matter: thread's aren't created/destroyed that often)
 * - 252: Type      (wouldn't matter: creating user-defined types on-the-fly has a
 *                   far greater GC-overhead, than any overhead from not having a
 *                   slab. Plus: types change size quite often)
 * - 296: _FileType (...)
 * - 408: _Compiler (Compilation only happens once)
 *
 * Some other internal types that *would* like to use slabs:
 * - 56: "struct ast"     (<deemon/compiler/ast.h>)
 * - 92: "struct symbol"  (<deemon/compiler/symbol.h>)
 */
#undef Dee_SLAB_CHUNKSIZE_MIN
#undef Dee_SLAB_CHUNKSIZE_MAX
#ifdef CONFIG_NO_OBJECT_SLABS
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    /* nothing */
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) /* nothing */
#elif __SIZEOF_POINTER__ == 4
#define Dee_SLAB_CHUNKSIZE_MIN 12
#define Dee_SLAB_CHUNKSIZE_MAX 52
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    cb(12, _) cb(16, _) cb(20, _) cb(24, _) cb(32, _) cb(40, _) cb(52, _)
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) cb(12/*20*/, _) cb(16/*24*/, _) cb(24/*32*/, _) cb(32/*40*/, _) cb(44/*52*/, _)
#elif __SIZEOF_POINTER__ == 8
#define Dee_SLAB_CHUNKSIZE_MIN 24
#define Dee_SLAB_CHUNKSIZE_MAX 104
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    cb(24, _) cb(32, _) cb(40, _) cb(48, _) cb(64, _) cb(80, _) cb(104, _)
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) cb(24/*40*/, _) cb(32/*48*/, _) cb(48/*64*/, _) cb(64/*80*/, _) cb(88/*104*/, _)
#else /* __SIZEOF_POINTER__ == ... */
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    /* nothing */
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) /* nothing */
#endif /* __SIZEOF_POINTER__ != ... */



#ifndef _Dee_PRIVATE_SLAB_SELECT_X
#define _Dee_PRIVATE_SLAB_SELECT_UNPACK(N, PREFIX, f, SUFFIX) N, PREFIX, f, SUFFIX
#define _Dee_PRIVATE_SLAB_SELECT_Z(n, N, PREFIX, f, SUFFIX)   n >= (N) ? (PREFIX f##n SUFFIX):
#define _Dee_PRIVATE_SLAB_SELECT_Y(args)                      _Dee_PRIVATE_SLAB_SELECT_Z args
#define _Dee_PRIVATE_SLAB_SELECT_X(n, args)                   _Dee_PRIVATE_SLAB_SELECT_Y((n, _Dee_PRIVATE_SLAB_SELECT_UNPACK args))
#endif /* !_Dee_PRIVATE_SLAB_SELECT_X */

#ifdef __INTELLISENSE__
/* ... */
#elif (defined(_MSC_VER) && !defined(__clang__) && \
       defined(__cplusplus) && !defined(__NO_builtin_choose_expr))
/* Use some compiler magic to force compile-time evaluation of the appropriate
 * expression in `_Dee_PRIVATE_SLAB_SELECT()' macro expansions, when the "N"
 * argument is a compile-time constant.
 *
 * Without this, MSVC will actually generate code for all dead branches in:
 * >> void *p = sizeof(FOO) <= 4 ? &a : sizeof(FOO) <= 8 ? &b : sizeof(FOO) <= 12 ? &c : ...;
 * Instead of just evaluating "sizeof(FOO)" at compile-time, and only emitting
 * the branch that is actually reachable (no idea why it does this).
 *
 * Anyways: by doing this special handling here, debug builds of deemon are
 *          almost 500KiB smaller than without this trick, which is kind-of
 *          important since this get rids of lots of bloat that'd only be
 *          there to ruin performance metrics, coverage, and easy of debugging
 *
 * Explanation: See BCE implementation below (this one does pretty much the same
 *              using the same trick used to implement __builtin_constant_p and
 *              __builtin_choose_expr using MSVC's __if_exists extension)
 *
 * As a matter of fact: without this hack/work-around here, MSVC will emit runtime
 * code to facilitate initialization of "Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC", and
 * in turn: initialization of pretty much every statically defined "DeeTypeObject",
 * including the ability to set breakpoints within the static initializers (which
 * is a completely unnecessary and useless thing to be able to do). */
#define _Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN_Z(n, N, PREFIX, f, SUFFIX) \
	__STATIC_IF(n>=(N)){(PREFIX f##n SUFFIX)}__STATIC_ELSE(n>=(N)){ /* Closed by `_Dee_PRIVATE_SLAB_SELECT_MSVC_CLOS' */
#define _Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN_Y(args)  _Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN_Z args
#define _Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN(n, args) _Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN_Y((n, _Dee_PRIVATE_SLAB_SELECT_UNPACK args))
#define _Dee_PRIVATE_SLAB_SELECT_MSVC_CLOS(n, _) }
#define _Dee_PRIVATE_SLAB_SELECT_IMPL(foreach, N, PREFIX, f, SUFFIX, else)     \
	__if_exists(::__intern::__msvc_static_if<!!(N) || !(N)>::__is_true__)      \
	{(foreach(_Dee_PRIVATE_SLAB_SELECT_MSVC_OPEN, (N, PREFIX, f, SUFFIX)) else \
	  foreach(_Dee_PRIVATE_SLAB_SELECT_MSVC_CLOS, ~))}                         \
	__if_not_exists(::__intern::__msvc_static_if<!!(N) || !(N)>::__is_true__)  \
	{(foreach(_Dee_PRIVATE_SLAB_SELECT_X, (N, PREFIX, f, SUFFIX)) else)}
#define _Dee_PRIVATE_SLAB_SELECT(N, PREFIX, f, SUFFIX, else) \
	_Dee_PRIVATE_SLAB_SELECT_IMPL(Dee_SLAB_CHUNKSIZE_FOREACH, N, PREFIX, f, SUFFIX, else)
#define _Dee_PRIVATE_SLAB_GC_SELECT(N, PREFIX, f, SUFFIX, else) \
	_Dee_PRIVATE_SLAB_SELECT_IMPL(Dee_SLAB_CHUNKSIZE_GC_FOREACH, N, PREFIX, f, SUFFIX, else)
#elif (!defined(__NO_builtin_constant_p) &&  \
       !defined(__NO_builtin_choose_expr) && \
       !defined(__builtin_choose_expr))
/* BCE = BuiltinChooseExpr
 *
 * Force compile-time branch selection (when possible) by doing:
 * >> __builtin_choose_expr(
 * >>     __builtin_constant_p(N),
 * >>     __builtin_choose_expr(4   >= (N), PREFIX f##4   SUFFIX,
 * >>     __builtin_choose_expr(8   >= (N), PREFIX f##8   SUFFIX,
 * >>     __builtin_choose_expr(12  >= (N), PREFIX f##12  SUFFIX,
 * >>     __builtin_choose_expr(... >= (N), PREFIX f##... SUFFIX,
 * >>                           else
 * >>     )))),
 * >>     4   >= (N) ? PREFIX f##4   SUFFIX :
 * >>     8   >= (N) ? PREFIX f##8   SUFFIX :
 * >>     12  >= (N) ? PREFIX f##12  SUFFIX :
 * >>     ... >= (N) ? PREFIX f##... SUFFIX :
 * >>     else
 * >> )
 */
#define _Dee_PRIVATE_SLAB_SELECT_BCE_OPEN_Z(n, N, PREFIX, f, SUFFIX) \
	__builtin_choose_expr(n>=(N), PREFIX f##n SUFFIX, /* Closed by `_Dee_PRIVATE_SLAB_SELECT_BCE_CLOS' */
#define _Dee_PRIVATE_SLAB_SELECT_BCE_OPEN_Y(args)  _Dee_PRIVATE_SLAB_SELECT_BCE_OPEN_Z args
#define _Dee_PRIVATE_SLAB_SELECT_BCE_OPEN(n, args) _Dee_PRIVATE_SLAB_SELECT_BCE_OPEN_Y((n, _Dee_PRIVATE_SLAB_SELECT_UNPACK args))
#define _Dee_PRIVATE_SLAB_SELECT_BCE_CLOS(n, _) )
#define _Dee_PRIVATE_SLAB_SELECT_IMPL(foreach, N, PREFIX, f, SUFFIX, else)                        \
	__builtin_choose_expr(__builtin_constant_p(N),                                                \
	                      foreach(_Dee_PRIVATE_SLAB_SELECT_BCE_OPEN, (N, PREFIX, f, SUFFIX)) else \
	                      foreach(_Dee_PRIVATE_SLAB_SELECT_BCE_CLOS, ~),                          \
	                      foreach(_Dee_PRIVATE_SLAB_SELECT_X, (N, PREFIX, f, SUFFIX)) else)
#define _Dee_PRIVATE_SLAB_SELECT(N, PREFIX, f, SUFFIX, else) \
	_Dee_PRIVATE_SLAB_SELECT_IMPL(Dee_SLAB_CHUNKSIZE_FOREACH, N, PREFIX, f, SUFFIX, else)
#define _Dee_PRIVATE_SLAB_GC_SELECT(N, PREFIX, f, SUFFIX, else) \
	_Dee_PRIVATE_SLAB_SELECT_IMPL(Dee_SLAB_CHUNKSIZE_GC_FOREACH, N, PREFIX, f, SUFFIX, else)
#endif /* ... */

#ifndef _Dee_PRIVATE_SLAB_SELECT
#define _Dee_PRIVATE_SLAB_SELECT(N, PREFIX, f, SUFFIX, else) \
	(Dee_SLAB_CHUNKSIZE_FOREACH(_Dee_PRIVATE_SLAB_SELECT_X, (N, PREFIX, f, SUFFIX)) else)
#define _Dee_PRIVATE_SLAB_GC_SELECT(N, PREFIX, f, SUFFIX, else) \
	(Dee_SLAB_CHUNKSIZE_GC_FOREACH(_Dee_PRIVATE_SLAB_SELECT_X, (N, PREFIX, f, SUFFIX)) else)
#endif /* !_Dee_PRIVATE_SLAB_SELECT */


#endif /* !GUARD_DEEMON_UTIL_SLAB_CONFIG_H */
