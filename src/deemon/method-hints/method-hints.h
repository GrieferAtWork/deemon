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

/* !!!!!!!!!!!!!!!!!!
 * After making changes to this file (or any of the files %[include]-ed below), you must run:
 * >> deemon -F include/deemon/method-hints.h include/deemon/operator-hints.h src/deemon/objects/generic-proxy.h src/deemon/objects/generic-proxy.c src/deemon/runtime/method-hint-defaults.h src/deemon/runtime/method-hint-defaults.c src/deemon/runtime/method-hints.h src/deemon/runtime/method-hints.c src/deemon/runtime/method-hint-select.h src/deemon/runtime/method-hint-select.c src/deemon/runtime/method-hint-super.h src/deemon/runtime/method-hint-super.c src/deemon/runtime/method-hint-super-invoke.c src/deemon/runtime/method-hint-wrappers.c src/deemon/runtime/operator-hints.c src/deemon/runtime/operator-hint-defaults.c src/deemon/runtime/operator-hint-errors.h src/deemon/runtime/operator-hint-errors.c src/deemon/runtime/operator-hint-invoke.c src/deemon/runtime/strings.h lib/rt/hints/method.dee lib/rt/hints/operator.dee
 */

/************************************************************************/
/* For `deemon.Object'                                                  */
/************************************************************************/
%[include("object_operator_assign.h")]
%[include("object_operator_moveassign.h")]
%[include("object_operator_str.h")]
%[include("object_operator_repr.h")]
%[include("object_operator_bool.h")]
%[include("object_operator_call.h")]
%[include("object_operator_next.h")]
%[include("object_operator_int.h")]
%[include("object_operator_float.h")]
%[include("object_operator_hash.h")]
%[include("object_operator_cmp.h")]
%[include("object_operator_iter.h")]
%[include("object_operator_size.h")]
%[include("object_operator_contains.h")]
%[include("object_operator_getitem.h")]
%[include("object_operator_delitem.h")]
%[include("object_operator_setitem.h")]
%[include("object_operator_getrange.h")]
%[include("object_operator_delrange.h")]
%[include("object_operator_setrange.h")]
%[include("object_operator_misc.h")]
%[include("object_operator_getattr.h")]
%[include("object_operator_delattr.h")]
%[include("object_operator_setattr.h")]


/* TODO: Get rid of all the `explicit_*' "declNameAlias" once
 *       `CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS' becomes
 *       mandatory.
 *
 * Instead of:
 * >> TYPE_METHOD_HINTREF(explicit_seq_removeif)
 *
 * Types should need to write the actual name of the explicit attribute:
 * >> TYPE_METHOD_HINTREF(__seq_removeif__)
 */

/************************************************************************/
/* For `deemon.Sequence'                                                */
/************************************************************************/
%[include("seq_operator_bool.h")]
%[include("seq_operator_size.h")]
%[include("seq_operator_iter.h")]

%[include("seq_operator_getitem.h")]
%[include("seq_operator_delitem.h")]
%[include("seq_operator_setitem.h")]

%[include("seq_operator_getrange.h")]
%[include("seq_operator_delrange.h")]
%[include("seq_operator_setrange.h")]

%[include("seq_operator_assign.h")]

%[include("seq_operator_hash.h")]
%[include("seq_operator_compare.h")]
%[include("seq_operator_cmp.h")]

%[include("seq_operator_inplace_add.h")]
%[include("seq_operator_inplace_mul.h")]

%[include("seq_enumerate.h")]
%[include("seq_enumerate_items.h")]

%[include("seq_enumerate_reverse.h")]

/* TODO: __seq_getitem_always_bound__
 * Not actually a method hint, but rather a behavioral hint:
 * >> public static final member __seq_getitem_always_bound__: bool;
 *
 * When defined as a class member of a type, and evaluating to
 * "true", __seq_getitem__ (or "operator []" when the type
 * classifies as "Sequence") will never throw UnboundItem; it
 * may only ever throw IndexError (if UnboundItem is still
 * thrown, behavior is weak undefined, meaning no crash, but
 * probably incorrect semantics)
 *
 *
 * To facilitate this property, add a way for method hint magic
 * to define type trait hints. These trait hints should also be
 * able to define more than just boolean-traits!
 *
 * XXX: What should happen when the trait is implemented as a
 *      property, and trying to access that property throws an
 *      error?
 *      >> public static property __seq_getitem_always_bound__: bool = {
 *      >>     get(): bool {
 *      >>         throw Error("Uh'oh");
 *      >>     }
 *      >> };
 *      Should property traits just be ignored? (meaning that
 *      only "final member" and "TYPE_MEMBER_CONST" be allowed?
 *      It would make sense in a way, and prevent this whole
 *      problem)
 */

%[include("seq_unpack.h")]
%[include("seq_unpack_ub.h")]

%[include("seq_first.h")]
%[include("seq_last.h")]

/* TODO: Similar to the old method hint system, the new system
 *       should try to cache the optimal impls for first/last/...
 *       when "Sequence.first" is directly invoked:
 *       >> class MyClass: Sequence {
 *       >>     property __seq_first__ = { get() -> 42; }
 *       >> }
 *       >> print MyClass().first; // Should cache `seq_getfirst' in `MyClass.tp_cache["first"]'
 */


%[include("seq_cached.h")]
%[include("seq_frozen.h")]

%[include("seq_any.h")]
%[include("seq_all.h")]
%[include("seq_parity.h")]
%[include("seq_reduce.h")]
%[include("seq_minmax.h")]
%[include("seq_sum.h")]
%[include("seq_count.h")]
%[include("seq_operator_contains.h")]

