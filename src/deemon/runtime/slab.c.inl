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
#ifndef GUARD_DEEMON_RUNTIME_SLAB_C_INL
#define GUARD_DEEMON_RUNTIME_SLAB_C_INL 1

#include "misc.c"

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/gc.h>
#include <hybrid/host.h>

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
#define CONFIG_NO_OBJECT_SLAB_STATS 1
#endif
#elif !defined(CONFIG_NO_OBJECT_SLAB_STATS)
#ifdef NDEBUG
#define CONFIG_NO_OBJECT_SLAB_STATS 1
#endif
#endif


#ifndef CONFIG_NO_OBJECT_SLABS
#if (!defined(__i386__) && !defined(__x86_64__)) || \
    (!defined(CONFIG_HOST_WINDOWS) && !defined(CONFIG_HOST_UNIX))
#define CONFIG_NO_OBJECT_SLABS 1 /* Unrecognized environment (disable slabs) */
#endif
#ifndef CONFIG_NO_OBJECT_SLABS
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#ifndef __NO_has_include
/* Check for dependencies, but default to assuming
 * that we have everything which we might need. */
#if !__has_include(<sys/mman.h>)
#define CONFIG_NO_OBJECT_SLABS 1
#endif
#if !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#if !__has_include(<sys/fcntl.h>)
#define CONFIG_NO_OBJECT_SLABS 1
#endif
#endif
#endif /* __NO_has_include */
#ifndef CONFIG_NO_OBJECT_SLABS
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#include <fcntl.h>
#endif
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_HOST_WINDOWS */

#ifndef CONFIG_NO_OBJECT_SLABS
#include <hybrid/limits.h>

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#ifndef __SIZEOF_POINTER__
#if defined(__x86_64__)
#define __SIZEOF_POINTER__ 8
#else
#define __SIZEOF_POINTER__ 4
#endif
#endif

DECL_BEGIN

typedef struct {
    uintptr_t sr_start;       /* Starting address of this slab region.
                               * NOTE: This pointer is aligned by `PAGESIZE' */
} SlabRegionDef;

typedef struct {
    /* The slab configuration is written to once during program
     * initialization, at which point slab caches get allocated. */
    union {
        SlabRegionDef sc_regions[DEEMON_SLAB_COUNT]; /* Slab region definitions.
                                                      * NOTE: The starting addresses of these are ordered ascendingly! */
        uintptr_t     sc_heap_start;                 /* [const] Starting address of the slab heap.
                                                      * NOTE: This pointer is aligned by `PAGESIZE' */
    };
    uintptr_t         sc_heap_end;   /* [const] End address of the slab heap.
                                      * NOTE: This pointer is aligned by `PAGESIZE' */
} SlabConfig;

PRIVATE SlabConfig slab_config = {
    {
         {
#define INIT_REGION(x)  { (uintptr_t)-1 },
              DEE_ENUMERATE_SLAB_SIZES(INIT_REGION)
#undef INIT_REGION
         }
    },
    /* .sc_heap_end = */0
};

#define IS_SLAB_POINTER(p) \
      ((uintptr_t)(p) >= slab_config.sc_heap_start && \
       (uintptr_t)(p) <  slab_config.sc_heap_end)


DECL_END
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_NO_OBJECT_SLABS */

