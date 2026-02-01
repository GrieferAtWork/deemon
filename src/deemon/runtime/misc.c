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
#ifndef GUARD_DEEMON_RUNTIME_MISC_C
#define GUARD_DEEMON_RUNTIME_MISC_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_BadAlloc, Dee_Free, Dee_TryMalloc */
#include <deemon/code.h>            /* DeeCodeObject, DeeCode_Type, DeeFunction_Type, Dee_code_frame, code_addr_t */
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>           /* DeeError_Handled, DeeError_UninstallKeyboardInterrupt, ERROR_HANDLED_RESTORE */
#include <deemon/file.h>            /* DeeFile_* */
#include <deemon/format.h>          /* DeeFormat_VPrintf, Dee_vsnprintf, PRFuSIZ, PRFxSIZ */
#include <deemon/gc.h>              /* DeeGC_Collect */
#include <deemon/heap.h>            /* DeeHeap_Trim */
#include <deemon/object.h>          /* DeeObject_Check, Dee_TYPE, Dee_formatprinter_t, Dee_int128_t, Dee_ssize_t, Dee_uint128_t */
#include <deemon/system-features.h> /* CONFIG_HAVE_*, EXIT_FAILURE, _Exit, abort, getenv, mempcpy, strlen */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_Self */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/overflow.h>        /* OVERFLOW_UADD, OVERFLOW_UMUL */
#include <hybrid/sched/yield.h>     /* SCHED_YIELD */
#include <hybrid/typecore.h>        /* __BYTE_TYPE__, __SIZEOF_SIZE_T__ */

#include <stdarg.h>  /* va_end, va_list, va_start */
#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uintptr_t */

#ifndef NDEBUG
#ifndef CONFIG_HOST_WINDOWS
#include <deemon/file.h>
#else /* !CONFIG_HOST_WINDOWS */
#include <hybrid/host.h>   /* __ARCH_PAGESIZE */
#include <hybrid/minmax.h> /* MIN */

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#include <Windows.h> /* OutputDebugStringA */
#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */
#endif /* CONFIG_HOST_WINDOWS */
#endif /* !NDEBUG */

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifndef CONFIG_NO_DEC
#include <deemon/dec.h> /* DecTime_ClearCache */
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

STATIC_ASSERT(sizeof(Dee_uint128_t) == 16);
STATIC_ASSERT(sizeof(Dee_int128_t) == 16);

#define Cs(x) INTDEF size_t DCALL x##_clear(size_t max_clear);
#define Co(x) INTDEF size_t DCALL x##_clear(size_t max_clear);
#include "caches.def"
#undef Co
#undef Cs


/* TODO: CONFIG_NO_CACHES */
typedef size_t (DCALL *pcacheclr)(size_t max_clear);
INTDEF size_t DCALL Dee_intcache_clearall(size_t max_clear);
INTDEF size_t DCALL Dee_tuplecache_clearall(size_t max_clear);
#ifdef CONFIG_STRING_LATIN1_CACHED
INTDEF size_t DCALL Dee_latincache_clearall(size_t max_clear);
#endif /* CONFIG_STRING_LATIN1_CACHED */
INTDEF size_t DCALL Dee_membercache_clearall(size_t max_clear);
#ifndef CONFIG_NO_THREADS
INTDEF size_t DCALL Dee_futex_clearall(size_t max_clear);
#endif /* !CONFIG_NO_THREADS */

PRIVATE pcacheclr caches[] = {
#define Cs(x) &x##_clear,
#define Co(x) &x##_clear,
#include "caches.def"
#undef Co
#undef Cs
	/* Custom object/data cache clear functions. */
	&Dee_intcache_clearall,
	&Dee_tuplecache_clearall,
#ifdef CONFIG_STRING_LATIN1_CACHED
	&Dee_latincache_clearall,
#endif /* CONFIG_STRING_LATIN1_CACHED */
	&Dee_membercache_clearall,
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifndef CONFIG_NO_DEC
	&DecTime_ClearCache,
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#ifndef CONFIG_NO_THREADS
	&Dee_futex_clearall,
#endif /* !CONFIG_NO_THREADS */

	NULL
};


