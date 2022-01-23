/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_SEQUENCE_H
#define GUARD_DEEMON_CXX_SEQUENCE_H 1

#include "api.h"

#include "../seq.h"
#include "../tuple.h"
#include "object.h"

DEE_CXX_BEGIN

namespace detail {
class sequence_base: public Object {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeSeq_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeSeq_Type);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeSeq_Type);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(sequence_base, Object)
	operator obj_sequence() const DEE_CXX_NOTHROW {
		return obj_sequence(this->ptr());
	}
	bool ismutable() const {
		return Object(inherit(DeeObject_GetAttrString(*this, "ismutable"))).bool_();
	}
	bool isresizable() const {
		return Object(inherit(DeeObject_GetAttrString(*this, "isresizable"))).bool_();
	}
	bool empty() const {
		return !bool_();
	}
	bool nonempty() const {
		return bool_();
	}
	bool(any)() const {
		return Object(inherit(DeeObject_CallAttrString(*this, "any", 0, NULL))).bool_();
	}
	bool(all)() const {
		return Object(inherit(DeeObject_CallAttrString(*this, "all", 0, NULL))).bool_();
	}
	bool(parity)() const {
		return Object(inherit(DeeObject_CallAttrString(*this, "parity", 0, NULL))).bool_();
	}
#define DEFINE_ELEM_SEARCH_FUNCTION(Treturn, name) \
	inline Treturn(name)(DeeObject *elem) const;   \
	inline Treturn(name)(DeeObject *elem, DeeObject *key) const;
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, count)
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, locate)
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, rlocate)
#undef DEFINE_ELEM_SEARCH_FUNCTION
#define DEFINE_ELEM_SEARCH_FUNCTION(Treturn, name)                                 \
	Treturn(name)(DeeObject *elem) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));        \
	}                                                                              \
	Treturn(name)(DeeObject *elem, DeeObject *key) const {                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));  \
	}
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::bool_, contains)
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::bool_, startswith)
	DEFINE_ELEM_SEARCH_FUNCTION(deemon::bool_, endswith)
#undef DEFINE_ELEM_SEARCH_FUNCTION
#define DEFINE_ELEM_FIND_FUNCTION(Treturn, name)                               \
	inline Treturn(name)(DeeObject *elem) const;                               \
	inline Treturn(name)(DeeObject *elem, DeeObject *key) const;               \
	inline Treturn(name)(DeeObject *elem, size_t start) const;                 \
	inline Treturn(name)(DeeObject *elem, size_t start, DeeObject *key) const; \
	inline Treturn(name)(DeeObject *elem, size_t start, size_t end) const;     \
	inline Treturn(name)(DeeObject *elem, size_t start, size_t end, DeeObject *key) const;
	DEFINE_ELEM_FIND_FUNCTION(deemon::int_, find)
	DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rfind)
	DEFINE_ELEM_FIND_FUNCTION(deemon::int_, index)
	DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rindex)
	DEFINE_ELEM_FIND_FUNCTION(deemon::int_, removeall)
#undef DEFINE_ELEM_FIND_FUNCTION
#define DEFINE_ELEM_FIND_FUNCTION(Treturn, name)                                                  \
	Treturn(name)(DeeObject *elem) const {                                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));                       \
	}                                                                                             \
	Treturn(name)(DeeObject *elem, DeeObject *key) const {                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));                 \
	}                                                                                             \
	Treturn(name)(DeeObject *elem, size_t start) const {                                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", elem, start));              \
	}                                                                                             \
	Treturn(name)(DeeObject *elem, size_t start, DeeObject *key) const {                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", elem, start, key));        \
	}                                                                                             \
	Treturn(name)(DeeObject *elem, size_t start, size_t end) const {                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", elem, start, end));       \
	}                                                                                             \
	Treturn(name)(DeeObject *elem, size_t start, size_t end, DeeObject *key) const {              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", elem, start, end, key)); \
	}
	DEFINE_ELEM_FIND_FUNCTION(deemon::bool_, remove)
	DEFINE_ELEM_FIND_FUNCTION(deemon::bool_, rremove)
