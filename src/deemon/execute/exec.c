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
#ifndef GUARD_DEEMON_EXECUTE_EXEC_C
#define GUARD_DEEMON_EXECUTE_EXEC_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/exec.h>
#include <deemon/thread.h>
#include <deemon/file.h>
#include <deemon/list.h>
#include <deemon/dex.h>
#include <deemon/module.h>
#include <deemon/error.h>
#ifndef CONFIG_NO_THREADS
#include <hybrid/sched/yield.h>
#endif

#ifndef CONFIG_NO_STDLIB
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif
#include <deemon/tuple.h>
#include <stdlib.h>
#endif


/* Pull in some header to form artificial dependencies in order
 * to force the timestamp of the builtin `deemon' module from being
 * incremented if anything in those headers changed. */
#include "../runtime/builtin.h"
#include <deemon/asm.h>



DECL_BEGIN



#ifndef CONFIG_NO_STDLIB

#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(atexit_lock);
#endif
struct atexit_entry {
    DREF DeeObject      *ae_func; /* [1..1] The function that would be invoked. */
    DREF DeeTupleObject *ae_args; /* [1..1] Arguments passed to the function. */
};

/* [lock(atexit_lock)] The atexit list used by deemon. */
PRIVATE size_t atexit_size = 0;
PRIVATE struct atexit_entry *atexit_list = NULL;

#define ATEXIT_FNORMAL 0x0000 /* Normal atexit flags. */
#define ATEXIT_FDIDRUN 0x0001 /* `atexit_callback' has been executed. */
#define ATEXIT_FDIDREG 0x0002 /* `atexit_callback' was registered using `atexit()' */
/* [lock(atexit_lock)] Set of `ATEXIT_F*' */
PRIVATE uint16_t atexit_mode = ATEXIT_FNORMAL;


PUBLIC int DCALL
Dee_RunAtExit(uint16_t flags) {
 struct atexit_entry *list; size_t size;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&atexit_lock);
#endif
 /* Only execute atexit() once! */
 if ((atexit_mode&ATEXIT_FDIDRUN) && !atexit_size) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&atexit_lock);
#endif
  goto done;
 }
 atexit_mode |= ATEXIT_FDIDRUN;
 list         = atexit_list;
 size         = atexit_size;
 atexit_list  = NULL;
 atexit_size  = 0;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&atexit_lock);
#endif
 while (size--) {
  if (!(flags&DEE_RUNATEXIT_FDONTRUN)) {
   DREF DeeObject *temp;
   /* Invoke the callback. */
   temp = DeeObject_Call(list[size].ae_func,
                         DeeTuple_SIZE(list[size].ae_args),
                         DeeTuple_ELEM(list[size].ae_args));
   if unlikely(!temp) {
    /* An error occurred. */
    if (flags&DEE_RUNATEXIT_FRUNALL) {
     /* Just dump the error (including interrupts). */
     DeeError_Print("Unhandled error in atexit() callback\n",
                    ERROR_PRINT_HANDLEINTR);
    } else {
     /* Restore the list (Since we've already set `ATEXIT_FDIDRUN'
      * flag, we can be sure that there is no way anything was
      * able to register additional callbacks, meaning that the
      * list must still be empty) */
     ++size; /* Restore the one entry we've just failed to execute. */
#ifndef CONFIG_NO_THREADS
     rwlock_write(&atexit_lock);
#endif
     ASSERT(atexit_list  == NULL);
     ASSERT(atexit_size  == 0);
     atexit_list  = list;
     atexit_size  = size;
#ifndef CONFIG_NO_THREADS
     rwlock_endwrite(&atexit_lock);
#endif
     return -1;
    }
   } else {
    Dee_Decref(temp);
   }
  }
  Dee_Decref(list[size].ae_args);
  Dee_Decref(list[size].ae_func);
 }
 Dee_Free(list);
done:
 return 0;
}

PRIVATE void atexit_callback(void) {
 /* Simply run all registered atexit() functions. */
 Dee_RunAtExit(DEE_RUNATEXIT_FRUNALL);
}

PUBLIC int DCALL
Dee_AtExit(DeeObject *__restrict callback,
           DeeObject *__restrict args) {
 struct atexit_entry *new_list;
 ASSERT_OBJECT(callback);
 ASSERT_OBJECT_TYPE_EXACT(args,&DeeTuple_Type);
again:
#ifndef CONFIG_NO_THREADS
 rwlock_write(&atexit_lock);
#endif
 /* Check if we're still allowed to register new entires. */
 if (atexit_mode&ATEXIT_FDIDRUN) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&atexit_lock);