PRIVATE size_t DCALL
DeeMem_ClearCaches_onepass(size_t max_collect) {
	pcacheclr *iter;
	size_t result = 0;
	/* Go through all the caches and collect memory. */
	for (iter = caches; *iter; ++iter) {
		result += (*iter)(max_collect - result);
		if (result >= max_collect)
			break;
	}
	return result;
}

PUBLIC size_t DCALL
DeeMem_ClearCaches(size_t max_collect) {
	size_t part, result = 0;
	do {
		part = DeeMem_ClearCaches_onepass(max_collect - result);
		result += part;
	} while (result < max_collect && part);
	return result;
}

/* Threshold before OOM is forced (without trying to collect memory) */
#define FORCED_OOM_THRESHOLD ((size_t)-1 / 3)

PRIVATE bool DCALL Dee_TryCollectMemory(size_t req_bytes) {
	size_t collect_bytes;

	/* Check for likely case: intentional allocation overflow.
	 * In this case, don't try to do GC collect, etc., since
	 * the OOM is probably intended by the caller. */
	if likely(req_bytes >= FORCED_OOM_THRESHOLD)
		return false;

	/* Clear caches and collect memory from various places. */
	collect_bytes = DeeMem_ClearCaches(req_bytes);
	if (collect_bytes >= req_bytes)
		return true;
	req_bytes -= collect_bytes;

	/* Collect GC objects.
	 * NOTE: When optimizing, only try to collect a single object
	 *       in order to prevent lag-time during memory shortages.
	 *       However for debug-mode, always collect everything in
	 *       order to try and harden the algorithm in times of need. */
#if defined(NDEBUG) || defined(__OPTIMIZE__)
	if (DeeGC_Collect(1))
		return true;
#else /* NDEBUG || __OPTIMIZE__ */
	if (DeeGC_Collect((size_t)-1))
		return true;
#endif /* !NDEBUG && !__OPTIMIZE__ */

	/* TODO: Call "tp_cc" of GC objects that are reachable */

	return collect_bytes != 0;
}

/* Try to clear caches and free up at most "req_bytes" memory. If
 * no memory could be free'd at all, Dee_BadAlloc() is called. Note
 * that this function is also allowed to invoke arbitrary user-code!
 *
 * @return: true:  Caches were cleared. - You should try to allocate memory again.
 * @return: false: Nope. - We're completely out of memory... (error was thrown) */
PUBLIC WUNUSED ATTR_COLD bool DCALL Dee_CollectMemory(size_t req_bytes) {
	void *_test;
	if unlikely(!Dee_TryCollectMemory(req_bytes))
		goto err_badalloc;

	/* Check if allocating "req_bytes" actually became
	 * possible, or if "Dee_TryCollectMemory" lied or
	 * the memory it freed has already been allocated
	 * for other purposes again. */
	_test = Dee_TryMalloc(req_bytes);
	if likely(_test) {
		Dee_Free(_test);
		return true;
	}

	/* Maybe "Dee_TryCollectMemory" just didn't try hard enough... */
	if (req_bytes < (FORCED_OOM_THRESHOLD - 1)) {
		if unlikely(!Dee_TryCollectMemory(FORCED_OOM_THRESHOLD - 1))
			goto err_badalloc;
		_test = Dee_TryMalloc(req_bytes);
		if likely(_test) {
			Dee_Free(_test);
			return true;
		}
	}

	/* Nope: there's no way to free up that much memory... */
err_badalloc:
	Dee_BadAlloc(req_bytes);
	return false;
}

/* Try to release as much memory as possible back to the host system.
 * @return: * : Amount of memory that was released back to the system.
 * @return: 0 : No memory could be released back to the system (no error was thrown) */
