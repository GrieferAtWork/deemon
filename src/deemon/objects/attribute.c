/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_ATTRIBUTE_C
#define GUARD_DEEMON_OBJECTS_ATTRIBUTE_C 1

#include <deemon/api.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/class.h>
#include <deemon/super.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/arg.h>
#include <deemon/util/string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable: 4611) /* Some nonsensical warning about how setjmp() is evil... */
#endif

DECL_BEGIN

typedef DeeAttributeObject        Attr;
typedef DeeEnumAttrObject         EnumAttr;
typedef DeeEnumAttrIteratorObject EnumAttrIter;


PRIVATE void DCALL
attr_fini(Attr *__restrict self) {
 attribute_info_fini(&self->a_info);
 Dee_Decref(self->a_name);
}
PRIVATE void DCALL
attr_visit(Attr *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->a_info.a_decl);
 Dee_XVisit(self->a_info.a_attrtype);
}
PRIVATE DREF DeeObject *DCALL
attr_str(Attr *__restrict self) {
 return DeeString_Newf("%k.%k",
                       self->a_info.a_decl,
                       self->a_name);
}

PRIVATE dhash_t DCALL
attr_hash(Attr *__restrict self) {
 return (DeeObject_Hash(self->a_info.a_decl) ^
         Dee_HashPointer(self->a_info.a_attrtype) ^ self->a_info.a_perm ^
         DeeString_Hash((DeeObject *)self->a_name));
}
PRIVATE DREF DeeObject *DCALL
attr_eq(Attr *__restrict self, Attr *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeAttribute_Type))
     return NULL;
 if (self->a_info.a_attrtype != other->a_info.a_attrtype)
     return_false;
 return_bool(self->a_name == other->a_name ||
            (DeeString_Hash((DeeObject *)self->a_name) ==
             DeeString_Hash((DeeObject *)other->a_name) &&
             self->a_name->s_len == other->a_name->s_len &&
             memcmp(self->a_name->s_str,other->a_name->s_str,
                    self->a_name->s_len*sizeof(char)) == 0));
}

PRIVATE struct type_cmp attr_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&attr_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&attr_eq
};

PRIVATE struct type_member attr_members[] = {
    TYPE_MEMBER_FIELD_DOC("decl",STRUCT_OBJECT,offsetof(Attr,a_info.a_decl),
                          "The type or object that is declaring this attribute"),
    TYPE_MEMBER_FIELD_DOC("name",STRUCT_OBJECT,offsetof(Attr,a_name),
                          "->string\n"
                          "The name of this attribute"),
    TYPE_MEMBER_FIELD_DOC("attrtype",STRUCT_CONST|STRUCT_OBJECT_OPT,offsetof(Attr,a_info.a_attrtype),
                          "->type\n"
                          "The type of this attribute, or :none if not known"),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeStringObject *DCALL
attr_getdoc(Attr *__restrict self) {
 DREF DeeStringObject *result = self->a_info.a_doc;
 if (!result) result = (DREF DeeStringObject *)Dee_EmptyString;
 return_reference_(result);
}

PRIVATE DREF DeeObject *DCALL
attr_canget(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PERMGET);
}
PRIVATE DREF DeeObject *DCALL
attr_candel(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PERMDEL);
}
PRIVATE DREF DeeObject *DCALL
attr_canset(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PERMSET);
}
PRIVATE DREF DeeObject *DCALL
attr_cancall(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PERMCALL);
}
PRIVATE DREF DeeObject *DCALL
attr_isprivate(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PRIVATE);
}
PRIVATE DREF DeeObject *DCALL
attr_isinstance(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_IMEMBER);
}
PRIVATE DREF DeeObject *DCALL
attr_isproperty(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_PROPERTY);
}
PRIVATE DREF DeeObject *DCALL
attr_iswrapper(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_WRAPPER);
}
PRIVATE DREF DeeObject *DCALL
attr_isclass(Attr *__restrict self) {
 return_bool_(self->a_info.a_perm&ATTR_CMEMBER);
}



PRIVATE char attr_flags[] = {
    /* [FFS(ATTR_PERMGET)-1]  = */'g',
    /* [FFS(ATTR_PERMDEL)-1]  = */'d',
    /* [FFS(ATTR_PERMSET)-1]  = */'s',
    /* [FFS(ATTR_PERMCALL)-1] = */'f',
    /* [FFS(ATTR_IMEMBER)-1]  = */'i',
    /* [FFS(ATTR_CMEMBER)-1]  = */'c',
    /* [FFS(ATTR_PRIVATE)-1]  = */'h',
    /* [FFS(ATTR_PROPERTY)-1] = */'p',
    /* [FFS(ATTR_WRAPPER)-1]  = */'w',
};


