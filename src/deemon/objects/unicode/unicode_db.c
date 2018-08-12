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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C
#define GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C 1

#include <deemon/api.h>
#include <deemon/string.h>

DECL_BEGIN

#ifndef __INTELLISENSE__
#include "unicode_db.h"

#if UNICODE_FOLD_DESCRIPTORS >= 0xff
#error "Too many unicode fold extension characters"
#endif

static struct unitraits const default_traits = { 0x0, 0, 0, 0, 0, 0 };
PUBLIC struct unitraits *DCALL
DeeUni_Descriptor(uint32_t ch) {
 if likely(ch < UNICODE_COUNT_VALID)
    return (struct unitraits *)&UNICODE_DESCRIPTOR(ch);
 return (struct unitraits *)&default_traits;
}
PUBLIC ATTR_PURE size_t
(DCALL DeeUni_ToFolded)(uint32_t ch,
                        uint32_t buf[UNICODE_FOLDED_MAX]) {
 struct unitraits *trt;
 struct unifold *fold;
 trt = DeeUni_Descriptor(ch);
 if (trt->ut_fold == 0xff) {
  buf[0] = DeeUni_ToLower(ch);
  return 1;
 }
 fold = &fold_descriptors[trt->ut_fold];
 buf[0] = fold->uf_repl[0];
 if ((buf[1] = fold->uf_repl[1]) == 0)
      return 1;
 if ((buf[2] = fold->uf_repl[2]) == 0)
      return 2;
 return 3;
}
#endif


#define UNICODE_FPRINT   0x0001
#define UNICODE_FALPHA   0x0002
#define UNICODE_FSPACE   0x0004
#define UNICODE_FLF      0x0008
#define UNICODE_FLOWER   0x0010
#define UNICODE_FUPPER   0x0020
#define UNICODE_FTITLE   0x0040
#define UNICODE_FCNTRL   0x0080
#define UNICODE_FDIGIT   0x0100
#define UNICODE_FDECIMAL 0x0200
#define UNICODE_FSYMSTRT 0x0400
#define UNICODE_FSYMCONT 0x0800

PUBLIC uniflag_t const DeeAscii_Flags[256] = {
/*[[[deemon
import string from deemon;
for (local i: [:256]) {
    local s = string.chr(i);
    local flags = 0;
    // This code right here is the definition of a logic loop:
    //    - This code uses the deemon unicode API to determine character traits
    //    - The deemon unicode API uses `DeeAscii_Flags' to determine those traits
    //    - This code is what generates `DeeAscii_Flags'
    // -> If this list ever becomes corrupt and deemon is re-built, you must find
    //   `#define DeeUni_Flags(ch) ...' in <deemon/string.h>, and temporarily replace
    //    it with `#define DeeUni_Flags(ch) (DeeUni_Descriptor(ch)->ut_flags)',
    //    then re-compile deemon, then run this format-script once again, then
    //    restore the original definition of `DeeUni_Flags', and finally re-compile
    //    deemon once again!
    if (s.isprint())   flags |= UNICODE_FPRINT;
    if (s.isalpha())   flags |= UNICODE_FALPHA;
    if (s.isspace())   flags |= UNICODE_FSPACE;
    if (s.islf())      flags |= UNICODE_FLF;
    if (s.islower())   flags |= UNICODE_FLOWER;
    if (s.isupper())   flags |= UNICODE_FUPPER;
    if (s.istitle())   flags |= UNICODE_FTITLE;
    if (s.iscntrl())   flags |= UNICODE_FCNTRL;
    if (s.isdigit())   flags |= UNICODE_FDIGIT;
    if (s.isdecimal()) flags |= UNICODE_FDECIMAL;
    if (s.issymstrt()) flags |= UNICODE_FSYMSTRT;
    if (s.issymcont()) flags |= UNICODE_FSYMCONT;
    if ((i % 16) == 0) print "    ",;
    print "%#.4x," % flags,;
    if ((i % 16) == 15) print;
}
]]]*/
    0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x00c4,0x00cc,0x00c4,0x00c4,0x00cc,0x0080,0x0080,
    0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x0080,0x00cc,0x00cc,0x00cc,0x00c4,
    0x0045,0x0001,0x0001,0x0001,0x0c01,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
    0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0b01,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
    0x0001,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,
    0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0c63,0x0001,0x0001,0x0001,0x0001,0x0c01,
    0x0001,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,
    0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0c13,0x0001,0x0001,0x0001,0x0001,0x0080,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
//[[[end]]]
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_DB_C */