#endif
  DeeError_Throwf(&DeeError_RuntimeError,
                  "atexit() cannot be called after "
                  "callbacks had already been executed");
  goto err;
 }
 /* Allocate more entries. */
 new_list = (struct atexit_entry *)Dee_TryRealloc(atexit_list,(atexit_size+1)*
                                                  sizeof(struct atexit_entry));
 if unlikely(!new_list) {
  size_t old_size = atexit_size;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&atexit_lock);
#endif
  if (Dee_CollectMemory((old_size+1)*sizeof(struct atexit_entry)))
      goto again;
  goto err;
 }
 atexit_list = new_list;
 new_list   += atexit_size++;
 /* If the atexit-callback hasn't been registered yet, do that now. */
 if (!(atexit_mode&ATEXIT_FDIDREG)) {
  /* Don't bother handling errors returned by `atexit()'... */
  atexit(&atexit_callback);
  atexit_mode |= ATEXIT_FDIDREG;
 }

 /* Initialize the new callback entry. */
 Dee_Incref(callback);
 Dee_Incref(args);
 new_list->ae_func = callback;
 new_list->ae_args = (DREF DeeTupleObject *)args;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&atexit_lock);
#endif
 return 0;
err:
 return -1;
}
#else
PUBLIC int DCALL
Dee_AtExit(DeeObject *__restrict UNUSED(callback),
           DeeObject *__restrict UNUSED(args)) {
 DeeError_Throwf(&DeeError_NotImplemented,
                 "Deemon was built without atexit() support");
 return -1;
}
DFUNDEF int DCALL
Dee_RunAtExit(uint16_t UNUSED(flags)) {
 return 0;
}
#endif



#ifdef CONFIG_NO_DEC
PRIVATE uint64_t exec_timestamp = (uint64_t)-1;
#define HAS_EXEC_TIMESTAMP    (exec_timestamp != (uint64_t)-1)
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x))
#else
#define exec_timestamp         deemon_module.mo_ctime
#define HAS_EXEC_TIMESTAMP    (deemon_module.mo_flags&MODULE_FHASCTIME)
#ifdef CONFIG_NO_THREADS
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x),deemon_module.mo_flags |= MODULE_FHASCTIME)
#else
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x),ATOMIC_FETCHOR(deemon_module.mo_flags,MODULE_FHASCTIME))
#endif
#endif

PUBLIC char const DeeExec_Timestamp[] = __DATE__ "|" __TIME__;

#define TIMESTAMP_MON  (DeeExec_Timestamp)
#define TIMESTAMP_MDAY (DeeExec_Timestamp+4)
#define TIMESTAMP_YEAR (DeeExec_Timestamp+7)
#define TIMESTAMP_HOUR (DeeExec_Timestamp+12)
#define TIMESTAMP_MIN  (DeeExec_Timestamp+15)
#define TIMESTAMP_SEC  (DeeExec_Timestamp+18)