PRIVATE DREF DeeObject *DCALL
attr_getflags(Attr *__restrict self) {
 DREF DeeObject *result;
 uint16_t mask = self->a_info.a_perm & 0x1ff;
 unsigned int num_flags = 0; char *dst;
 while (mask) {
  if (mask & 1) ++num_flags;
  mask >>= 1;
 }
 result = DeeString_NewBuffer(num_flags);
 if unlikely(!result) goto done;
 dst = DeeString_STR(result);
 mask = self->a_info.a_perm & 0x1ff;
 num_flags = 0;
 while (mask) {
  if (mask & 1) {
   *dst++ = attr_flags[num_flags];
  }
  mask >>= 1;
  ++num_flags;
 }
 ASSERT(dst == DeeString_END(result));
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
attr_repr(Attr *__restrict self) {
 DREF DeeObject *flags_str,*result;
 flags_str = attr_getflags(self);
 result = DeeString_Newf("attribute(%r,%r,%r,%r,%r)",
                         self->a_info.a_decl,
                         self->a_name,
                         flags_str,
                         flags_str,
                         self->a_info.a_decl);
 Dee_Decref(flags_str);
 return result;
}



PRIVATE struct type_getset attr_getsets[] = {
    { "doc", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_getdoc, NULL, NULL,
      DOC("->string\n"
          "The documentation string of this attribute, or an empty string when no documentation is present") },
    { "canget", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_canget, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute has a way of being read from") },
    { "candel", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_candel, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute has a way of being deleted") },
    { "canset", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_canset, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute has a way of being written to") },
    { "cancall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_cancall, NULL, NULL,
      DOC("->bool\n"
          "Returns :true if the attribute is intended to be called as a function. "
          "Note that this feature alone does not meant that the attribute really can, or "
          "cannot be called, only that calling it as a function might be the inteded use.") },
    { "isprivate", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_isprivate, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute is considered to be private\n"
          "Private attributes only appear in user-classes, prohibiting access to only thiscall "
          "functions with a this-argument that is an instance of the declaring class.") },
    { "isproperty", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_isproperty, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute is property-like, meaning that access by "
          "reading, deletion, or writing causes unpredictable side-effects") },
    { "iswrapper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_iswrapper, NULL, NULL,
      DOC("->bool\n"
          "Check if the attribute is accessed from the implementing type, which "
          "exposes it as a wrapper for an instance member (e.g. ${string.find} is an unbound "
          "wrapper (aka. ${attribute(string,\"find\").iswrapper == true}) for the instance function, "
          "member or property that would be bound in ${\"foo\".find} (aka. "
          "${attribute(\"foo\",\"find\").iswrapper == false}))") },
    { "isinstance", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_isinstance, NULL, NULL,
      DOC("->bool\n"
          "Check if accessing this attribute requires an instance of the declaring object "
          "#decl, rather than being an attribute of the declaring object #decl itself.\n"
          "Note that practically all attributes, such as member functions, are available as both "
          "instance and class attribute, while in other cases an attribute will evaluate to different "
          "objects depending on being invoked on a class or an instance (such as :dict.keys)") },
    { "isclass", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_isclass, NULL, NULL,
      DOC("->bool\n"
          "Check if access to this attribute must be made though the declaring type #decl.\n"
          "To test if an attribute can only be accessed through an instance, use #isinstance instead") },
    { "flags", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_getflags, NULL, NULL,
      DOC("->strings\n"
          "Return a set of characters descripting the flags of @this attribute:\n"
          "%{table Character|Mnemonic|Field|Flag description\n"
          "$\"g\"|get|#canget|The attribute has a way of being read from\n"
          "$\"d\"|del|#candel|The attribute has a way of being deleted\n"
          "$\"s\"|set|#canset|The attribute has a way of being written to\n"
          "$\"f\"|function|#cancall|The attribute is intended to be called as a function\n"
          "$\"i\"|instance|#isinstance|The attribute requires an instance of the declaring object\n"
          "$\"c\"|class|#isclass|The attribute is accessed though the declaring type #decl\n"
          "$\"h\"|hidden|#isprivate|The attribute is considered to be private\n"
          "$\"p\"|property|#isproperty|The attribute is property-like\n"
          "$\"w\"|wrapper|#iswrapper|The attribute is provided by the type as a class member that wraps around an instance member}")
          },
    { NULL }
};


LOCAL int DCALL
string_to_attrflags(char const *__restrict str,
                    uint16_t *__restrict presult) {
 while (*str) {
  char ch = *str++;
  unsigned int i;
  for (i = 0; attr_flags[i] != ch; ++i) {
   if unlikely(i >= COMPILER_LENOF(attr_flags)) {
    DeeError_Throwf(&DeeError_ValueError,
                    "Unknown attribute flag %:1q",
                    str-1);
    goto err;
   }
  }
  *presult |= (uint16_t)1 << i;
 }
 return 0;
err:
 return -1;
}


PRIVATE int DCALL
attribute_init(DeeAttributeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 int lookup_error;
 DeeObject *search_self;
 struct attribute_lookup_rules rules;
 rules.alr_decl       = NULL;
 rules.alr_perm_mask  = 0;
 rules.alr_perm_value = 0;
 if (argc < 2) {
invalid_argc:
  err_invalid_argc("attribute",argc,2,5);
  goto err;
 }
 if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
     goto err;
 search_self    = argv[0];
 rules.alr_name = DeeString_STR(argv[1]);
 rules.alr_hash = DeeString_Hash(argv[1]);
 switch (argc) {
 case 2:
  break;
 case 3:
  if (DeeString_Check(argv[2])) {
   if unlikely(string_to_attrflags(DeeString_STR(argv[2]),&rules.alr_perm_mask))
      goto err;
  } else {
   if (DeeObject_AsUInt16(argv[2],&rules.alr_perm_mask))
       goto err;
  }
  rules.alr_perm_value = rules.alr_perm_mask;
  break;
 case 4:
 case 5:
  if (DeeString_Check(argv[2])) {
   if unlikely(string_to_attrflags(DeeString_STR(argv[2]),&rules.alr_perm_mask))
      goto err;
  } else {
   if (DeeObject_AsUInt16(argv[2],&rules.alr_perm_mask))
       goto err;
  }
  if (DeeString_Check(argv[3])) {
   if unlikely(string_to_attrflags(DeeString_STR(argv[3]),&rules.alr_perm_value))
      goto err;
  } else {
   if (DeeObject_AsUInt16(argv[3],&rules.alr_perm_value))
       goto err;
  }
  if (argc == 5)
      rules.alr_decl = argv[4];
  break;
 default:
  goto invalid_argc;
 }
 lookup_error = DeeAttribute_Lookup(Dee_TYPE(search_self),
                                    search_self,
                                   &self->a_info,
                                   &rules);
 if (lookup_error > 0) {
  /* Attribute wasn't found... */
  DeeError_Throwf(&DeeError_AttributeError,
                  "Unknown attribute `%k.%s'",
                   Dee_TYPE(search_self),
                   rules.alr_name);
  goto err;
 }
 if likely(!lookup_error) {
  self->a_name = (DREF struct string_object *)argv[1];
  Dee_Incref(argv[1]);
 }
 return lookup_error;
err:
 return -1;
}



PUBLIC DeeTypeObject DeeAttribute_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_attribute),
    /* .tp_doc      = */DOC("The descriptor object for abstract object attributes\n"
                            "\n"
                            "(ob,string name,int flagmask=0,int flagval=flagmask,decl?)\n"
                            "(ob,string name,string flagmask=\"\",int flagval=flagmask,decl?)\n"
                            "(ob,string name,int flagmask=0,string flagval=flagmask,decl?)\n"
                            "(ob,string name,string flagmask=\"\",string flagval=flagmask,decl?)\n"
                            "@param flagmask Set of attribute flags to mask when searching for matches (s.a. #flags)\n"
                            "@param flagval Set of attribute flags required when searching for matches (s.a. #flags)\n"
                            "@throw AttributeError No attribute matching the specified restrictions could be found\n"
                            "@throw ValueError The given @flagmask or @flagval contains an unrecognized flag character\n"
                            "Lookup an attribute enumerated by ${enumattr(ob)} or ${enumattr(tp)}, matching "
                            "the given @name, as well as having its set of flags match @flagval, when masked by @flagmask\n"
                            "Additionally, @decl may be specified to narrow down valid matches to only those declared by it\n"
                            ">function findattr(ob,name,flagmask,flagval,decl?) {\n"
                            "> import enumattr, Error, hashset from deemon;\n"
                            "> flagmask = hashset(flagmask);\n"
                            "> for (local attr: enumattr(ob)) {\n"
                            ">  if (attr.name != name)\n"
                            ">   continue;\n"
                            ">  if (decl is bound && attr.decl !== decl)\n"
                            ">   continue;\n"
                            ">  if ((flagmask & attr.flags) != flagval)\n"
                            ">   continue;\n"
                            ">  return attr;\n"
                            "> }\n"
                            "> throw Error.AttributeError(...);\n"
                            ">}\n"
                            "Using @flagmask and @flagval, you can easily restrict a search to only class-, or instance-attributes:\n"
                            ">import attribute, dict from deemon;\n"
                            ">// The class-variant (attribute cannot be accessed from an instance)\n"
                            ">print repr attribute(dict,\"keys\",\"i\",\"\");\n"
                            ">// The class-variant (attribute is a wrapper)\n"
                            ">print repr attribute(dict,\"keys\",\"w\");\n"
                            ">// The instance-variant (attribute can be accessed from an instance)\n"
                            ">print repr attribute(dict,\"keys\",\"i\");\n"
                            ">// The instance-variant (attribute isn't a wrapper)\n"
                            ">print repr attribute(dict,\"keys\",\"w\",\"\");\n"
                            ),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&attribute_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Attr)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&attr_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&attr_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&attr_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&attr_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */attr_getsets,
    /* .tp_members       = */attr_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



