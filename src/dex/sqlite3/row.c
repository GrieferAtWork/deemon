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
#ifndef GUARD_DEX_SQLITE3_ROW_C
#define GUARD_DEX_SQLITE3_ROW_C 1
#define CONFIG_BUILDING_LIBSQLITE3
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/util/lock.h>

#include <stddef.h>
#include <stdint.h>

/**/
#include "libsqlite3.h"

DECL_BEGIN

/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Cell                                                                 */
/*                                                                      */
/************************************************************************/
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cell_getvalue(struct cell *__restrict self) {
	switch (self->c_type) {
	case CELLTYPE_OBJECT:
		return_reference(self->c_data.d_obj);
	case CELLTYPE_NONE:
		return_none;
	case CELLTYPE_INT:
		return DeeInt_NewInt64(self->c_data.d_int);
	case CELLTYPE_FLOAT:
		return DeeFloat_New(self->c_data.d_float);
	default: __builtin_unreachable();
	}
}

/* Destroy cell data */
INTERN NONNULL((1)) void DCALL
cell_destroyrow(struct cell *__restrict data, unsigned int ncol) {
	while (ncol--)
		cell_fini(&data[ncol]);
	Dee_Free(data);
}

INTERN NONNULL((1, 3)) void DCALL
cell_visitrow(struct cell *__restrict data, unsigned int ncol, Dee_visit_t proc, void *arg) {
	while (ncol--)
		cell_visit(&data[ncol]);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF CellFmt *DCALL
CellFmt_New(RowFmt *__restrict rowfmt, struct cellfmt const *__restrict cell) {
	DREF CellFmt *result = DeeObject_MALLOC(CellFmt);
	if likely(result) {
		DeeObject_Init(result, &CellFmt_Type);
		result->cfo_fmt  = rowfmt;
		result->cfo_cell = cell;
		Dee_Incref(rowfmt);
	}
	return result;
}

PRIVATE NONNULL((1)) void DCALL
ob_cellfmt_fini(CellFmt *__restrict self) {
	Dee_Decref_unlikely(self->cfo_fmt);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
cellfmt_print(CellFmt *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *decltype_ = self->cfo_cell->cfmt_decltype;
	return DeeFormat_Printf(printer, arg, "<cell '%#q%s%#q'>",
	                        DeeString_STR(self->cfo_cell->cfmt_name),
	                        decltype_ ? " " : "",
	                        decltype_ ? DeeString_STR(decltype_) : "");
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
cellfmt_printrepr(CellFmt *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r[%r]",
	                        self->cfo_fmt,
	                        self->cfo_cell->cfmt_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cellfmt_getname(CellFmt *__restrict self) {
	return_reference(self->cfo_cell->cfmt_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
cellfmt_getdecltype(CellFmt *__restrict self) {
	DeeStringObject *result = self->cfo_cell->cfmt_decltype;
	if likely(result) {
		Dee_Incref(result);
		return result;
	}
	err_unbound_attribute_string(&CellFmt_Type, "decltype");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cellfmt_bounddecltype(CellFmt *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->cfo_cell->cfmt_decltype);
}

PRIVATE struct type_getset tpconst cellfmt_getsets[] = {
	TYPE_GETTER_AB_F("name", &cellfmt_getname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dstring"),
	TYPE_GETTER_BOUND_F("decltype", &cellfmt_getdecltype, &cellfmt_bounddecltype,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cellfmt_members[] = {
	TYPE_MEMBER_FIELD_DOC("__rowfmt__", STRUCT_OBJECT, offsetof(CellFmt, cfo_fmt), "->?G_RowFmt"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CellFmt_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CellFmt",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL, /* TODO */
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(CellFmt),
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ob_cellfmt_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cellfmt_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cellfmt_printrepr,
	},
	/* .tp_visit         = */ NULL, /* Not needed -- only references DeeStringObject objects (or other objects that only reference such) */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cellfmt_getsets,
	/* .tp_members       = */ cellfmt_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};







/************************************************************************/
/************************************************************************/
/*                                                                      */
/* RowFmtColumns                                                        */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
rowfmtcolumns_fini(RowFmtColumns *__restrict self) {
	Dee_Decref(self->rfc_fmt);
}

PRIVATE WUNUSED NONNULL((1)) DREF RowFmtColumns *DCALL
RowFmtColumns_NewInherited(/*inherit(always)*/DREF RowFmt *__restrict self) {
	DREF RowFmtColumns *result = DeeObject_MALLOC(RowFmtColumns);
	if likely(result) {
		DeeObject_Init(result, &RowFmtColumns_Type);
		result->rfc_fmt = self; /* Inherited */
	} else {
		Dee_Decref_unlikely(self); /* always inherited */
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RowFmtColumns *DCALL
RowFmtColumns_New(RowFmt *__restrict self) {
	Dee_Incref(self);
	return RowFmtColumns_NewInherited(self);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rowfmtcolumns_size_fast(RowFmtColumns *__restrict self) {
	return self->rfc_fmt->rf_ncol;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
rowfmtcolumns_getitem_index_fast(RowFmtColumns *__restrict self, size_t index) {
	DeeStringObject *result;
	ASSERT(index < self->rfc_fmt->rf_ncol);
	result = self->rfc_fmt->rf_cols[index].cfmt_name;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
rowfmtcolumns_getitem_index(RowFmtColumns *__restrict self, size_t index) {
	if unlikely(index >= self->rfc_fmt->rf_ncol)
		goto err_oob;
	return rowfmtcolumns_getitem_index_fast(self, index);
err_oob:
	err_index_out_of_bounds((DeeObject *)self, index, self->rfc_fmt->rf_ncol);
	return NULL;
}

PRIVATE struct type_seq rowfmtcolumns_seq = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&rowfmtcolumns_size_fast,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&rowfmtcolumns_size_fast,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rowfmtcolumns_getitem_index,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rowfmtcolumns_getitem_index_fast,
};

PRIVATE struct type_member tpconst rowfmtcolumns_members[] = {
	TYPE_MEMBER_FIELD_DOC("__rowfmt__", STRUCT_OBJECT,
	                      offsetof(RowFmtColumns, rfc_fmt),
	                      "->?G_RowFmt"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rowfmtcolumns_class_members[] = {
	TYPE_MEMBER_CONST("ItemType", &DeeString_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RowFmtColumns_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RowFmtColumns",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL, /* TODO */
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(RowFmtColumns),
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rowfmtcolumns_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ NULL, /* Not needed -- only references DeeStringObject objects */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rowfmtcolumns_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rowfmtcolumns_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rowfmtcolumns_class_members,
	/* .tp_method_hints  = */ NULL,
};







/************************************************************************/
/************************************************************************/
/*                                                                      */
/* RowFmt                                                               */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
rowfmt_fini(RowFmt *__restrict self) {
	unsigned int i;
	for (i = 0; i < self->rf_ncol; ++i)
		cellfmt_fini(&self->rf_cols[i]);
}

/* Returns the index of column "column_name", or `(size_t)-1' if not found */
PRIVATE NONNULL((1, 2)) unsigned int DCALL
rowfmt_indexof_string(RowFmt *__restrict self, char const *column_name) {
	unsigned int i;
	for (i = 0; i < self->rf_ncol; ++i) {
		struct cellfmt *cell = &self->rf_cols[i];
		if (DeeString_EqualsCStr(cell->cfmt_name, column_name))
			return i;
	}
	return (unsigned int)-1;
}

PRIVATE NONNULL((1, 2)) unsigned int DCALL
rowfmt_indexof_string_len(RowFmt *__restrict self,
                          char const *column_name,
                          size_t column_namelen) {
	unsigned int i;
	for (i = 0; i < self->rf_ncol; ++i) {
		struct cellfmt *cell = &self->rf_cols[i];
		if (DeeString_EqualsBuf(cell->cfmt_name, column_name, column_namelen))
			return i;
	}
	return (unsigned int)-1;
}

PRIVATE NONNULL((1)) size_t DCALL
rowfmt_size_fast(RowFmt *__restrict self) {
	return self->rf_ncol;
}

PRIVATE NONNULL((1)) DREF CellFmt *DCALL
rowfmt_getitem_index(RowFmt *__restrict self, size_t index) {
	if unlikely(index >= self->rf_ncol)
		goto err_obb;
	return CellFmt_New(self, &self->rf_cols[index]);
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->rf_ncol);
	return NULL;
}

PRIVATE NONNULL((1)) int DCALL
rowfmt_bounditem_index(RowFmt *__restrict self, size_t index) {
	return Dee_BOUND_FROMPRESENT_BOUND(index < self->rf_ncol);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CellFmt *DCALL
rowfmt_getitem_string_hash(RowFmt *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	unsigned int index = rowfmt_indexof_string(self, name);
	if unlikely(index == (unsigned int)-2)
		return NULL;
	if unlikely(index == (unsigned int)-1) {
		err_unknown_key_str((DeeObject *)self, name);
		return NULL;
	}
	return rowfmt_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CellFmt *DCALL
rowfmt_getitem_string_len_hash(RowFmt *__restrict self, char const *name, size_t namelen, Dee_hash_t UNUSED(hash)) {
	unsigned int index = rowfmt_indexof_string_len(self, name, namelen);
	if unlikely(index == (unsigned int)-2)
		return NULL;
	if unlikely(index == (unsigned int)-1) {
		err_unknown_key_str_len((DeeObject *)self, name, namelen);
		return NULL;
	}
	return rowfmt_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CellFmt *DCALL
rowfmt_getitem(RowFmt *self, DeeObject *key) {
	size_t index;
	if (DeeString_Check(key))
		return rowfmt_getitem_string_len_hash(self, DeeString_STR(key), DeeString_SIZE(key), 0);
	if (DeeObject_AsSize(key, &index))
		goto err;
	return rowfmt_getitem_index(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rowfmt_bounditem_string_hash(RowFmt *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	unsigned int index = rowfmt_indexof_string(self, name);
	if unlikely(index == (unsigned int)-2)
		return Dee_BOUND_ERR;
	return Dee_BOUND_FROMPRESENT_BOUND(index == (unsigned int)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rowfmt_bounditem_string_len_hash(RowFmt *__restrict self, char const *name, size_t namelen, Dee_hash_t UNUSED(hash)) {
	unsigned int index = rowfmt_indexof_string_len(self, name, namelen);
	if unlikely(index == (unsigned int)-2)
		return Dee_BOUND_ERR;
	return Dee_BOUND_FROMPRESENT_BOUND(index == (unsigned int)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rowfmt_bounditem(RowFmt *self, DeeObject *key) {
	size_t index;
	if (DeeString_Check(key))
		return rowfmt_bounditem_string_len_hash(self, DeeString_STR(key), DeeString_SIZE(key), 0);
	if (DeeObject_AsSize(key, &index))
		goto err;
	return rowfmt_bounditem_index(self, index);
err:
	return Dee_BOUND_ERR;
}

#define rowfmt_keys RowFmtColumns_New

PRIVATE struct type_seq rowfmt_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rowfmt_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&rowfmt_bounditem,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rowfmt_size_fast,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rowfmt_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rowfmt_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rowfmt_bounditem_index,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rowfmt_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rowfmt_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rowfmt_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rowfmt_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_getset tpconst rowfmt_getsets[] = {
	TYPE_GETTER_AB_F("keys", &rowfmt_keys, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?G_RowFmtColumns"),
	TYPE_GETTER_AB_F("frozen", &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETTER_AB_F("cached", &DeeObject_NewRef, METHOD_FCONSTCALL, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst rowfmt_class_members[] = {
	TYPE_MEMBER_CONST("Keys", &RowFmtColumns_Type),
	TYPE_MEMBER_CONST("KeyType", &DeeString_Type),
	TYPE_MEMBER_CONST("ValueType", &CellFmt_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RowFmt_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RowFmt",
	/* .tp_doc      = */ DOC("[](key:?Dstring)->?G_CellFmt\n"
	                         "[](index:?Dint)->?G_CellFmt"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				/* .tp_free      = */ (Dee_funptr_t)NULL,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rowfmt_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rowfmt_print,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ NULL, /* Not needed -- only references DeeStringObject objects */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rowfmt_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rowfmt_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rowfmt_class_members,
	/* .tp_method_hints  = */ NULL,
};












/************************************************************************/
/************************************************************************/
/*                                                                      */
/* Row                                                                  */
/*                                                                      */
/************************************************************************/
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
row_fini(Row *__restrict self) {
	if (self->r_query) {
		ASSERT(!self->r_rowfmt);
		ASSERT(!self->r_cells);
		Dee_Decref_unlikely(self->r_query);
	} else {
		ASSERT(self->r_rowfmt);
		ASSERT(self->r_cells);
		cell_destroyrow(self->r_cells, self->r_rowfmt->rf_ncol);
		Dee_Decref(self->r_rowfmt);
	}
	weakref_support_fini(self);
}

PRIVATE NONNULL((1, 2)) void DCALL
row_visit(Row *__restrict self, Dee_visit_t proc, void *arg) {
	Row_LockRead(self);
	if (self->r_query) {
		ASSERT(!self->r_rowfmt);
		ASSERT(!self->r_cells);
		Dee_Visit(self->r_query);
	} else {
		ASSERT(self->r_rowfmt);
		ASSERT(self->r_cells);
		cell_visitrow(self->r_cells, self->r_rowfmt->rf_ncol, proc, arg);
		Dee_Visit(self->r_rowfmt);
	}
	Row_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF Query *DCALL
row_getquery(Row *__restrict self) {
	DREF Query *result;
	Row_LockRead(self);
	result = self->r_query;
	if unlikely(!result) {
		Row_LockEndRead(self);
		err_unbound_attribute_string(&Row_Type, "query");
		return NULL;
	}
	Dee_Incref(result);
	Row_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
row_boundquery(Row *__restrict self) {
	int result;
	Row_LockRead(self);
	result = Dee_BOUND_FROMBOOL(self->r_query != NULL);
	Row_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RowFmt *DCALL
row_getfmt(Row *__restrict self) {
	DREF RowFmt *result;
	Row_LockRead(self);
	if (self->r_query) {
		DREF Query *query;
		ASSERT(!self->r_rowfmt);
		query = self->r_query;
		Dee_Incref(query);
		Row_LockEndRead(self);
		result = Query_GetRowFmt(query);
		Dee_XIncref(result);
		Dee_Decref(query);
	} else {
		ASSERT(self->r_rowfmt);
		result = self->r_rowfmt;
		Dee_Incref(result);
		Row_LockEndRead(self);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
row_size_fast(Row *__restrict self) {
	unsigned int result;
	Row_LockRead(self);
	if (self->r_query) {
		result = (unsigned int)sqlite3_column_count(self->r_query->q_stmt);
	} else {
		result = self->r_rowfmt->rf_ncol;
	}
	Row_LockEndRead(self);
	return (size_t)result;
}

PRIVATE WUNUSED NONNULL((1)) unsigned int DCALL
row_indexof_string(Row *__restrict self, char const *column_name) {
	unsigned int result;
	DREF RowFmt *fmt = row_getfmt(self);
	if unlikely(!fmt)
		goto err;
	result = rowfmt_indexof_string(fmt, column_name);
	Dee_Decref(fmt);
	return result;
err:
	return (unsigned int)-2;
}

PRIVATE WUNUSED NONNULL((1)) unsigned int DCALL
row_indexof_string_len(Row *__restrict self,
                       char const *column_name,
                       size_t column_namelen) {
	unsigned int result;
	DREF RowFmt *fmt = row_getfmt(self);
	if unlikely(!fmt)
		goto err;
	result = rowfmt_indexof_string_len(fmt, column_name, column_namelen);
	Dee_Decref(fmt);
	return result;
err:
	return (unsigned int)-2;
}


/* Upgrade a read-lock to `self->r_lock' (which gets released)
 * into locks equivalent to `Query_LockDB()' */
#define ROW_READUPGRADE_LOCKDB_SUCCESS 1    /* Success */
#define ROW_READUPGRADE_LOCKDB_RETRY   0    /* Try again (lock to "self" was lost) */
#define ROW_READUPGRADE_LOCKDB_ERROR   (-1) /* Error was thrown (lock to "self" was lost) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Row_ReadUpgradeLockDB(Row *__restrict self) {
	int result;
	Query *query = self->r_query;
	ASSERT(Row_LockReading(self));
	ASSERT(query);
	if likely(Query_TryLockDB(query)) {
		Row_LockEndRead(self);
		ASSERT(self->r_query == query);
		return ROW_READUPGRADE_LOCKDB_SUCCESS;
	}
	Dee_Incref(query);
	Row_LockEndRead(self);
	result = Query_WaitForDB(query);
#if ROW_READUPGRADE_LOCKDB_RETRY != 0
	if likely(result == 0)
		result = ROW_READUPGRADE_LOCKDB_RETRY;
#endif /* ROW_READUPGRADE_LOCKDB_RETRY != 0 */
	Dee_Decref_unlikely(query);
	ASSERT(result == ROW_READUPGRADE_LOCKDB_RETRY ||
	       result == ROW_READUPGRADE_LOCKDB_ERROR);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
row_getitem_index(Row *__restrict self, size_t index) {
	DREF DeeObject *result;
	Query *query;
	size_t length;
	int column_type;
	unsigned int data_length;
again:
	Row_LockRead(self);
	if ((query = self->r_query) == NULL) {
		struct cell *cell;
		Row_LockEndRead(self);
		length = self->r_rowfmt->rf_ncol;
		if unlikely(index >= length)
			goto err_oob;
		cell = &self->r_cells[index];
		return cell_getvalue(cell);
	}

	/* Complicated (but likely) case: active row */
	switch (Row_ReadUpgradeLockDB(self)) {
	case ROW_READUPGRADE_LOCKDB_SUCCESS:
		break;
	case ROW_READUPGRADE_LOCKDB_RETRY:
		goto again;
	case ROW_READUPGRADE_LOCKDB_ERROR:
		goto err;
	default: __builtin_unreachable();
	}

	/* Got a lock to the query -> read data from sqlite */
	length = (size_t)sqlite3_column_count(query->q_stmt);
	if unlikely(index >= length) {
		Query_UnlockDB(query);
		goto err_oob;
	}

	column_type = sqlite3_column_type(query->q_stmt, (int)index);
	switch (column_type) {

	case SQLITE_INTEGER: {
		int64_t value = sqlite3_column_int64(query->q_stmt, (int)index);
		Query_UnlockDB(query);
		result = DeeInt_NewInt64(value);
	}	break;

	case SQLITE_FLOAT: {
		double value = sqlite3_column_double(query->q_stmt, (int)index);
		Query_UnlockDB(query);
		result = DeeFloat_New(value);
	}	break;

	case SQLITE_TEXT: {
		unsigned char const *text;
		data_length = (unsigned int)sqlite3_column_bytes(query->q_stmt, (int)index);
		text = sqlite3_column_text(query->q_stmt, (int)index);
		if unlikely(!text) {
unlock_and_collect_length_memory:
			Query_UnlockDB(query);
			if (!Dee_CollectMemoryc((data_length + 1), sizeof(char)))
				goto err;
			goto again;
		}
		result = DeeString_TryNewUtf8((char const *)text, data_length,
		                              STRING_ERROR_FIGNORE);
		if unlikely(!result)
			goto unlock_and_collect_length_memory;
		Query_UnlockDB(query);
	}	break;

	case SQLITE_BLOB: {
		void const *blob = NULL;
		data_length = (unsigned int)sqlite3_column_bytes(query->q_stmt, (int)index);
		if (data_length) {
			blob = sqlite3_column_blob(query->q_stmt, (int)index);
			if unlikely(!blob)
				goto unlock_and_collect_length_memory;
		}
		result = DeeBytes_TryNewBufferData(blob, data_length);
		if unlikely(!result)
			goto unlock_and_collect_length_memory;
		Query_UnlockDB(query);
	}	break;

	default:
		Query_UnlockDB(query);
		result = DeeNone_NewRef();
		break;
	}
	return result;
err_oob:
	err_index_out_of_bounds((DeeObject *)self, index, length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
row_getitem_string_hash(Row *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	unsigned int index = row_indexof_string(self, name);
	if unlikely(index == (unsigned int)-2)
		return NULL;
	if unlikely(index == (unsigned int)-1) {
		err_unknown_key_str((DeeObject *)self, name);
		return NULL;
	}
	return row_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
row_getitem_string_len_hash(Row *__restrict self, char const *name,
                            size_t namelen, Dee_hash_t UNUSED(hash)) {
	unsigned int index = row_indexof_string_len(self, name, namelen);
	if unlikely(index == (unsigned int)-2)
		return NULL;
	if unlikely(index == (unsigned int)-1) {
		err_unknown_key_str_len((DeeObject *)self, name, namelen);
		return NULL;
	}
	return row_getitem_index(self, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
row_bounditem_index(Row *__restrict self, size_t index) {
	size_t length = row_size_fast(self);
	return Dee_BOUND_FROMPRESENT_BOUND(index < length);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
row_bounditem_string_hash(Row *__restrict self, char const *name, Dee_hash_t UNUSED(hash)) {
	unsigned int index = row_indexof_string(self, name);
	if unlikely(index == (unsigned int)-2)
		return Dee_BOUND_ERR;
	return Dee_BOUND_FROMPRESENT_BOUND(index != (unsigned int)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
row_bounditem_string_len_hash(Row *__restrict self, char const *name,
                              size_t namelen, Dee_hash_t UNUSED(hash)) {
	unsigned int index = row_indexof_string_len(self, name, namelen);
	if unlikely(index == (unsigned int)-2)
		return Dee_BOUND_ERR;
	return Dee_BOUND_FROMPRESENT_BOUND(index != (unsigned int)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
row_getitem(Row *self, DeeObject *key) {
	size_t index;
	if (DeeString_Check(key))
		return row_getitem_string_len_hash(self, DeeString_STR(key), DeeString_SIZE(key), 0);
	if (DeeObject_AsSize(key, &index))
		goto err;
	return row_getitem_index(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
row_bounditem(Row *self, DeeObject *key) {
	size_t index;
	if (DeeString_Check(key))
		return row_bounditem_string_len_hash(self, DeeString_STR(key), DeeString_SIZE(key), 0);
	if (DeeObject_AsSize(key, &index))
		goto err;
	return row_bounditem_index(self, index);
err:
	return Dee_BOUND_ERR;
}



PRIVATE WUNUSED NONNULL((1)) DREF RowFmtColumns *DCALL
row_getkeys(Row *__restrict self) {
	DREF RowFmt *rowfmt = row_getfmt(self);
	if unlikely(!rowfmt)
		goto err;
	return RowFmtColumns_NewInherited(rowfmt);
err:
	return NULL;
}

PRIVATE struct type_seq row_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&row_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&row_bounditem,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&row_size_fast,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&row_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&row_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&row_bounditem_index,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&row_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&row_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&row_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&row_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_getset tpconst row_getsets[] = {
	TYPE_GETTER_AB("keys", &row_getkeys, "->?G_RowFmtColumns"),
	TYPE_GETTER_BOUND("query", &row_getquery, &row_boundquery,
	                  "->?GQuery\n"
	                  "Returns the query linked to this row. "
	                  /**/ "Throws :UnboundAttribute if @this row has been "
	                  /**/ "detached (which happens when another row is read)"),
	TYPE_GETTER_AB("_fmt", &row_getfmt,
	               "->?G_RowFmt\n"
	               "Returns the format descriptor for rows of the associated query"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst row_class_members[] = {
	TYPE_MEMBER_CONST("Keys", &RowFmtColumns_Type),
	TYPE_MEMBER_CONST("KeyType", &DeeString_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject Row_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Row",
	/* .tp_doc      = */ DOC("[](key:?Dstring)->" T_SQL_OBJECT "\n"
	                         "[](index:?Dint)->" T_SQL_OBJECT),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Row),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Row),
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&row_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&row_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &row_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ row_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ row_class_members,
	/* .tp_method_hints  = */ NULL,
};

DECL_END

#endif /* !GUARD_DEX_SQLITE3_ROW_C */
