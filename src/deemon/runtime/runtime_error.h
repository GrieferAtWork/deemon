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
#ifndef GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H
#define GUARD_DEEMON_RUNTIME_RUNTIME_ERROR_H 1

#include <deemon/api.h>

#include <deemon/object.h>

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

struct kwds_object;
struct module_object;
struct code_object;
struct class_attribute;
struct class_desc;
struct function_object;

#define err_unimplemented_constructor(tp, argc, argv) err_unimplemented_constructor_kw(tp, argc, argv, NULL)
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unimplemented_constructor_kw(DeeTypeObject *tp, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_cannot_weak_reference(DeeObject *__restrict ob);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_reference_loop(DeeObject *a, DeeObject *b);
INTDEF ATTR_COLD int DCALL err_cannot_lock_weakref(void);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_bytes_not_writable(DeeObject *__restrict bytes_ob);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_single_character_string(DeeObject *__restrict str);
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
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_keywords_bad_for_argc(struct kwds_object *kwds, size_t argc, DeeObject *const *argv);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_keywords_shadows_positional(DeeObject *__restrict keyword);
INTDEF ATTR_COLD int DCALL err_invalid_segment_size(size_t segsz);
INTDEF ATTR_COLD int DCALL err_invalid_distribution_count(size_t distcnt);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_argc_missing_kw(char const *__restrict argument_name, char const *function_name, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc(char const *function_name, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc_len(char const *function_name, size_t function_size, size_t argc_cur, size_t argc_min, size_t argc_max);
INTDEF ATTR_COLD int DCALL err_invalid_argc_va(char const *function_name, size_t argc_cur, size_t argc_min);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_unbound_global(struct module_object *__restrict module, uint16_t global_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_static(struct code_object *code, void *ip, uint16_t static_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_readonly_local(struct code_object *code, void *ip, uint16_t local_index);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_illegal_instruction(struct code_object *code, void *ip);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_requires_class(DeeTypeObject *__restrict tp_self);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_class_addr(DeeTypeObject *__restrict tp_self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_invalid_instance_addr(DeeTypeObject *tp_self, DeeObject *self, uint16_t addr);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_invalid_refs_size(struct code_object *__restrict code, size_t num_refs);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_changed_sequence(DeeObject *__restrict seq);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_no_super_class(DeeTypeObject *__restrict type);

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

INTDEF ATTR_COLD NONNULL((1)) int DCALL err_expected_string_for_attribute(DeeObject *__restrict but_instead_got);

#define ATTR_ACCESS_GET  0
#define ATTR_ACCESS_DEL  1
#define ATTR_ACCESS_SET  2
#define ATTR_ACCESS_MASK 3

/* @param: access: One of `ATTR_ACCESS_*' */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr_string(struct module_object *__restrict self, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_not_loaded_attr_string_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
/* TODO: Get rid of `err_module_*' -- must use DeeRT_Err* functions! */
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global(struct module_object *self, DeeObject *name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global_string(struct module_object *__restrict self, char const *__restrict name, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_no_such_global_string_len(struct module_object *__restrict self, char const *__restrict name, size_t namelen, int access);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_readonly_global_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_read_property_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_delete_property_string(struct module_object *__restrict self, char const *__restrict name);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_module_cannot_write_property_string(struct module_object *__restrict self, char const *__restrict name);

INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found_string(char const *__restrict filename);
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_file_not_found(DeeObject *__restrict filename);


#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define err_unimplemented_constructor_kw(tp, argc, argv, kw)                                              Dee_ASSUMED_VALUE(err_unimplemented_constructor_kw(tp, argc, argv, kw), -1)
#define err_cannot_weak_reference(ob)                                                                     Dee_ASSUMED_VALUE(err_cannot_weak_reference(ob), -1)
#define err_reference_loop(a, b)                                                                          Dee_ASSUMED_VALUE(err_reference_loop(a, b), -1)
#define err_cannot_lock_weakref()                                                                         Dee_ASSUMED_VALUE(err_cannot_lock_weakref(), -1)
#define err_bytes_not_writable(bytes_ob)                                                                  Dee_ASSUMED_VALUE(err_bytes_not_writable(bytes_ob), -1)
#define err_expected_single_character_string(str)                                                         Dee_ASSUMED_VALUE(err_expected_single_character_string(str), -1)
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
#define err_keywords_bad_for_argc(kwds, argc, argv)                                                       Dee_ASSUMED_VALUE(err_keywords_bad_for_argc(kwds, argc, argv), -1)
#define err_keywords_shadows_positional(keyword)                                                          Dee_ASSUMED_VALUE(err_keywords_shadows_positional(keyword), -1)
#define err_invalid_segment_size(segsz)                                                                   Dee_ASSUMED_VALUE(err_invalid_segment_size(segsz), -1)
#define err_invalid_distribution_count(distcnt)                                                           Dee_ASSUMED_VALUE(err_invalid_distribution_count(distcnt), -1)
#define err_invalid_argc_missing_kw(argument_name, function_name, argc_cur, argc_min, argc_max)           Dee_ASSUMED_VALUE(err_invalid_argc_missing_kw(argument_name, function_name, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc(function_name, argc_cur, argc_min, argc_max)                                     Dee_ASSUMED_VALUE(err_invalid_argc(function_name, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc_len(function_name, function_size, argc_cur, argc_min, argc_max)                  Dee_ASSUMED_VALUE(err_invalid_argc_len(function_name, function_size, argc_cur, argc_min, argc_max), -1)
#define err_invalid_argc_va(function_name, argc_cur, argc_min)                                            Dee_ASSUMED_VALUE(err_invalid_argc_va(function_name, argc_cur, argc_min), -1)
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
#define err_changed_sequence(seq)                                                                         Dee_ASSUMED_VALUE(err_changed_sequence(seq), -1)
#define err_no_super_class(type)                                                                          Dee_ASSUMED_VALUE(err_no_super_class(type), -1)
#define err_srt_invalid_sp(frame, access_sp)                                                              Dee_ASSUMED_VALUE(err_srt_invalid_sp(frame, access_sp), -1)
#define err_srt_invalid_static(frame, sid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_static(frame, sid), -1)
#define err_srt_invalid_const(frame, cid)                                                                 Dee_ASSUMED_VALUE(err_srt_invalid_const(frame, cid), -1)
#define err_srt_invalid_locale(frame, lid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_locale(frame, lid), -1)
#define err_srt_invalid_ref(frame, rid)                                                                   Dee_ASSUMED_VALUE(err_srt_invalid_ref(frame, rid), -1)
#define err_srt_invalid_module(frame, mid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_module(frame, mid), -1)
#define err_srt_invalid_global(frame, gid)                                                                Dee_ASSUMED_VALUE(err_srt_invalid_global(frame, gid), -1)
#define err_srt_invalid_extern(frame, mid, gid)                                                           Dee_ASSUMED_VALUE(err_srt_invalid_extern(frame, mid, gid), -1)
#define err_unimplemented_operator(tp, operator_name)                                                     Dee_ASSUMED_VALUE(err_unimplemented_operator(tp, operator_name), -1)
#define err_expected_string_for_attribute(but_instead_got)                                                Dee_ASSUMED_VALUE(err_expected_string_for_attribute(but_instead_got), -1)
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#define err_module_not_loaded_attr_string(self, name, access)                                             Dee_ASSUMED_VALUE(err_module_not_loaded_attr_string(self, name, access), -1)
#define err_module_not_loaded_attr_string_len(self, name, namelen, access)                                Dee_ASSUMED_VALUE(err_module_not_loaded_attr_string_len(self, name, namelen, access), -1)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#define err_module_no_such_global(self, name, access)                                                     Dee_ASSUMED_VALUE(err_module_no_such_global(self, name, access), -1)
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