#ifndef CONFIG_LONGJMP_ENUMATTR
struct attr_list {
 size_t      al_c;
 size_t      al_a;
 DREF Attr **al_v;
};

PRIVATE dssize_t DCALL
save_attr(DeeObject *__restrict declarator,
          char const *__restrict attr_name, char const *attr_doc,
          uint16_t perm, DeeTypeObject *attr_type,
          struct attr_list *__restrict self) {
 DREF Attr *new_attr;
 /* Make sure that the collection vector has sufficient space. */
 if (self->al_c == self->al_a) {
  DREF Attr **new_vector;
  size_t new_alloc = self->al_a*2;
  if (!new_alloc) new_alloc = 8;
do_realloc:
  new_vector = (DREF Attr **)Dee_TryRealloc(self->al_v,new_alloc*
                                            sizeof(DREF Attr *));
  if unlikely(!new_vector) {
   if (new_alloc != self->al_c+1) { new_alloc = self->al_c+1; goto do_realloc; }
   if (Dee_CollectMemory(new_alloc*sizeof(DREF Attr *))) goto do_realloc;
   goto err;
  }
  self->al_v = new_vector;
  self->al_a = new_alloc;
 }
 /* Allocate a new attribute descriptor. */
 new_attr = DeeObject_MALLOC(Attr);
 if unlikely(!new_attr) goto err;
 if (perm&ATTR_NAMEOBJ) {
  new_attr->a_name = COMPILER_CONTAINER_OF(attr_name,DeeStringObject,s_str);
  Dee_Incref(new_attr->a_name);
 } else {
  new_attr->a_name = (DREF DeeStringObject *)DeeString_New(attr_name);
  if unlikely(!new_attr->a_name) goto err_attr;
 }
 if (!attr_doc) new_attr->a_info.a_doc = NULL;
 else if (perm&ATTR_DOCOBJ) {
  new_attr->a_info.a_doc = COMPILER_CONTAINER_OF(attr_doc,DeeStringObject,s_str);
  Dee_Incref(new_attr->a_info.a_doc);
 } else {
  new_attr->a_info.a_doc = (DREF DeeStringObject *)DeeString_NewUtf8(attr_doc,
                                                                     strlen(attr_doc),
                                                                     STRING_ERROR_FIGNORE);
  if unlikely(!new_attr->a_info.a_doc) goto err_name;
 }
 new_attr->a_info.a_decl     = declarator;
 new_attr->a_info.a_perm     = perm;
 new_attr->a_info.a_attrtype = attr_type;
 Dee_Incref(declarator);
 Dee_XIncref(attr_type);
 DeeObject_Init(new_attr,&DeeAttribute_Type);
 self->al_v[self->al_c++] = new_attr; /* Inherit reference. */
 return 0;
err_name: Dee_Decref(new_attr->a_name);
err_attr: DeeObject_Free(new_attr);
err:      return -1;
}
#endif


