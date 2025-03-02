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
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

struct module_object;
struct code_object;
struct class_attribute;
struct class_desc;

INTDEF ATTR_COLD int DCALL err_no_active_exception(void);
INTDEF ATTR_COLD int DCALL err_subclass_final_type(DeeTypeObject *__restrict tp);
#define err_unimplemented_constructor(tp, argc, argv) \
	err_unimplemented_constructor_kw(tp, argc, argv, NULL)
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_constructor_kw(DeeTypeObject *tp, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_divide_by_zero(DeeObject *a, DeeObject *b);
INTDEF ATTR_COLD int DCALL err_divide_by_zero_i(dssize_t a);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_shift_negative(DeeObject *a, DeeObject *b, bool is_left_shift);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_int_negative(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_int_negative_or_zero(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_int_zero(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_cannot_weak_reference(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_reference_loop(DeeObject *a, DeeObject *b);
INTDEF ATTR_COLD int DCALL err_cannot_lock_weakref(void);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_bytes_not_writable(DeeObject *__restrict bytes_ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_index_out_of_bounds(DeeObject *__restrict self, size_t index, size_t size);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_index_out_of_bounds_ob(DeeObject *self, DeeObject *index);
INTDEF ATTR_COLD NONNULL((1, 2, 3)) int DCALL err_index_out_of_bounds_ob_x(DeeObject *self, DeeObject *index, DeeObject *sizeob);
INTDEF ATTR_COLD int DCALL err_va_index_out_of_bounds(size_t index, size_t size);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_index(DeeObject *__restrict self, size_t index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_index_ob(DeeObject *self, DeeObject *indexob);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_key(DeeObject *self, DeeObject *key);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_key_int(DeeObject *self, size_t key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_key_str(DeeObject *self, char const *key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_key_str_len(DeeObject *self, char const *key, size_t keylen);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_readonly_key(DeeObject *self, DeeObject *key);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_single_character_string(DeeObject *__restrict str);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_integer_overflow(DeeObject *__restrict overflowing_object, size_t cutoff_bits, bool positive_overflow);
INTDEF ATTR_COLD int DCALL err_integer_overflow_i(size_t cutoff_bits, bool positive_overflow);
#define xcheck_empty_keywords(kw) (!(kw) ? 0 : check_empty_keywords(kw))
INTDEF NONNULL((1, 2)) int DFCALL check_empty_keywords(DeeObject *kw, DeeTypeObject *tp_self);
INTDEF NONNULL((1)) int DFCALL check_empty_keywords_obj(DeeObject *__restrict kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_keywords_not_accepted(DeeTypeObject *tp_self, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2, 3)) int DCALL err_keywords_func_not_accepted_string(DeeTypeObject *tp_self, char const *__restrict name, DeeObject *__restrict kw);
INTDEF ATTR_COLD NONNULL((1, 2, 4)) int DCALL err_keywords_func_not_accepted_string_len(DeeTypeObject *tp_self, char const *__restrict name, size_t namelen, DeeObject *__restrict kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_keywords_ctor_not_accepted(DeeTypeObject *tp_self, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classmember_requires_1_argument_string(DeeTypeObject *tp_self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classmember_requires_1_argument_string_len(DeeTypeObject *tp_self, char const *__restrict name, size_t namelen);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classproperty_requires_1_argument_string(DeeTypeObject *tp_self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classproperty_requires_1_argument_string_len(DeeTypeObject *tp_self, char const *__restrict name, size_t namelen);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classmethod_requires_at_least_1_argument_string(DeeTypeObject *tp_self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_classmethod_requires_at_least_1_argument_string_len(DeeTypeObject *tp_self, char const *__restrict name, size_t namelen);
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
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_global(struct module_object *__restrict module, uint16_t global_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_static(struct code_object *code, void *ip, uint16_t static_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_readonly_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_illegal_instruction(struct code_object *code, void *ip);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_requires_class(DeeTypeObject *__restrict tp_self);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_class_addr(DeeTypeObject *__restrict tp_self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_invalid_instance_addr(DeeTypeObject *tp_self, DeeObject *self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_refs_size(DeeObject *__restrict code, size_t num_refs);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key(DeeObject *map, DeeObject *key);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unknown_key_int(DeeObject *__restrict map, size_t key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key_str(DeeObject *__restrict map, char const *__restrict key);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_key_str_len(DeeObject *__restrict map, char const *__restrict key, size_t keylen);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_empty_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_changed_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_item_not_found(DeeObject *seq, DeeObject *item);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_index_not_found(DeeObject *seq, DeeObject *item);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_regex_index_not_found(DeeObject *seq);
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
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator(DeeTypeObject const *__restrict tp, Dee_operator_t operator_name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator2(DeeTypeObject const *__restrict tp, Dee_operator_t operator_name, Dee_operator_t operator_name2);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_operator3(DeeTypeObject const *__restrict tp, Dee_operator_t operator_name, Dee_operator_t operator_name2, Dee_operator_t operator_name3);

#define ATTR_ACCESS_GET     0
#define ATTR_ACCESS_DEL     1
#define ATTR_ACCESS_SET     2
#define ATTR_ACCESS_MASK    3
/* @param: access: One of `ATTR_ACCESS_*' */
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_attribute_string(DeeTypeObject *__restrict tp, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unknown_attribute_string_len(DeeTypeObject *__restrict tp, char const *name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unknown_attribute_lookup_string(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nodoc_attribute_string(char const *base, char const *__restrict name);
/* @param: access: One of `ATTR_ACCESS_*' */
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_cant_access_attribute_string(DeeTypeObject *__restrict tp, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_cant_access_attribute_string_len(DeeTypeObject *__restrict tp, char const *name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_cant_access_attribute_string_c(struct class_desc *__restrict desc, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_attribute_string(DeeTypeObject *__restrict tp, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_attribute_string_c(struct class_desc *__restrict desc, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_string_for_attribute(DeeObject *__restrict but_instead_got);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_class_protected_member(DeeTypeObject *__restrict class_type, struct class_attribute *__restrict member);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr_string(struct module_object *__restrict self, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr_string_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global_string(struct module_object *__restrict self, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global_string_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_readonly_global_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_read_property_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_delete_property_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_write_property_string(struct module_object *__restrict self, char const *__restrict name);

INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found_string(char const *__restrict filename);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found(DeeObject *__restrict filename);


#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define err_no_active_exception()                                                                         Dee_ASSUMED_VALUE(err_no_active_exception(), -1)
#define err_subclass_final_type(tp)                                                                       Dee_ASSUMED_VALUE(err_subclass_final_type(tp), -1)
#define err_unimplemented_constructor_kw(tp, argc, argv, kw)                                              Dee_ASSUMED_VALUE(err_unimplemented_constructor_kw(tp, argc, argv, kw), -1)
#define err_divide_by_zero(a, b)                                                                          Dee_ASSUMED_VALUE(err_divide_by_zero(a, b), -1)
#define err_divide_by_zero_i(a)                                                                           Dee_ASSUMED_VALUE(err_divide_by_zero_i(a), -1)
#define err_shift_negative(a, b, is_left_shift)                                                           Dee_ASSUMED_VALUE(err_shift_negative(a, b, is_left_shift), -1)
#define err_cannot_weak_reference(ob)                                                                     Dee_ASSUMED_VALUE(err_cannot_weak_reference(ob), -1)
#define err_reference_loop(a, b)                                                                          Dee_ASSUMED_VALUE(err_reference_loop(a, b), -1)
#define err_cannot_lock_weakref()                                                                         Dee_ASSUMED_VALUE(err_cannot_lock_weakref(), -1)
#define err_bytes_not_writable(bytes_ob)                                                                  Dee_ASSUMED_VALUE(err_bytes_not_writable(bytes_ob), -1)
#define err_index_out_of_bounds(self, index, size)                                                        Dee_ASSUMED_VALUE(err_index_out_of_bounds(self, index, size), -1)
#define err_index_out_of_bounds_ob(self, index)                                                           Dee_ASSUMED_VALUE(err_index_out_of_bounds_ob(self, index), -1)
#define err_index_out_of_bounds_ob_x(self, index, sizeob)                                                 Dee_ASSUMED_VALUE(err_index_out_of_bounds_ob_x(self, index, sizeob), -1)
#define err_va_index_out_of_bounds(index, size)                                                           Dee_ASSUMED_VALUE(err_va_index_out_of_bounds(index, size), -1)
#define err_unbound_index(self, index)                                                                    Dee_ASSUMED_VALUE(err_unbound_index(self, index), -1)
#define err_unbound_index_ob(self, indexob)                                                               Dee_ASSUMED_VALUE(err_unbound_index_ob(self, indexob), -1)
#define err_unbound_key(self, key)                                                                        Dee_ASSUMED_VALUE(err_unbound_key(self, key), -1)
#define err_unbound_key_int(self, key)                                                                    Dee_ASSUMED_VALUE(err_unbound_key_int(self, key), -1)
#define err_unbound_key_str(self, key)                                                                    Dee_ASSUMED_VALUE(err_unbound_key_str(self, key), -1)
#define err_unbound_key_str_len(self, key, keylen)                                                        Dee_ASSUMED_VALUE(err_unbound_key_str_len(self, key, keylen), -1)
#define err_readonly_key(self, key)                                                                       Dee_ASSUMED_VALUE(err_readonly_key(self, key), -1)
#define err_expected_single_character_string(str)                                                         Dee_ASSUMED_VALUE(err_expected_single_character_string(str), -1)
#define err_integer_overflow(overflowing_object, cutoff_bits, positive_overflow)                          Dee_ASSUMED_VALUE(err_integer_overflow(overflowing_object, cutoff_bits, positive_overflow), -1)
#define err_integer_overflow_i(cutoff_bits, positive_overflow)                                            Dee_ASSUMED_VALUE(err_integer_overflow_i(cutoff_bits, positive_overflow), -1)
#define err_keywords_not_accepted(tp_self, kw)                                                            Dee_ASSUMED_VALUE(err_keywords_not_accepted(tp_self, kw), -1)
#define err_keywords_func_not_accepted_string(tp_self, name, kw)                                          Dee_ASSUMED_VALUE(err_keywords_func_not_accepted_string(tp_self, name, kw), -1)
#define err_keywords_func_not_accepted_string_len(tp_self, name, namelen, kw)                             Dee_ASSUMED_VALUE(err_keywords_func_not_accepted_string_len(tp_self, name, namelen, kw), -1)
#define err_keywords_ctor_not_accepted(tp_self, kw)                                                       Dee_ASSUMED_VALUE(err_keywords_ctor_not_accepted(tp_self, kw), -1)
#define err_classmember_requires_1_argument_string(tp_self, name)                                         Dee_ASSUMED_VALUE(err_classmember_requires_1_argument_string(tp_self, name), -1)
#define err_classmember_requires_1_argument_string_len(tp_self, name, namelen)                            Dee_ASSUMED_VALUE(err_classmember_requires_1_argument_string_len(tp_self, name, namelen), -1)
#define err_classproperty_requires_1_argument_string(tp_self, name)                                       Dee_ASSUMED_VALUE(err_classproperty_requires_1_argument_string(tp_self, name), -1)
#define err_classproperty_requires_1_argument_string_len(tp_self, name, namelen)                          Dee_ASSUMED_VALUE(err_classproperty_requires_1_argument_string_len(tp_self, name, namelen), -1)
#define err_classmethod_requires_at_least_1_argument_string(tp_self, name)                                Dee_ASSUMED_VALUE(err_classmethod_requires_at_least_1_argument_string(tp_self, name), -1)
#define err_classmethod_requires_at_least_1_argument_string_len(tp_self, name, namelen)                   Dee_ASSUMED_VALUE(err_classmethod_requires_at_least_1_argument_string_len(tp_self, name, namelen), -1)
#define err_keywords_bad_for_argc(argc, kwdc)                                                             Dee_ASSUMED_VALUE(err_keywords_bad_for_argc(argc, kwdc), -1)
#define err_keywords_not_found(keyword)                                                                   Dee_ASSUMED_VALUE(err_keywords_not_found(keyword), -1)
#define err_keywords_shadows_positional(keyword)                                                          Dee_ASSUMED_VALUE(err_keywords_shadows_positional(keyword), -1)
#define err_invalid_segment_size(segsz)                                                                   Dee_ASSUMED_VALUE(err_invalid_segment_size(segsz), -1)
#define err_invalid_distribution_count(distcnt)                                                           Dee_ASSUMED_VALUE(err_invalid_distribution_count(distcnt), -1)
#define err_invalid_argc_missing_kw(argument_name, function_name, argc_cur, argc_min, argc_max)           Dee_ASSUMED_VALUE(err_invalid_argc_missing_kw(argument_name, function_name, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc(function_name, argc_cur, argc_min, argc_max)                                     Dee_ASSUMED_VALUE(err_invalid_argc(function_name, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc_len(function_name, function_size, argc_cur, argc_min, argc_max)                  Dee_ASSUMED_VALUE(err_invalid_argc_len(function_name, function_size, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc_va(function_name, argc_cur, argc_min)                                            Dee_ASSUMED_VALUE(err_invalid_argc_va(function_name, argc_cur, argc_min), -1)
#define err_invalid_argc_unpack(unpack_object, argc_cur, argc_min, argc_max)                              Dee_ASSUMED_VALUE(err_invalid_argc_unpack(unpack_object, argc_cur, argc_min, argc_max), -1)
#define err_invalid_unpack_size(unpack_object, need_size, real_size)                                      Dee_ASSUMED_VALUE(err_invalid_unpack_size(unpack_object, need_size, real_size), -1)
#define err_invalid_unpack_size_minmax(unpack_object, need_size_min, need_size_max, real_size)            Dee_ASSUMED_VALUE(err_invalid_unpack_size_minmax(unpack_object, need_size_min, need_size_max, real_size), -1)
#define err_invalid_va_unpack_size(need_size, real_size)                                                  Dee_ASSUMED_VALUE(err_invalid_va_unpack_size(need_size, real_size), -1)
#define err_invalid_unpack_iter_size(unpack_object, unpack_iterator, need_size)                           Dee_ASSUMED_VALUE(err_invalid_unpack_iter_size(unpack_object, unpack_iterator, need_size), -1)
#define err_unbound_global(module, global_index)                                                          Dee_ASSUMED_VALUE(err_unbound_global(module, global_index), -1)
#define err_unbound_local(code, ip, local_index)                                                          Dee_ASSUMED_VALUE(err_unbound_local(code, ip, local_index), -1)
#define err_unbound_static(code, ip, static_index)                                                        Dee_ASSUMED_VALUE(err_unbound_static(code, ip, static_index), -1)
#define err_unbound_arg(code, ip, arg_index)                                                              Dee_ASSUMED_VALUE(err_unbound_arg(code, ip, arg_index), -1)
#define err_readonly_local(code, ip, local_index)                                                         Dee_ASSUMED_VALUE(err_readonly_local(code, ip, local_index), -1)
#define err_illegal_instruction(code, ip)                                                                 Dee_ASSUMED_VALUE(err_illegal_instruction(code, ip), -1)
#define err_requires_class(tp_self)                                                                       Dee_ASSUMED_VALUE(err_requires_class(tp_self), -1)
#define err_invalid_class_addr(tp_self, addr)                                                             Dee_ASSUMED_VALUE(err_invalid_class_addr(tp_self, addr), -1)
#define err_invalid_instance_addr(tp_self, self, addr)                                                    Dee_ASSUMED_VALUE(err_invalid_instance_addr(tp_self, self, addr), -1)
#define err_invalid_refs_size(code, num_refs)                                                             Dee_ASSUMED_VALUE(err_invalid_refs_size(code, num_refs), -1)
#define err_unknown_key(map, key)                                                                         Dee_ASSUMED_VALUE(err_unknown_key(map, key), -1)
#define err_unknown_key_int(map, key)                                                                     Dee_ASSUMED_VALUE(err_unknown_key_int(map, key), -1)
#define err_unknown_key_str(map, key)                                                                     Dee_ASSUMED_VALUE(err_unknown_key_str(map, key), -1)
#define err_unknown_key_str_len(map, key, keylen)                                                         Dee_ASSUMED_VALUE(err_unknown_key_str_len(map, key, keylen), -1)
#define err_empty_sequence(seq)                                                                           Dee_ASSUMED_VALUE(err_empty_sequence(seq), -1)
#define err_changed_sequence(seq)                                                                         Dee_ASSUMED_VALUE(err_changed_sequence(seq), -1)
#define err_item_not_found(seq, item)                                                                     Dee_ASSUMED_VALUE(err_item_not_found(seq, item), -1)
#define err_index_not_found(seq, item)                                                                    Dee_ASSUMED_VALUE(err_index_not_found(seq, item), -1)
#define err_regex_index_not_found(seq)                                                                    Dee_ASSUMED_VALUE(err_regex_index_not_found(seq), -1)
#define err_no_super_class(type)                                                                          Dee_ASSUMED_VALUE(err_no_super_class(type), -1)
#define err_immutable_sequence(self)                                                                      Dee_ASSUMED_VALUE(err_immutable_sequence(self), -1)
#define err_fixedlength_sequence(self)                                                                    Dee_ASSUMED_VALUE(err_fixedlength_sequence(self), -1)
#define err_srt_invalid_sp(frame, access_sp)                                                              Dee_ASSUMED_VALUE(err_srt_invalid_sp(frame, access_sp), -1)
#define err_srt_invalid_static(frame, sid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_static(frame, sid), -1)
#define err_srt_invalid_const(frame, cid)                                                                 Dee_ASSUMED_VALUE(err_srt_invalid_const(frame, cid), -1)
#define err_srt_invalid_locale(frame, lid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_locale(frame, lid), -1)
#define err_srt_invalid_ref(frame, rid)                                                                   Dee_ASSUMED_VALUE(err_srt_invalid_ref(frame, rid), -1)
#define err_srt_invalid_module(frame, mid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_module(frame, mid), -1)
#define err_srt_invalid_global(frame, gid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_global(frame, gid), -1)
#define err_srt_invalid_extern(frame, mid, gid)                                                           Dee_ASSUMED_VALUE(err_srt_invalid_extern(frame, mid, gid), -1)
#define err_unimplemented_operator(tp, operator_name)                                                     Dee_ASSUMED_VALUE(err_unimplemented_operator(tp, operator_name), -1)
#define err_unimplemented_operator2(tp, operator_name, operator_name2)                                    Dee_ASSUMED_VALUE(err_unimplemented_operator2(tp, operator_name, operator_name2), -1)
#define err_unimplemented_operator3(tp, operator_name, operator_name2, operator_name3)                    Dee_ASSUMED_VALUE(err_unimplemented_operator3(tp, operator_name, operator_name2, operator_name3), -1)
#define err_unknown_attribute(tp, name, access)                                                           err_unknown_attribute_string(tp, DeeString_STR(name), access)
#define err_unknown_attribute_string(tp, name, access)                                                    Dee_ASSUMED_VALUE(err_unknown_attribute_string(tp, name, access), -1)
#define err_unknown_attribute_string_len(tp, name, namelen, access)                                       Dee_ASSUMED_VALUE(err_unknown_attribute_string_len(tp, name, namelen, access), -1)
#define err_unknown_attribute_lookup_string(tp, name)                                                     Dee_ASSUMED_VALUE(err_unknown_attribute_lookup_string(tp, name), -1)
#define err_nodoc_attribute_string(base, name)                                                            Dee_ASSUMED_VALUE(err_nodoc_attribute_string(base, name), -1)
#define err_cant_access_attribute(tp, name, access)                                                       err_cant_access_attribute_string(tp, DeeString_STR(name), access)
#define err_cant_access_attribute_string(tp, name, access)                                                Dee_ASSUMED_VALUE(err_cant_access_attribute_string(tp, name, access), -1)
#define err_cant_access_attribute_string_len(tp, name, namelen, access)                                   Dee_ASSUMED_VALUE(err_cant_access_attribute_string_len(tp, name, namelen, access), -1)
#define err_cant_access_attribute_string_c(desc, name, access)                                            Dee_ASSUMED_VALUE(err_cant_access_attribute_string_c(desc, name, access), -1)
#define err_unbound_attribute_string(tp, name)                                                            Dee_ASSUMED_VALUE(err_unbound_attribute_string(tp, name), -1)
#define err_unbound_attribute_string_c(desc, name)                                                        Dee_ASSUMED_VALUE(err_unbound_attribute_string_c(desc, name), -1)
#define err_expected_string_for_attribute(but_instead_got)                                                Dee_ASSUMED_VALUE(err_expected_string_for_attribute(but_instead_got), -1)
#define err_class_protected_member(class_type, member)                                                    Dee_ASSUMED_VALUE(err_class_protected_member(class_type, member), -1)
#define err_module_not_loaded_attr_string(self, name, access)                                             Dee_ASSUMED_VALUE(err_module_not_loaded_attr_string(self, name, access), -1)
#define err_module_not_loaded_attr_string_len(self, name, namelen, access)                                Dee_ASSUMED_VALUE(err_module_not_loaded_attr_string_len(self, name, namelen, access), -1)
#define err_module_no_such_global_string(self, name, access)                                              Dee_ASSUMED_VALUE(err_module_no_such_global_string(self, name, access), -1)
#define err_module_no_such_global_string_len(self, name, namelen, access)                                 Dee_ASSUMED_VALUE(err_module_no_such_global_string_len(self, name, namelen, access), -1)
#define err_module_readonly_global_string(self, name)                                                     Dee_ASSUMED_VALUE(err_module_readonly_global_string(self, name), -1)
#define err_module_cannot_read_property_string(self, name)                                                Dee_ASSUMED_VALUE(err_module_cannot_read_property_string(self, name), -1)
#define err_module_cannot_delete_property_string(self, name)                                              Dee_ASSUMED_VALUE(err_module_cannot_delete_property_string(self, name), -1)
#define err_module_cannot_write_property_string(self, name)                                               Dee_ASSUMED_VALUE(err_module_cannot_write_property_string(self, name), -1)
#define err_file_not_found_string(filename)                                                               Dee_ASSUMED_VALUE(err_file_not_found_string(filename), -1)
#define err_file_not_found(filename)                                                                      Dee_ASSUMED_VALUE(err_file_not_found(filename), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H */
