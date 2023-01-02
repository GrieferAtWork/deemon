/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_MAPPING_H
#define GUARD_DEEMON_CXX_MAPPING_H 1

#include "api.h"

#include "../map.h"
#include "../string.h"
#include "object.h"
#include "sequence.h"

DEE_CXX_BEGIN

template<class KeyType = Object, class ValueType = Object> class Mapping;

namespace detail {
class mapping_base: public sequence_base {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeMapping_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeMapping_Type);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeMapping_Type);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(mapping_base, sequence_base)
	operator obj_mapping() const DEE_CXX_NOTHROW {
		return obj_mapping(this->ptr());
	}
	Sequence<Object> popitem() const {
		return inherit(DeeObject_CallAttrString(this->ptr(), "popitem", 0, NULL));
	}
	Sequence<Sequence<Object> > items() const {
		return inherit(DeeObject_GetAttrString(this->ptr(), "item"));
	}
	void update(Sequence<Sequence<Object> > const &new_items) const {
		Dee_Decref(throw_if_null(DeeObject_CallAttrString(this->ptr(), "update", 1, (DeeObject **)&new_items)));
	}
	void update(DeeObject *__restrict new_items) const {
		Dee_Decref(throw_if_null(DeeObject_CallAttrString(this->ptr(), "update", 1, (DeeObject **)&new_items)));
	}
};
}


template<class KeyType, class ValueType>
class Mapping: public detail::mapping_base {
	template<class T_> class proxy_base {
	public:
		ValueType get() const {
			return inherit(((T_ const *)this)->getref());
		}
		operator ValueType() const {
			return inherit(((T_ const *)this)->getref());
		}
		void set(DeeObject *__restrict value) const {
			*((T_ const *)this) = value;
		}
		void set(ValueType const &value) const {
			*((T_ const *)this) = value;
		}
	};
	class item_proxy_obj: public proxy_base<item_proxy_obj> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_key;

	public:
		item_proxy_obj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_key(str) {}
		item_proxy_obj(item_proxy_obj const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_key(right.m_key) {}
		bool has() const {
			return throw_if_negative(DeeObject_HasItem(m_ptr, m_key)) != 0;
		}
		bool bound(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItem(m_ptr, m_key, allow_missing)) > 0;
		}
		DREF DeeObject *getref() const {
			return DeeObject_GetItem(m_ptr, m_key);
		}
		DREF DeeObject *getref_def(DeeObject *__restrict def) const {
			return DeeObject_GetItemDef(m_ptr, m_key, def);
		}
		ValueType getdef(DeeObject *__restrict def) const {
			return inherit(getref_def(def));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelItem(m_ptr, m_key));
		}
		item_proxy_obj const &operator=(DeeObject *__restrict value) const {
			throw_if_nonzero(DeeObject_SetItem(m_ptr, m_key, value));
			return *this;
		}
		item_proxy_obj const &operator=(ValueType const &value) const {
			throw_if_nonzero(DeeObject_SetItem(m_ptr, m_key, value));
			return *this;
		}
	};
	class item_proxy_sth: public proxy_base<item_proxy_sth> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		Dee_hash_t m_hsh;

	public:
		item_proxy_sth(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_hsh(Dee_HashStr(str)) {}
		item_proxy_sth(DeeObject *ptr, char const *str, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_hsh(hsh) {}
		item_proxy_sth(item_proxy_sth const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_hsh(right.m_hsh) {}
		bool has() const {
			return throw_if_negative(DeeObject_HasItemString(m_ptr, m_str, m_hsh)) != 0;
		}
		bool bound(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItemString(m_ptr, m_str, m_hsh, allow_missing)) > 0;
		}
		DREF DeeObject *getref() const {
			return DeeObject_GetItemString(m_ptr, m_str, m_hsh);
		}
		DREF DeeObject *getref_def(DeeObject *__restrict def) const {
			return DeeObject_GetItemStringDef(m_ptr, m_str, m_hsh, def);
		}
		ValueType getdef(DeeObject *__restrict def) const {
			return inherit(getref_def(def));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelItemString(m_ptr, m_str, m_hsh));
		}
		item_proxy_sth const &operator=(DeeObject *__restrict value) const {
			throw_if_nonzero(DeeObject_SetItemString(m_ptr, m_str, m_hsh, value));
			return *this;
		}
		item_proxy_sth const &operator=(ValueType const &value) const {
			throw_if_nonzero(DeeObject_SetItemString(m_ptr, m_str, m_hsh, value));
			return *this;
		}
	};
	class item_proxy_shn: public proxy_base<item_proxy_shn> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		size_t m_len;
		Dee_hash_t m_hsh;

	public:
		item_proxy_shn(DeeObject *ptr, char const *str, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_len(len)
		    , m_hsh(hsh) {}
		item_proxy_shn(item_proxy_shn const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_len(right.m_len)
		    , m_hsh(right.m_hsh) {}
		bool has() const {
			return throw_if_negative(DeeObject_HasItemStringLen(m_ptr, m_str, m_len, m_hsh)) != 0;
		}
		bool bound(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItemStringLen(m_ptr, m_str, m_len, m_hsh, allow_missing)) > 0;
		}
		DREF DeeObject *getref() const {
			return DeeObject_GetItemStringLen(m_ptr, m_str, m_len, m_hsh);
		}
		DREF DeeObject *getref_def(DeeObject *__restrict def) const {
			return DeeObject_GetItemStringLenDef(m_ptr, m_str, m_len, m_hsh, def);
		}
		ValueType getdef(DeeObject *__restrict def) const {
			return inherit(getref_def(def));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelItemStringLen(m_ptr, m_str, m_len, m_hsh));
		}
		item_proxy_shn const &operator=(DeeObject *__restrict value) const {
			throw_if_nonzero(DeeObject_SetItemStringLen(m_ptr, m_str, m_len, m_hsh, value));
			return *this;
		}
		item_proxy_shn const &operator=(ValueType const &value) const {
			throw_if_nonzero(DeeObject_SetItemStringLen(m_ptr, m_str, m_len, m_hsh, value));
			return *this;
		}
	};

