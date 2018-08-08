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
#ifndef GUARD_DEX_IPC_WINDOWS_CMDLINE_C_INL
#define GUARD_DEX_IPC_WINDOWS_CMDLINE_C_INL 1
#define _KOS_SOURCE 1

#include "libipc.h"

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/list.h>

#include <string.h>

DECL_BEGIN

/* Encode the process commandline in a way that is compatible
 * with the decoder found in any program compiled using VC/VC++
 * Additionally, try to escape as few characters as possible, and
 * use quotation marks as sparingly as possible.
 * RULES:
 *   - Only use `"' for escaping (`'' seems to be missing/broken)
 *   - When argv[i] contains any ` ' or `\t' characters,
 *     the argument is to be escaped by surrounding it with `"'
 *      - `foo bar'  --> `"foo bar"'
 *   - When argv[i] contains a `"' character, that character
 *     is prefixed with a `\' character
 *      - `foo"'  --> `foo\"'
 *   - Any number of `\' that are followed by a `"' is replaced
 *     with double that number of `\' characters, 
 *      - `foo\'  --> `foo\'
 *      - `foo\"' --> `foo\\\"'
 * HINT: The algorithm used by VC/VC++ is located
 *       in `crt/src/stdargv.c' of Visual Studio
 */
INTERN int DCALL
cmdline_add_arg(struct ascii_printer *__restrict printer,
                DeeStringObject *__restrict arg) {
 char *begin,*iter,*end,*flush_start;
 bool must_quote = false;
 size_t start_length;
 if (printer->ap_length &&
     ascii_printer_putc(printer,' '))
     goto err;
 start_length = printer->ap_length;
 end = (iter = begin = flush_start = DeeString_STR(arg))+DeeString_SIZE(arg);
 for (; iter != end; ++iter) {
  if (*iter == '\"') {
   char *quote_start = iter;
   while (iter != begin && iter[-1] == '\\') --iter;
   if (ascii_printer_print(printer,flush_start,(size_t)(iter-flush_start)) < 0)
       goto err;
   /* Escape by writing double the number of slashes. */
   if (quote_start != iter) {
    if (ascii_printer_print(printer,iter,(size_t)(quote_start-iter)) < 0 ||
        ascii_printer_print(printer,iter,(size_t)(quote_start-iter)) < 0)
        goto err;
   }
   /* Following this, write the escaped quote. */
   if (ascii_printer_print(printer,"\\\"",2) < 0)
       goto err;
   flush_start = iter+1;
  } else if (*iter == ' ' || *iter == '\t') {
   must_quote = true;
  }
 }
 /* Flush the remainder of the argument. */
 if (ascii_printer_print(printer,flush_start,(size_t)(iter-flush_start)) < 0)
     goto err;
 /* Surround the argument with quotation marks. */
 if (must_quote) {
  char *start; size_t length;
  length = printer->ap_length-start_length;
  if unlikely(!ascii_printer_alloc(printer,2)) goto err;
  start  = printer->ap_string->s_str+start_length;
  /* Shift the argument text. */
  memmove(start+1,start,length*sizeof(char));
  /* Add surrounding quotes. */
  start[0]        = '\"';
  start[length+1] = '\"';
 }
 return 0;
err:
 return -1;
}
INTERN int DCALL
cmdline_add_args(struct ascii_printer *__restrict printer,
                 DeeObject *__restrict args) {
 DREF DeeObject *iter,*elem; int result = 0;
 iter = DeeObject_IterSelf(args);
 if unlikely(!iter) goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
  if (DeeObject_AssertTypeExact(elem,&DeeString_Type))
       result = -1;
  else result = cmdline_add_arg(printer,(DeeStringObject *)elem);
  Dee_Decref(elem);
  if unlikely(result) break;
  if (DeeThread_CheckInterrupt())
      goto err_iter;
 }
 if unlikely(!elem)
    goto err_iter;
 Dee_Decref(iter);
 return result;
err_iter:
 Dee_Decref(iter);
err:
 return -1;
}



INTERN DREF DeeObject *DCALL
cmdline_split(DeeStringObject *__restrict cmdline) {
 char *iter,*end; struct ascii_printer printer; int temp;
 DREF DeeObject *arg,*result = DeeList_New();
 /* Since this function isn't actually used that often, it doesn't use
  * an anonymous sequence type, but simply creates and returns a list
  * object. */
 if unlikely(!result) goto done;
 end = (iter = DeeString_STR(cmdline))+DeeString_SIZE(cmdline);
 while (iter != end) {
  bool is_quoting = false; unsigned int num_slashes;
  char *flush_start;
  ascii_printer_init(&printer);
  /* Skip leading whitespace. */
  while (*iter == ' ' || *iter == '\t') ++iter;
  if (iter == end) goto done_printer; /* End of argument list. */
  flush_start = iter;
  while (iter != end &&
        (is_quoting || (*iter != ' ' && *iter != '\t'))) {
   char *part_start = iter;
   num_slashes = 0;
   while (*iter == '\\') ++iter,++num_slashes;
   if (*iter == '\"') {
    /* Special handling for escaped quotation marks. */
    /* Print one backslash for every second leading backslash. */
    part_start += num_slashes/2;
    if (ascii_printer_print(&printer,flush_start,(size_t)(part_start-flush_start)) < 0)
        goto err_r_printer;
    /* Continue flushing after the quotation character. */
    flush_start = iter+1;
    /* Special extension: When inside a quoted string, double-quotes are
     * extended into a single quote. Implement this by moving the flush_start
     * pointer back by one (so-as to print the `\"' that got us here). */
    if (is_quoting && iter[1] == '\"')
        --flush_start;
    is_quoting = !is_quoting;
   } else {
    /* Without a following quotation mark,
     * backslashes do not act as escaping. */
   }
   ++iter;
  }
  /* Flush the remainder of the argument. */
  if (ascii_printer_print(&printer,flush_start,(size_t)(iter-flush_start)) < 0)
      goto err_r_printer;
  /* Pack together the argument. */
  arg = ascii_printer_pack(&printer);
  if unlikely(!arg) goto err_r;
  /* Add the argument to the resulting list. */
  temp = DeeList_Append(result,arg);
  Dee_Decref(arg);
  if unlikely(temp) goto err_r;
 }
done:
 return result;
done_printer:
 ascii_printer_fini(&printer);
 goto done;
err_r_printer:
 ascii_printer_fini(&printer);
err_r:
 Dee_Decref(result);
 return NULL;
}



DECL_END

#endif /* !GUARD_DEX_IPC_WINDOWS_CMDLINE_C_INL */