#undef DEFINE_ELEM_FIND_FUNCTION
	WUNUSED size_t(fastsize)() const DEE_CXX_NOTHROW {
		return DeeFastSeq_GetSize(*this);
	}
	WUNUSED size_t(fastsize_nb)() const DEE_CXX_NOTHROW {
		return DeeFastSeq_GetSizeNB(*this);
	}
	/* Functions for mutable sequences. */
	void(insert)(size_t index, DeeObject *item) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "insert", "Iuo", index, item)));
	}
	void(insert)(DeeObject *index, DeeObject *item) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "insert", "oo", index, item)));
	}
	void(insertall)(size_t index, DeeObject *items) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "insertall", "Iuo", index, items)));
	}
	void(insertall)(DeeObject *index, DeeObject *items) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "insertall", "oo", index, items)));
	}
	void(insertall)(size_t index, std::initializer_list<DeeObject *> const &items) const {
		DREF DeeObject *result, *items_ob;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(items.size(), items.begin()));
		result   = DeeObject_CallAttrStringf(*this, "insertall", "Iuo", index, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	void(insertall)(DeeObject *index, std::initializer_list<DeeObject *> const &items) const {
		DREF DeeObject *result, *items_ob;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(items.size(), items.begin()));
		result   = DeeObject_CallAttrStringf(*this, "insertall", "oo", index, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	void(insertall)(size_t index, size_t objc, DeeObject *const *objv) const {
		DREF DeeObject *items_ob, *result;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(objc, objv));
		result   = DeeObject_CallAttrStringf(*this, "insertall", "Iuo", index, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	void(insertall)(DeeObject *index, size_t objc, DeeObject *const *objv) const {
		DREF DeeObject *items_ob, *result;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(objc, objv));
		result   = DeeObject_CallAttrStringf(*this, "insertall", "oo", index, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	void(insertall)(size_t index, size_t objc, DeeObject **objv) const {
		insertall(index, objc, (DeeObject *const *)objv);
	}
	void(insertall)(DeeObject *index, size_t objc, DeeObject **objv) const {
		insertall(index, objc, (DeeObject *const *)objv);
	}
	void(insertiter)(size_t index, DeeObject *iter) const {
		DREF DeeObject *pending, *result;
		pending = throw_if_null(DeeObject_GetAttrString(iter, "pending"));
		result  = DeeObject_CallAttrStringf(*this, "insertall", "Iuo", index, pending);
		Dee_Decref(pending);
		decref_unlikely(throw_if_null(result));
	}
	void(insertiter)(DeeObject *index, DeeObject *iter) const {
		DREF DeeObject *pending, *result;
		pending = throw_if_null(DeeObject_GetAttrString(iter, "pending"));
		result  = DeeObject_CallAttrStringf(*this, "insertall", "oo", index, pending);
		Dee_Decref(pending);
		decref_unlikely(throw_if_null(result));
	}
	void(append)(DeeObject *item) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "append", "o", item)));
	}
	void(appenditer)(DeeObject *iter) const {
		DREF DeeObject *pending, *result;
		pending = throw_if_null(DeeObject_GetAttrString(iter, "pending"));
		result  = DeeObject_CallAttrStringf(*this, "extend", "o", pending);
		Dee_Decref(pending);
		decref_unlikely(throw_if_null(result));
	}
	void(extend)(DeeObject *items) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "extend", "o", items)));
	}
	void(extend)(size_t objc, DeeObject *const *objv) const {
		DREF DeeObject *items_ob, *result;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(objc, objv));
		result   = DeeObject_CallAttrStringf(*this, "extend", "o", items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	void(extend)(size_t objc, DeeObject **objv) const {
		extend(objc, (DeeObject *const *)objv);
	}
	void(extend)(std::initializer_list<DeeObject *> const &items) const {
		DREF DeeObject *result, *items_ob;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(items.size(), items.begin()));
		result   = DeeObject_CallAttrStringf(*this, "extend", "o", items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		decref_unlikely(throw_if_null(result));
	}
	deemon::int_(erase)(size_t index) const;
	deemon::int_(erase)(size_t index, size_t count) const;
	deemon::int_(erase)(DeeObject *index) const;
	deemon::int_(erase)(DeeObject *index, size_t count) const;
	deemon::int_(erase)(DeeObject *index, DeeObject *count) const;
	void(pushfront)(DeeObject *item) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "pushfront", "o", item)));
	}
	void(pushback)(DeeObject *item) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "pushback", "o", item)));
	}
	void(removeif)(DeeObject *should) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "o", should)));
	}
	void(removeif)(DeeObject *should, size_t start) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "oIu", should, start)));
	}
	void(removeif)(DeeObject *should, size_t start, size_t end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "oIuIu", should, start, end)));
	}
	void(removeif)(DeeObject *should, DeeObject *start) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "oo", should, start)));
	}
	void(removeif)(DeeObject *should, DeeObject *start, size_t end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "ooIu", should, start, end)));
	}
	void(removeif)(DeeObject *should, DeeObject *start, DeeObject *end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "removeif", "ooo", should, start, end)));
	}
	void(clear)() const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrString(*this, "clear", 0, NULL)));
	}
	void(resize)(size_t new_size) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "resize", "Iu", new_size)));
	}
	void(resize)(size_t new_size, DeeObject *filler) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "resize", "Iuo", new_size, filler)));
	}
	void(resize)(DeeObject *new_size) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "resize", "o", new_size)));
	}
	void(resize)(DeeObject *new_size, DeeObject *filler) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "resize", "oo", new_size, filler)));
	}
	void(fill)() const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrString(*this, "fill", 0, NULL)));
	}
	void(fill)(size_t start) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "Iu", start)));
	}
	void(fill)(size_t start, size_t end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "IuIu", start, end)));
	}
	void(fill)(size_t start, size_t end, DeeObject *filler) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "IuIuo", start, end, filler)));
	}
	void(fill)(DeeObject *start) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "o", start)));
	}
	void(fill)(DeeObject *start, size_t end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "oIu", start, end)));
	}
	void(fill)(DeeObject *start, size_t end, DeeObject *filler) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "oIuo", start, end, filler)));
	}
	void(fill)(DeeObject *start, DeeObject *end) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "oo", start, end)));
	}
	void(fill)(DeeObject *start, DeeObject *end, DeeObject *filler) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "fill", "ooo", start, end, filler)));
	}
	void(reverse)() const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrString(*this, "reverse", 0, NULL)));
	}
	void(sort)() const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrString(*this, "sort", 0, NULL)));
	}
	void(sort)(DeeObject *key) const {
		decref_unlikely(throw_if_null(DeeObject_CallAttrStringf(*this, "sort", "o", key)));
	}