public: /* `Mapping from deemon' */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Mapping, mapping_base)
	WUNUSED item_proxy_obj item(KeyType const &key) const {
		return item_proxy_obj(this->ptr(), key);
	}
	WUNUSED item_proxy_obj item(DeeObject *__restrict key) const {
		return item_proxy_obj(this->ptr(), key);
	}
	WUNUSED item_proxy_sth item(char const *__restrict key) const {
		return item_proxy_sth(this->ptr(), key);
	}
	WUNUSED item_proxy_sth item(char const *__restrict key, Dee_hash_t hash) const {
		return item_proxy_sth(this->ptr(), key, hash);
	}
	WUNUSED item_proxy_shn item(char const *__restrict key, size_t len, Dee_hash_t hash) const {
		return item_proxy_shn(this->ptr(), key, len, hash);
	}
	WUNUSED ValueType getitem(KeyType const &key) const {
		return inherit(DeeObject_GetItem(this->ptr(), key));
	}
	WUNUSED ValueType getitem(KeyType const &key, DeeObject *__restrict def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType getitem(KeyType const &key, ValueType const &def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType getitem(DeeObject *__restrict key) const {
		return inherit(DeeObject_GetItem(this->ptr(), key));
	}
	WUNUSED ValueType getitem(DeeObject *__restrict key, DeeObject *__restrict def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType getitem(DeeObject *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType getitem(char const *__restrict key) const {
		return inherit(DeeObject_GetItemString(this->ptr(), key, Dee_HashStr(key)));
	}
	WUNUSED ValueType getitem(char const *__restrict key, DeeObject *__restrict def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, Dee_HashStr(key), def));
	}
	WUNUSED ValueType getitem(char const *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, Dee_HashStr(key), def));
	}
	WUNUSED ValueType getitem(char const *__restrict key, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemString(this->ptr(), key, hash));
	}
	WUNUSED ValueType getitem(char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, hash, def));
	}
	WUNUSED ValueType getitem(char const *__restrict key, Dee_hash_t hash, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, hash, def));
	}
	WUNUSED ValueType getitem(char const *__restrict key, size_t len, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemStringLen(this->ptr(), key, len, hash));
	}
	WUNUSED ValueType getitem(char const *__restrict key, size_t len, Dee_hash_t hash, DeeObject *__restrict def) const {
		return inherit(DeeObject_GetItemStringLenDef(this->ptr(), key, len, hash, def));
	}
	WUNUSED ValueType getitem(char const *__restrict key, size_t len, Dee_hash_t hash, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringLenDef(this->ptr(), key, len, hash, def));
	}
	using detail::mapping_base::bounditem;
	using detail::mapping_base::delitem;
	using detail::mapping_base::hasitem;
	using detail::mapping_base::setitem;
	bool bounditem(KeyType const &key, bool allow_missing = true) const {
		int result = DeeObject_BoundItem(this->ptr(), key, allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool hasitem(KeyType const &key) const {
		int result = DeeObject_HasItem(this->ptr(), key);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	void delitem(KeyType const &index) const {
		throw_if_negative(DeeObject_DelItem(this->ptr(), index));
	}
	void setitem(KeyType const &index, DeeObject *__restrict value) const {
		throw_if_negative(DeeObject_SetItem(this->ptr(), index, value));
	}
	void setitem(KeyType const &index, ValueType const &value) const {
		throw_if_negative(DeeObject_SetItem(this->ptr(), index, value));
	}
	void setitem(DeeObject *__restrict index, ValueType const &value) const {
		throw_if_negative(DeeObject_SetItem(this->ptr(), index, value));
	}
	WUNUSED item_proxy_obj operator[](KeyType const &key) const {
		return item_proxy_obj(this->ptr(), key);
	}
	WUNUSED item_proxy_obj operator[](DeeObject *__restrict key) const {
		return item_proxy_obj(this->ptr(), key);
	}
	WUNUSED item_proxy_sth operator[](char const *__restrict key) const {
		return item_proxy_sth(this->ptr(), key);
	}

	WUNUSED ValueType get(KeyType const &key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType get(KeyType const &key, ValueType const &def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType get(DeeObject *__restrict key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType get(DeeObject *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_GetItemDef(this->ptr(), key, def));
	}
	WUNUSED ValueType get(char const *__restrict key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, Dee_HashStr(key), def));
	}
	WUNUSED ValueType get(char const *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, Dee_HashStr(key), def));
	}
	WUNUSED ValueType get(char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, hash, def));
	}
	WUNUSED ValueType get(char const *__restrict key, Dee_hash_t hash, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringDef(this->ptr(), key, hash, def));
	}
	WUNUSED ValueType get(char const *__restrict key, size_t len, Dee_hash_t hash, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_GetItemStringLenDef(this->ptr(), key, len, hash, def));
	}
	WUNUSED ValueType get(char const *__restrict key, size_t len, Dee_hash_t hash, ValueType const &def) const {
		return inherit(DeeObject_GetItemStringLenDef(this->ptr(), key, len, hash, def));
	}

	ValueType setdefault(KeyType const &key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", key, def));
	}
	ValueType setdefault(KeyType const &key, ValueType const &def) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", key, def));
	}
	ValueType setdefault(DeeObject *__restrict key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", key, def));
	}
	ValueType setdefault(DeeObject *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", key, def));
	}
	ValueType setdefault(char const *__restrict key, DeeObject *__restrict def = Dee_None) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "so", key, def));
	}
	ValueType setdefault(char const *__restrict key, ValueType const &def) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "setdefault", "so", key, def));
	}
	ValueType setdefault(char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict def = Dee_None) const {
		DREF DeeObject *keyob  = throw_if_null(DeeString_NewWithHash(key, hash));
		DREF DeeObject *result = DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", keyob, def);
		Dee_Decref(keyob);
		return inherit(result);
	}
	ValueType setdefault(char const *__restrict key, Dee_hash_t hash, ValueType const &def) const {
		DREF DeeObject *keyob  = throw_if_null(DeeString_NewWithHash(key, hash));
		DREF DeeObject *result = DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", keyob, def);
		Dee_Decref(keyob);
		return inherit(result);
	}
	ValueType setdefault(char const *__restrict key, size_t len, Dee_hash_t hash, DeeObject *__restrict def = Dee_None) const {
		DREF DeeObject *keyob  = throw_if_null(DeeString_NewSizedWithHash(key, len, hash));
		DREF DeeObject *result = DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", keyob, def);
		Dee_Decref(keyob);
		return inherit(result);
	}
	ValueType setdefault(char const *__restrict key, size_t len, Dee_hash_t hash, ValueType const &def) const {
		DREF DeeObject *keyob  = throw_if_null(DeeString_NewSizedWithHash(key, len, hash));
		DREF DeeObject *result = DeeObject_CallAttrStringf(this->ptr(), "setdefault", "oo", keyob, def);
		Dee_Decref(keyob);
		return inherit(result);
	}
	Sequence<KeyType> keys() const {
		return inherit(DeeObject_GetAttrString(this->ptr(), "keys"));
	}
	Sequence<ValueType> values() const {
		return inherit(DeeObject_GetAttrString(this->ptr(), "values"));
	}
	Mapping frozen() const {
		return inherit(DeeObject_GetAttrString(this->ptr(), "frozen"));
	}
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_MAPPING_H */