PUBLIC ATTR_COLD size_t DCALL Dee_TryReleaseSystemMemory(void) {
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
	size_t trim = DeeHeap_Trim((size_t)-1);
	if (trim == 0) {
		Dee_TryCollectMemory((size_t)-1);
		trim = DeeHeap_Trim((size_t)-1);
	}
	return trim;
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	return Dee_TryCollectMemory((size_t)-1) ? 1 : 0;
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
}

/* Same as `Dee_TryReleaseSystemMemory()', but also tries to free
 * @return: * : Amount of memory that was released back to the system.
 * @return: 0 : No memory could be released back to the system (an error was thrown) */
PUBLIC ATTR_COLD WUNUSED size_t DCALL Dee_ReleaseSystemMemory(void) {
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
	size_t trim = DeeHeap_Trim((size_t)-1);
	if unlikely(trim == 0) {
		Dee_TryCollectMemory((size_t)-1);
		trim = DeeHeap_Trim((size_t)-1);
		if (trim == 0 && Dee_CollectMemory((size_t)-1)) {
			trim = DeeHeap_Trim((size_t)-1);
			if unlikely(trim == 0)
				Dee_BadAlloc(1);
		}
	}
	return trim;
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	return Dee_CollectMemory((size_t)-1) ? 1 : 0;
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
}



#ifndef Dee_DPRINT_IS_NOOP

#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#endif /* !CONFIG_OUTPUTDEBUGSTRINGA_DEFINED */

PRIVATE Dee_ssize_t DPRINTER_CC
debug_printer(void *UNUSED(closure),
              char const *__restrict buffer, size_t bufsize) {
#ifdef CONFIG_HOST_WINDOWS
	size_t result = bufsize;
#ifdef __ARCH_PAGESIZE_MIN
	/* (ab-)use the fact that the kernel can't keep us from reading
	 * beyond the end of a buffer so long as that memory location
	 * is located within the same page as the last byte of said
	 * buffer (Trust me... I've written by own OS) */
	if ((bufsize <= 1000) && /* There seems to be some kind of limitation by `OutputDebugStringA()' here... */
	    (((uintptr_t)buffer + bufsize) & ~(uintptr_t)(__ARCH_PAGESIZE_MIN - 1)) ==
	    (((uintptr_t)buffer + bufsize - 1) & ~(uintptr_t)(__ARCH_PAGESIZE_MIN - 1)) &&
	    (*(char *)((uintptr_t)buffer + bufsize)) == '\0') {
		DBG_ALIGNMENT_DISABLE();
		OutputDebugStringA((char *)buffer);
		DBG_ALIGNMENT_ENABLE();
	} else
#endif /* __ARCH_PAGESIZE_MIN */
	{
		char temp[512];
		while (bufsize) {
			size_t part;
			part = MIN(bufsize, sizeof(temp) - sizeof(char));
			*(char *)mempcpy(temp, buffer, part) = '\0';
			DBG_ALIGNMENT_DISABLE();
			OutputDebugStringA(temp);
			DBG_ALIGNMENT_ENABLE();
			buffer = (char const *)((byte_t *)buffer + part);
			bufsize -= part;
		}
	}
	return (Dee_ssize_t)result;
#else /* CONFIG_HOST_WINDOWS */
	return (Dee_ssize_t)DeeFile_Write(DeeFile_DefaultStddbg, buffer, bufsize);
#endif /* !CONFIG_HOST_WINDOWS */
}
#endif /* !Dee_DPRINT_IS_NOOP */