/* A couple of helper macros taken from the libtime DEX. */
#define time_yer2day(x)     (((146097*(x))/400)/*-1*/)
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define MICROSECONDS_PER_SECOND (MICROSECONDS_PER_MILLISECOND*MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE (MICROSECONDS_PER_SECOND*SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR   (MICROSECONDS_PER_MINUTE*MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY    (MICROSECONDS_PER_HOUR*HOURS_PER_DAY)

LOCAL uint64_t parse_timestamp(void) {
 unsigned int monthday;
 unsigned int monthstart;
 unsigned int year;
 unsigned int is_leap_year;
 unsigned int hour;
 unsigned int minute;
 unsigned int second;
#define MONTHSTART_JAN is_leap_year ? 0   : 0
#define MONTHSTART_FEB is_leap_year ? 31  : 31
#define MONTHSTART_MAR is_leap_year ? 60  : 59
#define MONTHSTART_APR is_leap_year ? 91  : 90
#define MONTHSTART_MAY is_leap_year ? 121 : 120
#define MONTHSTART_JUN is_leap_year ? 152 : 151
#define MONTHSTART_JUL is_leap_year ? 182 : 181
#define MONTHSTART_AUG is_leap_year ? 213 : 212
#define MONTHSTART_SEP is_leap_year ? 244 : 243
#define MONTHSTART_OCT is_leap_year ? 274 : 273
#define MONTHSTART_NOV is_leap_year ? 305 : 304
#define MONTHSTART_DEC is_leap_year ? 335 : 334
 /* With any luck, a decent optimize will get rid of most of this stuff...
  * NOTE: That's also the reason why everything is written without
  *       loops, or the use of api functions such as sscanf().
  *       Additionally, every variable is assigned to once (potentially
  *       in different branches of the same of-statement who's condition
  *       should already be known at compile-time), meaning that constant
  *       propagation should be fairly easy to achieve. */
 monthday = (((TIMESTAMP_MDAY[0] - '0')*10)+
              (TIMESTAMP_MDAY[1] - '0')) - 1;
 year     = (((TIMESTAMP_YEAR[0] - '0')*1000)+
             ((TIMESTAMP_YEAR[1] - '0')*100)+
             ((TIMESTAMP_YEAR[2] - '0')*10)+
              (TIMESTAMP_YEAR[3] - '0'));
 is_leap_year = !(year%400) || ((year%100) && !(year%4));
 if (TIMESTAMP_MON[0] == 'J') {
  if (TIMESTAMP_MON[1] == 'a') monthstart = MONTHSTART_JAN;
  else if (TIMESTAMP_MON[2] == 'n') monthstart = MONTHSTART_JUN;
  else monthstart = MONTHSTART_JUL;
 } else if (TIMESTAMP_MON[0] == 'F') {
  monthstart = MONTHSTART_FEB;
 } else if (TIMESTAMP_MON[0] == 'M') {
  if (TIMESTAMP_MON[2] == 'r') monthstart = MONTHSTART_MAR;
  else monthstart = MONTHSTART_MAY;
 } else if (TIMESTAMP_MON[0] == 'A') {
  if (TIMESTAMP_MON[1] == 'p') monthstart = MONTHSTART_APR;
  else monthstart = MONTHSTART_AUG;
 } else if (TIMESTAMP_MON[0] == 'S') {
  monthstart = MONTHSTART_SEP;
 } else if (TIMESTAMP_MON[0] == 'O') {
  monthstart = MONTHSTART_OCT;
 } else if (TIMESTAMP_MON[0] == 'N') {
  monthstart = MONTHSTART_NOV;
 } else {
  monthstart = MONTHSTART_DEC;
 }
 /* Figure out the time. */
 hour   = ((TIMESTAMP_HOUR[0] - '0')*10+
           (TIMESTAMP_HOUR[1] - '0'));
 minute = ((TIMESTAMP_MIN[0] - '0')*10+
           (TIMESTAMP_MIN[1] - '0'));
 second = ((TIMESTAMP_SEC[0] - '0')*10+
           (TIMESTAMP_SEC[1] - '0'));

 /* Pack everything together and return the result. */
 return (((uint64_t)((time_yer2day(year)+monthstart+monthday)-
                      /* Subtract the year 1970 from the result. */
                      time_yer2day(1970))*MICROSECONDS_PER_DAY)+
           /* Add the hour, minute and second to the calculation. */
         ((uint64_t)hour*MICROSECONDS_PER_HOUR)+
         ((uint64_t)minute*MICROSECONDS_PER_MINUTE)+
         ((uint64_t)second*MICROSECONDS_PER_SECOND));
}

PUBLIC uint64_t DCALL DeeExec_GetTimestamp(void) {
 uint64_t result = exec_timestamp;
 if (!HAS_EXEC_TIMESTAMP) {
  result = parse_timestamp();
  SET_EXEC_TIMESTAMP(result);
 }
 return result;
}




/* A reference to the user-code ARGV tuple. */
PRIVATE DREF DeeTupleObject *usercode_argv = (DREF DeeTupleObject *)Dee_EmptyTuple;
#ifdef CONFIG_NO_THREADS
PUBLIC ATTR_RETNONNULL
/*Tuple*/DREF DeeObject *DCALL Dee_GetArgv(void) {
 DREF DeeTupleObject *result;
 result = usercode_argv;
 Dee_Incref(result);
 ASSERT_OBJECT_TYPE_EXACT(result,&DeeTuple_Type);
 return (DREF DeeObject *)result;
}
PUBLIC NONNULL((1)) void DCALL
Dee_SetArgv(/*Tuple*/DeeObject *__restrict argv) {
 DREF DeeTupleObject *old_argv;
 ASSERT_OBJECT_TYPE_EXACT(argv,&DeeTuple_Type);
 Dee_Incref(argv);
 old_argv = usercode_argv;
 usercode_argv = argv;
 Dee_Decref(old_argv);
}
#else
PUBLIC ATTR_RETNONNULL
/*Tuple*/DREF DeeObject *DCALL Dee_GetArgv(void) {
 DREF DeeTupleObject *result;
 for (;;) {
  result = ATOMIC_XCH(usercode_argv,NULL);
  if (result) break;
  SCHED_YIELD();
 }
 Dee_Incref(result);
 ATOMIC_WRITE(usercode_argv,result);
 ASSERT_OBJECT_TYPE_EXACT(result,&DeeTuple_Type);
 return (DREF DeeObject *)result;
}
PUBLIC NONNULL((1)) void DCALL
Dee_SetArgv(/*Tuple*/DeeObject *__restrict argv) {
 DREF DeeTupleObject *old_argv;
 ASSERT_OBJECT_TYPE_EXACT(argv,&DeeTuple_Type);
 Dee_Incref(argv);
 do {
  for (;;) {
   old_argv = ATOMIC_READ(usercode_argv);
   if (old_argv) break;
   SCHED_YIELD();
  }
 } while (!ATOMIC_CMPXCH(usercode_argv,old_argv,argv));
 Dee_Decref(old_argv);
}
#endif

INTDEF bool DCALL libcodecs_shutdown(void);

PRIVATE bool DCALL shutdown_globals(void) {
 bool result;
 result  = DeeModule_FiniPath();
 result |= libcodecs_shutdown();
 result |= DeeFile_ResetStd();
 result |= DeeThread_ClearTls();
#ifndef CONFIG_NO_THREADS
 result |= DeeThread_JoinAll();
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEX
 result |= DeeDex_Cleanup();
#endif /* !CONFIG_NO_DEX */
#ifndef CONFIG_NO_NOTIFICATIONS
 result |= DeeNotify_Shutdown();
#endif /* !CONFIG_NO_NOTIFICATIONS */
 return result;
}

/* The max number of GC iterations yielding
 * no results before user-code is killed. */
#define MAX_EMPTY_GC_ITERATIONS_BEFORE_KILL    16
/* The total max number of GC iterations before user-code is killed. */
#define MAX_NONEMPTY_GC_ITERATIONS_BEFORE_KILL 128

#ifndef NDEBUG
INTDEF void DCALL gc_dump_all(void);
#endif


PUBLIC size_t DCALL Dee_Shutdown(void) {
#if 1
 size_t result = 0,temp;
 size_t num_gc = 0,num_empty_gc = 0;
 for (;;) {
  bool must_continue = false;
  /* Track how often we've already invoked the GC. */
  if (++num_gc == MAX_NONEMPTY_GC_ITERATIONS_BEFORE_KILL) {
do_kill_user:
   num_gc = 0;
   num_empty_gc = 0;
#ifndef CONFIG_NO_THREADS
   /* Make sure that no secondary threads could enter an
    * undefined state by us tinkering with their code. */
   must_continue |= DeeThread_JoinAll();
#endif
   /* Tell the user about what's happening (stddbg is forwarded to stderr) */
   DeeFile_Printf(DeeFile_DefaultStddbg,
                  "Stop executing user-code to fix unresolvable reference loop\n");
   if (!DeeExec_KillUserCode()) {
    /* Well... shit!
     * If we've gotten here, that probably means that there is some sort of
     * resource leak caused by deemon itself, meaning it's probably my fault...
     */
#ifndef NDEBUG
    /* Log all GC objects still alive */
    gc_dump_all();
#ifdef CONFIG_TRACE_REFCHANGES
    DEE_DPRINT("\n\n\nReference Leaks:\n");
    Dee_DumpReferenceLeaks();
#endif
    BREAKPOINT();
#else
    /* Silently hide the shame when not built for debug mode... */
#endif
    break;
   }
  }
  /* Do an initial shutdown (clear) on all global variables. */
  must_continue |= shutdown_globals();
  /* Collect as many GC objects as possible. */
  temp = DeeGC_Collect((size_t)-1);
  if (temp) {
   result      += temp;
   num_empty_gc = 0; /* Reset the empty-gc counter. */
   continue;
  }
  /* Make sure that nothing is hiding in globals,
   * now that the garbage has been collected. */
  must_continue |= shutdown_globals();

  /* If we already know that we must continue, don't even
   * bother looking at the GC chain as it stands right now. */
  if (must_continue)
      continue;

  /* Check if the GC truly is empty, and also
   * ensure that globals haven't been trying
   * to hide in here. */
  if (!DeeGC_IsEmptyWithoutDex()) {
   ++num_empty_gc;
   if (num_empty_gc == MAX_EMPTY_GC_ITERATIONS_BEFORE_KILL)
       goto do_kill_user;
   continue;
  }
  /* If nothing's left to-be done, _then_ we can stop. */
  if (!must_continue)
       break;
 }

 /* Shutdown all loaded DEX extensions. */
 DEE_CHECKMEMORY();
 DeeDex_Finalize();
 DEE_CHECKMEMORY();

 return result;

#else
 size_t n = 0,temp = 0;
 bool must_continue;
 do {
  /* XXX: After say... 16 iterations, somehow disable
   *      the execution of any user-assembly.
   *      Because let's face it. If stuff is still
   *      running that that point, we've got some
   *      user-defined cleanup code that keeps on
   *      re-adding global hooks somewhere.
   */
  n += temp;
  must_continue = shutdown_globals();
 } while ((temp = DeeGC_Collect((size_t)-1)) != 0 ||
           must_continue);
 return n;
#endif
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_EXEC_C */