#ifndef __INTELLISENSE__
/*[[[deemon
for (local x = 20; x >= 2; x = x - 1) {
    print "#if DEEMON_SLAB_HASSIZE(" + x + ")";
    print "#define SIZE",x;
    print "#include \"slab-impl.c.inl\"";
    print "#define NEXT_LARGER",x;
    print "#endif /" "* DEEMON_SLAB_HASSIZE(" + x + ") *" "/";
}
print "#undef NEXT_LARGER";
]]]*/
#if DEEMON_SLAB_HASSIZE(20)
#define SIZE 20
#include "slab-impl.c.inl"
#define NEXT_LARGER 20
#endif /* DEEMON_SLAB_HASSIZE(20) */
#if DEEMON_SLAB_HASSIZE(19)
#define SIZE 19
#include "slab-impl.c.inl"
#define NEXT_LARGER 19
#endif /* DEEMON_SLAB_HASSIZE(19) */
#if DEEMON_SLAB_HASSIZE(18)
#define SIZE 18
#include "slab-impl.c.inl"
#define NEXT_LARGER 18
#endif /* DEEMON_SLAB_HASSIZE(18) */
#if DEEMON_SLAB_HASSIZE(17)
#define SIZE 17
#include "slab-impl.c.inl"
#define NEXT_LARGER 17
#endif /* DEEMON_SLAB_HASSIZE(17) */
#if DEEMON_SLAB_HASSIZE(16)
#define SIZE 16
#include "slab-impl.c.inl"
#define NEXT_LARGER 16
#endif /* DEEMON_SLAB_HASSIZE(16) */
#if DEEMON_SLAB_HASSIZE(15)
#define SIZE 15
#include "slab-impl.c.inl"
#define NEXT_LARGER 15
#endif /* DEEMON_SLAB_HASSIZE(15) */
#if DEEMON_SLAB_HASSIZE(14)
#define SIZE 14
#include "slab-impl.c.inl"
#define NEXT_LARGER 14
#endif /* DEEMON_SLAB_HASSIZE(14) */
#if DEEMON_SLAB_HASSIZE(13)
#define SIZE 13
#include "slab-impl.c.inl"
#define NEXT_LARGER 13
#endif /* DEEMON_SLAB_HASSIZE(13) */
#if DEEMON_SLAB_HASSIZE(12)
#define SIZE 12
#include "slab-impl.c.inl"
#define NEXT_LARGER 12
#endif /* DEEMON_SLAB_HASSIZE(12) */
#if DEEMON_SLAB_HASSIZE(11)
#define SIZE 11
#include "slab-impl.c.inl"
#define NEXT_LARGER 11
#endif /* DEEMON_SLAB_HASSIZE(11) */
#if DEEMON_SLAB_HASSIZE(10)
#define SIZE 10
#include "slab-impl.c.inl"
#define NEXT_LARGER 10
#endif /* DEEMON_SLAB_HASSIZE(10) */
#if DEEMON_SLAB_HASSIZE(9)
#define SIZE 9
#include "slab-impl.c.inl"
#define NEXT_LARGER 9
#endif /* DEEMON_SLAB_HASSIZE(9) */
#if DEEMON_SLAB_HASSIZE(8)
#define SIZE 8
#include "slab-impl.c.inl"
#define NEXT_LARGER 8
#endif /* DEEMON_SLAB_HASSIZE(8) */
#if DEEMON_SLAB_HASSIZE(7)
#define SIZE 7
#include "slab-impl.c.inl"
#define NEXT_LARGER 7
#endif /* DEEMON_SLAB_HASSIZE(7) */
#if DEEMON_SLAB_HASSIZE(6)
#define SIZE 6
#include "slab-impl.c.inl"
#define NEXT_LARGER 6
#endif /* DEEMON_SLAB_HASSIZE(6) */
#if DEEMON_SLAB_HASSIZE(5)
#define SIZE 5
#include "slab-impl.c.inl"
#define NEXT_LARGER 5
#endif /* DEEMON_SLAB_HASSIZE(5) */
#if DEEMON_SLAB_HASSIZE(4)
#define SIZE 4
#include "slab-impl.c.inl"
#define NEXT_LARGER 4
#endif /* DEEMON_SLAB_HASSIZE(4) */
#if DEEMON_SLAB_HASSIZE(3)
#define SIZE 3
#include "slab-impl.c.inl"
#define NEXT_LARGER 3
#endif /* DEEMON_SLAB_HASSIZE(3) */
#if DEEMON_SLAB_HASSIZE(2)
#define SIZE 2
#include "slab-impl.c.inl"
#define NEXT_LARGER 2
#endif /* DEEMON_SLAB_HASSIZE(2) */
#undef NEXT_LARGER
//[[[end]]]
#endif

#ifndef CONFIG_NO_OBJECT_SLABS
#include <stdlib.h>
#include <hybrid/overflow.h>
#endif /* !CONFIG_NO_OBJECT_SLABS */

DECL_BEGIN

#ifdef CONFIG_NO_OBJECT_SLABS
INTERN void DCALL DeeSlab_Finalize(void) {
 /* nothing */
}
INTERN void DCALL DeeSlab_Initialize(void) {
 /* nothing */
}
#else /* CONFIG_NO_OBJECT_SLABS */

#ifdef __INTELLISENSE__
struct {
#ifndef CONFIG_NO_THREADS
    rwlock_t  s_lock; /* Lock for this slab */
#endif
     void    *s_free; /* [0..1][lock(s_lock)] Chain of free pages */
     void    *s_full; /* [0..1][lock(s_lock)] Chain of pages that fully in-use */
     void    *s_tail; /* [0..1][lock(s_lock)] Pointer to the next page that should be allocated from the system. */
#ifndef CONFIG_NO_OBJECT_SLAB_STATS
    ATOMIC_DATA size_t s_num_free;      /* Number of items currently marked as free. */
    ATOMIC_DATA size_t s_max_free;      /* Max number of items ever marked as free. */
    ATOMIC_DATA size_t s_num_alloc;     /* Number of items currently allocated from this slab. */
    ATOMIC_DATA size_t s_max_alloc;     /* Max number of items that were ever allocated at once. */
    ATOMIC_DATA size_t s_num_fullpages; /* Number of full pages currently allocated from this slab. */
    ATOMIC_DATA size_t s_max_fullpages; /* Max number of full pages that were ever allocated at once. */
    ATOMIC_DATA size_t s_num_freepages; /* Number of pages containing unused items. */
    ATOMIC_DATA size_t s_max_freepages; /* Max number of pages containing unused items to ever exist. */
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */
}
#define DEFINE_SLAB_VAR(x) slab ## x,
    DEE_ENUMERATE_SLAB_SIZES(DEFINE_SLAB_VAR)
#undef DEFINE_SLAB_VAR
    _slab_intellisense_trailing
;

#endif

