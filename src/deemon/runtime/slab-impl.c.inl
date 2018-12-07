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
#ifdef __INTELLISENSE__
#include "slab.c.inl"
#define SIZE         10
//#define NEXT_LARGER  5
#endif
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

#define FUNC3(x,y) x ## y
#define FUNC2(x,y) FUNC3(x,y)
#define FUNC(x) FUNC2(x,SIZE)

#define ITEMSIZE (SIZE * __SIZEOF_POINTER__)
#define INDEX     DEEMON_SLAB_INDEXOF(ITEMSIZE)

#if 0
#define LOG_SLAB(...)  DEE_DPRINTF(__VA_ARGS__)
#else
#define LOG_SLAB(...) (void)0
#endif


DECL_BEGIN

#if !defined(CONFIG_NO_OBJECT_SLABS) && 1

#define SLAB_RAW_PAGECOUNT         (PAGESIZE / ITEMSIZE)
#define SLAB_INUSE_BITSET_LENGTH  ((SLAB_RAW_PAGECOUNT + (__SIZEOF_POINTER__ * 8 - 1)) / (__SIZEOF_POINTER__ * 8))
#define SLAB_INUSE_BITSET_SIZE     (SLAB_INUSE_BITSET_LENGTH * __SIZEOF_POINTER__)
#define SLAB_PAGECOUNT            ((PAGESIZE - (SLAB_INUSE_BITSET_SIZE + 3 * __SIZEOF_POINTER__)) / ITEMSIZE)

typedef struct {
    uint8_t si_data[ITEMSIZE];
} FUNC(SlabItem);


#define SLAB_PAGE_INVALID    ((FUNC(SlabPage) *)-1)
typedef struct FUNC(slab_page) FUNC(SlabPage);
struct FUNC(slab_page) {
    FUNC(SlabItem)        sp_items[SLAB_PAGECOUNT];
    FUNC(SlabPage)      **sp_pself; /* [1..1][1..1] Page self-pointer. */
    FUNC(SlabPage)       *sp_next;  /* [0..1] Next page (of the same type; aka. free or full). */
    ATOMIC_DATA uintptr_t sp_free;  /* Number of free items in this page. */
    ATOMIC_DATA uintptr_t sp_inuse[SLAB_INUSE_BITSET_LENGTH]; /* Bitset of items that are currently in-use. */
};

STATIC_ASSERT(sizeof(FUNC(SlabPage)) <= PAGESIZE);


typedef struct {
#ifndef CONFIG_NO_THREADS
    rwlock_t        s_lock; /* Lock for this slab */
#endif /* !CONFIG_NO_THREADS */
    FUNC(SlabPage) *s_free; /* [0..1|null(SLAB_PAGE_INVALID)][lock(s_lock)] Chain of free pages */
    FUNC(SlabPage) *s_full; /* [0..1|null(SLAB_PAGE_INVALID)][lock(s_lock)] Chain of pages that fully in-use */
    FUNC(SlabPage) *s_tail; /* [0..1|null(SLAB_PAGE_INVALID)][lock(s_lock)] Pointer to the next page that should be allocated from the system. */
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
} FUNC(Slab);

PRIVATE FUNC(Slab) FUNC(slab) = {
#ifndef CONFIG_NO_THREADS
    /* .s_lock = */RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
    /* .s_free = */SLAB_PAGE_INVALID,
    /* .s_full = */SLAB_PAGE_INVALID,
    /* .s_tail = */SLAB_PAGE_INVALID
#ifndef CONFIG_NO_OBJECT_SLAB_STATS
    ,
    /* .s_num_free      = */0,
    /* .s_max_free      = */0,
    /* .s_num_alloc     = */0,
    /* .s_max_alloc     = */0,
    /* .s_num_fullpages = */0,
    /* .s_max_fullpages = */0,
    /* .s_num_freepages = */0,
    /* .s_max_freepages = */0
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */
};

#ifndef CONFIG_NO_OBJECT_SLAB_STATS
#define DEC_MAXPAIR(cur,max) \
    ATOMIC_FETCHDEC(cur)