%[include("seq_locate.h")]
%[include("seq_rlocate.h")]

%[include("seq_startswith.h")]
%[include("seq_endswith.h")]

%[include("seq_find.h")]
%[include("seq_rfind.h")]

%[include("seq_erase.h")]
%[include("seq_insert.h")]
%[include("seq_insertall.h")]
%[include("seq_pushfront.h")]
%[include("seq_append.h")]
%[include("seq_extend.h")]
%[include("seq_xchitem.h")]
%[include("seq_clear.h")]
%[include("seq_pop.h")]

%[include("seq_remove.h")]
%[include("seq_rremove.h")]
%[include("seq_removeall.h")]
%[include("seq_removeif.h")]

%[include("seq_resize.h")]
%[include("seq_fill.h")]
%[include("seq_reverse.h")]
%[include("seq_reversed.h")]
%[include("seq_sort.h")]
%[include("seq_sorted.h")]

%[include("seq_bsearch.h")]




/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
%[include("set_operator_iter.h")]
%[include("set_operator_size.h")]
%[include("set_operator_hash.h")]
%[include("set_operator_compare_eq.h")]
%[include("set_operator_cmp.h")]

%[include("set_operator_inv.h")]
%[include("set_operator_add.h")]
%[include("set_operator_sub.h")]
%[include("set_operator_and.h")]
%[include("set_operator_xor.h")]

%[include("set_operator_inplace_add.h")]
%[include("set_operator_inplace_sub.h")]
%[include("set_operator_inplace_and.h")]
%[include("set_operator_inplace_xor.h")]

%[include("set_frozen.h")]
// TODO: set_cached

%[include("set_unify.h")]
%[include("set_insert.h")]
%[include("set_insertall.h")]

%[include("set_remove.h")]
%[include("set_removeall.h")]

%[include("set_pop.h")]






/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/

/* Need a dedicated "map_operator_iter" (using "set_operator_iter" doesn't work):
 * >> assert 1 == #({ ("foo", 10), ("foo", 20) } as Mapping);
 * ^ This would fail because it gets treated as:
 * >> assert 1 == #({ ("foo", 10), ("foo", 20) } as Set);
 * ... which actually does evaluate to 2 (because the 2 pairs have a different 2nd item)
 * For that reason, also need "map_operator_size" and "map_operator_sizeob" */
%[include("map_operator_iter.h")]
%[include("map_operator_size.h")]

%[include("map_operator_getitem.h")]
%[include("map_operator_delitem.h")]
%[include("map_operator_setitem.h")]
%[include("map_operator_contains.h")]

%[include("map_keys.h")]
%[include("map_iterkeys.h")]

%[include("map_values.h")]
%[include("map_itervalues.h")]

%[include("map_enumerate.h")]
%[include("map_enumerate_items.h")]

%[include("map_operator_compare_eq.h")]
%[include("map_operator_cmp.h")]

%[include("map_operator_add.h")]
%[include("map_operator_sub.h")]
%[include("map_operator_and.h")]
%[include("map_operator_xor.h")]

%[include("map_operator_inplace_add.h")]
%[include("map_operator_inplace_sub.h")]
%[include("map_operator_inplace_and.h")]
%[include("map_operator_inplace_xor.h")]

%[include("map_frozen.h")]
// TODO: map_cached

%[include("map_setold.h")]
%[include("map_setold_ex.h")]
%[include("map_setnew.h")]
%[include("map_setnew_ex.h")]
%[include("map_setdefault.h")]

%[include("map_update.h")]

%[include("map_remove.h")]
%[include("map_removekeys.h")]

%[include("map_pop.h")]
%[include("map_popitem.h")]





/* TODO: Once everything has been migrated, write a unit test that asserts that the correct
 *       default impls of method hints get linked for certain operators:
 *       >> assert "default__seq_operator_getitem_index__with__seq_operator_foreach" == (
 *       >>         class { __seq_iter__() -> []{ yield 10, 20; }.operator iter(); }
 *       >> ).__method_hints__["seq_operator_getitem_index"];
 * To get the names of symbols on...
 * - ...windows: https://learn.microsoft.com/en-us/windows/win32/debug/retrieving-symbol-information-by-address
 * - ...linux:   use `ipc.Process("addr2line -f")'
 * It's OK if it doesn't work in all environments (especially since it can't
 * if you've deleted the .pdb files, or did `strip deemon`). The "assert"
 * above is just meant to illustrate a point, and the actual user-code API
 * to access symbol names of internal method hints will look different (and
 * have an implicit fallback for when the runtime can't determine the names
 * of symbols, in which case nothing will be asserted)
 */





/************************************************************************/
/* For `deemon.Numeric'                                                 */
/************************************************************************/
/* TODO: __numeric_add__ (including the "operator + (rhs) { return this - (-rhs); }" alias) */
/* TODO: __numeric_sub__ (including the "operator - (rhs) { return this + (-rhs); }" alias) */
/* TODO: __numeric_mul__ */
/* TODO: __numeric_...__ */
/* TODO: __numeric_inplace_add__ (including the "operator += (rhs) { return this -= (-rhs); }" alias) */
/* TODO: __numeric_inplace_sub__ (including the "operator -= (rhs) { return this += (-rhs); }" alias) */
/* TODO: __numeric_inplace_mul__ */
/* TODO: __numeric_inplace_...__ */
/* TODO: __numeric_divmod__ */
/* TODO: __numeric_isfloat__ */
/* TODO: __numeric_...__ (Just look at the other functions currently defined by "Numeric") */
