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
#ifndef GUARD_CMDLINE_H
#define GUARD_CMDLINE_H 1

#include <deemon/api.h>

#include <stddef.h> /* NULL */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

struct cmd_option {
#define CMD_FNORMAL               0x0000 /* Normal command options. */
#define CMD_FJOINABLE             0x0001 /* The command's short name can be joined with
                                          * other options. (e.g.: `-ES' is the same as `-E -S')
                                          * NOTE: Only the first option in a pair of joined
                                          *       options must have this flag set. */
#define CMD_FARG                  0x0002 /* The command takes an operand from the next argument.
                                          * When `CMD_FJOINABLE' is set and the option is joined,
                                          * this command must appear last. */
#define CMD_FARGIMM               0x0004 /* Flag for `CMD_FARG' - The argument is allowed to appear
                                          * as an immediate operand directly following this option.
                                          * e.g.: `-DFOO=42' */
#define CMD_FARGOPT               0x0008 /* Flag for `CMD_FARG' - The argument is optional and is
                                          * only accepted when it doesn't start with a dash `-'. */
#define CMD_FARGEQ                0x0010 /* Flag for `CMD_FARGIMM' - The argument must start with
                                          * a leading `=' character that is stripped before the being
                                          * interpreted by passing it to `co_func' or using it in a sub-group.
                                          * >> `--message-format=msvc' */
#define CMD_FARGONLYIMM           0x0020 /* Flag for `CMD_FARGIMM' - The argument may only
                                          * be passed as an immediate operand: when set,
                                          * `-Dfoo' is allowed, but `-D foo' would not be. */
#define CMD_FLONG1DASH            0x0040 /* `co_longname' is also recognized as a valid option when
                                          * the command-name starts with a single dash, rather than
                                          * 2 dashes. */
#define CMD_FRUNLATER             0x2000 /* The comment should be executed at a later point in time.
                                          * This refers to commands that are executed when the initial
                                          * compiler or TPP context has already been become active. */
#define CMD_FREMAINDER            0x4000 /* Flag for `CMD_FARG': Use the remainder of the commandline as argument. */
#define CMD_FGROUP                0x8000 /* The command acts as the leader of a comma-is-space-separated
                                          * list of sub-commands, where actually intended commas can be
                                          * escaped by prefixing a `\\' character:
                                          * >> `-Wl,-fopt-a,-fopt-b,-fopt-c=foo\,bar'
                                          *    Group:       `Wl' (short name)
                                          *    Sub-options: `-fopt-a', `-fopt-b', `-fopt-c=foo,bar' */
	uint16_t    co_flags;                /* Command options (Set of `CMD_F*') */
	char        co_shortnam[6];          /* Short argument name (NUL-terminated string; empty when not set) */
	char const *co_longname;             /* [0..1] Long argument name, or NULL when not set.
	                                      *  To be accepted as a long argument name, the commandline option
	                                      *  must feature 2 leading dashes (e.g.: `--long-option')
	                                      *  However, when `CMD_FLONG1DASH' is set, a single leading dash is accepted as well. */
	union {
		void       *co_hook;
		WUNUSED_T
		int (DCALL *co_func)(char *arg); /* [0..1][valid_if(!CMD_FGROUP)] Function called when the argument is encountered.
		                                  *  When NULL, the command acts as a terminating sentinel.
		                                  *  @param: arg: The NUL-terminated argument passed to the command,
		                                  *               or NULL when `CMD_FARG' isn't set, or `CMD_FARGOPT'
		                                  *               is and no argument was given.
		                                  *  @return: -1: An error occurred.
		                                  *  @return:  0: Successfully executed the command function.
		                                  *  HINT: The given `arg' will not be re-allocated, freed, or modified
		                                  *        anywhere until deemon exits, meaning that the pointer can be
		                                  *        weakly referenced for any purpose, as well as be modified by
		                                  *        this function itself.
		                                  */
		struct cmd_option *co_group;     /* [0..1][valid_if(CMD_FGROUP)] A sub-group of commandline options:
		                                  * >> `-Wl,-fopt-a,-fopt-b,-fopt-c=foo\,bar'
		                                  *    Group:       `Wl' (short name)
		                                  *    Sub-options: `-fopt-a', `-fopt-b', `-fopt-c=foo,bar'
		                                  */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define co_hook  _dee_aunion.co_hook
#define co_func  _dee_aunion.co_func
#define co_group _dee_aunion.co_group
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	char const *co_doc;                  /* [0..1] An optional documentation string for this command. */
};
#define CMD_OPTION_SENTINEL      { CMD_FNORMAL, "", NULL, { NULL }, NULL }
#define CMD_OPTION_ISSENTINEL(x) ((x)->co_func == NULL)

/* Parse commandline options:
 * >> cmd_parse(["-foo", "-bar", "source.dee", "bar"])
 *    Execute commands `foo' and `bar', leave `*p_argc' and `*p_argv' as `["source.dee", "bar"]'
 * >> cmd_parse(["-foo", "-bar", "--", "-source.dee", "bar"])
 *    Execute commands `foo' and `bar', leave `*p_argc' and `*p_argv' as `["-source.dee", "bar"]'
 * @param: exec_all:     When true, always execute all given arguments as commands,
 *                       allowing the leading dhash to be omitted.
 *                       Otherwise, arguments are parsed as shown in the previous example.
 * @throw: RuntimeError: At least one of the given arguments could not be interpreted.
 * @return:  0:          Successfully parsed arguments.
 * @return: -1:          An error was thrown.
 * NOTE: Options are always searched in ascending order, meaning that lower
 *       indices have a greater priority and may out-weigh greater indices. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
cmd_parse(int *__restrict p_argc, char ***__restrict p_argv,
          struct cmd_option const *__restrict options,
          bool exec_all);

/* Execute all encountered commands with the `CMD_FRUNLATER' flag set. */
INTDEF int DCALL cmd_runlate(void);


DECL_END

#endif /* !GUARD_CMDLINE_H */