#define INC_MAXPAIR(cur,max) \
do{ size_t newval = ATOMIC_INCFETCH(cur); \
    size_t oldval; \
    for (;;) { \
        oldval = ATOMIC_READ(max); \
        if (newval < oldval) break; \
        if (ATOMIC_CMPXCH_WEAK(max,oldval,newval)) break; \
    } \
}__WHILE0
#define ADD_MAXPAIR(cur,max,count) \
do{ size_t newval = ATOMIC_ADDFETCH(cur,count); \
    size_t oldval; \
    for (;;) { \
        oldval = ATOMIC_READ(max); \
        if (newval < oldval) break; \
        if (ATOMIC_CMPXCH_WEAK(max,oldval,newval)) break; \
    } \
}__WHILE0
#else /* !CONFIG_NO_OBJECT_SLAB_STATS */
#define DEC_MAXPAIR(cur,max)       (void)0
#define INC_MAXPAIR(cur,max)       do{}__WHILE0
#define ADD_MAXPAIR(cur,max,count) do{}__WHILE0
#endif /* CONFIG_NO_OBJECT_SLAB_STATS */

#define MY_REGION_START slab_config.sc_regions[INDEX].sr_start
#ifdef NEXT_LARGER
#define MY_REGION_END   slab_config.sc_regions[INDEX + 1].sr_start
#else
#define MY_REGION_END   slab_config.sc_heap_end
#endif


#ifdef NDEBUG
#define INIT_DEBUG(ptr) (void)0
#define FINI_DEBUG(ptr) (void)0
#else
#define INIT_DEBUG(ptr) memset(ptr,0xcc,ITEMSIZE)
#define FINI_DEBUG(ptr) memset(ptr,0xde,ITEMSIZE)
#endif




LOCAL void DCALL
FUNC(DeeSlab_StatSlab)(DeeSlabInfo *__restrict info) {
 size_t total_pages; uintptr_t tail;
 info->si_slabstart      = MY_REGION_START;
 info->si_slabend        = MY_REGION_END;
 info->si_itemsize       = ITEMSIZE;
 info->si_items_per_page = SLAB_PAGECOUNT;
 total_pages = (MY_REGION_END - MY_REGION_START) / PAGESIZE;
 info->si_totalpages = total_pages;
 info->si_totalitems = total_pages * SLAB_PAGECOUNT;
#ifdef CONFIG_NO_OBJECT_SLAB_STATS
 rwlock_read(&FUNC(slab).s_lock);
 {
  size_t freepages = 0;
  size_t freeitems = 0;
  FUNC(SlabPage) *iter = FUNC(slab).s_free;
  tail = (uintptr_t)ATOMIC_READ(FUNC(slab).s_tail);
  for (; iter; iter = iter->sp_next) {
   freeitems += ATOMIC_READ(iter->sp_free);
   ++freepages;
  }
  info->si_usedpages = (tail - MY_REGION_START) / PAGESIZE;
  info->si_tailpages = (MY_REGION_END - tail) / PAGESIZE;
  info->si_cur_fullpages = info->si_usedpages - freepages;
  info->si_cur_freepages = freepages;
  info->si_cur_free = freeitems;
  info->si_cur_alloc = (freepages * SLAB_PAGECOUNT) - freeitems;
  info->si_cur_alloc += info->si_cur_fullpages * SLAB_PAGECOUNT;
 }
 rwlock_endread(&FUNC(slab).s_lock);
 info->si_max_alloc     = info->si_cur_alloc;
 info->si_max_free      = info->si_cur_free;
 info->si_max_fullpages = info->si_cur_fullpages;
 info->si_max_freepages = info->si_cur_freepages;
#else /* CONFIG_NO_OBJECT_SLAB_STATS */
 rwlock_read(&FUNC(slab).s_lock);
read_again:
 COMPILER_READ_BARRIER();
 info->si_cur_alloc     = ATOMIC_READ(FUNC(slab).s_num_alloc);
 info->si_max_alloc     = ATOMIC_READ(FUNC(slab).s_max_alloc);
 info->si_cur_free      = ATOMIC_READ(FUNC(slab).s_num_free);
 info->si_max_free      = ATOMIC_READ(FUNC(slab).s_max_free);
 info->si_max_fullpages = ATOMIC_READ(FUNC(slab).s_max_fullpages);
 info->si_cur_freepages = ATOMIC_READ(FUNC(slab).s_num_freepages);
 info->si_max_freepages = ATOMIC_READ(FUNC(slab).s_max_freepages);
 tail = (uintptr_t)ATOMIC_READ(FUNC(slab).s_tail);
 COMPILER_BARRIER();
 if (info->si_cur_alloc != ATOMIC_READ(FUNC(slab).s_num_alloc))
     goto read_again;
 if (info->si_max_alloc != ATOMIC_READ(FUNC(slab).s_max_alloc))
     goto read_again;
 if (info->si_cur_free != ATOMIC_READ(FUNC(slab).s_num_free))
     goto read_again;
 if (info->si_max_free != ATOMIC_READ(FUNC(slab).s_max_free))
     goto read_again;
 if (info->si_max_fullpages != ATOMIC_READ(FUNC(slab).s_max_fullpages))
     goto read_again;
 if (info->si_cur_freepages != ATOMIC_READ(FUNC(slab).s_num_freepages))
     goto read_again;
 if (info->si_max_freepages != ATOMIC_READ(FUNC(slab).s_max_freepages))
     goto read_again;
 if (tail != (uintptr_t)ATOMIC_READ(FUNC(slab).s_tail))
     goto read_again;
 rwlock_endread(&FUNC(slab).s_lock);
 info->si_usedpages = (tail - MY_REGION_START) / PAGESIZE;
 info->si_tailpages = (MY_REGION_END - tail) / PAGESIZE;
 if (info->si_max_freepages > info->si_usedpages)
     info->si_max_freepages = info->si_usedpages;
 if (info->si_cur_freepages > info->si_usedpages)
     info->si_cur_freepages = info->si_usedpages;
 info->si_cur_fullpages = info->si_usedpages - info->si_cur_freepages;
 if (info->si_max_fullpages < info->si_cur_fullpages)
     info->si_max_fullpages = info->si_cur_fullpages;
 if (info->si_max_freepages < info->si_cur_freepages)
     info->si_max_freepages = info->si_cur_freepages;
 if (info->si_cur_alloc > info->si_cur_freepages * SLAB_PAGECOUNT)
     info->si_cur_alloc = info->si_cur_freepages * SLAB_PAGECOUNT;
 if (info->si_cur_free > info->si_cur_freepages * SLAB_PAGECOUNT)
     info->si_cur_free = info->si_cur_freepages * SLAB_PAGECOUNT;
 if (info->si_max_alloc < info->si_cur_alloc)
     info->si_max_alloc = info->si_cur_alloc;
 if (info->si_max_free < info->si_cur_free)
     info->si_max_free = info->si_cur_free;
 if (info->si_max_alloc > info->si_cur_freepages * SLAB_PAGECOUNT)
     info->si_max_alloc = info->si_cur_freepages * SLAB_PAGECOUNT;
 if (info->si_max_free > info->si_cur_freepages * SLAB_PAGECOUNT)
     info->si_max_free = info->si_cur_freepages * SLAB_PAGECOUNT;
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */
}

