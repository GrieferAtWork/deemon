/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_H
#define GUARD_DEEMON_CXX_H 1

#include <deemon/cxx/api.h>

/* To re-generate the dynamically genearted parts of the C++ API:
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/bool.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/bytes.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/callable.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/cell.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/dict.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/file.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/float.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/function.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/hashset.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/int.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/iterator.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/list.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/mapping.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/none.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/numeric.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/object.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/sequence.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/set.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/string.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/tuple.h
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/type.h
 * Or the 1-liner:
 * >> deemon -Wno-user -Wno-usage -F include/deemon/cxx/bool.h include/deemon/cxx/bytes.h include/deemon/cxx/callable.h include/deemon/cxx/cell.h include/deemon/cxx/dict.h include/deemon/cxx/file.h include/deemon/cxx/float.h include/deemon/cxx/function.h include/deemon/cxx/hashset.h include/deemon/cxx/int.h include/deemon/cxx/iterator.h include/deemon/cxx/list.h include/deemon/cxx/mapping.h include/deemon/cxx/none.h include/deemon/cxx/numeric.h include/deemon/cxx/object.h include/deemon/cxx/sequence.h include/deemon/cxx/set.h include/deemon/cxx/string.h include/deemon/cxx/tuple.h include/deemon/cxx/type.h
 */


/*[[[deemon
import opendir from posix;
for (local name: [opendir("cxx").each.d_name...].sorted()) {
	if (name != "api.h")
		print("#include <deemon/cxx/", name, ">");
}
]]]*/
#include <deemon/cxx/bool.h>
#include <deemon/cxx/bytes.h>
#include <deemon/cxx/callable.h>
#include <deemon/cxx/cell.h>
#include <deemon/cxx/dict.h>
#include <deemon/cxx/file.h>
#include <deemon/cxx/float.h>
#include <deemon/cxx/function.h>
#include <deemon/cxx/hashset.h>
#include <deemon/cxx/int.h>
#include <deemon/cxx/iterator.h>
#include <deemon/cxx/list.h>
#include <deemon/cxx/mapping.h>
#include <deemon/cxx/none.h>
#include <deemon/cxx/numeric.h>
#include <deemon/cxx/object.h>
#include <deemon/cxx/sequence.h>
#include <deemon/cxx/set.h>
#include <deemon/cxx/string.h>
#include <deemon/cxx/tuple.h>
#include <deemon/cxx/type.h>
/*[[[end]]]*/

#endif /* !GUARD_DEEMON_CXX_H */
