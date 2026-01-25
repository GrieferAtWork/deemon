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
#ifndef GUARD_CMDLINE_C
#define GUARD_CMDLINE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_CollectMemoryc, Dee_Freea, Dee_UntrackAlloc */
#include <deemon/error.h>           /* DeeError_RuntimeError, DeeError_Throwf */
#include <deemon/system-features.h> /* bcmpc, memchr, memmovedownc, mempcpyc, strlen */

#include "cmdline.h"

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN


struct late_cmd_option {
	int (DCALL *lco_func)(char *arg); /* [1..1] The command to execute. */
	char       *lco_arg;              /* [0..1] The argument for `lco_func' */
};
struct late_cmd_options {
	size_t                  lco_opta; /* Allocated amount of late-options. */
	size_t                  lco_optc; /* Total amount of late-options. */
	struct late_cmd_option *lco_optv; /* [0..lco_optc|ALLOC(lco_opta)][owned] Vector of late-options. */
};

PRIVATE struct late_cmd_options late_options = { 0, 0, NULL };

/* Execute all late-options and free the `lco_optv' of the internal late-options list. */
INTERN int DCALL cmd_runlate(void) {
	struct late_cmd_option *iter, *end;
	int result = 0;
	end = (iter = late_options.lco_optv) + late_options.lco_optc;
	for (; iter < end; ++iter) {
		result = (*iter->lco_func)(iter->lco_arg);
		if unlikely(result != 0)
			break;
	}
	return result;
}


PRIVATE int DCALL
cmd_addlater(int (DCALL *func)(char *arg), char *arg) {
	struct late_cmd_option *entry;
	ASSERT(late_options.lco_optc <= late_options.lco_opta);
	if (late_options.lco_optc == late_options.lco_opta) {
		/* Allocate more memory. */
		size_t new_alloc = late_options.lco_opta * 2;
		if (!new_alloc)
			new_alloc = 2;
do_realloc:
		entry = (struct late_cmd_option *)Dee_UntrackAlloc(Dee_TryReallocc(late_options.lco_optv, new_alloc,
		                                                                   sizeof(struct late_cmd_option)));
		if unlikely(!entry) {
			if (new_alloc != late_options.lco_optc + 1) {
				new_alloc = late_options.lco_optc + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemoryc(new_alloc, sizeof(struct late_cmd_option)))
				goto do_realloc;
			return -1;
		}
		late_options.lco_opta = new_alloc;
		late_options.lco_optv = entry;
	}
	/* Add a new entry for the late-callback. */
	entry = late_options.lco_optv;
	entry += late_options.lco_optc++;
	entry->lco_func = func;
	entry->lco_arg  = arg;
	return 0;
}


#define SHORT_OPTION_MAXLEN \
	(COMPILER_LENOF(((struct cmd_option *)0)->co_shortnam) - 1)

/* Find and return a short option with all bits
 * set in `req_flags', as well as matching `name'.
 * If not found, return `NULL' */
PRIVATE struct cmd_option const *DCALL
find_short_option(struct cmd_option const *__restrict options,
                  char const *__restrict name, size_t name_len,
                  uint16_t req_mask, uint16_t req_flags) {
	if unlikely(!name_len)
		goto done;
	if unlikely(name_len > SHORT_OPTION_MAXLEN)
		goto done;
	for (; !CMD_OPTION_ISSENTINEL(options); ++options) {
		if ((options->co_flags & req_mask) != req_flags)
			continue; /* Invalid flags. */
		if (bcmpc(options->co_shortnam, name, name_len, sizeof(char)) != 0)
			continue; /* Different name. */
		if (options->co_shortnam[name_len])
			continue; /* Name too long. */
		return options;
	}
done:
	return NULL;
}