#ifdef Dee_DPRINT_IS_NOOP
PUBLIC int _Dee_dprint_enabled = 0;
#else /* Dee_DPRINT_IS_NOOP */
PUBLIC int _Dee_dprint_enabled = 2;
PRIVATE void DCALL determine_is_dprint_enabled(void) {
	char const *env;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HOST_WINDOWS
	if (!IsDebuggerPresent()) {
		DBG_ALIGNMENT_ENABLE();
		_Dee_dprint_enabled = 0;
		return;
	}
#endif /* CONFIG_HOST_WINDOWS */
	env = getenv("DEEMON_SILENT");
	DBG_ALIGNMENT_ENABLE();
#ifdef CONFIG_HOST_WINDOWS
	if (env && *env) {
		_Dee_dprint_enabled = 0;
	} else {
		_Dee_dprint_enabled = 1;
	}
#else /* CONFIG_HOST_WINDOWS */
	if (env && *env) {
		_Dee_dprint_enabled = *env == '0';
	} else {
		_Dee_dprint_enabled = 0;
	}
#endif /* !CONFIG_HOST_WINDOWS */
}
#endif /* !Dee_DPRINT_IS_NOOP */



PUBLIC NONNULL((1)) void
(DCALL _Dee_vdprintf)(char const *__restrict format, va_list args) {
#ifdef Dee_DPRINT_IS_NOOP
	Dee_vsnprintf(NULL, 0, format, args);
#else /* Dee_DPRINT_IS_NOOP */
	if (_Dee_dprint_enabled == 2)
		determine_is_dprint_enabled();
	if (!_Dee_dprint_enabled) {
		/* Must still use the given format-string for something,
		 * as it may be used to inherit object references! */
		Dee_vsnprintf(NULL, 0, format, args);
		return;
	}
	if (DeeFormat_VPrintf(&debug_printer, NULL, format, args) < 0)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
#endif /* !Dee_DPRINT_IS_NOOP */
}

PUBLIC NONNULL((1)) void
(DCALL _Dee_dprint)(char const *__restrict message) {
#ifdef Dee_DPRINT_IS_NOOP
	(void)message;
#else /* Dee_DPRINT_IS_NOOP */
	if (_Dee_dprint_enabled == 2)
		determine_is_dprint_enabled();
	if (!_Dee_dprint_enabled)
		return;
#ifdef CONFIG_HOST_WINDOWS
	DBG_ALIGNMENT_DISABLE();
	OutputDebugStringA(message);
	DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_HOST_WINDOWS */
	if (debug_printer(NULL, message, strlen(message)) < 0)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !Dee_DPRINT_IS_NOOP */
}

PUBLIC NONNULL((1)) void
(_Dee_dprintf)(char const *__restrict format, ...) {
#ifdef Dee_DPRINT_IS_NOOP
	va_list args;
	va_start(args, format);
	Dee_vsnprintf(NULL, 0, format, args);
	va_end(args);
#else /* Dee_DPRINT_IS_NOOP */
	va_list args;
	if (_Dee_dprint_enabled == 2)
		determine_is_dprint_enabled();
	va_start(args, format);
	if (!_Dee_dprint_enabled) {
		/* Must still use the given format-string for something,
		 * as it may be used to inherit object references! */
		Dee_vsnprintf(NULL, 0, format, args);
	} else {
		if (DeeFormat_VPrintf(&debug_printer, NULL, format, args) < 0)
			DeeError_Handled(ERROR_HANDLED_RESTORE);
	}
	va_end(args);
#endif /* !Dee_DPRINT_IS_NOOP */
}

PUBLIC Dee_ssize_t
(DPRINTER_CC _Dee_dprinter)(void *arg, char const *__restrict data, size_t datalen) {
#ifdef Dee_DPRINT_IS_NOOP
	(void)arg;
	(void)data;
	return (Dee_ssize_t)datalen;
#else /* Dee_DPRINT_IS_NOOP */
	Dee_ssize_t result;
	if (_Dee_dprint_enabled == 2)
		determine_is_dprint_enabled();
	if (!_Dee_dprint_enabled)
		return (Dee_ssize_t)datalen;
	result = debug_printer(arg, data, datalen);
	if (result < 0) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		result = 0;
	}
	return result;
#endif /* !Dee_DPRINT_IS_NOOP */
}


