/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

struct module_object;
struct code_object;
struct class_attribute;
struct class_desc;

#define CATCH_ATTRIBUTE_ERROR()                  \
	(DeeError_Catch(&DeeError_AttributeError) || \
	 DeeError_Catch(&DeeError_NotImplemented))

INTDEF ATTR_COLD int DCALL err_no_active_exception(void);
INTDEF ATTR_COLD int DCALL err_subclass_final_type(DeeTypeObject *__restrict tp);
#define err_unexpected_type(self, wanted_type) DeeObject_TypeAssertFailed(self, wanted_type)
#define err_unimplemented_constructor(tp, argc, argv) \
	err_unimplemented_constructor_kw(tp, argc, argv, NULL)
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_constructor_kw(DeeTypeObject *tp, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_divide_by_zero(DeeObject *a, DeeObject *b);
INTDEF ATTR_COLD int DCALL err_divide_by_zero_i(dssize_t a);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_shift_negative(DeeObject *a, DeeObject *b, bool is_left_shift);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_cannot_weak_reference(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_reference_loop(DeeObject *a, DeeObject *b);
INTDEF ATTR_COLD int DCALL err_cannot_lock_weakref(void);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_bytes_not_writable(DeeObject *__restrict bytes_ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_index_out_of_bounds(DeeObject *__restrict self, size_t index, size_t size);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_index_out_of_bounds_ob(DeeObject *self, DeeObject *index);
INTDEF ATTR_COLD int DCALL err_va_index_out_of_bounds(size_t index, size_t size);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_index(DeeObject *__restrict self, size_t index);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_single_character_string(DeeObject *__restrict str);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_integer_overflow(DeeObject *__restrict overflowing_object, size_t cutoff_bits, bool positive_overflow);
INTDEF ATTR_COLD int DCALL err_integer_overflow_i(size_t cutoff_bits, bool positive_overflow);
#define xcheck_empty_keywords(kw) (!(kw) ? 0 : check_empty_keywords(kw))
INTDEF NONNULL((1, 2)) int FCALL check_empty_keywords(DeeObject *kw, DeeTypeObject *tp_self);
INTDEF NONNULL((1)) int FCALL check_empty_keywords_obj(DeeObject *__restrict kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_keywords_not_accepted(DeeTypeObject *tp_self, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_keywords_func_not_accepted(char const *__restrict name, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_keywords_ctor_not_accepted(DeeTypeObject *tp_self, DeeObject *kw);
INTDEF ATTR_COLD int DCALL err_keywords_bad_for_argc(size_t argc, size_t kwdc);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_keywords_not_found(char const *__restrict keyword);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_keywords_shadows_positional(char const *__restrict keyword);
INTDEF ATTR_COLD int DCALL err_invalid_segment_size(size_t segsz);
INTDEF ATTR_COLD int DCALL err_invalid_distribution_count(size_t distcnt);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_argc_missing_kw(char const *__restrict argument_name, char const *function_name, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc(char const *function_name, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc_len(char const *function_name, size_t function_size, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc_va(char const *function_name, size_t argc_cur, size_t argc_min);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_argc_unpack(DeeObject *__restrict unpack_object, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_unpack_size(DeeObject *__restrict unpack_object, size_t need_size, size_t real_size);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_unpack_size_minmax(DeeObject *__restrict unpack_object, size_t need_size_min, size_t need_size_max, size_t real_size);
INTDEF ATTR_COLD int DCALL err_invalid_va_unpack_size(size_t need_size, size_t real_size);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_invalid_unpack_iter_size(DeeObject *unpack_object, DeeObject *unpack_iterator, size_t need_size);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_invalid_unpack_iter_size_minmax(DeeObject *unpack_object, DeeObject *unpack_iterator, size_t need_size_min, size_t need_size_max);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_global(struct module_object *__restrict module, uint16_t global_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_readonly_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_illegal_instruction(struct code_object *code, void *ip);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_requires_class(DeeTypeObject *__restrict tp_self);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_class_addr(DeeTypeObject *__restrict tp_self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_invalid_instance_addr(DeeTypeObject *tp_self, DeeObject *self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_refs_size(DeeObject *__restrict code, size_t num_refs);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key(DeeObject *map, DeeObject *key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key_str(DeeObject *__restrict map, char const *__restrict key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key_str_len(DeeObject *__restrict map, char const *__restrict key, size_t keylen);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_empty_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_changed_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_item_not_found(DeeObject *seq, DeeObject *item);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_index_not_found(DeeObject *seq, DeeObject *item);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_no_super_class(DeeTypeObject *__restrict type);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_immutable_sequence(DeeObject *__restrict self);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_fixedlength_sequence(DeeObject *__restrict self);

/* Safe-exec error handling. */
struct code_frame;
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_sp(struct code_frame *__restrict frame, size_t access_sp);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_static(struct code_frame *__restrict frame, uint16_t sid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_const(struct code_frame *__restrict frame, uint16_t cid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_locale(struct code_frame *__restrict frame, uint16_t lid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_ref(struct code_frame *__restrict frame, uint16_t rid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_module(struct code_frame *__restrict frame, uint16_t mid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_global(struct code_frame *__restrict frame, uint16_t gid);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_srt_invalid_extern(struct code_frame *__restrict frame, uint16_t mid, uint16_t gid);


/* @param: operator_name: One of `OPERATOR_*' */
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator(DeeTypeObject *__restrict tp, uint16_t operator_name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator2(DeeTypeObject *__restrict tp, uint16_t operator_name, uint16_t operator_name2);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator3(DeeTypeObject *__restrict tp, uint16_t operator_name, uint16_t operator_name2, uint16_t operator_name3);

#define ATTR_ACCESS_GET     0
#define ATTR_ACCESS_DEL     1
#define ATTR_ACCESS_SET     2
#define ATTR_ACCESS_MASK    3
/* @param: access: One of `ATTR_ACCESS_*' */
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_attribute(DeeTypeObject *__restrict tp, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_attribute_len(DeeTypeObject *__restrict tp, char const *__restrict name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_attribute_lookup(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nodoc_attribute(char const *base, char const *__restrict name);
/* @param: access: One of `ATTR_ACCESS_*' */
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_cant_access_attribute(DeeTypeObject *__restrict tp, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_cant_access_attribute_len(DeeTypeObject *__restrict tp, char const *__restrict name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_cant_access_attribute_c(struct class_desc *__restrict desc, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_attribute(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_attribute_c(struct class_desc *__restrict desc, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_string_for_attribute(DeeObject *__restrict but_instead_got);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_class_protected_member(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict member);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr(struct module_object *__restrict self, char const *__restrict name, int access);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global(struct module_object *__restrict self, char const *__restrict name, int access);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_readonly_global(struct module_object *__restrict self, char const *__restrict name);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_read_property(struct module_object *__restrict self, char const *__restrict name);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_delete_property(struct module_object *__restrict self, char const *__restrict name);
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_write_property(struct module_object *__restrict self, char const *__restrict name);

INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found(char const *__restrict filename);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found_ob(DeeObject *__restrict filename);

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H */