#ifndef CONFIG_NO_OBJECT_SLAB_STATS
LOCAL void DCALL
FUNC(DeeSlab_ResetStatSlab)(void) {
 rwlock_write(&FUNC(slab).s_lock);
 ATOMIC_WRITE(FUNC(slab).s_max_free,ATOMIC_READ(FUNC(slab).s_num_free));
 ATOMIC_WRITE(FUNC(slab).s_max_alloc,ATOMIC_READ(FUNC(slab).s_num_alloc));
 ATOMIC_WRITE(FUNC(slab).s_max_fullpages,ATOMIC_READ(FUNC(slab).s_num_fullpages));
 ATOMIC_WRITE(FUNC(slab).s_max_freepages,ATOMIC_READ(FUNC(slab).s_num_freepages));
 rwlock_endwrite(&FUNC(slab).s_lock);
}
#endif /* !CONFIG_NO_OBJECT_SLAB_STATS */


LOCAL WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_DoAlloc))(void) {
 FUNC(SlabPage) *page;
again:
 page = ATOMIC_READ(FUNC(slab).s_free);
 if likely(page != SLAB_PAGE_INVALID) {
  /* Allocate from this page. */
  unsigned int i,j; uintptr_t alloc;
  for (i = 0; i < SLAB_INUSE_BITSET_LENGTH; ++i) {
   uintptr_t mask = ATOMIC_READ(page->sp_inuse[i]);
   if (mask == (uintptr_t)-1)
       continue; /* Fully in use. */
   /* Find the first free bit. */
   for (j = 0,alloc = 1; mask & alloc; ++j,alloc <<= 1);
   if (!ATOMIC_CMPXCH_WEAK(page->sp_inuse[i],mask,mask | alloc))
        goto again;
   /* Got it! */
   if (ATOMIC_DECFETCH(page->sp_free) == 0) {
    /* Try to remove this page from the set of available pages. */
    rwlock_write(&FUNC(slab).s_lock);
    COMPILER_READ_BARRIER();
    if (ATOMIC_READ(page->sp_free) == 0) {
     if ((*page->sp_pself = page->sp_next) != SLAB_PAGE_INVALID)
           page->sp_next->sp_pself = page->sp_pself;
     if ((page->sp_next = FUNC(slab).s_full) != SLAB_PAGE_INVALID)
          FUNC(slab).s_full->sp_pself = &page->sp_next;
     page->sp_pself = &FUNC(slab).s_full;
     FUNC(slab).s_full = page;
     DEC_MAXPAIR(FUNC(slab).s_num_freepages,FUNC(slab).s_max_freepages);
     INC_MAXPAIR(FUNC(slab).s_num_fullpages,FUNC(slab).s_max_fullpages);
    }
    rwlock_endwrite(&FUNC(slab).s_lock);
   }
   DEC_MAXPAIR(FUNC(slab).s_num_free,FUNC(slab).s_max_free);
   INC_MAXPAIR(FUNC(slab).s_num_alloc,FUNC(slab).s_max_alloc);
   LOG_SLAB("[SLAB] Alloc: %p (%Iu bytes)\n",&page->sp_items[(i * (__SIZEOF_POINTER__ * 8)) + j],(size_t)ITEMSIZE);
   return &page->sp_items[(i * (__SIZEOF_POINTER__ * 8)) + j];
  }
  SCHED_YIELD();
  goto again;
 }
 /* Must allocate a new page from the slab tail. */
 page = ATOMIC_READ(FUNC(slab).s_tail);
 if unlikely((uintptr_t)page >= MY_REGION_END)
    return NULL; /* Slab allocator has been fully exhausted */
 /* XXX: Tell the OS that we want to use this page now, and give it a chance
  *      to indicate failure if there is insufficient memory, rather than
  *      giving us a SEGFAULT once we try to initialize it below... */
 if (!ATOMIC_CMPXCH_WEAK(FUNC(slab).s_tail,page,(FUNC(SlabPage) *)((uintptr_t)page + PAGESIZE)))
    goto again;
 COMPILER_BARRIER();
 /* Initialize the new page, and add it as a free one. */
#if SLAB_INUSE_BITSET_LENGTH > 1
 MEMSET_PTR(page->sp_inuse + 1,0,SLAB_INUSE_BITSET_LENGTH - 1);
#endif
 /* Set up the page such that we've already allocated its the first item. */
 page->sp_inuse[0] = 0x0001;
 page->sp_free = SLAB_PAGECOUNT - 1;
 COMPILER_WRITE_BARRIER();
#if SLAB_PAGECOUNT >= 2
 /* Remember the newly allocated page as containing free elements. */
 rwlock_write(&FUNC(slab).s_lock);
 COMPILER_READ_BARRIER();
 if ((page->sp_next = FUNC(slab).s_free) != SLAB_PAGE_INVALID)
      FUNC(slab).s_free->sp_pself = &page->sp_next;
 page->sp_pself = &FUNC(slab).s_free;
 FUNC(slab).s_free = page;
 INC_MAXPAIR(FUNC(slab).s_num_freepages,
             FUNC(slab).s_max_freepages);
 rwlock_endwrite(&FUNC(slab).s_lock);
 INC_MAXPAIR(FUNC(slab).s_num_alloc,
             FUNC(slab).s_max_alloc);
 ADD_MAXPAIR(FUNC(slab).s_num_free,
             FUNC(slab).s_max_free,
             SLAB_PAGECOUNT - 1);
#endif
 return &page->sp_items[0];
}

