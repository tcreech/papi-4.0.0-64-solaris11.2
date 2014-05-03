

#ifndef _PAPI_PERFMON_H
#define _PAPI_PERFMON_H
/*
* File:    perf_events.h
* CVS:     $Idl.h$
* Author:  Corey Ashford
*          cjashfor@us.ibm.com
*          - based on perfmon.h by -
*          Phil Mucci
*          mucci@cs.utk.edu
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <inttypes.h>
#include <libgen.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/ucontext.h>
#include <sys/ptrace.h>
#include "perfmon/pfmlib.h"
/* #include "perf_counter.h" */
/* It appears that for linux kernel 2.6.31, the /usr/include/linux dir
 does not include perf_counter.h. Since this dir is populated by the
 kernel-headers package, we'll have to wait for an update. 
 For now, we use the workaround: */
#include PEINCLUDE
#include "syscalls.h"


/* The following block of defines should only be true for kernel 2.6.31 */
#ifdef KERNEL31
	#define perf_event_sample_format	perf_counter_sample_format
	#define perf_event_read_format	perf_counter_read_format
	#define perf_event_attr			perf_counter_attr

	#define PERF_EVENT_IOC_ENABLE		PERF_COUNTER_IOC_ENABLE
	#define PERF_EVENT_IOC_DISABLE	PERF_COUNTER_IOC_DISABLE
	#define PERF_EVENT_IOC_REFRESH	PERF_COUNTER_IOC_REFRESH
	#define PERF_EVENT_IOC_RESET		PERF_COUNTER_IOC_RESET
	#define PERF_EVENT_IOC_PERIOD		PERF_COUNTER_IOC_PERIOD
	#define PERF_EVENT_IOC_SET_OUTPUT	PERF_COUNTER_IOC_SET_OUTPUT

	#define perf_event_ioc_flags		perf_counter_ioc_flags
	#define perf_event_mmap_page		perf_counter_mmap_page
	#define perf_event_wakeup		perf_counter_wakeup

	#define PERF_RECORD_MISC_CPUMODE_MASK	PERF_EVENT_MISC_CPUMODE_MASK
	#define PERF_RECORD_MISC_CPUMODE_UNKNOWN PERF_EVENT_MISC_CPUMODE_UNKNOWN
	#define PERF_RECORD_MISC_KERNEL	PERF_EVENT_MISC_KERNEL
	#define PERF_RECORD_MISC_USER		PERF_EVENT_MISC_USER
	#define PERF_RECORD_MISC_HYPERVISOR	PERF_EVENT_MISC_HYPERVISOR

	#define PERF_RECORD_MMAP		PERF_EVENT_MMAP
	#define PERF_RECORD_LOST		PERF_EVENT_LOST
	#define PERF_RECORD_COMM		PERF_EVENT_COMM
	#define PERF_RECORD_EXIT		PERF_EVENT_EXIT
	#define PERF_RECORD_THROTTLE		PERF_EVENT_THROTTLE
	#define PERF_RECORD_UNTHROTTLE	PERF_EVENT_UNTHROTTLE
	#define PERF_RECORD_FORK		PERF_EVENT_FORK
	#define PERF_RECORD_READ		PERF_EVENT_READ
	#define PERF_RECORD_SAMPLE		PERF_EVENT_SAMPLE
	#define PERF_RECORD_MAX			PERF_EVENT_MAX

#endif


/* Take a guess at this value for now - FIXME */
#define MAX_MPX_EVENTS 64

#define WAKEUP_MODE_COUNTER_OVERFLOW 0
#define WAKEUP_MODE_PROFILING 1

typedef struct {
  unsigned char wakeup_mode;
} per_event_info_t;

typedef struct {
  int num_events;
  int num_groups;
  unsigned domain;
  unsigned multiplexed;
  struct perf_event_attr events[MAX_MPX_EVENTS];
  per_event_info_t per_event_info[MAX_MPX_EVENTS];
  /* Buffer to gather counters */
  long long counts[PFMLIB_MAX_PMDS];
} control_state_t;

/* Perf events uses an FD per event counter */

/* just an unlikely magic cookie */
#define CTX_INITIALIZED 0xdc1dc1

#define PERF_EVENTS_RUNNING 0x01

typedef struct {
  int group_leader;       /* index of leader */
  int event_fd;
  int event_id;
  uint32_t nr_mmap_pages; /* number pages in the mmap buffer */
  void * mmap_buf; /* used to contain profiling data samples as well as control */
  uint64_t tail; /* current location in the mmap buffer to read from */
  uint64_t mask; /* mask used for wrapping the pages */
} evt_t;