PRIVATE NONNULL((1)) void
assert_vprintf(char const *format, va_list args) {
	Dee_ssize_t error;
	error = DeeFile_VPrintf(DeeFile_DefaultStddbg, format, args);
	if unlikely(error < 0)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
}

PRIVATE NONNULL((1)) void
assert_printf(char const *format, ...) {
	va_list args;
	va_start(args, format);
	assert_vprintf(format, args);
	va_end(args);
}

#ifdef CONFIG_HOST_WINDOWS
#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#endif /* !CONFIG_OUTPUTDEBUGSTRINGA_DEFINED */

PRIVATE void assert_attach_debugger_loop(void) {
	if (!IsDebuggerPresent()) {
#if 0
		char const *env = getenv("DEEMON_SILENT");
		if (!env || !*env)
#endif
		{
			/* Restore the default CTRL+C behavior, so that the user can easily
			 * kill deemon without having to go through the Task Manager. */
			DeeError_UninstallKeyboardInterrupt();

			/* Wait to allow a debugger to be attached. */
			for (;;)
				SCHED_YIELD();
		}
	}
}
#else /* CONFIG_HOST_WINDOWS */
#define assert_attach_debugger_loop() (void)0
#endif /* !CONFIG_HOST_WINDOWS */

INTDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
print_ddi(Dee_formatprinter_t printer, void *arg,
          DeeCodeObject *__restrict code, code_addr_t ip);

PRIVATE void DCALL do_assert_print_usercode_trace(void) {
	DeeThreadObject *current = DeeThread_Self();
	uint16_t sz = current->t_execsz;
	struct Dee_code_frame *exec = current->t_exec;
	for (; sz && exec; --sz, exec = exec->cf_prev) {
		DeeCodeObject *code;
		if unlikely(!DeeObject_Check(exec->cf_func))
			break;
		if unlikely(Dee_TYPE(exec->cf_func) != &DeeFunction_Type)
			break;
		code = exec->cf_func->fo_code;
		if unlikely(!DeeObject_Check(code))
			break;
		if unlikely(Dee_TYPE(code) != &DeeCode_Type)
			break;
		if unlikely(exec->cf_ip < code->co_code)
			break;
		if unlikely(exec->cf_ip >= (code->co_code + code->co_codebytes))
			break;
		if (print_ddi((Dee_formatprinter_t)&DeeFile_WriteAll, DeeFile_DefaultStddbg,
		              code, (code_addr_t)(exec->cf_ip - code->co_code)) < 0)
			goto err;
	}
	return;
err:
	DeeError_Handled(ERROR_HANDLED_RESTORE);
}

PRIVATE bool in_assert_print_usercode_trace = false;
INTERN void DCALL assert_print_usercode_trace(void) {
	/* Safety check: prevent recursion on double-assert */
	if (!in_assert_print_usercode_trace) {
		in_assert_print_usercode_trace = true;
		do_assert_print_usercode_trace();
		in_assert_print_usercode_trace = false;
	}
}


PUBLIC void
(_DeeAssert_Failf)(char const *expr, char const *file,
                   int line, char const *format, ...) {
	assert_printf("\n\n\n"
	              "%s(%d) : Assertion failed : %s\n",
	              file, line, expr);
	if (format) {
		va_list args;
		va_start(args, format);
		assert_vprintf(format, args);
		va_end(args);
		assert_printf("\n");
	}
	assert_print_usercode_trace();
	assert_attach_debugger_loop();
}

PUBLIC ATTR_NORETURN void
(_DeeAssert_XFailf)(char const *expr, char const *file,
                    int line, char const *format, ...) {
	assert_printf("\n\n\n"
	              "%s(%d) : Assertion failed : %s\n",
	              file, line, expr);
	if (format) {
		va_list args;
		va_start(args, format);
		assert_vprintf(format, args);
		va_end(args);
		assert_printf("\n");
	}
	assert_print_usercode_trace();
	assert_attach_debugger_loop();
#if defined(CONFIG_HAVE_abort) && !defined(CONFIG_HAVE_abort_IS_ASSERT_XFAIL)
	abort();
#elif defined(CONFIG_HAVE__Exit)
	_Exit(EXIT_FAILURE);
#else /* ... */
	for (;;) {
		char volatile *volatile ptr;
		ptr  = (char volatile *)NULL;
		*ptr = 'X';
	}
#endif /* !... */
}