FORCELOCAL void
(DCALL FUNC(DeeSlab_DoFree))(void *__restrict ptr) {
#if 0
 (void)ptr;
#else
#ifdef NEXT_LARGER
 if ((uintptr_t)ptr < MY_REGION_END)
#endif
 {
  FUNC(SlabPage) *page; unsigned int index,i; uintptr_t mask;
  LOG_SLAB("[SLAB] Free: %p (%Iu bytes)\n",ptr,(size_t)ITEMSIZE);
  page = (FUNC(SlabPage) *)((uintptr_t)ptr & ~(PAGESIZE - 1));
  ASSERTF((((uintptr_t)ptr - (uintptr_t)page) % ITEMSIZE) == 0,
          "Invalid slab-pointer %p is improperly aligned for slab of size %#Ix",
          ptr,(size_t)ITEMSIZE);
  index = ((uintptr_t)ptr - (uintptr_t)page) / ITEMSIZE;
  ASSERTF(index < (unsigned int)SLAB_PAGECOUNT,
          "Invalid slab-pointer %p is part of slab page controller (index = %u/%u)",
          ptr,index,(unsigned int)SLAB_PAGECOUNT);
  FINI_DEBUG(ptr);
  i    = index / (__SIZEOF_POINTER__ * 8);
  ASSERT(i < SLAB_INUSE_BITSET_LENGTH);
  mask = (uintptr_t)1 << (index % (__SIZEOF_POINTER__ * 8));
  /* Clear the in-use bit in the availability bitset. */
#ifdef NDEBUG
  ATOMIC_FETCHAND(page->sp_inuse[i],~mask);
#else
  {
   uintptr_t oldval;
   oldval = ATOMIC_FETCHAND(page->sp_inuse[i],~mask);
   ASSERTF((oldval & mask) != 0,"Item at %p didn't have the in-use bit set",ptr);
  }
#endif
  DEC_MAXPAIR(FUNC(slab).s_num_alloc,FUNC(slab).s_max_alloc);
  INC_MAXPAIR(FUNC(slab).s_num_free,FUNC(slab).s_max_free);
  /* Mark the new item as free */
  if (ATOMIC_FETCHINC(page->sp_free) == 0) {
   /* Add the page to the set to of pages with available items. */
   rwlock_write(&FUNC(slab).s_lock);
   COMPILER_READ_BARRIER();
   if (ATOMIC_READ(page->sp_free) != 0) {
    if ((*page->sp_pself = page->sp_next) != SLAB_PAGE_INVALID)
          page->sp_next->sp_pself = page->sp_pself;
    if ((page->sp_next = FUNC(slab).s_free) != SLAB_PAGE_INVALID)
         FUNC(slab).s_free->sp_pself = &page->sp_next;
    page->sp_pself = &FUNC(slab).s_free;
    FUNC(slab).s_free = page;
    DEC_MAXPAIR(FUNC(slab).s_num_fullpages,FUNC(slab).s_max_fullpages);
    INC_MAXPAIR(FUNC(slab).s_num_freepages,FUNC(slab).s_max_freepages);
   }
   rwlock_endwrite(&FUNC(slab).s_lock);
  }
 }
#ifdef NEXT_LARGER
 else {
  /* Page is actually apart of a larger slab! */
  PP_CAT2(DeeSlab_DoFree,NEXT_LARGER)(ptr);
 }
#endif
#endif
}