typedef struct {
  /* Array of event fd's, event group leader is event_fd[0] */
  int cookie;
  int state;
  int num_evts;
  evt_t evt[MAX_MPX_EVENTS];
} context_t;

#if defined(DEBUG)
#define DEBUGCALL(a,b) { if (ISLEVEL(a)) { b; } }
#else
#define DEBUGCALL(a,b)
#endif

#define inline_static inline static

typedef pfmlib_event_t pfm_register_t;
typedef int reg_alloc_t;

#define MY_VECTOR _papi_pe_vector

/* Native events consist of a flag field, an event field, and a unit mask field.
 * The next 4 macros define the characteristics of the event and unit mask fields.
 */
#define PAPI_NATIVE_EVENT_AND_MASK 0x000003ff	/* 10 bits == 1024 max events */
#define PAPI_NATIVE_EVENT_SHIFT 0
#define PAPI_NATIVE_UMASK_AND_MASK 0x03fffc00	/* 16 bits for unit masks */
#define PAPI_NATIVE_UMASK_MAX 16				/* 16 possible unit masks */
#define PAPI_NATIVE_UMASK_SHIFT 10

#define MAX_COUNTERS PFMLIB_MAX_PMCS
#define MAX_COUNTER_TERMS PFMLIB_MAX_PMCS

extern volatile unsigned int _papi_hwd_lock_data[PAPI_MAX_LOCK];
#define MUTEX_OPEN 0
#define MUTEX_CLOSED 1

/* Locking functions */

#if defined(__ia64__)
#ifdef __INTEL_COMPILER
#define _papi_hwd_lock(lck) { while(_InterlockedCompareExchange_acq(&_papi_hwd_lock_data[lck],MUTEX_CLOSED,MUTEX_OPEN) != MUTEX_OPEN) { ; } }
#define _papi_hwd_unlock(lck) { _InterlockedExchange((volatile int *)&_papi_hwd_lock_data[lck], MUTEX_OPEN); }
#else                           /* GCC */
#define _papi_hwd_lock(lck)			 			      \
   { int res = 0;							      \
    do {								      \
      __asm__ __volatile__ ("mov ar.ccv=%0;;" :: "r"(MUTEX_OPEN));            \
      __asm__ __volatile__ ("cmpxchg4.acq %0=[%1],%2,ar.ccv" : "=r"(res) : "r"(&_papi_hwd_lock_data[lck]), "r"(MUTEX_CLOSED) : "memory");				      \
    } while (res != MUTEX_OPEN); }

#define _papi_hwd_unlock(lck) {  __asm__ __volatile__ ("st4.rel [%0]=%1" : : "r"(&_papi_hwd_lock_data[lck]), "r"(MUTEX_OPEN) : "memory"); }
#endif
#elif defined(__i386__)||defined(__x86_64__)
#define  _papi_hwd_lock(lck)                    \
do                                              \
{                                               \
   unsigned int res = 0;                        \
   do {                                         \
      __asm__ __volatile__ ("lock ; " "cmpxchg %1,%2" : "=a"(res) : "q"(MUTEX_CLOSED), "m"(_papi_hwd_lock_data[lck]), "0"(MUTEX_OPEN) : "memory");  \
   } while(res != (unsigned int)MUTEX_OPEN);   \
} while(0)
#define  _papi_hwd_unlock(lck)                  \
do                                              \
{                                               \
   unsigned int res = 0;                       \
   __asm__ __volatile__ ("xchg %0,%1" : "=r"(res) : "m"(_papi_hwd_lock_data[lck]), "0"(MUTEX_OPEN) : "memory");                                \
} while(0)
#elif defined(mips)
static inline void __raw_spin_lock(volatile unsigned int *lock)
{
  unsigned int tmp;
  extern int _perfmon2_pfm_pmu_type;
  if (_perfmon2_pfm_pmu_type == PFMLIB_MIPS_R10000_PMU)
    {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 li	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		"	sync						\n"
		"	.set	reorder					\n"
		: "=m" (*lock), "=&r" (tmp)
		: "m" (*lock)
		: "memory");
    }
  else if (_perfmon2_pfm_pmu_type == PFMLIB_MIPS_ICE9A_PMU)
    {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_lock	\n"
		"1:	ll	%1, %2					\n"
		"  	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 li	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 sync						\n"
		"	.set	reorder					\n"
		: "=m" (*lock), "=&r" (tmp)
		: "m" (*lock)
		: "memory");
    }
  else
    {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 li	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 sync						\n"
		"	.set	reorder					\n"
		: "=m" (*lock), "=&r" (tmp)
		: "m" (*lock)
		: "memory");
    }
}