PUBLIC void
(DCALL _DeeAssert_Fail)(char const *expr, char const *file, int line) {
	_DeeAssert_Failf(expr, file, line, NULL);
}

PUBLIC ATTR_NORETURN void
(DCALL _DeeAssert_XFail)(char const *expr, char const *file, int line) {
	_DeeAssert_XFailf(expr, file, line, NULL);
}

#if __SIZEOF_SIZE_T__ == 4
#define PRFxSIZ_FULLWIDTH ".8" PRFxSIZ
#elif __SIZEOF_SIZE_T__ == 8
#define PRFxSIZ_FULLWIDTH ".16" PRFxSIZ
#elif __SIZEOF_SIZE_T__ == 2
#define PRFxSIZ_FULLWIDTH ".4" PRFxSIZ
#elif __SIZEOF_SIZE_T__ == 1
#define PRFxSIZ_FULLWIDTH ".2" PRFxSIZ
#else /* __SIZEOF_SIZE_T__ == ... */
#define PRFxSIZ_FULLWIDTH PRFxSIZ
#endif /* __SIZEOF_SIZE_T__ != ... */

/* Debug version of malloc buffer size calculation functions.
 * These will trigger an assertion failure when an overflow *does* happen.
 * As such, these functions should be used in debug builds to assert that
 * no unexpected overflows happen. */
PUBLIC ATTR_CONST WUNUSED size_t
(DCALL _Dee_MalloccBufsizeDbg)(size_t elem_count, size_t elem_size,
                               char const *file, int line) {
	size_t result;
	if (OVERFLOW_UMUL(elem_count, elem_size, &result)) {
		_DeeAssert_Failf("_Dee_MalloccBufsizeDbg(...)", file, line,
		                 "Unexpected overflow when multiplying elem_count * elem_size:\n"
		                 "elem_count = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")\n"
		                 "elem_size  = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")",
		                 elem_count, elem_count,
		                 elem_size, elem_size);
		Dee_BREAKPOINT();
		return (size_t)-1;
	}
	return result;
}

PUBLIC ATTR_CONST WUNUSED size_t
(DCALL _Dee_MallococBufsizeDbg)(size_t base_offset, size_t elem_count, size_t elem_size,
                                char const *file, int line) {
	size_t result;
	if (OVERFLOW_UMUL(elem_count, elem_size, &result)) {
		_DeeAssert_Failf("_Dee_MallococBufsizeDbg(...)", file, line,
		                 "Unexpected overflow when multiplying `elem_count * elem_size':\n"
		                 "elem_count   = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")\n"
		                 "elem_size    = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")\n"
		                 "[base_offset = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")]",
		                 elem_count, elem_count,
		                 elem_size, elem_size,
		                 base_offset, base_offset);
		Dee_BREAKPOINT();
		return (size_t)-1;
	}
	if (OVERFLOW_UADD(result, base_offset, &result)) {
		_DeeAssert_Failf("_Dee_MallococBufsizeDbg(...)", file, line,
		                 "Unexpected overflow when adding `base_offset' to `elem_count * elem_size':\n"
		                 "base_offset            = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")\n"
		                 "elem_count * elem_size = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")\n"
		                 "[elem_count            = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")]\n"
		                 "[elem_size             = %#" PRFxSIZ_FULLWIDTH " (%" PRFuSIZ ")]",
		                 base_offset, base_offset, result, result,
		                 elem_count, elem_count,
		                 elem_size, elem_size);
		Dee_BREAKPOINT();
		return (size_t)-1;
	}
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_MISC_C */
