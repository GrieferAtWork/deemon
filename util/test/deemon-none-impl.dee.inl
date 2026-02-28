#!/usr/bin/deemon
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

import * from deemon;
import * from errors;
import .common.test;

#define TRY(x) (try x catch (e...) e)

final local None = type(none);

/* Sanity-check the "is none" expression */
assert None is Type;
assert none is None;
test.assertNone(none);

/* Sanity-check that stuff that isn't "none"... isn't none */
assert type(none) !is none;
assert true !is none;
assert false !is none;
assert 42 !is none;
assert "foo" !is none;

/* Assert constructors of "none" */
test.assertNone(None());
test.assertNone(None("args", "are", "ignored"));
test.assertNone(None(keyword: "args", are: "also", ignored: true));
test.assertNone(copy none);
test.assertNone(deepcopy none);

/* Assert string operators */
test.assertEqualsTyped("none", str none);
test.assertEqualsTyped("none", repr none);

/* Assert that interaction with bools works ("none" should evaluate to "false" here) */
assert false == none;
assert true  == !none;
assert false == !!none;

/* Assert that interaction with int/float works ("none" should evaluate to 0/0.0 here) */
test.assertEqualsTyped(0, int(none));
test.assertEqualsTyped(0, none.operator int());
if (float.hasoperator("constructor")) {
	test.assertEqualsTyped(0.0, float(none));
	test.assertEqualsTyped(0.0, none.operator float());
}
#if __DEEMON_VARIANT__ == "gatw"
/* This hash-value isn't a hard requirement -- other deemon impls might return something else */
test.assertEqualsTyped(0, hash(none));
test.assertEqualsTyped(0, Sequence.__hash__(none));
test.assertEqualsTyped(0, Set.__hash__(none));
test.assertEqualsTyped(0, Mapping.__hash__(none));
#endif /* __DEEMON_VARIANT__ == "gatw" */

/* Assert that comparison works for "none". Even though these could re-return "none",
 * comparison actually returns bools since that's more useful as it allows for doing
 * conventional "none == EXPR" to see if "EXPR is none"
 *
 * Also note for this purpose that "none" is supposed to be "less" than anything other
 * than itself (which it is equal to). As such, when sorting a list containing "none",
 * those "none" instances will always be moved to the front of the list. */
test.assertEqualsTyped(0,     compare(none, none));
test.assertEqualsTyped(true,  equals(none, none));
test.assertEqualsTyped(true,  none == none);
test.assertEqualsTyped(false, none != none);
test.assertEqualsTyped(false, none <  none);
test.assertEqualsTyped(true,  none <= none);
test.assertEqualsTyped(false, none >  none);
test.assertEqualsTyped(true,  none >= none);
for (local someObject: { 42, "foo", false, Object() }) {
	test.assertEqualsTyped(-1,    compare(none, someObject));
	test.assertEqualsTyped(false, equals(none, someObject));
	test.assertEqualsTyped(false, none == someObject);
	test.assertEqualsTyped(true,  none != someObject);
	test.assertEqualsTyped(true,  none <  someObject);
	test.assertEqualsTyped(true,  none <= someObject);
	test.assertEqualsTyped(false, none >  someObject);
	test.assertEqualsTyped(false, none >= someObject);
}


/* Here are some operators that must be no-ops (but re-return the rhs
 * operand because that's what the compiler enforces for these operators) */
test.assertEqualsTyped(42, none := 42);
test.assertEqualsTyped(43, none.operator move:= (43));