public: /* Static functions. */
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t end);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, Dee_ssize_t end);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t step);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *step);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, DeeObject *end);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, DeeObject *end, Dee_ssize_t step);
	static inline Sequence<deemon::int_>(makerange)(Dee_ssize_t start, DeeObject *end, DeeObject *step);
	static inline Sequence<deemon::int_>(makerange)(DeeObject *start, Dee_ssize_t end);
	static inline Sequence<deemon::int_>(makerange)(DeeObject *start, Dee_ssize_t end, Dee_ssize_t step);
	static inline Sequence<deemon::int_>(makerange)(DeeObject *start, Dee_ssize_t end, DeeObject *step);
	static inline Sequence<deemon::int_>(makerange)(DeeObject *start, DeeObject *end, Dee_ssize_t step);
	static inline Sequence<Object>(makerange)(DeeObject *end);
	static inline Sequence<Object>(makerange)(DeeObject *start, DeeObject *end);
	static inline Sequence<Object>(makerange)(DeeObject *start, DeeObject *end, DeeObject *step);
	static inline Sequence<Object>(repeat)(DeeObject *item, size_t count);
	static inline Sequence<Object>(repeat)(DeeObject *item, DeeObject *count);
	static inline Sequence<Object>(repeatseq)(DeeObject *seq, size_t count);
	static inline Sequence<Object>(repeatseq)(DeeObject *seq, DeeObject *count);
	static inline Sequence<Object>(concat)(size_t nseq, DeeObject **seqv);
	static inline Sequence<Object>(concat)(size_t nseq, DeeObject *const *seqv);
	static inline Sequence<Object>(concat)(size_t nseq, Object **seqv);
	static inline Sequence<Object>(concat)(size_t nseq, Object *const *seqv);
	static inline Sequence<Object>(concat)(std::initializer_list<DeeObject *> const &sequences);
};
}

template<class T>
class Iterator: public Object {
public: /* iterator from deemon */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Iterator, Object)
	T(next)() const {
		DREF DeeObject *result = throw_if_null(DeeObject_IterNext(*this));
		if (result == ITER_DONE)
			result = NULL;
		return inherit(maybenull(result));
	}
	Sequence<T>(seq)() const {
		return inherit(DeeObject_GetAttrString(*this, "seq"));
	}
	Sequence<T>(future)() const {
		return inherit(DeeObject_GetAttrString(*this, "future"));
	}
	Sequence<T>(pending)() const {
		return inherit(DeeObject_GetAttrString(*this, "pending"));
	}
};

inline deemon::Iterator<Object> Object::iter() const {
	return inherit(DeeObject_IterSelf(*this));
}
template<class T> inline deemon::Iterator<T> Object::iter() const {
	return inherit(DeeObject_IterSelf(*this));
}