PRIVATE int DCALL
enumattr_init(EnumAttr *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *a,*b = NULL;
 if (DeeArg_Unpack(argc,argv,"o|o:enumattr",&a,&b))
     return -1;
 if (b) {
  if (DeeObject_AssertType(a,&DeeType_Type) ||
      DeeObject_AssertType(b,(DeeTypeObject *)a))
      return -1;
  self->ea_type = (DREF DeeTypeObject *)a;
  self->ea_obj  = b;
  Dee_Incref(a);
  Dee_Incref(b);
 } else {
  self->ea_type = Dee_TYPE(a);
  self->ea_obj  = a;
  Dee_Incref(Dee_TYPE(a));
  Dee_Incref(a);
 }
#ifndef CONFIG_LONGJMP_ENUMATTR
 /* Collect all attributes */
 {
  struct attr_list list;
  list.al_a = list.al_c = 0;
  list.al_v = NULL;
  /* Enumerate all the attributes. */
  if (DeeObject_EnumAttr(self->ea_type,self->ea_obj,(denum_t)&save_attr,&list) < 0) {
   while (list.al_c--) Dee_Decref(list.al_v[list.al_c]);
   Dee_Free(list.al_v);
   Dee_Decref(self->ea_type);
   Dee_XDecref(self->ea_obj);
   return -1;
  }
  /* Truncate the collection vector. */
  if (list.al_c != list.al_a) {
   DREF Attr **new_vector;
   new_vector = (DREF Attr **)Dee_TryRealloc(list.al_v,list.al_c*
                                             sizeof(DREF Attr *));
   if likely(new_vector) list.al_v = new_vector;
  }
  /* Assign the attribute vector. */
  self->ea_attrc = list.al_c;
  self->ea_attrv = list.al_v; /* Inherit. */
 }
#endif
 return 0;
}
PRIVATE void DCALL
enumattr_fini(EnumAttr *__restrict self) {
 Dee_Decref(self->ea_type);
 Dee_XDecref(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
 {
  size_t i;
  for (i = 0; i < self->ea_attrc; ++i)
       Dee_Decref(self->ea_attrv[i]);
 }
 Dee_Free(self->ea_attrv);
#endif
}
PRIVATE void DCALL
enumattr_visit(EnumAttr *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->ea_type);
 Dee_XVisit(self->ea_obj);
#ifndef CONFIG_LONGJMP_ENUMATTR
 {
  size_t i;
  for (i = 0; i < self->ea_attrc; ++i)
       Dee_Visit(self->ea_attrv[i]);
 }
#endif
}

PRIVATE void DCALL
enumattriter_init(EnumAttrIter *__restrict self,
                  EnumAttr *__restrict seq) {
 self->ei_seq = seq;
 Dee_Incref(seq);
#ifdef CONFIG_LONGJMP_ENUMATTR
 rwlock_init(&self->ei_lock);
 /* Set the buffer pointer to its initial state. */
 self->ei_bufpos = NULL;
#else /* CONFIG_LONGJMP_ENUMATTR */
 self->ei_iter = seq->ea_attrv;
 self->ei_end  = seq->ea_attrv+seq->ea_attrc;
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}