static inline void __raw_spin_unlock(volatile unsigned int *lock)
{
	__asm__ __volatile__(
	"	.set	noreorder	# __raw_spin_unlock	\n"
	"	sync						\n"
	"	sw	$0, %0					\n"
	"	.set\treorder					\n"
	: "=m" (*lock)
	: "m" (*lock)
	: "memory");
}
#define  _papi_hwd_lock(lck) __raw_spin_lock(&_papi_hwd_lock_data[lck]);
#define  _papi_hwd_unlock(lck) __raw_spin_unlock(&_papi_hwd_lock_data[lck])
#elif defined(__powerpc__)

/*
 * These functions are slight modifications of the functions in
 * /usr/include/asm-ppc/system.h.
 *
 *  We can't use the ones in system.h directly because they are defined
 *  only when __KERNEL__ is defined.
 */

static __inline__ unsigned long
papi_xchg_u32(volatile void *p, unsigned long val)
{
        unsigned long prev;

        __asm__ __volatile__ ("\n\
        sync \n\
1:      lwarx   %0,0,%2 \n\
        stwcx.  %3,0,%2 \n\
        bne-    1b \n\
        isync"
        : "=&r" (prev), "=m" (*(volatile unsigned long *)p)
        : "r" (p), "r" (val), "m" (*(volatile unsigned long *)p)
        : "cc", "memory");

        return prev;
}

/*
 * The two defines below are taken directly from the MIPS implementation.
 */
#define  _papi_hwd_lock(lck)                          \
do {                                                    \
  unsigned int retval;                                 \
  do {                                                  \
  retval = papi_xchg_u32(&_papi_hwd_lock_data[lck],MUTEX_CLOSED);  \
  } while(retval != (unsigned int)MUTEX_OPEN);	        \
} while(0)
#define  _papi_hwd_unlock(lck)                          \
do {                                                    \
  unsigned int retval;                                 \
  retval = papi_xchg_u32(&_papi_hwd_lock_data[lck],MUTEX_OPEN); \
} while(0)
#elif defined(__sparc__)
static inline void __raw_spin_lock(volatile unsigned int *lock)
{
	__asm__ __volatile__(
	"\n1:\n\t"
	"ldstub	[%0], %%g2\n\t"
	"orcc	%%g2, 0x0, %%g0\n\t"
	"bne,a	2f\n\t"
	" ldub	[%0], %%g2\n\t"
	".subsection	2\n"
	"2:\n\t"
	"orcc	%%g2, 0x0, %%g0\n\t"
	"bne,a	2b\n\t"
	" ldub	[%0], %%g2\n\t"
	"b,a	1b\n\t"
	".previous\n"
	: /* no outputs */
	: "r" (lock)
	: "g2", "memory", "cc");
}
static inline void __raw_spin_unlock(volatile unsigned int *lock)
{
	__asm__ __volatile__("stb %%g0, [%0]" : : "r" (lock) : "memory");
}
#define  _papi_hwd_lock(lck) __raw_spin_lock(&_papi_hwd_lock_data[lck]);
#define  _papi_hwd_unlock(lck) __raw_spin_unlock(&_papi_hwd_lock_data[lck])
#else
#error "_papi_hwd_lock/unlock undefined!"
#endif

/* Signal handling functions */

#undef hwd_siginfo_t
typedef struct siginfo hwd_siginfo_t;
#undef hwd_ucontext_t
typedef ucontext_t hwd_ucontext_t;

#if defined(__ia64__)
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.sc_ip
#elif defined(__i386__)
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.gregs[REG_EIP]
#elif defined(__x86_64__)
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.gregs[REG_RIP]
#elif defined(mips)
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.pc
#elif defined(__powerpc__) && !defined(__powerpc64__)
/*
 * The index of the Next IP (REG_NIP) was obtained by looking at kernel
 * source code.  It wasn't documented anywhere else that I could find.
 */
#define REG_NIP 32
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.uc_regs->gregs[REG_NIP]
#elif defined(__powerpc64__)
#define OVERFLOW_ADDRESS(ctx) ctx.ucontext->uc_mcontext.regs->nip
#elif defined(__sparc__)
#define OVERFLOW_ADDRESS(ctx) ((struct sigcontext *)ctx.ucontext)->si_regs.pc
#else
#error "OVERFLOW_ADDRESS() undefined!"
#endif

#define GET_OVERFLOW_ADDRESS(ctx) (caddr_t)(OVERFLOW_ADDRESS(ctx))

#endif