/* Same as `find_short_option', but for long names. */
PRIVATE struct cmd_option const *DCALL
find_long_option(struct cmd_option const *__restrict options,
                 char const *__restrict name, size_t name_len,
                 uint16_t req_mask, uint16_t req_flags) {
	if unlikely(!name_len)
		goto done;
	for (; !CMD_OPTION_ISSENTINEL(options); ++options) {
		if ((options->co_flags & req_mask) != req_flags)
			continue; /* Invalid flags. */
		if (!options->co_longname)
			continue; /* No long name available. */
		if (bcmpc(options->co_longname, name, name_len, sizeof(char)) != 0)
			continue; /* Different name. */
		if (options->co_longname[name_len])
			continue; /* Name too long. */
		return options;
	}
done:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
cmd_parse(int *__restrict p_argc, char ***__restrict p_argv,
          struct cmd_option const *__restrict options,
          bool exec_all) {
	struct cmd_option const *opt;
	int result  = 0;
	int argc    = *p_argc;
	char **argv = *p_argv;
	for (;;) {
		char *cmd, *arg = NULL;
		uint16_t long_flags;
		size_t cmd_len;
		if (!argc)
			break;
		cmd = *argv;
#if 0
		if (!*cmd) {
			/* Skip empty arguments. */
			--argc;
			++argv;
			continue;
		}
#endif
		if (!exec_all && cmd[0] != '-')
			break; /* End of command list. */
		if (cmd[0] == '-') {
			++cmd; /* Skip the initial dash. */
			if (!*cmd)
				break; /* Nothing after the `-' (used in places as alias for /dev/stdin) */
		}
		++argv, --argc; /* Consume this argument. */
		if (cmd[0] == '-') {
			char *eq_sign;
			++cmd;
			long_flags = CMD_FNORMAL;
check_long:
			cmd_len = strlen(cmd);
			if (!exec_all && !cmd_len)
				break; /* Explicit command list end (double dash: `--') */
			/* Long option name. */
			opt = find_long_option(options, cmd, cmd_len,
			                       long_flags | CMD_FARGONLYIMM,
			                       long_flags);
			if (opt)
				goto has_opt;
			/* Check for long options that accept their argument following a `=' character. */
			eq_sign = (char *)memchr(cmd, '=', cmd_len);
			if (eq_sign) {
				opt = find_long_option(options, cmd, (size_t)(eq_sign - cmd),
				                       long_flags | CMD_FARGEQ | CMD_FARGONLYIMM,
				                       long_flags | CMD_FARGEQ);
				if (opt) {
					arg = eq_sign + 1;
					goto has_opt;
				}
			}
			while (cmd_len) {
				--cmd_len;
				/* Search for long commands that take an immediate argument. */
				opt = find_long_option(options, cmd, cmd_len,
				                       long_flags | CMD_FARG | CMD_FARGIMM,
				                       long_flags | CMD_FARG | CMD_FARGIMM);
				if (!opt)
					continue;
				/* Found one! */
				arg = cmd + cmd_len; /* Immediate argument operand. */
				goto has_opt;
			}
			if (long_flags & CMD_FLONG1DASH)
				goto check_short_options;
		} else {
			/* Search for long options with the 1DASH flag set. */
search_joined_cmd:
			long_flags = CMD_FLONG1DASH;
			goto check_long;
check_short_options:
			/* Search for short options. */
			cmd_len = strlen(cmd);
			opt = find_short_option(options, cmd, cmd_len,
			                        CMD_FARGONLYIMM | CMD_FGROUP,
			                        CMD_FNORMAL);
			if (opt)
				goto has_opt;
			/* Search for combined short options / short options with immediate operands. */
			if (cmd_len > SHORT_OPTION_MAXLEN)
				cmd_len = SHORT_OPTION_MAXLEN;
			while (cmd_len) {
				--cmd_len;
				opt = find_short_option(options, cmd, cmd_len,
				                        CMD_FNORMAL, CMD_FNORMAL);
				if (!opt)
					continue;
				if (opt->co_flags & (CMD_FARG | CMD_FGROUP)) {
					if ((opt->co_flags & CMD_FARGEQ) && cmd[cmd_len] == '=') {
						/* Immediate argument, following an equal sign. */
						arg = cmd + cmd_len + 1;
					} else if (opt->co_flags & CMD_FJOINABLE) {
						arg = NULL;
					} else {
						if (!(opt->co_flags & (CMD_FARGIMM | CMD_FGROUP)))
							continue; /* Not allowed with an immediate argument. */
						if ((opt->co_flags & CMD_FGROUP) && cmd[cmd_len] != ',')
							continue; /* Grouped-commands require a trailing comma. */
						arg = cmd + cmd_len;
					}
				} else {
					/* Check for joined options. */
					if (!(opt->co_flags & CMD_FJOINABLE))
						continue;
				}
				goto has_opt;
			}
		}
		/* Finally: execute the option. */
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Unknown commandline option `%#q'",
		                cmd);
		goto err;
has_opt:
		/*ASSERT(opt != NULL);*/
		ASSERT(opt->co_func);
		if (!arg && argc && !(opt->co_flags & CMD_FARGEQ) &&
		    ((opt->co_flags & (CMD_FARG | CMD_FARGONLYIMM)) == (CMD_FARG) ||
		     (opt->co_flags & CMD_FGROUP)) &&
		    (!(opt->co_flags & CMD_FARGOPT) || **argv != '-')) {
			/* Take the argument from the next command. */
			if (opt->co_flags & CMD_FREMAINDER) {
				char *cmdline, *dst; /* NOTE: This variable is leaked intentionally! */
				size_t argv_len = (size_t)argc;
				unsigned int i;
				for (i = 0; i < (unsigned int)argc; ++i)
					argv_len += strlen(argv[i]);
				cmdline = (char *)(Dee_Malloc)((argv_len + 1) * sizeof(char));
				if unlikely(!cmdline)
					goto err;
				for (dst = cmdline, i = 0; i < (unsigned int)argc; ++i) {
					size_t len = strlen(argv[i]);
					/* Copy the argument. */
					dst = (char *)mempcpyc(dst, argv[i], len, sizeof(char));
					*dst++ = ' ';
				}
				dst[-1] = '\0';
				/* Consume all remaining arguments. */
				argv += argc;
				argc = 0;
			} else {
				arg = *argv;
				--argc, ++argv;
			}
		}
		if (opt->co_flags & CMD_FGROUP) {
			int sub_argc;
			char *iter, *argend;
			char **sub_argv, **sub_argv_mirror;
			/* Options sub-group. (comma-separated argument list) */
			if (!arg || !*arg)
				continue; /* No sub-commands listed. */
			while (*arg == ',')
				++arg; /* Skip leading commas. */
			/* Count the number of comma-separated commands. */
			sub_argc = 1;
			for (iter = arg; *iter; ++iter) {
				if (*iter == ',' && iter[-1] != '\\')
					++sub_argc;
			}
			sub_argv = (char **)Dee_Mallocac((unsigned int)sub_argc, sizeof(char *));
			if unlikely(!sub_argv)
				goto err;
			argend             = iter;
			sub_argv_mirror    = sub_argv;
			*sub_argv_mirror++ = arg;
			/* Split sub-arguments. */
			for (iter = arg; iter != argend; ++iter) {
				if (iter[0] == '\\' && iter != argend - 1 && iter[1] == ',') {
					/* Decode an escaped comma. */
					--argend;
					memmovedownc(iter, iter + 1,
					             (size_t)(argend - iter),
					             sizeof(char)); /* Delete the backslash. */
					++iter;                     /* Skip the comma. */
					continue;                   /* Continue parsing. */
				}
				if (*iter == ',') {
					/* Argument separator (split argument list). */
					*iter = '\0';
					/* Add the start of the next argument to the sub-argv vector. */
					*sub_argv_mirror++ = iter + 1;
				}
			}
			*argend = '\0'; /* ZERO-Terminate the last argument. */
			ASSERT(sub_argv_mirror == sub_argv + (unsigned int)sub_argc);
			/* Execute commands from the sub-group. */
			sub_argv_mirror = sub_argv;
			result = cmd_parse(&sub_argc, &sub_argv_mirror,
			                   opt->co_group, true);
			Dee_Freea(sub_argv);
		} else {
			/* Check if an absent argument is acceptable by the command. */
			if (!arg && (opt->co_flags & (CMD_FARG | CMD_FARGOPT)) == (CMD_FARG)) {
				DeeError_Throwf(&DeeError_RuntimeError,
				                "Commandline option `%s%s' requires an argument",
				                opt->co_longname ? "--" : "-",
				                opt->co_longname ? opt->co_longname : opt->co_shortnam);
				goto err;
			}
			/* Execute a command-option, or query it for later execution. */
			if (opt->co_flags & CMD_FRUNLATER) {
				result = cmd_addlater(opt->co_func, arg);
			} else {
				result = (*opt->co_func)(arg);
			}
			if ((opt->co_flags & CMD_FJOINABLE) && !arg) {
				cmd += cmd_len;
				if (*cmd)
					goto search_joined_cmd;
			}
		}
		/* Check for errors. */
		if unlikely(result != 0)
			goto done;
	}
done:
	*p_argc = argc;
	*p_argv = argv;
	return result;
err:
	result = -1;
	goto done;
}


DECL_END

#endif /* !GUARD_CMDLINE_C */
