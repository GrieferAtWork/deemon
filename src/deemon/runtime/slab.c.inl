/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_SLAB_C_INL
#define GUARD_DEEMON_RUNTIME_SLAB_C_INL 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* sscanf(), bzero(), ... */
#include <deemon/util/lock.h>

#include <hybrid/host.h>
#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include "misc.c"

#ifndef CONFIG_NO_OBJECT_SLABS

#undef NO_OBJECT_SLABS
#undef Dee_Malloc
#undef Dee_Calloc
#undef Dee_Realloc
#undef Dee_TryMalloc
#undef Dee_TryCalloc
#undef Dee_TryRealloc
#undef Dee_Free
#undef DeeObject_Free

#ifdef CONFIG_OBJECT_SLAB_STATS
#undef CONFIG_NO_OBJECT_SLAB_STATS
#if (CONFIG_OBJECT_SLAB_STATS+0) == 0
#undef CONFIG_OBJECT_SLAB_STATS
#define CONFIG_NO_OBJECT_SLAB_STATS
#endif /* CONFIG_OBJECT_SLAB_STATS == 0 */
#elif !defined(CONFIG_NO_OBJECT_SLAB_STATS)
#ifdef NDEBUG
#define CONFIG_NO_OBJECT_SLAB_STATS
#endif /* !NDEBUG */
#endif /* CONFIG_OBJECT_SLAB_STATS... */


#ifndef NO_OBJECT_SLABS
#if ((!defined(__i386__) && !defined(__x86_64__)) || \
     (!defined(CONFIG_HOST_WINDOWS) && !defined(CONFIG_HOST_UNIX)))
#define NO_OBJECT_SLABS 1 /* Unrecognized environment (disable slabs) */
#endif /* ... */
#ifndef NO_OBJECT_SLABS
#ifdef CONFIG_HOST_WINDOWS
#define USE_WINDOWS_VIRTUALALLOC 1
#include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#include <deemon/system-features.h>
#if !defined(CONFIG_HAVE_SYS_MMAN_H) || !defined(CONFIG_HAVE_mmap)
#define NO_OBJECT_SLABS 1
#elif !defined(MAP_ANONYMOUS) && !defined(MAP_ANON) && !defined(CONFIG_HAVE_open)
#define NO_OBJECT_SLABS 1
#endif
#ifndef NO_OBJECT_SLABS
#define USE_UNIX_MMAP 1
#endif /* !NO_OBJECT_SLABS */
#endif /* !CONFIG_HOST_WINDOWS */

#ifndef NO_OBJECT_SLABS

/* Figure out the alignment in which to allocate slab pages. */
#ifndef CONFIG_SLAB_PAGESIZE
#include <hybrid/host.h>
#ifdef __ARCH_PAGESIZE
#define CONFIG_SLAB_PAGESIZE __ARCH_PAGESIZE
#elif defined(__ARCH_PAGESIZE_MIN)
#define CONFIG_SLAB_PAGESIZE __ARCH_PAGESIZE_MIN
#else
#define CONFIG_SLAB_PAGESIZE 4096
#endif
#endif /* !CONFIG_SLAB_PAGESIZE */

DECL_BEGIN

typedef struct {
	uintptr_t sr_start;       /* Starting address of this slab region.
	                           * NOTE: This pointer is aligned by `CONFIG_SLAB_PAGESIZE' */
} SlabRegionDef;