PRIVATE DREF EnumAttrIter *DCALL
enumattr_iter(EnumAttr *__restrict self) {
 DREF EnumAttrIter *result;
 result = DeeObject_MALLOC(EnumAttrIter);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeEnumAttrIterator_Type);
 enumattriter_init(result,self);
done:
 return result;
}

PRIVATE dhash_t DCALL
enumattr_hash(EnumAttr *__restrict self) {
 return ((self->ea_obj ? DeeObject_Hash(self->ea_obj) : 0) ^
          Dee_HashPointer(self->ea_type));
}

PRIVATE struct type_cmp enumattr_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&enumattr_hash,
    /* .tp_eq   = */NULL, /* TODO */
    /* .tp_ne   = */NULL  /* TODO */
};

PRIVATE struct type_seq enumattr_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattr_iter
};

PRIVATE struct type_member enumattr_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeEnumAttrIterator_Type),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeEnumAttr_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_enumattr),
    /* .tp_doc      = */DOC("(type tp)\n"
                            "Enumerate attributes of the :type @tp and its bases\n"
                            "\n"
                            "(ob)\n"
                            "Same as ${enumattr(type(ob),ob)}\n"
                            "\n"
                            "(type tp,ob)\n"
                            "Create a new sequence for enumerating the :{attribute}s of a given object.\n"
                            "When @tp is given, only enumerate objects implemented by @tp or "
                            "one of its bases and those accessible through a superview of @ob using @tp.\n"
                            "Note that iterating this object may be expensive, and that conversion to "
                            "a different sequence before iterating multiple times may be desireable"),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&enumattr_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(EnumAttr)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&enumattr_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&enumattr_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&enumattr_cmp,
    /* .tp_seq           = */&enumattr_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */enumattr_class_members
};

#ifdef CONFIG_LONGJMP_ENUMATTR
/* Signal values with which `ei_continue' can be invoked. */
#define CNTSIG_CONTINUE  1 /* Continue execution. */
#define CNTSIG_STOP      2 /* Tear down execution. */
#define BRKSIG_YIELD     1 /* Yield more items. */
#define BRKSIG_ERROR     2 /* An error occurred. */
#define BRKSIG_STOP      3 /* Stop yielding items. */
#define BRKSIG_COLLECT   4 /* Collect memory and try again. */
#endif


PRIVATE int DCALL
enumattriter_ctor(EnumAttrIter *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 EnumAttr *seq;
 if (DeeArg_Unpack(argc,argv,"o:enumattr.iterator",&seq) ||
     DeeObject_AssertType((DeeObject *)seq,&DeeEnumAttr_Type))
     return -1;
 enumattriter_init(self,seq);
 return 0;
}
PRIVATE void DCALL
enumattriter_fini(EnumAttrIter *__restrict self) {
#ifdef CONFIG_LONGJMP_ENUMATTR
 DREF Attr **iter = self->ei_bufpos;
 if (iter && iter != (DREF Attr **)ITER_DONE) {
  int error;
  ASSERT(iter >= self->ei_buffer);
  ASSERT(iter <= COMPILER_ENDOF(self->ei_buffer));
  /* Discard all remaining items. */
  for (; iter != COMPILER_ENDOF(self->ei_buffer); ++iter)
      Dee_Decref(*iter);
  self->ei_bufpos = (DREF Attr **)ITER_DONE; /* Indicate that we want to stop iteration. */
  /* Resolve execution of the iterator normally. */
  if ((error = setjmp(self->ei_break)) == 0) {
   longjmp(self->ei_continue,CNTSIG_STOP);
  }
  if (error == BRKSIG_ERROR)
      DeeError_Handled(ERROR_HANDLED_RESTORE);
 }
#endif /* CONFIG_LONGJMP_ENUMATTR */
 Dee_Decref(self->ei_seq);
}
PRIVATE void DCALL
enumattriter_visit(EnumAttrIter *__restrict self, dvisit_t proc, void *arg) {
#ifdef CONFIG_LONGJMP_ENUMATTR
 DREF Attr **iter = self->ei_bufpos;
 if (iter && iter != (DREF Attr **)ITER_DONE) {
  ASSERT(iter >= self->ei_buffer);
  ASSERT(iter <= COMPILER_ENDOF(self->ei_buffer));
  for (; iter != COMPILER_ENDOF(self->ei_buffer); ++iter)
      Dee_Visit(*iter);
 }
#endif /* CONFIG_LONGJMP_ENUMATTR */
 Dee_Visit(self->ei_seq);
}

#ifdef CONFIG_LONGJMP_ENUMATTR

/* Same as the non-try version, but will not invoke memory collection. */
PRIVATE DREF DeeStringObject *DCALL
DeeString_TryNew(char const *__restrict str) {
 DREF DeeStringObject *result; size_t len = strlen(str);
 result = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject,s_str)+
                                                     (len+1)*sizeof(char));
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeString_Type);
 result->s_data = NULL;
 result->s_hash = (dhash_t)-1;
 result->s_len  = len;
 memcpy(result->s_str,str,(len+1)*sizeof(char));
 return result;
}

