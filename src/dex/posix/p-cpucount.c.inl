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
#ifndef GUARD_DEX_POSIX_P_CPUCOUNT_C_INL
#define GUARD_DEX_POSIX_P_CPUCOUNT_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

/* Figure out how we want to implement `cpu_count()' */
#undef posix_cpu_count_USE_GETSYSTEMINFO
#undef posix_cpu_count_USE_SYSCONF_SC_NPROCESSORS_ONLN
#undef posix_cpu_count_USE_SYSCTL_HW_AVAILCPU_HW_NCPU
#undef posix_cpu_count_USE_SYSCTL_HW_AVAILCPU
#undef posix_cpu_count_USE_SYSCTL_HW_NCPU
#undef posix_cpu_count_USE_MPCTL_MPC_GETNUMSPUS
#undef posix_cpu_count_USE_SYSCONF_SC_NPROC_ONLN
#undef posix_cpu_count_USE_PROC_CPUINFO
#undef posix_cpu_count_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_cpu_count_USE_GETSYSTEMINFO 1
#elif defined(CONFIG_HAVE_sysconf) && defined(CONFIG_HAVE__SC_NPROCESSORS_ONLN)
#define posix_cpu_count_USE_SYSCONF_SC_NPROCESSORS_ONLN 1
#elif defined(CONFIG_HAVE_sysctl) && defined(CONFIG_HAVE_CTL_HW) && defined(CONFIG_HAVE_HW_AVAILCPU) && defined(CONFIG_HAVE_HW_NCPU)
#define posix_cpu_count_USE_SYSCTL_HW_AVAILCPU_HW_NCPU 1
#elif defined(CONFIG_HAVE_sysctl) && defined(CONFIG_HAVE_CTL_HW) && defined(CONFIG_HAVE_HW_AVAILCPU)
#define posix_cpu_count_USE_SYSCTL_HW_AVAILCPU 1
#elif defined(CONFIG_HAVE_sysctl) && defined(CONFIG_HAVE_CTL_HW) && defined(CONFIG_HAVE_HW_NCPU)
#define posix_cpu_count_USE_SYSCTL_HW_NCPU 1
#elif defined(CONFIG_HAVE_mpctl) && defined(CONFIG_HAVE_MPC_GETNUMSPUS)
#define posix_cpu_count_USE_MPCTL_MPC_GETNUMSPUS 1
#elif defined(CONFIG_HAVE_sysconf) && defined(CONFIG_HAVE__SC_NPROC_ONLN)
#define posix_cpu_count_USE_SYSCONF_SC_NPROC_ONLN 1
#elif defined(CONFIG_HAVE_PROCFS)
#define posix_cpu_count_USE_PROC_CPUINFO 1
#else
#define posix_cpu_count_USE_STUB 1
#endif

/*[[[deemon import("_dexutils").gw("cpu_count", "->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_cpu_count_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_cpu_count_f(size_t argc, DeeObject *const *argv);
#define POSIX_CPU_COUNT_DEF { "cpu_count", (DeeObject *)&posix_cpu_count, MODSYM_FNORMAL, DOC("->?Dint") },
#define POSIX_CPU_COUNT_DEF_DOC(doc) { "cpu_count", (DeeObject *)&posix_cpu_count, MODSYM_FNORMAL, DOC("->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_cpu_count, posix_cpu_count_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_cpu_count_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":cpu_count"))
		goto err;
	return posix_cpu_count_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_cpu_count_f_impl(void)
//[[[end]]]
{
	/* Implementation variants taken from here:
	 * https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine */

#ifdef posix_cpu_count_USE_GETSYSTEMINFO
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return DeeInt_NewU32(sysinfo.dwNumberOfProcessors);
#endif /* posix_cpu_count_USE_GETSYSTEMINFO */

#ifdef posix_cpu_count_USE_SYSCONF_SC_NPROCESSORS_ONLN
	int result;
	result = sysconf(_SC_NPROCESSORS_ONLN);
	if unlikely(result <= 0)
		result = 1; /* Shouldn't happen... */
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_SYSCONF_SC_NPROCESSORS_ONLN */

#ifdef posix_cpu_count_USE_SYSCTL_HW_AVAILCPU_HW_NCPU
	int mib[4], result = 0;
	size_t len = sizeof(result);
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;
	sysctl(mib, 2, &result, &len, NULL, 0);
	if (result <= 0) {
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &result, &len, NULL, 0);
		if (result <= 0)
			result = 1;
	}
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_SYSCTL_HW_AVAILCPU_HW_NCPU */

#ifdef posix_cpu_count_USE_SYSCTL_HW_AVAILCPU
	int mib[4], result = 0;
	size_t len = sizeof(result);
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;
	sysctl(mib, 2, &result, &len, NULL, 0);
	if (result <= 0)
		result = 1;
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_SYSCTL_HW_AVAILCPU */

#ifdef posix_cpu_count_USE_SYSCTL_HW_NCPU
	int mib[4], result = 0;
	size_t len = sizeof(result);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	sysctl(mib, 2, &result, &len, NULL, 0);
	if (result <= 0)
		result = 1;
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_SYSCTL_HW_NCPU */

#ifdef posix_cpu_count_USE_MPCTL_MPC_GETNUMSPUS
	int result;
	result = mpctl(MPC_GETNUMSPUS, NULL, NULL);
	if unlikely(result <= 0)
		result = 1; /* Shouldn't happen... */
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_MPCTL_MPC_GETNUMSPUS */

#ifdef posix_cpu_count_USE_SYSCONF_SC_NPROC_ONLN
	int result;
	result = sysconf(_SC_NPROC_ONLN);
	if unlikely(result <= 0)
		result = 1; /* Shouldn't happen... */
	return DeeInt_NewUInt((unsigned int)result);
#endif /* posix_cpu_count_USE_SYSCONF_SC_NPROC_ONLN */

#ifdef posix_cpu_count_USE_PROC_CPUINFO
	unsigned int result = 0;
	DREF DeeObject *file;
	file = DeeFile_OpenString("/proc/cpuinfo",
	                          OPEN_FRDONLY, 0);
	if unlikely(!file)
		goto fallback;
	/* Count the # of lines that begin with `processor'
	 * The cpuinfo file contains one such line for every configured processor on the system. */
	for (;;) {
		PRIVATE char const str_processor[] = "processor";
		DREF DeeBytesObject *line;
		line = (DREF DeeBytesObject *)DeeFile_ReadLine(file, (size_t)-1, false);
		if (!ITER_ISOK(line)) {
			if unlikely(!line)
				goto fallback_fp;
			break;
		}
		ASSERT(DeeBytes_Check(line));
		if (DeeBytes_SIZE(line) >= COMPILER_STRLEN(str_processor) &&
		    memcmp(DeeBytes_DATA(line), str_processor,
		           COMPILER_STRLEN(str_processor) * sizeof(char)) == 0)
			++result; /* Found one! */
		Dee_Decref(line);
	}
	Dee_Decref_likely(file);
	if unlikely(!result)
		result = 1; /* Shouldn't happen... */
	return DeeInt_NewUInt(result);
fallback_fp:
	Dee_Decref_likely(file);
fallback:
	DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
	return_reference(&DeeInt_One);
#endif /* posix_cpu_count_USE_PROC_CPUINFO */

#ifdef posix_cpu_count_USE_STUB
	return_reference(&DeeInt_One);
#endif /* posix_cpu_count_USE_STUB */
}




DECL_END

#endif /* !GUARD_DEX_POSIX_P_CPUCOUNT_C_INL */