/* Here are some operators that always re-return "none" */
test.assertNone(~none);
test.assertNone(+none);
test.assertNone(-none);
#if 0 /* XXX: maybe compiler should allow this? */
test.assertNone(++none);
test.assertNone(--none);
test.assertNone(none++);
test.assertNone(none--);
#endif
test.assertNone(none.operator iter());
test.assertNone(#none);
assert TRY(none.operator next()) is StopIteration;
test.assertNone(none());
with (none) {
	test.assertNone(none.operator enter());
	test.assertNone(none.operator leave());
}

local anythingList = { none, 0, 42, "foo", false, Object() };
for (local anything: anythingList) {
	test.assertNone(none + anything);
	test.assertNone(none - anything);
	test.assertNone(none * anything);
	test.assertNone(none / anything);
	test.assertNone(none % anything);
	test.assertNone(none << anything);
	test.assertNone(none >> anything);
	test.assertNone(none & anything);
	test.assertNone(none | anything);
	test.assertNone(none ^ anything);
	test.assertNone(none ** anything);
#if 0 /* XXX: maybe compiler should allow this? */
	test.assertNone(none += anything);
	test.assertNone(none -= anything);
	test.assertNone(none *= anything);
	test.assertNone(none /= anything);
	test.assertNone(none %= anything);
	test.assertNone(none <<= anything);
	test.assertNone(none >>= anything);
	test.assertNone(none &= anything);
	test.assertNone(none |= anything);
	test.assertNone(none ^= anything);
	test.assertNone(none **= anything);
#endif
	/* Could have also returned "none" for this one, but chose not to. */
	test.assertNone(anything in none);
	test.assertNone(none[anything]);
	test.assertEqualsTyped(true, none[anything] is bound);
	test.assertEqualsTyped(true, bounditem(none, anything, false));
	test.assertEqualsTyped(true, bounditem(none, anything, true));
	test.assertEqualsTyped(true, bounditem(none, anything));
	test.assertEqualsTyped(true, hasitem(none, anything));
	test.assertNone(none[anything:anything]);
	del none[anything];
	del none[anything:anything];
	test.assertEqualsTyped(44, none[anything] = 44);
	test.assertEqualsTyped(44, none[anything:anything] = 44);
	test.assertNone(none(anything));
	test.assertNone(none(anything, anything, anything));
	test.assertNone(none(foo: anything, bar: anything));
	test.assertNone(none(anything, foo: anything, bar: anything));
}

for (local attr: {
		"anyAttribute", "any", "attribute",
		"literally", "just", "anything"
}) {
	test.assertNone(none.operator . (attr));
	test.assertEqualsTyped(true, none.operator . (attr) is bound);
	test.assertEqualsTyped(true, boundattr(none, attr, false));
	test.assertEqualsTyped(true, boundattr(none, attr, true));
	test.assertEqualsTyped(true, boundattr(none, attr));
	del none.operator . (attr);
	test.assertEqualsTyped(42, none.operator . (attr) = 42);
}


test.assertEqualsTyped(Tuple(), Tuple(none));
test.assertEqualsTyped(List(), List(none));
test.assertEqualsTyped(Dict(), Dict(none));
test.assertEqualsTyped(Bytes(), Bytes(none));
test.assertEqualsTyped("none", string(none));

/* "none" also has a bunch of special behavior when used as Sequence/Set/Mapping */
for (local anything: anythingList) {
	test.assertNone(Sequence.__getitem__(none, anything));
	test.assertNone(Sequence.__delitem__(none, anything));
	test.assertNone(Sequence.__setitem__(none, anything, anything));
	test.assertNone(Sequence.__getrange__(none, anything, anything));
	test.assertNone(Sequence.__delrange__(none, anything, anything));
	test.assertNone(Sequence.__setrange__(none, anything, anything, anything));
	test.assertNone(Sequence.__assign__(none, anything));
	test.assertEqualsTyped(0,     Sequence.__compare__(none, {}));
	test.assertEqualsTyped(-1,    Sequence.__compare__(none, {anything}));
	test.assertEqualsTyped(true,  Sequence.__compare_eq__(none, {}));
	test.assertEqualsTyped(false, Sequence.__compare_eq__(none, {anything}));
	test.assertEqualsTyped(true,  Sequence.__eq__(none, {}));
	test.assertEqualsTyped(false, Sequence.__eq__(none, {anything}));
	test.assertEqualsTyped(false, Sequence.__ne__(none, {}));
	test.assertEqualsTyped(true,  Sequence.__ne__(none, {anything}));
	test.assertEqualsTyped(false, Sequence.__lo__(none, {}));
	test.assertEqualsTyped(true,  Sequence.__lo__(none, {anything}));
	test.assertEqualsTyped(true,  Sequence.__le__(none, {}));
	test.assertEqualsTyped(true,  Sequence.__le__(none, {anything}));
	test.assertEqualsTyped(false, Sequence.__gr__(none, {}));
	test.assertEqualsTyped(false, Sequence.__gr__(none, {anything}));
	test.assertEqualsTyped(true,  Sequence.__ge__(none, {}));
	test.assertEqualsTyped(false, Sequence.__ge__(none, {anything}));
	test.assertNone(Sequence.__inplace_add__(none, {}));
	test.assertNone(Sequence.__inplace_add__(none, {anything}));
	test.assertNone(Sequence.__inplace_mul__(none, {}));
	test.assertNone(Sequence.__inplace_mul__(none, {anything}));

	/* Special handling for "unpack" operations: "none" can be unpacked into **anything** */
	test.assertSequence(Sequence.unpack(none, 0), {});
	test.assertSequence(Sequence.unpack(none, 1), {none});
	test.assertSequence(Sequence.unpack(none, 2), {none, none});
	test.assertSequence(Sequence.unpack(none, 3), {none, none, none});
#if 0 /* TODO: Sequence compare doesn't quite work for nullable types, yet... */
	test.assertSequence(Sequence.unpackub(none, 0), {});
	test.assertSequence(Sequence.unpackub(none, 1), {none});
	test.assertSequence(Sequence.unpackub(none, 2), {none, none});
	test.assertSequence(Sequence.unpackub(none, 3), {none, none, none});
#else
	import rt;
	test.assertEqualsTyped(Sequence.unpackub(none, 0), rt.NullableTuple {});
	test.assertEqualsTyped(Sequence.unpackub(none, 1), rt.NullableTuple {none});
	test.assertEqualsTyped(Sequence.unpackub(none, 2), rt.NullableTuple {none, none});
	test.assertEqualsTyped(Sequence.unpackub(none, 3), rt.NullableTuple {none, none, none});
#endif
	test.assertNone(Sequence.first.set(none, anything));
	test.assertNone(Sequence.last.set(none, anything));
	test.assertEqualsTyped(0, Sequence.count(none, anything));
	test.assertEqualsTyped(0, Sequence.count(none, anything, 10, 20));
	test.assertEqualsTyped(0, Sequence.count(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(0, Sequence.count(none, anything, 10, 20, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.contains(none, anything));
	test.assertEqualsTyped(false, Sequence.contains(none, anything, 10, 20));
	test.assertEqualsTyped(false, Sequence.contains(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.contains(none, anything, 10, 20, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.startswith(none, anything));
	test.assertEqualsTyped(false, Sequence.startswith(none, anything, 10, 20));
	test.assertEqualsTyped(false, Sequence.startswith(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.startswith(none, anything, 10, 20, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.endswith(none, anything));
	test.assertEqualsTyped(false, Sequence.endswith(none, anything, 10, 20));
	test.assertEqualsTyped(false, Sequence.endswith(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.endswith(none, anything, 10, 20, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(-1, Sequence.find(none, anything, 10, 20));
	test.assertEqualsTyped(-1, Sequence.find(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(-1, Sequence.rfind(none, anything, 10, 20));
	test.assertEqualsTyped(-1, Sequence.rfind(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.remove(none, anything, 10, 20));
	test.assertEqualsTyped(false, Sequence.remove(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(false, Sequence.rremove(none, anything, 10, 20));
	test.assertEqualsTyped(false, Sequence.rremove(none, anything, key: x -> { throw "NEVER"; }));
	test.assertNone(Sequence.insert(none, 10, anything));
	test.assertNone(Sequence.insertall(none, 10, {anything}));
	test.assertNone(Sequence.pushfront(none, anything));
	test.assertNone(Sequence.append(none, anything));
	test.assertNone(Sequence.extend(none, {anything}));
	test.assertNone(Sequence.xchitem(none, 10, anything));
	test.assertEqualsTyped(0, Sequence.removeall(none, anything));
	test.assertEqualsTyped(0, Sequence.removeall(none, anything, 10, 20));
	test.assertEqualsTyped(0, Sequence.removeall(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(0, Sequence.removeall(none, anything, 10, 20, key: x -> { throw "NEVER"; }));
	test.assertNone(Sequence.resize(none, 10));
	test.assertNone(Sequence.resize(none, 10, anything));
	test.assertNone(Sequence.fill(none, 10, 20, anything));
	test.assertEqualsTyped(-1, Sequence.bfind(none, anything, 10, 20));
	test.assertEqualsTyped(-1, Sequence.bfind(none, anything, key: x -> { throw "NEVER"; }));
	test.assertEqualsTyped(0, Sequence.bposition(none, anything, 10, 20));
	test.assertEqualsTyped(0, Sequence.bposition(none, anything, key: x -> { throw "NEVER"; }));
	test.assertSequence(Sequence.brange(none, anything, 10, 20), {0,0});
	test.assertSequence(Sequence.brange(none, anything, key: x -> { throw "NEVER"; }), {0,0});

	test.assertEqualsTyped(true,  Set.__compare_eq__(none, {}));
	test.assertEqualsTyped(false, Set.__compare_eq__(none, {anything}));
	test.assertEqualsTyped(true,  Set.__eq__(none, {}));
	test.assertEqualsTyped(false, Set.__eq__(none, {anything}));
	test.assertEqualsTyped(false, Set.__ne__(none, {}));
	test.assertEqualsTyped(true,  Set.__ne__(none, {anything}));
	test.assertEqualsTyped(false, Set.__lo__(none, {}));
	test.assertEqualsTyped(true,  Set.__lo__(none, {anything}));
	test.assertEqualsTyped(true,  Set.__le__(none, {}));
	test.assertEqualsTyped(true,  Set.__le__(none, {anything}));
	test.assertEqualsTyped(false, Set.__gr__(none, {}));
	test.assertEqualsTyped(false, Set.__gr__(none, {anything}));
	test.assertEqualsTyped(true,  Set.__ge__(none, {}));
	test.assertEqualsTyped(false, Set.__ge__(none, {anything}));
	test.assertNone(Set.__inplace_add__(none, {}));
	test.assertNone(Set.__inplace_add__(none, {anything}));
	test.assertNone(Set.__inplace_sub__(none, {}));
	test.assertNone(Set.__inplace_sub__(none, {anything}));
	test.assertNone(Set.__inplace_and__(none, {}));
	test.assertNone(Set.__inplace_and__(none, {anything}));
	test.assertNone(Set.__inplace_xor__(none, {}));
	test.assertNone(Set.__inplace_xor__(none, {anything}));
	test.assertNone(Set.__add__(none, {}));
	test.assertNone(Set.__add__(none, {anything}));
	test.assertNone(Set.__sub__(none, {}));
	test.assertNone(Set.__sub__(none, {anything}));
	test.assertNone(Set.__and__(none, {}));
	test.assertNone(Set.__and__(none, {anything}));
	test.assertNone(Set.__xor__(none, {}));
	test.assertNone(Set.__xor__(none, {anything}));
	test.assertEqualsTyped(false, Set.insert(none, anything));
	test.assertNone(Set.insertall(none, {anything}));
	test.assertEqualsTyped(false, Set.remove(none, anything));
	test.assertNone(Set.removeall(none, {anything}));
	test.assertNone(Set.pop(none));
	test.assertNone(Set.pop(none, anything));
	test.assertNone(Set.first.set(none, anything));
	test.assertNone(Set.last.set(none, anything));

	test.assertNone(Mapping.__getitem__(none, anything));
	test.assertNone(Mapping.__delitem__(none, anything));
	test.assertNone(Mapping.__setitem__(none, anything, anything));
	test.assertEqualsTyped(false, Mapping.__contains__(none, anything));
	test.assertEqualsTyped(true,  Mapping.__compare_eq__(none, {}));
	test.assertEqualsTyped(false, Mapping.__compare_eq__(none, {anything: anything}));
	test.assertEqualsTyped(true,  Mapping.__eq__(none, {}));
	test.assertEqualsTyped(false, Mapping.__eq__(none, {anything: anything}));
	test.assertEqualsTyped(false, Mapping.__ne__(none, {}));
	test.assertEqualsTyped(true,  Mapping.__ne__(none, {anything: anything}));
	test.assertEqualsTyped(false, Mapping.__lo__(none, {}));
	test.assertEqualsTyped(true,  Mapping.__lo__(none, {anything: anything}));
	test.assertEqualsTyped(true,  Mapping.__le__(none, {}));
	test.assertEqualsTyped(true,  Mapping.__le__(none, {anything: anything}));
	test.assertEqualsTyped(false, Mapping.__gr__(none, {}));
	test.assertEqualsTyped(false, Mapping.__gr__(none, {anything: anything}));
	test.assertEqualsTyped(true,  Mapping.__ge__(none, {}));
	test.assertEqualsTyped(false, Mapping.__ge__(none, {anything: anything}));
	test.assertNone(Mapping.__add__(none, {anything: anything}));
	test.assertNone(Mapping.__sub__(none, {anything}));
	test.assertNone(Mapping.__and__(none, {anything}));
	test.assertNone(Mapping.__xor__(none, {anything: anything}));
	test.assertNone(Mapping.__inplace_add__(none, {anything: anything}));
	test.assertNone(Mapping.__inplace_sub__(none, {anything}));
	test.assertNone(Mapping.__inplace_and__(none, {anything}));
	test.assertNone(Mapping.__inplace_xor__(none, {anything: anything}));
	test.assertEqualsTyped(false, Mapping.setold(none, anything, anything));
	test.assertEqualsTyped(false, Mapping.setnew(none, anything, anything));
	test.assertSequence(Mapping.setold_ex(none, anything, anything), {false, none});
	test.assertSequence(Mapping.setnew_ex(none, anything, anything), {false, none});
	test.assertNone(Mapping.setdefault(none, anything, anything));
	test.assertNone(Mapping.update(none, {anything: anything}));
	test.assertEqualsTyped(false, Mapping.remove(none, anything));
	test.assertNone(Mapping.removekeys(none, {anything}));
	test.assertNone(Mapping.pop(none, anything));
	test.assertNone(Mapping.pop(none, anything, anything));
	test.assertSame(anything, Iterator.peek(none, anything));
	test.assertSame(anything, Iterator.prev(none, anything));
}

test.assertEqualsTyped(false, Sequence.__bool__(none));
test.assertNone(Sequence.__size__(none));
test.assertNone(Sequence.__iter__(none));
test.assertNone(Sequence.first(none));
test.assertNone(Sequence.last(none));
test.assertNone(Sequence.cached(none));
test.assertNone(Sequence.frozen(none));
test.assertNone(Sequence.first.delete(none));
test.assertNone(Sequence.last.delete(none));
test.assertEqualsTyped(true, Sequence.first.isbound(none));
test.assertEqualsTyped(true, Sequence.last.isbound(none));
test.assertNone(Sequence.__enumerate__(none, (v?) -> { throw "NEVER"; }));
test.assertNone(Sequence.__enumerate_items__(none));
test.assertEqualsTyped(false, Sequence.any(none));
test.assertEqualsTyped(false, Sequence.any(none, 10, 20));
test.assertEqualsTyped(false, Sequence.any(none, key: x -> { throw "NEVER"; }));
test.assertEqualsTyped(false, Sequence.any(none, 10, 20, key: x -> { throw "NEVER"; }));
test.assertEqualsTyped(true,  Sequence.all(none));
test.assertEqualsTyped(true,  Sequence.all(none, 10, 20));
test.assertEqualsTyped(true,  Sequence.all(none, key: x -> { throw "NEVER"; }));
test.assertEqualsTyped(true,  Sequence.all(none, 10, 20, key: x -> { throw "NEVER"; }));
test.assertEqualsTyped(false, Sequence.parity(none));
test.assertEqualsTyped(false, Sequence.parity(none, 10, 20));
test.assertEqualsTyped(false, Sequence.parity(none, key: x -> { throw "NEVER"; }));
test.assertEqualsTyped(false, Sequence.parity(none, 10, 20, key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }, 10, 20));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }, 10, 20));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }, 10, 20, "INIT"));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }));
test.assertNone(Sequence.reduce(none, (a, b) -> { throw "NEVER"; }, init: "INIT"));
test.assertNone(Sequence.min(none, def: "EMPTY"));
test.assertNone(Sequence.min(none, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.min(none, 10, 20, def: "EMPTY"));
test.assertNone(Sequence.min(none, 10, 20, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.max(none, def: "EMPTY"));
test.assertNone(Sequence.max(none, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.max(none, 10, 20, def: "EMPTY"));
test.assertNone(Sequence.max(none, 10, 20, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.sum(none, def: "EMPTY"));
test.assertNone(Sequence.sum(none, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.sum(none, 10, 20, def: "EMPTY"));
test.assertNone(Sequence.sum(none, 10, 20, def: "EMPTY", key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.locate(none, x -> { throw "NEVER"; }, def: "EMPTY"));
test.assertNone(Sequence.locate(none, x -> { throw "NEVER"; }, 10, 20, def: "EMPTY"));
test.assertNone(Sequence.rlocate(none, x -> { throw "NEVER"; }, def: "EMPTY"));
test.assertNone(Sequence.rlocate(none, x -> { throw "NEVER"; }, 10, 20, def: "EMPTY"));
test.assertNone(Sequence.erase(none, 10, 20));
test.assertNone(Sequence.clear(none));
test.assertEqualsTyped(0, Sequence.removeif(none, x -> { throw "NEVER"; }));
test.assertEqualsTyped(0, Sequence.removeif(none, x -> { throw "NEVER"; }, 10, 20));
test.assertNone(Sequence.reverse(none, 10, 20));
test.assertNone(Sequence.reversed(none, 10, 20));
test.assertNone(Sequence.sort(none));
test.assertNone(Sequence.sort(none, key: x -> { throw "NEVER"; }));
test.assertNone(Sequence.sorted(none));
test.assertNone(Sequence.sorted(none, key: x -> { throw "NEVER"; }));

test.assertEqualsTyped(false, Set.__bool__(none));
test.assertNone(Set.__size__(none));
test.assertNone(Set.__iter__(none));
test.assertNone(Set.__inv__(none));
test.assertNone(Set.cached(none));
test.assertNone(Set.frozen(none));
test.assertNone(Set.first.delete(none));
test.assertNone(Set.last.delete(none));
test.assertEqualsTyped(true, Set.first.isbound(none));
test.assertEqualsTyped(true, Set.last.isbound(none));

test.assertNone(Mapping.__size__(none));
test.assertNone(Mapping.__iter__(none));
test.assertNone(Mapping.keys(none));
test.assertNone(Mapping.iterkeys(none));
test.assertNone(Mapping.values(none));
test.assertNone(Mapping.itervalues(none));
test.assertNone(Mapping.__enumerate__(none, (v?) -> { throw "NEVER"; }));
test.assertNone(Mapping.__enumerate__(none, (v?) -> { throw "NEVER"; }, "foo", "bar"));
test.assertNone(Mapping.__enumerate_items__(none));
test.assertNone(Mapping.__enumerate_items__(none, "foo", "bar"));
test.assertNone(Mapping.cached(none));
test.assertNone(Mapping.frozen(none));
#if 1 /* The other variant would also be OK, but since "Sequence.unpack(none, 2)"
       * behaves identical to  "Sequence.unpack({none, none}, 2)", the runtime is
       * allowed to (and simply does) return "none" in this case also. */
test.assertNone(Mapping.popitem(none));
#else
test.assertSequence(Mapping.popitem(none), {none, none});
#endif

test.assertEqualsTyped(0, Iterator.advance(none, 42));
test.assertEqualsTyped(0, Iterator.revert(none, 42));
test.assertEqualsTyped(false, Iterator.__bool__(none));
assert TRY(Iterator.peek(none)) is StopIteration;
assert TRY(Iterator.prev(none)) is StopIteration;
test.assertNone(Iterator.rewind(none));
test.assertNone(Iterator.seq(none));
test.assertNone(Iterator.index.delete(none));
test.assertNone(Iterator.index.set(none, 42));
test.assertEqualsTyped(0, Iterator.index(none));


/* Assert that "none" can be used as a target of "print"-statements */
print none: "You will never get to see this";

/* XXX: Do some more tests with "none as Object" -- that one seems to behave kind-of weird */