PRIVATE dssize_t DCALL
enumattr_longjmp(DeeObject *__restrict declarator,
                 char const *__restrict attr_name, char const *attr_doc,
                 uint16_t perm, DeeTypeObject *attr_type,
                 EnumAttrIter *__restrict iterator) {
 int error; DREF Attr *new_attribute;
 ASSERT(iterator->ei_bufpos != COMPILER_ENDOF(iterator->ei_buffer));
again:
 /* Create a new descriptor for the information passed. */
 new_attribute = DeeObject_TRYMALLOC(Attr);
 if unlikely(!new_attribute) goto err_collect; /* Error. */
 new_attribute->a_info.a_decl     = declarator;
 new_attribute->a_info.a_perm     = perm;
 new_attribute->a_info.a_attrtype = attr_type;
 if (perm&ATTR_NAMEOBJ) {
  new_attribute->a_name = COMPILER_CONTAINER_OF(attr_name,DeeStringObject,s_str);
  Dee_Incref(new_attribute->a_name);
 } else {
  new_attribute->a_name = DeeString_TryNew(attr_name);
  if unlikely(!new_attribute->a_name) goto err_collect_2;
 }
 if (!attr_doc) new_attribute->a_info.a_doc = NULL;
 else if (perm&ATTR_DOCOBJ) {
  new_attribute->a_info.a_doc = COMPILER_CONTAINER_OF(attr_doc,DeeStringObject,s_str);
  Dee_Incref(new_attribute->a_info.a_doc);
 } else {
  new_attribute->a_info.a_doc = DeeString_TryNew(attr_doc);
  if unlikely(!new_attribute->a_info.a_doc) goto err_collect_3;
 }
 Dee_XIncref(declarator);
 Dee_XIncref(attr_type);
 DeeObject_Init(new_attribute,&DeeAttribute_Type);
 /* Done! Now save the attribute in the collection buffer. */
 *iterator->ei_bufpos++ = new_attribute;
 /* If the buffer is not full, jump over to the
  * caller and let them yield what we've collected. */
 if (iterator->ei_bufpos == COMPILER_ENDOF(iterator->ei_buffer)) {
  iterator->ei_bufpos = iterator->ei_buffer;
  if ((error = setjmp(iterator->ei_continue)) == 0)
       longjmp(iterator->ei_break,BRKSIG_YIELD);
  /* Stop iteration if the other end requested this. */
  if (error == CNTSIG_STOP) return -2;
 }
 return 0;
err_collect_3:
 Dee_Decref(new_attribute->a_name);
err_collect_2:
 DeeObject_Free(new_attribute);
err_collect:
 /* Let the other end collect memory for us.
  * With how small our stack is, we'd probably not be able to
  * safely execute user-code that may be invoked from gc-callbacks
  * associated with the memory collection sub-system. */
 if ((error = setjmp(iterator->ei_continue)) == 0)
      longjmp(iterator->ei_break,BRKSIG_COLLECT);
 /* Stop iteration if the other end requested this. */
 if (error == CNTSIG_STOP) return -2;
 goto again;
}


#if defined(CONFIG_HOST_WINDOWS) && defined(__x86_64__)
ATTR_MSABI
#endif
PRIVATE ATTR_NOINLINE ATTR_NORETURN ATTR_USED void
enumattr_start(EnumAttrIter *__restrict self) {
 dssize_t enum_error;
 /* This is where execution on the fake stack starts. */
 self->ei_bufpos = self->ei_buffer;
 enum_error = DeeObject_EnumAttr(self->ei_seq->ea_type,
                                 self->ei_seq->ea_obj,
                                (denum_t)&enumattr_longjmp,self);
 /* -1 indicates an internal error, rather than stop-enumeration (with is -2). */
 if unlikely(enum_error == -1) {
  /* Discard all unyielded attributes and enter an error state. */
  while (self->ei_bufpos != self->ei_buffer) {
   --self->ei_bufpos;
   Dee_Decref(*self->ei_bufpos);
  }
  self->ei_bufpos = (DREF Attr **)ITER_DONE;
  longjmp(self->ei_break,BRKSIG_ERROR);
  __builtin_unreachable();
 }
 /* Notify of the last remaining attributes (if there are any). */
 if (self->ei_bufpos != (DREF Attr **)ITER_DONE &&
     self->ei_bufpos != self->ei_buffer) {
  size_t count = (size_t)(self->ei_bufpos-self->ei_buffer);
  Attr **new_pos = COMPILER_ENDOF(self->ei_buffer)-count;
  ASSERT(count < CONFIG_LONGJMP_ENUMATTR_CLUSTER);
  MEMMOVE_PTR(new_pos,self->ei_buffer,count);
  self->ei_bufpos = new_pos;
  if (setjmp(self->ei_continue) == 0)
      longjmp(self->ei_break,BRKSIG_YIELD);
  ASSERT(self->ei_bufpos == self->ei_buffer);
 }
 /* Mark the buffer as exhausted. */
 self->ei_bufpos = (DREF Attr **)ITER_DONE;
 longjmp(self->ei_break,BRKSIG_STOP);
 __builtin_unreachable();
}
#endif