template<class T>
class Sequence: public detail::sequence_base {
	template<class T_> class proxy_base {
	public:
		T(get)() const {
			return inherit(((T_ const *)this)->getref());
		}
		operator T() const {
			return inherit(((T_ const *)this)->getref());
		}
		void(set)(DeeObject *value) const {
			*((T_ const *)this) = value;
		}
	};
	template<class T_> class seq_proxy_base {
	public:
		Sequence(get)() const {
			return inherit(((T_ const *)this)->getref());
		}
		operator Sequence() const {
			return inherit(((T_ const *)this)->getref());
		}
		void(set)(DeeObject *value) const {
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
		bool(has)() const {
			return throw_if_negative(DeeObject_HasItem(m_ptr, m_key)) != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItem(m_ptr, m_key, allow_missing)) > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItem(m_ptr, m_key);
		}
		DREF DeeObject *(getref_def)(DeeObject *def) const {
			return DeeObject_GetItemDef(m_ptr, m_key, def);
		}
		T(getdef)(DeeObject *def) const {
			return inherit(getref_def(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItem(m_ptr, m_key));
		}
		item_proxy_obj const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItem(m_ptr, m_key, value));
			return *this;
		}
	};
	class item_proxy_idx: public proxy_base<item_proxy_idx> {
	private:
		DeeObject *m_ptr;
		size_t m_idx;

	public:
		item_proxy_idx(DeeObject *ptr, size_t idx) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_idx(idx) {}
		item_proxy_idx(item_proxy_idx const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_idx(right.m_idx) {}
		bool(has)() const {
			return throw_if_negative(DeeObject_HasItemIndex(m_ptr, m_idx)) != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItemIndex(m_ptr, m_idx, allow_missing)) > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemIndex(m_ptr, m_idx);
		}
		DREF DeeObject *(getref_def)(DeeObject *def) const {
			DREF DeeObject *result, *index_ob;
			index_ob = throw_if_null(DeeInt_NewSize(m_idx));
			result   = DeeObject_GetItemDef(m_ptr, index_ob, def);
			Dee_Decref(index_ob);
			return inherit(result);
		}
		T(getdef)(DeeObject *def) const {
			return inherit(getref_def(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemIndex(m_ptr, m_idx));
		}
		item_proxy_idx const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItemIndex(m_ptr, m_idx, value));
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
		bool(has)() const {
			return throw_if_negative(DeeObject_HasItemString(m_ptr, m_str, m_hsh)) != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItemString(m_ptr, m_str, m_hsh, allow_missing)) > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemString(m_ptr, m_str, m_hsh);
		}
		DREF DeeObject *(getref_def)(DeeObject *def) const {
			return DeeObject_GetItemStringDef(m_ptr, m_str, m_hsh, def);
		}
		T(getdef)(DeeObject *def) const {
			return inherit(getref_def(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemString(m_ptr, m_str, m_hsh));
		}
		item_proxy_sth const &operator=(DeeObject *value) const {
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
		bool(has)() const {
			return throw_if_negative(DeeObject_HasItemStringLen(m_ptr, m_str, m_len, m_hsh)) != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			return throw_if_minusone(DeeObject_BoundItemStringLen(m_ptr, m_str, m_len, m_hsh, allow_missing)) > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemStringLen(m_ptr, m_str, m_len, m_hsh);
		}
		DREF DeeObject *(getref_def)(DeeObject *def) const {
			return DeeObject_GetItemStringLenDef(m_ptr, m_str, m_len, m_hsh, def);
		}
		T(getdef)(DeeObject *def) const {
			return inherit(getref_def(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemStringLen(m_ptr, m_str, m_len, m_hsh));
		}
		item_proxy_shn const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItemStringLen(m_ptr, m_str, m_len, m_hsh, value));
			return *this;
		}
	};
	class range_proxy_oo: public seq_proxy_base<range_proxy_oo> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_bgn;
		DeeObject *m_end;

	public:
		range_proxy_oo(DeeObject *ptr, DeeObject *bgn, DeeObject *end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_oo(range_proxy_oo const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRange(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelRange(m_ptr, m_bgn, m_end));
		}
		range_proxy_oo const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRange(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_io: public seq_proxy_base<range_proxy_io> {
	private:
		DeeObject *m_ptr;
		size_t m_bgn;
		DeeObject *m_end;

	public:
		range_proxy_io(DeeObject *ptr, size_t bgn, DeeObject *end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_io(range_proxy_io const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeBeginIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn));
			int error                = DeeObject_DelRange(m_ptr, begin_ob, m_end);
			Dee_Decref(begin_ob);
			throw_if_nonzero(error);
		}
		range_proxy_io const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeBeginIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_oi: public seq_proxy_base<range_proxy_oi> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_bgn;
		size_t m_end;

	public:
		range_proxy_oi(DeeObject *ptr, DeeObject *bgn, size_t end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_oi(range_proxy_oi const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeEndIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *end_ob = throw_if_null(DeeInt_NewSize(m_end));
			int error              = DeeObject_DelRange(m_ptr, m_bgn, end_ob);
			Dee_Decref(end_ob);
			throw_if_nonzero(error);
		}
		range_proxy_oi const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeEndIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_ii: public seq_proxy_base<range_proxy_ii> {
	private:
		DeeObject *m_ptr;
		size_t m_bgn;
		size_t m_end;

	public:
		range_proxy_ii(DeeObject *ptr, size_t bgn, size_t end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_ii(range_proxy_ii const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn));
			DREF DeeObject *end_ob   = DeeInt_NewSize(m_end);
			if unlikely(!end_ob) {
				Dee_Decref(begin_ob);
				throw_last_deemon_exception();
			}
			int error = DeeObject_DelRange(m_ptr, begin_ob, end_ob);
			Dee_Decref(end_ob);
			Dee_Decref(begin_ob);
			throw_if_nonzero(error);
		}
		range_proxy_ii const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class first_proxy: public proxy_base<first_proxy> {
	private:
		DeeObject *m_ptr;

	public:
		first_proxy(DeeObject *ptr) DEE_CXX_NOTHROW: m_ptr(ptr) {}
		first_proxy(first_proxy const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr) {}
		bool(bound)() const {
			int result = DeeObject_BoundAttrString(m_ptr, "first");
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetAttrString(m_ptr, "first");
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelAttrString(m_ptr, "first"));
		}
		first_proxy const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttrString(m_ptr, "first", value));
			return *this;
		}
	};
	class last_proxy: public proxy_base<last_proxy> {
	private:
		DeeObject *m_ptr;

	public:
		last_proxy(DeeObject *ptr) DEE_CXX_NOTHROW: m_ptr(ptr) {}
		last_proxy(last_proxy const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr) {}
		bool(bound)() const {
			int result = DeeObject_BoundAttrString(m_ptr, "last");
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetAttrString(m_ptr, "last");
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelAttrString(m_ptr, "last"));
		}
		last_proxy const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttrString(m_ptr, "last", value));
			return *this;
		}
	};