typedef struct {
	/* The slab configuration is written to once during program
	 * initialization, at which point slab caches get allocated. */
	union {
		SlabRegionDef sc_regions[Dee_SLAB_COUNT]; /* Slab region definitions.
		                                              * NOTE: The starting addresses of these are ordered ascendingly! */
		uintptr_t     sc_heap_start;                 /* [const] Starting address of the slab heap.
		                                              * NOTE: This pointer is aligned by `CONFIG_SLAB_PAGESIZE' */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define sc_regions    _dee_aunion.sc_regions
#define sc_heap_start _dee_aunion.sc_heap_start
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	uintptr_t         sc_heap_end;   /* [const] End address of the slab heap.
	                                  * NOTE: This pointer is aligned by `CONFIG_SLAB_PAGESIZE' */
} SlabConfig;

PRIVATE SlabConfig slab_config = {
	{
		{
#define INIT_REGION(index, size)  { (uintptr_t)-1 },
			DeeSlab_ENUMERATE(INIT_REGION)
#undef INIT_REGION
		}
	},
	/* .sc_heap_end = */ 0
};

#define IS_SLAB_POINTER(p)                          \
	((uintptr_t)(p) >= slab_config.sc_heap_start && \
	 (uintptr_t)(p) < slab_config.sc_heap_end)


DECL_END
#endif /* !NO_OBJECT_SLABS */
#endif /* !NO_OBJECT_SLABS */
#endif /* !NO_OBJECT_SLABS */


#ifndef __INTELLISENSE__
/*[[[deemon
for (local x = 20; x >= 2; x = x - 1) {
	print "#if DeeSlab_HasSize(" + x + ")";
	print "#define SIZE", x;
	print "#include \"slab-impl.c.inl\"";
	print "#define NEXT_LARGER", x;
	print "#endif /" "* DeeSlab_HasSize(" + x + ") *" "/";
}
print "#undef NEXT_LARGER";
]]]*/
#if DeeSlab_HasSize(20)
#define SIZE 20
#include "slab-impl.c.inl"
#define NEXT_LARGER 20
#endif /* DeeSlab_HasSize(20) */
#if DeeSlab_HasSize(19)
#define SIZE 19
#include "slab-impl.c.inl"
#define NEXT_LARGER 19
#endif /* DeeSlab_HasSize(19) */
#if DeeSlab_HasSize(18)
#define SIZE 18
#include "slab-impl.c.inl"
#define NEXT_LARGER 18
#endif /* DeeSlab_HasSize(18) */
#if DeeSlab_HasSize(17)
#define SIZE 17
#include "slab-impl.c.inl"
#define NEXT_LARGER 17
#endif /* DeeSlab_HasSize(17) */
#if DeeSlab_HasSize(16)
#define SIZE 16
#include "slab-impl.c.inl"
#define NEXT_LARGER 16
#endif /* DeeSlab_HasSize(16) */
#if DeeSlab_HasSize(15)
#define SIZE 15
#include "slab-impl.c.inl"
#define NEXT_LARGER 15
#endif /* DeeSlab_HasSize(15) */
#if DeeSlab_HasSize(14)
#define SIZE 14
#include "slab-impl.c.inl"
#define NEXT_LARGER 14
#endif /* DeeSlab_HasSize(14) */
#if DeeSlab_HasSize(13)
#define SIZE 13
#include "slab-impl.c.inl"
#define NEXT_LARGER 13
#endif /* DeeSlab_HasSize(13) */
#if DeeSlab_HasSize(12)
#define SIZE 12
#include "slab-impl.c.inl"
#define NEXT_LARGER 12
#endif /* DeeSlab_HasSize(12) */
#if DeeSlab_HasSize(11)
#define SIZE 11
#include "slab-impl.c.inl"
#define NEXT_LARGER 11
#endif /* DeeSlab_HasSize(11) */
#if DeeSlab_HasSize(10)
#define SIZE 10
#include "slab-impl.c.inl"
#define NEXT_LARGER 10
#endif /* DeeSlab_HasSize(10) */
#if DeeSlab_HasSize(9)
#define SIZE 9
#include "slab-impl.c.inl"
#define NEXT_LARGER 9
#endif /* DeeSlab_HasSize(9) */
#if DeeSlab_HasSize(8)
#define SIZE 8
#include "slab-impl.c.inl"
#define NEXT_LARGER 8
#endif /* DeeSlab_HasSize(8) */
#if DeeSlab_HasSize(7)
#define SIZE 7
#include "slab-impl.c.inl"
#define NEXT_LARGER 7
#endif /* DeeSlab_HasSize(7) */
#if DeeSlab_HasSize(6)
#define SIZE 6
#include "slab-impl.c.inl"
#define NEXT_LARGER 6
#endif /* DeeSlab_HasSize(6) */
#if DeeSlab_HasSize(5)
#define SIZE 5
#include "slab-impl.c.inl"
#define NEXT_LARGER 5
#endif /* DeeSlab_HasSize(5) */
#if DeeSlab_HasSize(4)
#define SIZE 4
#include "slab-impl.c.inl"
#define NEXT_LARGER 4
#endif /* DeeSlab_HasSize(4) */
#if DeeSlab_HasSize(3)
#define SIZE 3
#include "slab-impl.c.inl"
#define NEXT_LARGER 3
#endif /* DeeSlab_HasSize(3) */
#if DeeSlab_HasSize(2)
#define SIZE 2
#include "slab-impl.c.inl"
#define NEXT_LARGER 2
#endif /* DeeSlab_HasSize(2) */
#undef NEXT_LARGER
//[[[end]]]
#endif /* __INTELLISENSE__ */

#ifndef NO_OBJECT_SLABS
#include <hybrid/overflow.h>
#endif /* !NO_OBJECT_SLABS */

DECL_BEGIN

#ifdef NO_OBJECT_SLABS
INTERN void DCALL DeeSlab_Finalize(void) {
	/* nothing */
}

INTERN void DCALL DeeSlab_Initialize(void) {
	/* nothing */
}
#else /* NO_OBJECT_SLABS */

#ifdef __INTELLISENSE__
struct {
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t s_lock; /* Lock for this slab */
#endif /* !CONFIG_NO_THREADS */
	void             *s_free; /* [0..1][lock(s_lock)] Chain of free pages */
	void             *s_full; /* [0..1][lock(s_lock)] Chain of pages that fully in-use */
	void             *s_tail; /* [0..1][lock(s_lock)] Pointer to the next page that should be allocated from the system. */
#ifndef CONFIG_NO_OBJECT_SLAB_STATS
	DWEAK size_t      s_num_free;      /* Number of items currently marked as free. */
	DWEAK size_t      s_max_free;      /* Max number of items ever marked as free. */
	DWEAK size_t      s_num_alloc;     /* Number of items currently allocated from this slab. */
	DWEAK size_t      s_max_alloc;     /* Max number of items that were ever allocated at once. */
	DWEAK size_t      s_num_fullpages; /* Number of full pages currently allocated from this slab. */
	DWEAK size_t      s_max_fullpages; /* Max number of full pages that were ever allocated at once. */
	DWEAK size_t      s_num_freepages; /* Number of pages containing unused items. */
	DWEAK size_t      s_max_freepages; /* Max number of pages containing unused items to ever exist. */
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */
}
#define DEFINE_SLAB_VAR(index, size) slab##size,
	DeeSlab_ENUMERATE(DEFINE_SLAB_VAR)
#undef DEFINE_SLAB_VAR
	_slab_intellisense_trailing
;

#endif

/* Default sizes for slabs (in whole pages) */
PRIVATE size_t const default_slab_sizes[Dee_SLAB_COUNT] = {
	/* XXX: Tweak these numbers according to statistics. */
	16,  /* 4  -- 16 / 32 */
	8,   /* 5  -- 20 / 40 */
	8,   /* 6  -- 24 / 48 */
	64,  /* 8  -- 32 / 64 */
	48,  /* 10 -- 40 / 80 */
};

/* Collect slab information and write that information to `info'
 * When `bufsize' is smaller that the required buffer size to write
 * all known slab information, the contents of `info' are undefined,
 * and the required size is returned (which is then `> bufsize')
 * Otherwise, `info' is filled with slab statistic information, and
 * the used buffer size is returned (which is then `<= bufsize')
 * In no case will this function throw an error.
 * When deemon has been built with `CONFIG_NO_OBJECT_SLAB_STATS',
 * this function will be significantly slower, and all max-fields
 * are set to match the cur-fields. */
PUBLIC NONNULL((1)) size_t DCALL
DeeSlab_Stat(DeeSlabStat *info, size_t bufsize) {
	if (bufsize >= sizeof(DeeSlabStat)) {
		info->st_slabcount = Dee_SLAB_COUNT;
#define GATHER_INFO(index, size) \
		DeeSlab_StatSlab##size(&info->st_slabs[index]);
		DeeSlab_ENUMERATE(GATHER_INFO)
#undef GATHER_INFO
	}
	return sizeof(DeeSlabStat);
}

/* Reset the slab max-statistics to the cur-values. */
PUBLIC void DCALL DeeSlab_ResetStat(void) {
#ifndef CONFIG_NO_OBJECT_SLAB_STATS
#define RESET_INFO(index, size) \
	DeeSlab_ResetStatSlab##size();
	DeeSlab_ENUMERATE(RESET_INFO)
#undef RESET_INFO
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */
}



INTERN void DCALL DeeSlab_Finalize(void) {
	if (slab_config.sc_heap_start == (uintptr_t)-1)
		return;
#ifdef USE_WINDOWS_VIRTUALALLOC
	VirtualFree((LPVOID)slab_config.sc_heap_start,
	            (SIZE_T)(slab_config.sc_heap_end - slab_config.sc_heap_start),
	            MEM_DECOMMIT);
#elif defined(USE_UNIX_MMAP)
	munmap((void *)slab_config.sc_heap_start,
	       (size_t)(slab_config.sc_heap_end - slab_config.sc_heap_start));
#else
#error "Invalid SLAB configuration"
#endif
}

INTERN void DCALL DeeSlab_Initialize(void) {
	size_t total;
	unsigned int i;
	size_t sizes[Dee_SLAB_COUNT];
	char *config;
	memcpy(sizes, default_slab_sizes,
	       sizeof(default_slab_sizes));
	config = getenv("DEEMON_SLABS");
	if (config && *config) {
		/* Load slab sizes from the configuration string, which is
		 * a comma-separated list of the slab sizes that should be
		 * used. */
#if Dee_SLAB_COUNT == 5
		bzero(sizes, sizeof(sizes));
		/* FIXME: We're not checking if sscanf() really exists!
		 *        There needs to be a fallback for when it doesn't exist! */
		sscanf(config,
		       "%u , %u , %u , %u , %u",
		       (unsigned int *)&sizes[0],
		       (unsigned int *)&sizes[1],
		       (unsigned int *)&sizes[2],
		       (unsigned int *)&sizes[3],
		       (unsigned int *)&sizes[4]);
#else
#error FIXME
#endif
	}
	for (i = 0, total = 0; i < Dee_SLAB_COUNT; ++i) {
		if (OVERFLOW_UADD(total, sizes[i], &total))
			goto disable_slabs;
		total += sizes[i];
	}
	if (!total)
		goto disable_slabs;
	if (OVERFLOW_UMUL(total, CONFIG_SLAB_PAGESIZE, &total))
		goto disable_slabs;
	{
		void *slab_memory;
#ifdef USE_WINDOWS_VIRTUALALLOC
		slab_memory = VirtualAlloc(NULL,
		                           total,
		                           MEM_COMMIT | MEM_RESERVE,
		                           PAGE_READWRITE);
		if (!slab_memory)
			goto disable_slabs;
#elif defined(USE_UNIX_MMAP)
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif /* !MAP_ANONYMOUS && MAP_ANON */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)(uintptr_t)-1)
#endif /* !MAP_FAILED */
#ifndef MAP_PRIVATE
#define MAP_PRIVATE 0
#endif /* !MAP_PRIVATE */
#ifndef MAP_FILE
#define MAP_FILE 0
#endif /* !MAP_FILE */
		/* XXX: Maybe use `MAP_NORESERVE' to try to keep the kernel from having
		 *      to reserve SWAP space when we may not actually be intending on
		 *      using a majority of allocated slab pages... */
#ifdef MAP_ANONYMOUS
		slab_memory = (void *)mmap(NULL,
		                           total,
		                           PROT_READ | PROT_WRITE,
		                           MAP_PRIVATE | MAP_ANONYMOUS,
		                           -1,
		                           0);
#else /* MAP_ANONYMOUS */
		{
			int fd = open("/dev/null", O_RDONLY);
			if unlikely(fd < 0)
				goto disable_slabs;
			slab_memory = (void *)mmap(NULL,
			                           total,
			                           PROT_READ | PROT_WRITE,
			                           MAP_PRIVATE | MAP_FILE,
			                           fd,
			                           0);
			close(fd);
		}
#endif /* !MAP_ANONYMOUS */
		if unlikely(slab_memory == MAP_FAILED)
			goto disable_slabs;
#else
#error "Invalid SLAB configuration"
#endif
		slab_config.sc_heap_end = (uintptr_t)slab_memory + total;
		for (i = 0, total = 0; i < Dee_SLAB_COUNT; ++i) {
			slab_config.sc_regions[i].sr_start = (uintptr_t)slab_memory;
			if (OVERFLOW_UADD(total, sizes[i], &total))
				goto disable_slabs;
			slab_memory = (void *)((uintptr_t)slab_memory + sizes[i] * CONFIG_SLAB_PAGESIZE);
		}
#define SET_SLAB_STARTING_PAGE(index, size)                    \
		*(void **)&slab##size.s_free = (void *)(uintptr_t)-1l; \
		*(void **)&slab##size.s_full = (void *)(uintptr_t)-1l; \
		*(void **)&slab##size.s_tail = (void *)slab_config.sc_regions[index].sr_start;
		DeeSlab_ENUMERATE(SET_SLAB_STARTING_PAGE)
#undef SET_SLAB_STARTING_PAGE
	}
	return;
disable_slabs:
	memset(&slab_config, 0xff, offsetof(SlabConfig, sc_heap_end));
	slab_config.sc_heap_end = 0;
#define SET_SLAB_STARTING_PAGE(index, size)                \
	*(void **)&slab##size.s_free = (void *)(uintptr_t)-1l; \
	*(void **)&slab##size.s_full = (void *)(uintptr_t)-1l; \
	*(void **)&slab##size.s_tail = (void *)(uintptr_t)-1l;
	DeeSlab_ENUMERATE(SET_SLAB_STARTING_PAGE)
#undef SET_SLAB_STARTING_PAGE
}

#endif /* !NO_OBJECT_SLABS */

DECL_END
#endif /* !CONFIG_NO_OBJECT_SLABS */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C_INL */