PRIVATE DREF Attr *DCALL
enumattriter_next(EnumAttrIter *__restrict self) {
#ifdef CONFIG_LONGJMP_ENUMATTR
 DREF Attr *result; int error;
 /* Quick check: is the iterator exhausted. */
again_locked:
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->ei_lock);
#endif
again:
 /* Check for case: Iterator exhausted. */
 if (self->ei_bufpos == (DREF Attr **)ITER_DONE)
     goto iter_done;
 if (self->ei_bufpos != COMPILER_ENDOF(self->ei_buffer)) {
  if (!self->ei_bufpos) {
   /* Special case: initial call. */
   uintptr_t *new_sp = COMPILER_ENDOF(self->ei_stack);
#ifndef __x86_64__
   *--new_sp = (uintptr_t)self; /* First argument to `enumattr_start' */
#endif
   if (setjmp(self->ei_break) == 0) {
    /* Set the  */
#ifdef __COMPILER_HAVE_GCC_ASM
    __asm__ __volatile__(
#ifdef __x86_64__
                         "movq %0, %%rsp\n\t"
#ifdef CONFIG_HOST_WINDOWS
                         "subq $32, %%rsp\n\t" /* 32: Register scratch area... */
#endif
#else
                         "movl %0, %%esp\n\t"
#endif
                         "call "
#ifdef __USER_LABEL_PREFIX__
                         PP_STR(__USER_LABEL_PREFIX__)
#endif
                         "enumattr_start\n\t"
                         : : "g" (new_sp)
#ifdef __x86_64__
#ifdef CONFIG_HOST_WINDOWS
                         , "c" (self)
#else
                         , "D" (self)
#endif
#endif
                         );
#else /* __COMPILER_HAVE_GCC_ASM */
    __asm {
#ifdef __x86_64__
#ifdef CONFIG_HOST_WINDOWS
        MOV  RCX, self
#else
        MOV  RDI, self
#endif
#ifdef CONFIG_HOST_WINDOWS
        LEA  RSP, [new_sp - 32]  /* 32: Register scratch area... */
#else
        MOV  RSP, new_sp
#endif
#else
        MOV  ESP, new_sp
#endif
        CALL enumattr_start
    }
#endif /* !__COMPILER_HAVE_GCC_ASM */
    __builtin_unreachable();
   }
   goto again;
  }
  ASSERT(self->ei_bufpos >= self->ei_buffer);
  ASSERT(self->ei_bufpos <  COMPILER_ENDOF(self->ei_buffer));
  /* Take away one of the generated attributes. */
  result = *self->ei_bufpos++;
  goto done;
 }
 /* Continue execution of the iterator. */
 self->ei_bufpos = self->ei_buffer;
 if ((error = setjmp(self->ei_break)) == 0)
      longjmp(self->ei_continue,CNTSIG_CONTINUE);
 /* Handle signal return signals from the enumeration sub-routine. */
 if (error == BRKSIG_COLLECT) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->ei_lock);
#endif
  if (Dee_CollectMemory(sizeof(Attr)))
      goto again_locked;
  return NULL;
 }
 if (error == BRKSIG_STOP) goto iter_done;
 if (error == BRKSIG_ERROR) { result = NULL; goto done; }
 /* Jump back and handle the attributes that got enumerated. */
 goto again;
iter_done:
 result = (Attr *)ITER_DONE;
done:
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->ei_lock);
#endif
 return result;
#else /* CONFIG_LONGJMP_ENUMATTR */
 DREF Attr **presult;
 do {
  presult = ATOMIC_READ(self->ei_iter);
  if (presult == self->ei_end)
      return (DREF Attr *)ITER_DONE;
 } while (!ATOMIC_CMPXCH(self->ei_iter,presult,presult+1));
 return_reference_(*presult);
#endif /* !CONFIG_LONGJMP_ENUMATTR */
}


PRIVATE struct type_member enumattriter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(EnumAttrIter,ei_seq)),
    TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeEnumAttrIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"enumattr.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&enumattriter_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(EnumAttrIter)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&enumattriter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&enumattriter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&enumattriter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */enumattriter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};




struct attribute_lookup_data {
    struct attribute_info               *ald_info;    /* [1..1] The result info. */
    struct attribute_lookup_rules const *ald_rules;   /* [1..1] Lookup rules */
    bool                                 ald_fnddecl; /* [valid_if(ald_rules->alr_decl != NULL)]
                                                       * Set to true after `alr_decl' had been encountered. */
};