public: /* `Sequence from deemon' */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Sequence, sequence_base)
	typedef detail::cxx_iterator<T> iterator;
	iterator(begin)() const {
		return inherit(DeeObject_IterSelf(*this));
	}
	iterator(end)() const {
		return iterator();
	}
	WUNUSED T(fastitem)(size_t index) const {
		return inherit(DeeFastSeq_GetItem(*this, index));
	}
	WUNUSED T(fastitem_nb)(size_t index) const DEE_CXX_NOTHROW {
		return inherit(nonnull(DeeFastSeq_GetItemNB(*this, index)));
	}
	WUNUSED item_proxy_obj(item)(DeeObject *index) const {
		return item_proxy_obj(*this, index);
	}
	WUNUSED item_proxy_idx(item)(size_t index) const {
		return item_proxy_idx(*this, index);
	}
	WUNUSED item_proxy_sth(item)(char const *name) const {
		return item_proxy_sth(*this, name);
	}
	WUNUSED item_proxy_sth(item)(char const *name, Dee_hash_t hash) const {
		return item_proxy_sth(*this, name, hash);
	}
	WUNUSED item_proxy_shn(item)(char const *name, size_t len, Dee_hash_t hash) const {
		return item_proxy_shn(*this, name, len, hash);
	}
	WUNUSED T(getitem)(DeeObject *index) const {
		return inherit(DeeObject_GetItem(*this, index));
	}
	WUNUSED T(getitem)(DeeObject *index, DeeObject *def) const {
		return inherit(DeeObject_GetItemDef(*this, index, def));
	}
	WUNUSED T(getitem)(size_t index) const {
		return inherit(DeeObject_GetItemIndex(*this, index));
	}
	WUNUSED T(getitem)(size_t index, DeeObject *def) const {
		DREF DeeObject *result, *index_ob;
		index_ob = throw_if_null(DeeInt_NewSize(index));
		result   = DeeObject_GetItemDef(*this, index_ob, def);
		Dee_Decref(index_ob);
		return inherit(result);
	}
	WUNUSED T(getitem)(char const *name) const {
		return inherit(DeeObject_GetItemString(*this, name, Dee_HashStr(name)));
	}
	WUNUSED T(getitem)(char const *name, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringDef(*this, name, Dee_HashStr(name), def));
	}
	WUNUSED T(getitem)(char const *name, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemString(*this, name, hash));
	}
	WUNUSED T(getitem)(char const *name, Dee_hash_t hash, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringDef(*this, name, hash, def));
	}
	WUNUSED T(getitem)(char const *name, size_t len, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemStringLen(*this, name, len, hash));
	}
	WUNUSED T(getitem)(char const *name, size_t len, Dee_hash_t hash, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringLenDef(*this, name, len, hash, def));
	}
	WUNUSED item_proxy_obj operator[](DeeObject *index) const {
		return item_proxy_obj(*this, index);
	}
	WUNUSED item_proxy_idx operator[](int index) const {
		return item_proxy_idx(*this, (size_t)(unsigned int)index);
	}
	WUNUSED item_proxy_idx operator[](unsigned int index) const {
		return item_proxy_idx(*this, (size_t)index);
	}
	WUNUSED item_proxy_idx operator[](long index) const {
		return item_proxy_idx(*this, (size_t)(unsigned long)index);
	}
	WUNUSED item_proxy_idx operator[](unsigned long index) const {
		return item_proxy_idx(*this, (size_t)index);
	}
	WUNUSED item_proxy_sth operator[](char const *name) const {
		return item_proxy_sth(*this, name);
	}
	range_proxy_oo(range)(DeeObject *begin, DeeObject *end) const {
		return range_proxy_oo(*this, begin, end);
	}
	range_proxy_io(range)(size_t begin, DeeObject *end) const {
		return range_proxy_io(*this, begin, end);
	}
	range_proxy_oi(range)(DeeObject *begin, size_t end) const {
		return range_proxy_oi(*this, begin, end);
	}
	range_proxy_ii(range)(size_t begin, size_t end) const {
		return range_proxy_ii(*this, begin, end);
	}
	Sequence(getrange)(DeeObject *begin, DeeObject *end) const {
		return inherit(DeeObject_GetRange(*this, begin, end));
	}
	Sequence(getrange)(size_t begin, DeeObject *end) const {
		return inherit(DeeObject_GetRangeBeginIndex(*this, begin, end));
	}
	Sequence(getrange)(DeeObject *begin, size_t end) const {
		return inherit(DeeObject_GetRangeEndIndex(*this, begin, end));
	}
	Sequence(getrange)(size_t begin, size_t end) const {
		return inherit(DeeObject_GetRangeIndex(*this, begin, end));
	}

	WUNUSED deemon::Iterator<T>(iter)() const {
		return inherit(DeeObject_IterSelf(*this));
	}
	WUNUSED size_t(length)() const {
		return size();
	}
	WUNUSED first_proxy(first)() const {
		return first_proxy(*this);
	}
	WUNUSED last_proxy(last)() const {
		return last_proxy(*this);
	}
	Sequence(frozen)() const {
		return inherit(DeeObject_GetAttrString(*this, "frozen"));
	}
	WUNUSED T(reduce)(DeeObject *merger) const {
		return inherit(DeeObject_CallAttrStringf(*this, "reduce", "o", merger));
	}
	WUNUSED T(reduce)(DeeObject *merger, DeeObject *init) const {
		return inherit(DeeObject_CallAttrStringf(*this, "reduce", "oo", merger, init));
	}
	WUNUSED Sequence(filter)(DeeObject *keep) const {
		return inherit(DeeObject_CallAttrStringf(*this, "filter", "o", keep));
	}
	WUNUSED T(sum)() const {
		return inherit(DeeObject_CallAttrString(*this, "sum", 0, NULL));
	}
	WUNUSED T(min)() const {
		return inherit(DeeObject_CallAttrString(*this, "min", 0, NULL));
	}
	WUNUSED T(min)(DeeObject *key) const {
		return inherit(DeeObject_CallAttrStringf(*this, "min", "o", key));
	}
	WUNUSED T(max)() const {
		return inherit(DeeObject_CallAttrString(*this, "max", 0, NULL));
	}
	WUNUSED T(max)(DeeObject *key) const {
		return inherit(DeeObject_CallAttrStringf(*this, "max", "o", key));
	}