/* Default sizes for slabs (in whole pages) */
PRIVATE size_t const default_slab_sizes[DEEMON_SLAB_COUNT] = {
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
PUBLIC size_t DCALL
DeeSlab_Stat(DeeSlabStat *info, size_t bufsize) {
 if (bufsize >= sizeof(DeeSlabStat)) {
  info->st_slabcount = DEEMON_SLAB_COUNT;
#define GATHER_INFO(x) \
  DeeSlab_StatSlab ## x(&info->st_slabs[DEEMON_SLAB_INDEXOF(x * __SIZEOF_POINTER__)]);
  DEE_ENUMERATE_SLAB_SIZES(GATHER_INFO)
#undef GATHER_INFO
 }
 return sizeof(DeeSlabStat);
}

/* Reset the slab max-statistics to the cur-values. */
PUBLIC void DCALL DeeSlab_ResetStat(void) {
#define RESET_INFO(x) \
 DeeSlab_ResetStatSlab ## x();
 DEE_ENUMERATE_SLAB_SIZES(RESET_INFO)
#undef RESET_INFO
}



INTERN void DCALL DeeSlab_Finalize(void) {
 if (slab_config.sc_heap_start == (uintptr_t)-1)
     return;
#ifdef CONFIG_HOST_WINDOWS
 VirtualFree((LPVOID)slab_config.sc_heap_start,
             (SIZE_T)(slab_config.sc_heap_end - slab_config.sc_heap_start),
              MEM_DECOMMIT);
#elif defined(CONFIG_HOST_UNIX)
 munmap((void *)slab_config.sc_heap_start,
        (size_t)(slab_config.sc_heap_end - slab_config.sc_heap_start));
#else
#error "Invalid SLAB configuration"
#endif
}

INTERN void DCALL DeeSlab_Initialize(void) {
 size_t total; unsigned int i;
 size_t sizes[DEEMON_SLAB_COUNT]; char *config;
 memcpy(sizes,default_slab_sizes,sizeof(default_slab_sizes));
 config = getenv("DEEMON_SLABS");
 if (config && *config) {
  /* Load slab sizes from the configuration string, which is
   * a comma-separated list of the slab sizes that should be
   * used. */
#if DEEMON_SLAB_COUNT == 5
  sscanf(config,"%u,%u,%u,%u,%u",
        &sizes[0],&sizes[1],&sizes[2],
        &sizes[3],&sizes[4]);
#else
#error FIXME
#endif
 }
 for (i = 0,total = 0; i < DEEMON_SLAB_COUNT; ++i) {
  if (OVERFLOW_UADD(total,sizes[i],&total))
      goto disable_slabs;
  total += sizes[i];
 }
 if (!total) goto disable_slabs;
 if (OVERFLOW_UMUL(total,PAGESIZE,&total))
     goto disable_slabs;
 {
  void *slab_memory;
#ifdef CONFIG_HOST_WINDOWS
  slab_memory = VirtualAlloc(NULL,
                             total,
                             MEM_COMMIT | MEM_RESERVE,
                             PAGE_READWRITE);
  if (!slab_memory)
       goto disable_slabs;
#elif defined(CONFIG_HOST_UNIX)
#if !defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE 0
#endif
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
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
#else
  {
   int fd = open("/dev/null",O_RDONLY);
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
#endif
  if unlikely(slab_memory == MAP_FAILED)
     goto disable_slabs;
#else
#error "Invalid SLAB configuration"
#endif
  slab_config.sc_heap_end = (uintptr_t)slab_memory + total;
  for (i = 0,total = 0; i < DEEMON_SLAB_COUNT; ++i) {
   slab_config.sc_regions[i].sr_start = (uintptr_t)slab_memory;
   if (OVERFLOW_UADD(total,sizes[i],&total))
       goto disable_slabs;
   slab_memory = (void *)((uintptr_t)slab_memory + sizes[i] * PAGESIZE);
  }
#define SET_SLAB_STARTING_PAGE(x) \
  *(void **)&slab ## x.s_free = (void *)(uintptr_t)-1l; \
  *(void **)&slab ## x.s_full = (void *)(uintptr_t)-1l; \
  *(void **)&slab ## x.s_tail = (void *)slab_config.sc_regions[DEEMON_SLAB_INDEXOF(x * __SIZEOF_POINTER__)].sr_start;
  DEE_ENUMERATE_SLAB_SIZES(SET_SLAB_STARTING_PAGE)
#undef SET_SLAB_STARTING_PAGE
 }
 return;
disable_slabs:
 memset(&slab_config,0xff,offsetof(SlabConfig,sc_heap_end));
 slab_config.sc_heap_end = 0;
#define SET_SLAB_STARTING_PAGE(x) \
 *(void **)&slab ## x.s_free = (void *)(uintptr_t)-1l; \
 *(void **)&slab ## x.s_full = (void *)(uintptr_t)-1l; \
 *(void **)&slab ## x.s_tail = (void *)(uintptr_t)-1l;
 DEE_ENUMERATE_SLAB_SIZES(SET_SLAB_STARTING_PAGE)
#undef SET_SLAB_STARTING_PAGE
}

#endif /* !CONFIG_NO_OBJECT_SLABS */

DECL_END


#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C_INL */
