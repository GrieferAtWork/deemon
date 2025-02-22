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
#ifndef GUARD_DEEMON_CXX_TYPE_H
#define GUARD_DEEMON_CXX_TYPE_H 1

#include "api.h"
/**/

#include "object.h"
#include "numeric.h"
/**/

#include "../format.h"
#include "../object.h"
/**/

DEE_CXX_BEGIN

class Type
	: public Object
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeType_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeType_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeType_CheckExact(ob);
	}

public:
	operator DeeTypeObject &() DEE_CXX_NOTHROW {
		return *(DeeTypeObject *)this;
	}
	operator DeeTypeObject const &() const DEE_CXX_NOTHROW {
		return *(DeeTypeObject const *)this;
	}

public:
	WUNUSED bool cisfinal() const DEE_CXX_NOTHROW {
		return DeeType_IsFinal(this);
	}
	WUNUSED bool cisinterrupt() const DEE_CXX_NOTHROW {
		return DeeType_IsInterrupt(this);
	}
	WUNUSED bool cisabstract() const DEE_CXX_NOTHROW {
		return DeeType_IsAbstract(this);
	}
	WUNUSED bool cisvariable() const DEE_CXX_NOTHROW {
		return DeeType_IsVariable(this);
	}
	WUNUSED bool cisgc() const DEE_CXX_NOTHROW {
		return DeeType_IsGC(this);
	}
	WUNUSED bool cisclass() const DEE_CXX_NOTHROW {
		return DeeType_IsClass(this);
	}
	WUNUSED bool cisarithmetic() const DEE_CXX_NOTHROW {
		return DeeType_IsArithmetic(this);
	}
	WUNUSED bool ciscomparable() const DEE_CXX_NOTHROW {
		return DeeType_IsComparable(this);
	}
	WUNUSED bool cissequence() const DEE_CXX_NOTHROW {
		return DeeType_IsSequence(this);
	}
	WUNUSED bool cisinttruncated() const DEE_CXX_NOTHROW {
		return DeeType_IsIntTruncated(this);
	}
	WUNUSED bool chasmoveany() const DEE_CXX_NOTHROW {
		return DeeType_HasMoveAny(this);
	}
	WUNUSED bool cisiterator() const DEE_CXX_NOTHROW {
		return DeeType_IsIterator(this);
	}
	WUNUSED bool cistypetype() const DEE_CXX_NOTHROW {
		return DeeType_IsTypeType(this);
	}
	WUNUSED bool ciscustom() const DEE_CXX_NOTHROW {
		return DeeType_IsCustom(this);
	}
	WUNUSED bool cissuperconstructible() const DEE_CXX_NOTHROW {
		return DeeType_IsSuperConstructible(this);
	}
	WUNUSED bool cisnoargconstructible() const DEE_CXX_NOTHROW {
		return DeeType_IsNoArgConstructible(this);
	}
	WUNUSED bool cisvarargconstructible() const DEE_CXX_NOTHROW {
		return DeeType_IsVarArgConstructible(this);
	}
	WUNUSED bool cisconstructible() const DEE_CXX_NOTHROW {
		return DeeType_IsConstructible(this);
	}
	WUNUSED bool ciscopyable() const DEE_CXX_NOTHROW {
		return DeeType_IsCopyable(this);
	}
	WUNUSED bool cbase() const DEE_CXX_NOTHROW {
		return DeeType_Base(this);
	}
	WUNUSED bool cgcpriority() const DEE_CXX_NOTHROW {
		return DeeType_GCPriority(this);
	}
	WUNUSED bool chasoperator(uint16_t name) const DEE_CXX_NOTHROW {
		return DeeType_HasOperator((DeeTypeObject *)this, name);
	}
	WUNUSED bool chasprivateoperator(uint16_t name) const DEE_CXX_NOTHROW {
		return DeeType_HasPrivateOperator((DeeTypeObject *)this, name);
	}
	WUNUSED bool cbaseof(DeeTypeObject *subclass_type) const DEE_CXX_NOTHROW {
		return !!DeeType_Extends(subclass_type, (DeeTypeObject *)this);
	}
	WUNUSED bool cextends(DeeTypeObject *extended_type) const DEE_CXX_NOTHROW {
		return !!DeeType_Extends((DeeTypeObject *)this, extended_type);
	}
	WUNUSED bool cimplements(DeeTypeObject *implemented_type) const DEE_CXX_NOTHROW {
		return !!DeeType_Implements((DeeTypeObject *)this, implemented_type);
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Type from deemon).printCxxApi();]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (baseof)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "baseof", _Dee_HashSelectC(0xe19d38f6, 0xe38a0484abc9bda3), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (extends)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "extends", _Dee_HashSelectC(0x7e2f0d24, 0x82d4dfb036a5b738), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (implements)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "implements", _Dee_HashSelectC(0x1c8a7e28, 0x4a1dbab53b386755), 1, args));
	}
	WUNUSED Ref<Object> (newinstance)() {
		return inherit(DeeObject_CallAttrStringHash(this, "newinstance", _Dee_HashSelectC(0x3d87f61d, 0x88cc2893ece6b9f5), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (newinstance)(DeeObject *fields_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer) {
		DeeObject *args[1];
		args[0] = fields_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer_or_initializer;
		return inherit(DeeObject_CallAttrStringHash(this, "newinstance", _Dee_HashSelectC(0x3d87f61d, 0x88cc2893ece6b9f5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasattribute)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "hasattribute", _Dee_HashSelectC(0x846bccee, 0x3b9b37c576849fcd), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (hasattribute)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasattribute", _Dee_HashSelectC(0x846bccee, 0x3b9b37c576849fcd), "s", name));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasprivateattribute)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "hasprivateattribute", _Dee_HashSelectC(0xa6b65eda, 0xf1c5fa687e041db1), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateattribute)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateattribute", _Dee_HashSelectC(0xa6b65eda, 0xf1c5fa687e041db1), "s", name));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasoperator)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (hasoperator)(DeeObject *name, DeeObject *argc) {
		DeeObject *args[2];
		args[0] = name;
		args[1] = argc;
		return inherit(DeeObject_CallAttrStringHash(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasoperator)(DeeObject *name, Dee_ssize_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "o" DEE_PCKdSIZ, name, argc));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasoperator)(DeeObject *name, size_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "o" DEE_PCKuSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasoperator)(Dee_ssize_t name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db),  DEE_PCKdSIZ, name));
	}
	WUNUSED Ref<deemon::bool_> (hasoperator)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "s", name));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (hasoperator)(char const *name, DeeObject *argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "so", name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasoperator)(char const *name, Dee_ssize_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "s" DEE_PCKdSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasoperator)(char const *name, size_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db), "s" DEE_PCKuSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasoperator)(size_t name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasoperator", _Dee_HashSelectC(0x42dfac16, 0x6fce8ab7562b35db),  DEE_PCKuSIZ, name));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasprivateoperator)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (hasprivateoperator)(DeeObject *name, DeeObject *argc) {
		DeeObject *args[2];
		args[0] = name;
		args[1] = argc;
		return inherit(DeeObject_CallAttrStringHash(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasprivateoperator)(DeeObject *name, Dee_ssize_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "o" DEE_PCKdSIZ, name, argc));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasprivateoperator)(DeeObject *name, size_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "o" DEE_PCKuSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateoperator)(Dee_ssize_t name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365),  DEE_PCKdSIZ, name));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateoperator)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "s", name));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (hasprivateoperator)(char const *name, DeeObject *argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "so", name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateoperator)(char const *name, Dee_ssize_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "s" DEE_PCKdSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateoperator)(char const *name, size_t argc) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365), "s" DEE_PCKuSIZ, name, argc));
	}
	WUNUSED Ref<deemon::bool_> (hasprivateoperator)(size_t name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasprivateoperator", _Dee_HashSelectC(0xfb82aa2c, 0xf3c5c25fc6acf365),  DEE_PCKuSIZ, name));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (getinstanceattr)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "getinstanceattr", _Dee_HashSelectC(0xba156470, 0x893f73567697a84), 1, args));
	}
	WUNUSED Ref<Object> (getinstanceattr)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "getinstanceattr", _Dee_HashSelectC(0xba156470, 0x893f73567697a84), "s", name));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Object> (callinstanceattr)(DeeObject *name, DeeObject *args, DeeObject *kwds) {
		DeeObject *args_[3];
		args_[0] = name;
		args_[1] = args;
		args_[2] = kwds;
		return inherit(DeeObject_CallAttrStringHash(this, "callinstanceattr", _Dee_HashSelectC(0x2b36b487, 0xfbb7d2adb9d099eb), 3, args_));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Object> (callinstanceattr)(char const *name, DeeObject *args, DeeObject *kwds) {
		return inherit(DeeObject_CallAttrStringHashf(this, "callinstanceattr", _Dee_HashSelectC(0x2b36b487, 0xfbb7d2adb9d099eb), "soo", name, args, kwds));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (hasinstanceattr)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "hasinstanceattr", _Dee_HashSelectC(0x4998413, 0xbf4fd286bd2efacb), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (hasinstanceattr)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hasinstanceattr", _Dee_HashSelectC(0x4998413, 0xbf4fd286bd2efacb), "s", name));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (boundinstanceattr)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (boundinstanceattr)(DeeObject *name, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = name;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (boundinstanceattr)(DeeObject *name, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), "ob", name, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (boundinstanceattr)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), "s", name));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (boundinstanceattr)(char const *name, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), "so", name, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (boundinstanceattr)(char const *name, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "boundinstanceattr", _Dee_HashSelectC(0xc3ed8ca5, 0xb2be11985ca971ec), "sb", name, allow_missing));
	}
	NONNULL_CXX((1)) void (delinstanceattr)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "delinstanceattr", _Dee_HashSelectC(0xb3fc0bb, 0xba6fb391746fa2da), 1, args)));
	}
	void (delinstanceattr)(char const *name) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "delinstanceattr", _Dee_HashSelectC(0xb3fc0bb, 0xba6fb391746fa2da), "s", name)));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (setinstanceattr)(DeeObject *name, DeeObject *value) {
		DeeObject *args[2];
		args[0] = name;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setinstanceattr", _Dee_HashSelectC(0xe46629d1, 0xf2c9ff5c9104ece0), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Object> (setinstanceattr)(char const *name, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "setinstanceattr", _Dee_HashSelectC(0xe46629d1, 0xf2c9ff5c9104ece0), "so", name, value));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (derivedfrom)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "derivedfrom", _Dee_HashSelectC(0xbfe72a33, 0x74a32a2fdaec5b52), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (same_or_derived_from)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "same_or_derived_from", _Dee_HashSelectC(0x2b773424, 0x8adddec6e3c7f1d5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (derived_from)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "derived_from", _Dee_HashSelectC(0xd5ca0338, 0xc9563f499d5ca8b9), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (is_vartype)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_vartype", _Dee_HashSelectC(0x7c864a03, 0x74cc77cd00dcc3d5), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_heaptype)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_heaptype", _Dee_HashSelectC(0x24f570de, 0x1581ef944a8aaa0b), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_gctype)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_gctype", _Dee_HashSelectC(0x76acb304, 0x6ef63a1ea060c5db), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_final)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_final", _Dee_HashSelectC(0x581c8ec6, 0x1efe4691869c396f), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_class)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_class", _Dee_HashSelectC(0x2947c448, 0x538c2dae31aecaff), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_complete)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_complete", _Dee_HashSelectC(0xe4974a1e, 0x9f51d1a812b7db87), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_classtype)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_classtype", _Dee_HashSelectC(0x78743248, 0x38068baf674ec3a8), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_pointer)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_pointer", _Dee_HashSelectC(0x51d02720, 0xde6ea15c6f83c796), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_lvalue)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_lvalue", _Dee_HashSelectC(0xd6755fdd, 0xfb7e17b7a9dd8393), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_structured)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_structured", _Dee_HashSelectC(0x2e2730f0, 0x747ac51301af2a7e), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_struct)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_struct", _Dee_HashSelectC(0x628ee615, 0x4d8ae8916873e4cd), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_array)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_array", _Dee_HashSelectC(0xc50bca46, 0x595fe48e3a39713b), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_foreign_function)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_foreign_function", _Dee_HashSelectC(0x443fcdf1, 0x2f8fe3a73336066d), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_file)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_file", _Dee_HashSelectC(0x202225d1, 0xd98e30093fc5ea17), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (is_super_base)() {
		return inherit(DeeObject_CallAttrStringHash(this, "is_super_base", _Dee_HashSelectC(0x63c8c0fc, 0x6878259a0fe9a07), 0, NULL));
	}
	class _Wrap_isbuffer
		: public deemon::detail::ConstGetRefProxy<_Wrap_isbuffer, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isbuffer(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isbuffer", _Dee_HashSelectC(0xeb98ec1e, 0x97a811be95a96c82));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isbuffer", _Dee_HashSelectC(0xeb98ec1e, 0x97a811be95a96c82)));
		}
	};
	WUNUSED _Wrap_isbuffer (isbuffer)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___bases__
		: public deemon::detail::ConstGetRefProxy<_Wrap___bases__, Sequence<> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___bases__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__bases__", _Dee_HashSelectC(0xff4ac0d2, 0x56bdc053fa64e4c9));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__bases__", _Dee_HashSelectC(0xff4ac0d2, 0x56bdc053fa64e4c9)));
		}
	};
	WUNUSED _Wrap___bases__ (__bases__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___mro__
		: public deemon::detail::ConstGetRefProxy<_Wrap___mro__, Sequence<> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___mro__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__mro__", _Dee_HashSelectC(0x4b5e22f6, 0x8dbbdb5c2f99ff7a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__mro__", _Dee_HashSelectC(0x4b5e22f6, 0x8dbbdb5c2f99ff7a)));
		}
	};
	WUNUSED _Wrap___mro__ (__mro__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___class__
		: public deemon::detail::ConstGetRefProxy<_Wrap___class__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___class__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__class__", _Dee_HashSelectC(0x1358a219, 0x7833c05adf185360));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__class__", _Dee_HashSelectC(0x1358a219, 0x7833c05adf185360)));
		}
	};
	WUNUSED _Wrap___class__ (__class__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___seqclass__
		: public deemon::detail::ConstGetRefProxy<_Wrap___seqclass__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___seqclass__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__seqclass__", _Dee_HashSelectC(0xf7f40c9d, 0xb36e7599478968f2));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__seqclass__", _Dee_HashSelectC(0xf7f40c9d, 0xb36e7599478968f2)));
		}
	};
	WUNUSED _Wrap___seqclass__ (__seqclass__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___issingleton__
		: public deemon::detail::ConstGetRefProxy<_Wrap___issingleton__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___issingleton__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__issingleton__", _Dee_HashSelectC(0xddda05c8, 0x2319af62149ba76f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__issingleton__", _Dee_HashSelectC(0xddda05c8, 0x2319af62149ba76f)));
		}
	};
	WUNUSED _Wrap___issingleton__ (__issingleton__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___module__
		: public deemon::detail::ConstGetRefProxy<_Wrap___module__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___module__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb)));
		}
	};
	WUNUSED _Wrap___module__ (__module__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___ctable__
		: public deemon::detail::ConstGetRefProxy<_Wrap___ctable__, Sequence<> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___ctable__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__ctable__", _Dee_HashSelectC(0xde5ff47, 0xf068a8ecedddcbef));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__ctable__", _Dee_HashSelectC(0xde5ff47, 0xf068a8ecedddcbef)));
		}
	};
	WUNUSED _Wrap___ctable__ (__ctable__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___operators__
		: public deemon::detail::ConstGetRefProxy<_Wrap___operators__, Sequence<Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___operators__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__operators__", _Dee_HashSelectC(0xed78c2be, 0xfc4c23ee6412f79c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__operators__", _Dee_HashSelectC(0xed78c2be, 0xfc4c23ee6412f79c)));
		}
	};
	WUNUSED _Wrap___operators__ (__operators__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___operatorids__
		: public deemon::detail::ConstGetRefProxy<_Wrap___operatorids__, Sequence<deemon::int_> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___operatorids__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__operatorids__", _Dee_HashSelectC(0x8b1b1d9e, 0x36d3770a50f555aa));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__operatorids__", _Dee_HashSelectC(0x8b1b1d9e, 0x36d3770a50f555aa)));
		}
	};
	WUNUSED _Wrap___operatorids__ (__operatorids__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___instancesize__
		: public deemon::detail::ConstGetRefProxy<_Wrap___instancesize__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___instancesize__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__instancesize__", _Dee_HashSelectC(0xfad7bde4, 0x2042e2e37306e26d));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__instancesize__", _Dee_HashSelectC(0xfad7bde4, 0x2042e2e37306e26d)));
		}
	};
	WUNUSED _Wrap___instancesize__ (__instancesize__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___instance_size__
		: public deemon::detail::ConstGetRefProxy<_Wrap___instance_size__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___instance_size__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__instance_size__", _Dee_HashSelectC(0x8908a28a, 0x9bc41903b06d6da9));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__instance_size__", _Dee_HashSelectC(0x8908a28a, 0x9bc41903b06d6da9)));
		}
	};
	WUNUSED _Wrap___instance_size__ (__instance_size__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___istypetype__
		: public deemon::detail::ConstGetRefProxy<_Wrap___istypetype__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___istypetype__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__istypetype__", _Dee_HashSelectC(0xb2bdc9b6, 0xf9c979c6fa38de96));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__istypetype__", _Dee_HashSelectC(0xb2bdc9b6, 0xf9c979c6fa38de96)));
		}
	};
	WUNUSED _Wrap___istypetype__ (__istypetype__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isvarargconstructible__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isvarargconstructible__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isvarargconstructible__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isvarargconstructible__", _Dee_HashSelectC(0x224bd880, 0x4ad17e7c056f4362));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isvarargconstructible__", _Dee_HashSelectC(0x224bd880, 0x4ad17e7c056f4362)));
		}
	};
	WUNUSED _Wrap___isvarargconstructible__ (__isvarargconstructible__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isconstructible__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isconstructible__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isconstructible__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isconstructible__", _Dee_HashSelectC(0x574ef5f1, 0x86a5c49c02d17c23));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isconstructible__", _Dee_HashSelectC(0x574ef5f1, 0x86a5c49c02d17c23)));
		}
	};
	WUNUSED _Wrap___isconstructible__ (__isconstructible__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___iscopyable__
		: public deemon::detail::ConstGetRefProxy<_Wrap___iscopyable__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___iscopyable__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__iscopyable__", _Dee_HashSelectC(0x4a442794, 0x6cd8653d147c94fa));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__iscopyable__", _Dee_HashSelectC(0x4a442794, 0x6cd8653d147c94fa)));
		}
	};
	WUNUSED _Wrap___iscopyable__ (__iscopyable__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isnamespace__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isnamespace__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isnamespace__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isnamespace__", _Dee_HashSelectC(0x51ea2523, 0x412fd04e6884621b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isnamespace__", _Dee_HashSelectC(0x51ea2523, 0x412fd04e6884621b)));
		}
	};
	WUNUSED _Wrap___isnamespace__ (__isnamespace__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isconstcastable__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isconstcastable__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isconstcastable__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isconstcastable__", _Dee_HashSelectC(0x9961c985, 0xb07a59c0b6d139fc));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isconstcastable__", _Dee_HashSelectC(0x9961c985, 0xb07a59c0b6d139fc)));
		}
	};
	WUNUSED _Wrap___isconstcastable__ (__isconstcastable__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___gcpriority__
		: public deemon::detail::ConstGetRefProxy<_Wrap___gcpriority__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___gcpriority__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__gcpriority__", _Dee_HashSelectC(0xcd14df35, 0x54dd6db0ab44f614));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__gcpriority__", _Dee_HashSelectC(0xcd14df35, 0x54dd6db0ab44f614)));
		}
	};
	WUNUSED _Wrap___gcpriority__ (__gcpriority__)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_TYPE_H */