#define DEFINE_ELEM_SEARCH_FUNCTION(Treturn, name)                                \
	WUNUSED Treturn(name)(DeeObject *elem) const {                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));       \
	}                                                                             \
	WUNUSED Treturn(name)(DeeObject *elem, DeeObject *key) const {                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key)); \
	}
	DEFINE_ELEM_SEARCH_FUNCTION(Sequence, locateall)
#undef DEFINE_ELEM_SEARCH_FUNCTION
	WUNUSED Sequence(transform)(DeeObject *transformation) const {
		return inherit(DeeObject_CallAttrStringf(*this, "transform", "o", transformation));
	}
	WUNUSED Sequence(reversed)() const {
		return inherit(DeeObject_CallAttrString(*this, "reversed", 0, NULL));
	}
	WUNUSED Sequence(sorted)() const {
		return inherit(DeeObject_CallAttrString(*this, "sorted", 0, NULL));
	}
	WUNUSED Sequence(sorted)(DeeObject *key) const {
		return inherit(DeeObject_CallAttrStringf(*this, "sorted", "o", key));
	}
	WUNUSED deemon::Sequence<Sequence> segments(size_t segment_size) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", "Iu", segment_size));
	}
	WUNUSED deemon::Sequence<Sequence> segments(DeeObject *segment_size) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", "o", segment_size));
	}
	WUNUSED deemon::Sequence<Sequence> distribute(size_t bucket_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", "Iu", bucket_count));
	}
	WUNUSED deemon::Sequence<Sequence> distribute(DeeObject *bucket_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", "o", bucket_count));
	}
	WUNUSED deemon::Sequence<Sequence> combinations(size_t r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "combinations", "Iu", r));
	}
	WUNUSED deemon::Sequence<Sequence> combinations(DeeObject *r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "combinations", "o", r));
	}
	WUNUSED deemon::Sequence<Sequence> repeatcombinations(size_t r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "repeatcombinations", "Iu", r));
	}
	WUNUSED deemon::Sequence<Sequence> repeatcombinations(DeeObject *r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "repeatcombinations", "o", r));
	}
	WUNUSED deemon::Sequence<Sequence> permutations() const {
		return inherit(DeeObject_CallAttrString(*this, "permutations", 0, NULL));
	}
	WUNUSED deemon::Sequence<Sequence> permutations(size_t r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "permutations", "Iu", r));
	}
	WUNUSED deemon::Sequence<Sequence> permutations(DeeObject *r) const {
		return inherit(DeeObject_CallAttrStringf(*this, "permutations", "o", r));
	}
	WUNUSED T xch(size_t index, DeeObject *value) const {
		return inherit(DeeObject_CallAttrStringf(*this, "xch", "Iuo", index, value));
	}
	WUNUSED T xch(DeeObject *index, DeeObject *value) const {
		return inherit(DeeObject_CallAttrStringf(*this, "xch", "oo", index, value));
	}
	WUNUSED T pop() const {
		return inherit(DeeObject_CallAttrString(*this, "pop", 0, NULL));
	}
	WUNUSED T pop(Dee_ssize_t index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "pop", "Id", index));
	}
	WUNUSED T pop(DeeObject *index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "pop", "o", index));
	}
	WUNUSED T popfront() const {
		return inherit(DeeObject_CallAttrString(*this, "popfront", 0, NULL));
	}
	WUNUSED T popback() const {
		return inherit(DeeObject_CallAttrString(*this, "popback", 0, NULL));
	}
	WUNUSED Sequence add(DeeObject *other_sequence) const {
		return inherit(DeeObject_Add(*this, other_sequence));
	}
	WUNUSED Sequence operator+(DeeObject *other_sequence) const {
		return inherit(DeeObject_Add(*this, other_sequence));
	}
	WUNUSED Sequence &inplace_add(DeeObject *other_sequence) {
		Object::inplace_add(other_sequence);
		return *this;
	}
	WUNUSED Sequence &operator+=(DeeObject *other_sequence) {
		Object::operator+=(other_sequence);
		return *this;
	}