#ifdef NDEBUG
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_Malloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (Dee_Malloc)ITEMSIZE;
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_Malloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (DeeDbg_Malloc)(ITEMSIZE,file,line);
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_TryMalloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (Dee_TryMalloc)ITEMSIZE;
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_TryMalloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (DeeDbg_TryMalloc)(ITEMSIZE,file,line);
 return result;
}
#else
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_Malloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (Dee_Malloc)ITEMSIZE;
 else INIT_DEBUG(result);
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_Malloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (DeeDbg_Malloc)(ITEMSIZE,file,line);
 else INIT_DEBUG(result);
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_TryMalloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (Dee_TryMalloc)ITEMSIZE;
 else INIT_DEBUG(result);
 return result;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_TryMalloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 if unlikely(!result)
    result = (DeeDbg_TryMalloc)(ITEMSIZE,file,line);
 else INIT_DEBUG(result);
 return result;
}
#endif
#undef DeeSlab_Calloc
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_Calloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 return likely(result)
      ? memset(result,0,ITEMSIZE)
      : (Dee_Calloc)ITEMSIZE;
}
#undef DeeSlab_TryCalloc
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_TryCalloc))(void) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 return likely(result)
      ? memset(result,0,ITEMSIZE)
      : (Dee_TryCalloc)ITEMSIZE;
}
#undef DeeDbgSlab_Calloc
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_Calloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 return likely(result)
      ? memset(result,0,ITEMSIZE)
      : (DeeDbg_Calloc)(ITEMSIZE,file,line);
}
#undef DeeDbgSlab_TryCalloc
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_TryCalloc))(char const *file, int line) {
 void *result;
 result = FUNC(DeeSlab_DoAlloc)();
 return likely(result)
      ? memset(result,0,ITEMSIZE)
      : (DeeDbg_TryCalloc)(ITEMSIZE,file,line);
}
#undef DeeSlab_Free
PUBLIC void
(DCALL FUNC(DeeSlab_Free))(void *__restrict ptr) {
 if (IS_SLAB_POINTER(ptr))
     FUNC(DeeSlab_DoFree)(ptr);
 else {
  (Dee_Free)(ptr);
 }
}
#undef DeeDbgSlab_Free
PUBLIC void
(DCALL FUNC(DeeDbgSlab_Free))(void *__restrict ptr,
                                    char const *file, int line) {
 if (IS_SLAB_POINTER(ptr))
     FUNC(DeeSlab_DoFree)(ptr);
 else {
  (DeeDbg_Free)(ptr,file,line);
 }
}


