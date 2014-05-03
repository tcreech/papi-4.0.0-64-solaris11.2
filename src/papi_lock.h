#ifdef USE_SEMAPHORES
extern int sem_set;

/* If lock == MUTEX_OPEN, lock = MUTEX_CLOSED, val = MUTEX_OPEN
 * else val = MUTEX_CLOSED */

inline void _papi_hwd_lock_init(void) 
{
  union semun val; 
  val.val=1;
  
  if ((retval = semget(IPC_PRIVATE,PAPI_MAX_LOCK,0666)) == -1)
    {
      PAPIERROR("semget errno %d",errno); return(PAPI_ESYS); 
    }
  sem_set = retval;
  for (i=0;i<PAPI_MAX_LOCK;i++)
    {
      if ((retval = semctl(sem_set,i,SETVAL,val)) == -1)
	{
	  abort();
	}
    }
}

inline void _papi_hwd_lock_fini(void)
{
  if ((retval = semctl(sem_set,0,IPC_RMID,0)) == -1)
    {
      abort();
    }
}

inline void _papi_hwd_lock(int lck)
{                                    
  struct sembuf sem_lock = { lck, -1, 0 }; 
  if (semop(sem_set, &sem_lock, 1) == -1 ) 
    {
      abort();
    }
}

inline void _papi_hwd_unlock(int lck)                
{                                                
  struct sembuf sem_unlock = { lck, 1, 0 }; 
  if (semop(sem_set, &sem_unlock, 1) == -1 ) 
    {    
      abort(); 
    } 
}

#else

extern volatile unsigned int _papi_hwd_lock_data[PAPI_MAX_LOCK];

inline void _papi_hwd_lock_init(void) 
{
  for (i=0;i<PAPI_MAX_LOCK;i++)
    _papi_hwd_lock_data[i] = MUTEX_OPEN;
}

inline void _papi_hwd_lock_fini(void)
{
  for (i=0;i<PAPI_MAX_LOCK;i++)
    _papi_hwd_lock_data[i] = MUTEX_OPEN;
}

#define MUTEX_OPEN 0
#define MUTEX_CLOSED 1

#ifdef __ia64__

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
#endif /* __INTEL_COMPILER */

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

#endif /* USE_SEMAPHORES */