protected:
	WUNUSED DREF DeeObject *_addref(std::initializer_list<T> const &items) const {
		DREF DeeObject *items_ob, *result;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(items.size(), (DeeObject **)items.begin()));
		result   = DeeObject_Add(*this, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		return result;
	}

public:
	WUNUSED Sequence add(std::initializer_list<T> const &items) const {
		return inherit(_addref(items));
	}
	WUNUSED Sequence operator+(std::initializer_list<T> const &items) const {
		return inherit(_addref(items));
	}
	WUNUSED Sequence &inplace_add(std::initializer_list<T> const &items) {
		DREF DeeObject *items_ob;
		int error;
		items_ob = throw_if_null(DeeTuple_NewVectorSymbolic(items.size(), (DeeObject **)items.begin()));
		error    = DeeObject_InplaceAdd(&this->m_ptr, items_ob);
		DeeTuple_DecrefSymbolic(items_ob);
		throw_if_nonzero(error);
		return *this;
	}
	WUNUSED Sequence &operator+=(std::initializer_list<T> const &items) {
		return inplace_add(items);
	}

protected:
	WUNUSED DREF DeeObject *_mulref(size_t n) const {
		DREF DeeObject *result, *temp;
		if (n <= 0x7f)
			return inherit(DeeObject_MulInt(*this, (int8_t)n));
		temp   = throw_if_null(DeeInt_NewSize(n));
		result = DeeObject_Mul(*this, temp);
		Dee_Decref(temp);
		return result;
	}

public:
	WUNUSED Sequence mul(DeeObject *n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED Sequence mul(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED Sequence mul(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED Sequence operator*(DeeObject *n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED Sequence operator*(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED Sequence operator*(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED Sequence &inplace_mul(DeeObject *n) {
		Object::inplace_mul(n);
		return *this;
	}
	WUNUSED Sequence &inplace_mul(int8_t n) {
		Object::inplace_mul(n);
		return *this;
	}
	WUNUSED Sequence &inplace_mul(size_t n) {
		if (n <= 0x7f) {
			Object::inplace_mul((int8_t)n);
		} else {
			DREF DeeObject *temp;
			int error;
			temp  = throw_if_null(DeeInt_NewSize(n));
			error = DeeObject_InplaceMul(&this->m_ptr, temp);
			Dee_Decref(temp);
			throw_if_nonzero(error);
		}
		return *this;
	}
	WUNUSED Sequence &operator*=(DeeObject *n) {
		return inplace_mul(n);
	}
	WUNUSED Sequence &operator*=(int8_t n) {
		return inplace_mul(n);
	}
	WUNUSED Sequence &operator*=(size_t n) {
		return inplace_mul(n);
	}

	using sequence_base::insert;
	void(insert)(size_t index, T const &item) const {
		sequence_base::insert(index, item);
	}
	void(insert)(DeeObject *index, T const &item) const {
		sequence_base::insert(index, item);
	}

	using sequence_base::insertall;
	void(insertall)(size_t index, std::initializer_list<T> const &items) const {
		sequence_base::insertall(index, (std::initializer_list<DeeObject *> const &)items);
	}
	void(insertall)(size_t index, size_t objc, T *const *objv) const {
		sequence_base::insertall(index, objc, (DeeObject *const *)objv);
	}
	void(insertall)(size_t index, size_t objc, T **objv) const {
		sequence_base::insertall(index, objc, (DeeObject *const *)objv);
	}
	void(insertall)(DeeObject *index, std::initializer_list<T> const &items) const {
		sequence_base::insertall(index, (std::initializer_list<DeeObject *> const &)items);
	}
	void(insertall)(DeeObject *index, size_t objc, T *const *objv) const {
		sequence_base::insertall(index, objc, (DeeObject *const *)objv);
	}
	void(insertall)(DeeObject *index, size_t objc, T **objv) const {
		sequence_base::insertall(index, objc, (DeeObject *const *)objv);
	}
	using sequence_base::extend;
	void(extend)(std::initializer_list<T> const &items) const {
		sequence_base::extend((std::initializer_list<DeeObject *> const &)items);
	}
	void(extend)(size_t objc, T *const *objv) const {
		sequence_base::extend(objc, (DeeObject *const *)objv);
	}
	void(extend)(size_t objc, T **objv) const {
		sequence_base::extend(objc, (DeeObject *const *)objv);
	}

	using sequence_base::append;
	void append(T const &item) const {
		sequence_base::append(item);
	}
	using sequence_base::pushfront;
	void pushfront(T const &item) const {
		sequence_base::pushfront(item);
	}
	using sequence_base::pushback;
	void pushback(T const &item) const {
		sequence_base::pushback(item);
	}
	using sequence_base::resize;
	void(resize)(size_t new_size, T const &filler) const {
		sequence_base::resize(new_size, filler);
	}
	void(resize)(DeeObject *new_size, T const &filler) const {
		sequence_base::resize(new_size, filler);
	}
	using sequence_base::fill;
	void(fill)(size_t start, size_t end, DeeObject *filler) const {
		sequence_base::fill(start, end, filler);
	}
	void(fill)(DeeObject *start, size_t end, DeeObject *filler) const {
		sequence_base::fill(start, end, filler);
	}
	void(fill)(DeeObject *start, DeeObject *end, DeeObject *filler) const {
		sequence_base::fill(start, end, filler);
	}
};

inline Sequence<Object> detail::sequence_base::makerange(DeeObject *end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "o", end));
}

inline Sequence<Object> detail::sequence_base::makerange(DeeObject *start, DeeObject *end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oo", start, end));
}

inline Sequence<Object> detail::sequence_base::makerange(DeeObject *start, DeeObject *end, DeeObject *step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "ooo", start, end, step));
}

inline Sequence<Object> detail::sequence_base::repeat(DeeObject *item, size_t count) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "repeat", "oIu", item, count));
}