PRIVATE dssize_t DCALL
attribute_lookup_enum(DeeObject *__restrict declarator,
                      char const *__restrict attr_name, char const *attr_doc,
                      uint16_t perm, DeeTypeObject *attr_type,
                      struct attribute_lookup_data *__restrict arg) {
 dhash_t attr_hash;
 struct attribute_info *result;
 struct attribute_lookup_rules const *rules = arg->ald_rules;
 if (rules->alr_decl) {
  if (declarator != rules->alr_decl) {
   if (arg->ald_fnddecl)
       return -3; /* The requested declarator came and went without a match... */
   return 0;
  }
  arg->ald_fnddecl = true;
 }
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
      return 0;
 if (perm & ATTR_NAMEOBJ)
  attr_hash = DeeString_Hash((DeeObject *)COMPILER_CONTAINER_OF(attr_name,DeeStringObject,s_str));
 else {
  attr_hash = hash_str(attr_name);
 }
 if (attr_hash != rules->alr_hash)
     return 0;
 if (strcmp(attr_name,arg->ald_rules->alr_name) != 0)
     return 0;
 /* This is the one! */
 result = arg->ald_info;
 if (!attr_doc)
  result->a_doc = NULL;
 else if (perm & ATTR_DOCOBJ) {
  result->a_doc = COMPILER_CONTAINER_OF(attr_doc,DeeStringObject,s_str);
  Dee_Incref(result->a_doc);
 } else {
  result->a_doc = (DREF struct string_object *)DeeString_NewUtf8(attr_doc,
                                                                 strlen(attr_doc),
                                                                 STRING_ERROR_FIGNORE);
  if unlikely(!result->a_doc) goto err;
 }
 result->a_decl = declarator;
 Dee_Incref(declarator);
 result->a_perm = perm;
 result->a_attrtype = attr_type;
 Dee_XIncref(attr_type);
 return -2; /* Stop enumeration! */
err:
 return -1;
}


INTDEF dssize_t DCALL
type_enumattr(DeeTypeObject *__restrict UNUSED(tp_self),
              DeeObject *__restrict self, denum_t proc, void *arg);

PUBLIC int DCALL
DeeAttribute_Lookup(DeeTypeObject *__restrict tp_self,
                    DeeObject *__restrict self,
                    struct attribute_info *__restrict result,
                    struct attribute_lookup_rules const *__restrict rules) {
#if 1
 int error; struct membercache *cache;
 DeeTypeObject *iter = tp_self;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT_OBJECT_TYPE(self,tp_self);
 ASSERT(result);
 ASSERT(rules);
 ASSERT(rules->alr_name);
 ASSERT_OBJECT_OPT(rules->alr_decl);
 if (tp_self->tp_attr) goto do_iter_attr;
 cache = &tp_self->tp_cache;
 /* Search through the cache for the requested attribute. */
 if ((error = membercache_find(cache,self,ATTR_IMEMBER|ATTR_CMEMBER,result,rules)) <= 0)
      goto done;
 for (;;) {
  if (rules->alr_decl && iter != (DeeTypeObject *)rules->alr_decl)
      goto next_iter;
  if (DeeType_IsClass(iter)) {
   struct class_desc *desc = DeeClass_DESC(iter);
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
   if ((error = DeeClass_FindInstanceAttribute(iter,self,result,rules)) <= 0)
        goto done;
#else
   if ((error = membertable_find(iter,self,desc->c_mem,result,rules)) <= 0)
        goto done;
#endif
  } else {
   if (iter->tp_methods &&
      (error = type_method_find(cache,(DeeObject *)iter,iter->tp_methods,
                                ATTR_IMEMBER|ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
   if (iter->tp_getsets &&
      (error = type_getset_find(cache,(DeeObject *)iter,iter->tp_getsets,
                                ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY,
                                result,rules)) <= 0)
       goto done;
   if (iter->tp_members &&
      (error = type_member_find(cache,(DeeObject *)iter,iter->tp_members,
                                ATTR_IMEMBER|ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
  }
next_iter:
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
   dssize_t enum_error;
   struct attribute_lookup_data data;
   dssize_t (DCALL *enumattr)(DeeTypeObject *__restrict,DeeObject *__restrict,denum_t,void *);
do_iter_attr:
   enumattr = iter->tp_attr->tp_enumattr;
   if (!enumattr) break;
   if (enumattr == &type_enumattr)
       return DeeType_FindAttrString((DeeTypeObject *)self,result,rules);
   /* TODO: Also add special case for modules. */
   data.ald_info    = result;
   data.ald_rules   = rules;
   data.ald_fnddecl = false;
   enum_error = (*enumattr)(iter,self,(denum_t)&attribute_lookup_enum,&data);
   if (enum_error == 0 || enum_error == -3)/* Not found */
       break; /* Don't consider attributes from lower levels for custom member access. */
   if (enum_error == -1) return -1; /* Error... */
   ASSERT(enum_error == -2); /* Found it! */
   return 0;
  }
 }
 return 1; /* Not found */
done:
 return error;
#else
 dssize_t error;
 struct attribute_lookup_data data;
 data.ald_info    = result;
 data.ald_rules   = rules;
 data.ald_fnddecl = false;
 error = DeeObject_EnumAttr(tp_self,self,(denum_t)&attribute_lookup_enum,&data);
 if (error == 0 || error == -3) return 1;   /* Not found */
 if (error == -1) return -1; /* Error... */
 ASSERT(error == -2); /* Found it! */
 return 0;
#endif
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ATTRIBUTE_C */