#undef INIT_DEBUG
#undef FINI_DEBUG
#undef MY_REGION_START
#undef MY_REGION_END
#undef DEC_MAXPAIR
#undef INC_MAXPAIR
#undef ADD_MAXPAIR
#undef SLAB_PAGE_INVALID
#undef SLAB_RAW_PAGECOUNT
#undef SLAB_INUSE_BITSET_LENGTH
#undef SLAB_INUSE_BITSET_SIZE
#undef SLAB_PAGECOUNT

#else /* !CONFIG_NO_OBJECT_SLABS */
#define MY_SLAB_IS_DISABLED 1
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_Malloc))(void) {
 return (Dee_Malloc)ITEMSIZE;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_Calloc))(void) {
 return (Dee_Calloc)ITEMSIZE;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_TryMalloc))(void) {
 return (Dee_TryMalloc)ITEMSIZE;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeSlab_TryCalloc))(void) {
 return (Dee_TryCalloc)ITEMSIZE;
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_Malloc))(char const *file, int line) {
 return (DeeDbg_Malloc)(ITEMSIZE,file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_Calloc))(char const *file, int line) {
 return (DeeDbg_Calloc)(ITEMSIZE,file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_TryMalloc))(char const *file, int line) {
 return (DeeDbg_TryMalloc)(ITEMSIZE,file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgSlab_TryCalloc))(char const *file, int line) {
 return (DeeDbg_TryCalloc)(ITEMSIZE,file,line);
}
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(FUNC(DeeSlab_Free),4),
                    ASSEMBLY_NAME(Dee_Free,4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(FUNC(DeeDbgSlab_Free),12),
                    ASSEMBLY_NAME(DeeDbg_Free,12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC void (DCALL FUNC(DeeSlab_Free))(void *__restrict ptr) { (Dee_Free)(ptr); }
PUBLIC void (DCALL FUNC(DeeDbgSlab_Free))(void *__restrict ptr, char const *file, int line) { (DeeDbg_Free)(ptr,file,line); }
#endif /* __NO_DEFINE_ALIAS */
#endif /* CONFIG_NO_OBJECT_SLABS */


#if SIZE == DEEMON_SLAB_MINSIZE
#ifndef MY_SLAB_IS_DISABLED
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_Free,4),
                    ASSEMBLY_NAME(FUNC(DeeSlab_Free),4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeDbgObject_Free,12),
                    ASSEMBLY_NAME(FUNC(DeeDbgSlab_Free),12));
#else
PUBLIC void (DCALL DeeObject_Free)(void *ptr) { (FUNC(DeeSlab_Free))(ptr); }
PUBLIC void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line) { (FUNC(DeeDbgSlab_Free))(ptr,file,line); }
#endif
#else /* !MY_SLAB_IS_DISABLED */
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_Free,4),
                    ASSEMBLY_NAME(Dee_Free,4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeDbgObject_Free,12),
                    ASSEMBLY_NAME(DeeDbg_Free,12));
#else
PUBLIC void (DCALL DeeObject_Free)(void *ptr) { Dee_Free(ptr); }
PUBLIC void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line) { DeeDbg_Free(ptr,file,line); }
#endif
#endif /* MY_SLAB_IS_DISABLED */
#endif /* SIZE == DEEMON_SLAB_MINSIZE */

DECL_END

#undef INDEX
#undef MY_SLAB_IS_DISABLED
#undef NEXT_LARGER
#undef SIZE
#undef ITEMSIZE
#undef LOG_SLAB