inline Sequence<Object> detail::sequence_base::repeat(DeeObject *item, DeeObject *count) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "repeat", "oo", item, count));
}

inline Sequence<Object> detail::sequence_base::repeatseq(DeeObject *seq, size_t count) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "repeatseq", "oIu", seq, count));
}

inline Sequence<Object> detail::sequence_base::repeatseq(DeeObject *seq, DeeObject *count) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "repeatseq", "oo", seq, count));
}

inline Sequence<Object> detail::sequence_base::concat(size_t nseq, DeeObject **seqv) {
	return inherit(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "concat", nseq, seqv));
}

inline Sequence<Object> detail::sequence_base::concat(size_t nseq, DeeObject *const *seqv) {
	return inherit(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "concat", nseq, (DeeObject **)seqv));
}

inline Sequence<Object> detail::sequence_base::concat(size_t nseq, Object **seqv) {
	return inherit(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "concat", nseq, (DeeObject **)seqv));
}

inline Sequence<Object> detail::sequence_base::concat(size_t nseq, Object *const *seqv) {
	return inherit(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "concat", nseq, (DeeObject **)seqv));
}

inline Sequence<Object> detail::sequence_base::concat(std::initializer_list<DeeObject *> const &sequences) {
	return inherit(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "concat", sequences.size(), (DeeObject **)sequences.begin()));
}

#ifdef GUARD_DEEMON_CXX_INT_H
inline deemon::int_ detail::sequence_base::erase(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "Iu", index));
}

inline deemon::int_ detail::sequence_base::erase(size_t index, size_t count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "IuIu", index, count));
}

inline deemon::int_ detail::sequence_base::erase(DeeObject *index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "o", index));
}

inline deemon::int_ detail::sequence_base::erase(DeeObject *index, size_t count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "oIu", index, count));
}

inline deemon::int_ detail::sequence_base::erase(DeeObject *index, DeeObject *count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "oo", index, count));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Id", end));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdId", start, end));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdIdId", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end, DeeObject *step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdIdo", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Ido", start, end));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdoId", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *end, DeeObject *step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Idoo", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *start, Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oId", start, end));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *start, Dee_ssize_t end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oIdId", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *start, Dee_ssize_t end, DeeObject *step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oIdo", start, end, step));
}

inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *start, DeeObject *end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "ooId", start, end, step));
}
#define DEFINE_ELEM_SEARCH_FUNCTION(Treturn, name)                                       \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem) const {                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));              \
	}                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, DeeObject *key) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));        \
	}
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, count)
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, locate)
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, rlocate)
#undef DEFINE_ELEM_SEARCH_FUNCTION
#define DEFINE_ELEM_FIND_FUNCTION(Treturn, name)                                                                   \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem) const {                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));                                        \
	}                                                                                                              \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, DeeObject *key) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));                                  \
	}                                                                                                              \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, size_t start) const {                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", elem, start));                               \
	}                                                                                                              \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, size_t start, DeeObject *key) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", elem, start, key));                         \
	}                                                                                                              \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, size_t start, size_t end) const {                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", elem, start, end));                        \
	}                                                                                                              \
	inline Treturn(detail::sequence_base::name)(DeeObject *elem, size_t start, size_t end, DeeObject *key) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", elem, start, end, key));                  \
	}
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, find)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rfind)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, index)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rindex)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, removeall)
#undef DEFINE_ELEM_FIND_FUNCTION
#endif /* GUARD_DEEMON_CXX_INT_H */


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SEQUENCE_H */
