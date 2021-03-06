/*
* File:    perfmon.c
* CVS:     $Id: perfmon.c,v 1.97.4.1 2010/01/27 22:26:42 jagode Exp $
* Author:  Philip Mucci
*          mucci@cs.utk.edu
* Mods:    Brian Sheely
*          bsheely@eecs.utk.edu
*/

/* TODO LIST:
   - Events for all platforms
   - Derived events for all platforms
   - Latency profiling
   - BTB/IPIEAR sampling
   - Test on ITA2, Pentium 4
   - hwd_ntv_code_to_name
   - Make native map carry major events, not umasks
   - Enum event uses native_map not pfm()
   - Hook up globals to be freed to sub_info
   - Better feature bit support for IEAR
*/

#include "papi.h"
#include "papi_internal.h"
#include "papi_vector.h"
#include "papi_memory.h"

typedef unsigned uint;

/* Globals declared extern elsewhere */

hwi_search_t *preset_search_map;
#ifdef WIN32
extern int __pfm_getcpuinfo_attr(const char *attr, char *ret_buf, size_t maxlen);
extern void OpenWinPMCDriver();
extern void CloseWinPMCDriver();
CRITICAL_SECTION _papi_hwd_lock_data[PAPI_MAX_LOCK];
#else
volatile unsigned int _papi_hwd_lock_data[PAPI_MAX_LOCK];
extern int get_cpu_info(PAPI_hw_info_t * hwinfo);
#endif
extern papi_vector_t _papi_pfm_vector;
extern int _papi_pfm_setup_presets(char *name, int type);
extern int _papi_pfm_ntv_enum_events(unsigned int *EventCode, int modifier);
extern int _papi_pfm_ntv_name_to_code(char *ntv_name, unsigned int *EventCode);
extern int _papi_pfm_ntv_code_to_name(unsigned int EventCode, char *ntv_name, int len);
extern int _papi_pfm_ntv_code_to_descr(unsigned int EventCode, char *ntv_descr, int len);
extern int _papi_pfm_ntv_code_to_bits(unsigned int EventCode, hwd_register_t *bits);
extern int _papi_pfm_ntv_bits_to_info(hwd_register_t *bits, char *names,
								unsigned int *values, int name_len, int count);

int _papi_pfm_set_overflow(EventSetInfo_t * ESI, int EventIndex, int threshold);

/* Static locals */

int _perfmon2_pfm_pmu_type = -1;
static pfmlib_regmask_t _perfmon2_pfm_unavailable_pmcs;
static pfmlib_regmask_t _perfmon2_pfm_unavailable_pmds;

/* Debug functions */

#ifdef DEBUG
static void dump_smpl_arg(pfm_dfl_smpl_arg_t *arg)
{
  SUBDBG("SMPL_ARG.buf_size = %llu\n",(unsigned long long)arg->buf_size);
  SUBDBG("SMPL_ARG.buf_flags = %d\n",arg->buf_flags);
}

static void dump_sets(pfarg_setdesc_t *set, int num_sets)
{
  int i;

  for (i=0;i<num_sets;i++)
    {
      SUBDBG("SET[%d]\n",i);
      SUBDBG("SET[%d].set_id = %d\n",i,set[i].set_id);
     // SUBDBG("SET[%d].set_id_next = %d\n",i,set[i].set_id_next);
      SUBDBG("SET[%d].set_flags = %d\n",i,set[i].set_flags);
      SUBDBG("SET[%d].set_timeout = %llu\n",i,(unsigned long long)set[i].set_timeout);
     //  SUBDBG("SET[%d].set_mmap_offset = 0x%016llx\n",i,(unsigned long long)set[i].set_mmap_offset);
    }
}

static void dump_setinfo(pfarg_setinfo_t *setinfo, int num_sets)
{
  int i;

  for (i=0;i<num_sets;i++)
    {
      SUBDBG("SETINFO[%d]\n",i);
      SUBDBG("SETINFO[%d].set_id = %d\n",i,setinfo[i].set_id);
      // SUBDBG("SETINFO[%d].set_id_next = %d\n",i,setinfo[i].set_id_next);
      SUBDBG("SETINFO[%d].set_flags = %d\n",i,setinfo[i].set_flags);
      SUBDBG("SETINFO[%d].set_ovfl_pmds[0] = 0x%016llx\n",i,(unsigned long long)setinfo[i].set_ovfl_pmds[0]);
      SUBDBG("SETINFO[%d].set_runs = %llu\n",i,(unsigned long long)setinfo[i].set_runs);
      SUBDBG("SETINFO[%d].set_timeout = %llu\n",i,(unsigned long long)setinfo[i].set_timeout);
      SUBDBG("SETINFO[%d].set_act_duration = %llu\n",i,(unsigned long long)setinfo[i].set_act_duration);
      // SUBDBG("SETINFO[%d].set_mmap_offset = 0x%016llx\n",i,(unsigned long long)setinfo[i].set_mmap_offset);
      SUBDBG("SETINFO[%d].set_avail_pmcs[0] = 0x%016llx\n",i,(unsigned long long)setinfo[i].set_avail_pmcs[0]);
      SUBDBG("SETINFO[%d].set_avail_pmds[0] = 0x%016llx\n",i,(unsigned long long)setinfo[i].set_avail_pmds[0]);
    }
}

static void dump_pmc(pfm_control_state_t *ctl)
{
  int i;
  pfarg_pmc_t *pc = ctl->pc;

  for (i=0;i<ctl->out.pfp_pmc_count;i++)
    {
      SUBDBG("PC[%d]\n",i);
      SUBDBG("PC[%d].reg_num = %d\n",i,pc[i].reg_num);
      SUBDBG("PC[%d].reg_set = %d\n",i,pc[i].reg_set);
      SUBDBG("PC[%d].reg_flags = 0x%08x\n",i,pc[i].reg_flags);
      SUBDBG("PC[%d].reg_value = 0x%016llx\n",i,(unsigned long long)pc[i].reg_value);
    }
}

static void dump_pmd(pfm_control_state_t *ctl)
{
  int i;
  pfarg_pmd_t *pd = ctl->pd;

  for (i=0;i<ctl->in.pfp_event_count;i++)
    {
      SUBDBG("PD[%d]\n",i);
      SUBDBG("PD[%d].reg_num = %d\n",i,pd[i].reg_num);
      SUBDBG("PD[%d].reg_set = %d\n",i,pd[i].reg_set);
      SUBDBG("PD[%d].reg_flags = 0x%08x\n",i,pd[i].reg_flags);
      SUBDBG("PD[%d].reg_value = 0x%016llx\n",i,(unsigned long long)pd[i].reg_value);
      SUBDBG("PD[%d].reg_long_reset = %llu\n",i,(unsigned long long)pd[i].reg_long_reset);
      SUBDBG("PD[%d].reg_short_reset = %llu\n",i,(unsigned long long)pd[i].reg_short_reset);
      SUBDBG("PD[%d].reg_last_reset_val = %llu\n",i,(unsigned long long)pd[i].reg_last_reset_val);
      SUBDBG("PD[%d].reg_ovfl_switch_cnt = %llu\n",i,(unsigned long long)pd[i].reg_ovfl_switch_cnt);
      SUBDBG("PD[%d].reg_reset_pmds[0] = 0x%016llx\n",i,(unsigned long long)pd[i].reg_reset_pmds[0]);
      SUBDBG("PD[%d].reg_smpl_pmds[0] = 0x%016llx\n",i,(unsigned long long)pd[i].reg_smpl_pmds[0]);
      SUBDBG("PD[%d].reg_smpl_eventid = %llu\n",i,(unsigned long long)pd[i].reg_smpl_eventid);
      SUBDBG("PD[%d].reg_random_mask = %llu\n",i,(unsigned long long)pd[i].reg_random_mask);
      SUBDBG("PD[%d].reg_random_seed = %d\n",i,pd[i].reg_random_seed);
    }
}

static void dump_smpl_hdr(pfm_dfl_smpl_hdr_t *hdr)
{
  SUBDBG("SMPL_HDR.hdr_count = %llu\n",(unsigned long long)hdr->hdr_count);
  SUBDBG("SMPL_HDR.hdr_cur_offs = %llu\n",(unsigned long long)hdr->hdr_cur_offs);
  SUBDBG("SMPL_HDR.hdr_overflows = %llu\n",(unsigned long long)hdr->hdr_overflows);
  SUBDBG("SMPL_HDR.hdr_buf_size = %llu\n",(unsigned long long)hdr->hdr_buf_size);
  SUBDBG("SMPL_HDR.hdr_min_buf_space = %llu\n",(unsigned long long)hdr->hdr_min_buf_space);
  SUBDBG("SMPL_HDR.hdr_version = %d\n",hdr->hdr_version);
  SUBDBG("SMPL_HDR.hdr_buf_flags = %d\n",hdr->hdr_buf_flags);
}

static void dump_smpl(pfm_dfl_smpl_entry_t *entry)
{
  SUBDBG("SMPL.pid = %d\n",entry->pid);
  SUBDBG("SMPL.ovfl_pmd = %d\n",entry->ovfl_pmd);
  SUBDBG("SMPL.last_reset_val = %llu\n",(unsigned long long)entry->last_reset_val);
  SUBDBG("SMPL.ip = 0x%llx\n",(unsigned long long)entry->ip);
  SUBDBG("SMPL.tstamp = %llu\n",(unsigned long long)entry->tstamp);
  SUBDBG("SMPL.cpu = %d\n",entry->cpu);
  SUBDBG("SMPL.set = %d\n",entry->set);
  SUBDBG("SMPL.tgid = %d\n",entry->tgid);
}
#endif

/* Hardware clock functions */

/* All architectures should set HAVE_CYCLES in configure if they have these. Not all do
   so for now, we have to guard at the end of the statement, instead of the top. When
   all archs set this, this region will be guarded with:
   #if defined(HAVE_CYCLE) 
   which is equivalent to
   #if !defined(HAVE_GETTIMEOFDAY) && !defined(HAVE_CLOCK_GETTIME_REALTIME)
*/

#if defined(HAVE_MMTIMER)
inline_static long long get_cycles(void)
{
  long long tmp = 0;
  tmp = *mmdev_timer_addr;
#error "This needs work"
  return(tmp);
} 
#elif defined(__ia64__)
inline_static long long get_cycles(void)
{
  long long tmp = 0;
#if defined(__INTEL_COMPILER)
  tmp = __getReg(_IA64_REG_AR_ITC);
#else
  __asm__ __volatile__("mov %0=ar.itc":"=r"(tmp)::"memory");
#endif
  switch (_perfmon2_pfm_pmu_type) {
  case PFMLIB_MONTECITO_PMU:
    tmp = tmp * 4;
    break;
  }
  return tmp ;
}
#elif (defined(__i386__)||defined(__x86_64__))
inline_static long long get_cycles(void) {
   long long ret = 0;
#ifdef __x86_64__
   do {
      unsigned int a,d;
      asm volatile("rdtsc" : "=a" (a), "=d" (d));
      (ret) = ((long long)a) | (((long long)d)<<32);
   } while(0);
#elif defined WIN32
   ret = __rdtsc();
#else
   __asm__ __volatile__("rdtsc"
                       : "=A" (ret)
                       : );
#endif
   return ret;
}
/* SiCortex only code, which works on V2.3 R81 or above
   anything below, must use gettimeofday() */
#elif defined(HAVE_CYCLE) && defined(mips)
inline_static long long get_cycles(void)
{
  long long count;
  
  __asm__ __volatile__(
		       ".set  push      \n"
		       ".set  mips32r2  \n"
		       "rdhwr $3, $30   \n"
		       ".set  pop       \n"
		       "move  %0, $3    \n"
		       : "=r"(count) : : "$3");
  return count*2;
}
/* #define get_cycles _rtc ?? */
#elif defined(__sparc__)
inline_static long long get_cycles(void)
{
	register unsigned long ret asm("g1");

	__asm__ __volatile__(".word 0x83410000" /* rd %tick, %g1 */
			     : "=r" (ret));
	return ret;
}
#elif defined(__powerpc__)
/*
 * It's not possible to read the cycles from user space on ppc970 and
 * POWER4/4+.  There is a 64-bit time-base register (TBU|TBL), but its
 * update rate is implementation-specific and cannot easily be translated
 * into a cycle count.  So don't implement get_cycles for POWER for now,
 * but instead, rely on the definition of HAVE_CLOCK_GETTIME_REALTIME in
 * _papi_hwd_get_real_usec() for the needed functionality.
*/
#elif !defined(HAVE_GETTIMEOFDAY) && !defined(HAVE_CLOCK_GETTIME_REALTIME)
#error "No get_cycles support for this architecture. Please modify perfmon.c or compile with a different timer"
#endif

#define PFM_MAX_PMCDS 20

int _papi_pfm_write_pmcs(pfm_context_t * ctx, pfm_control_state_t * ctl)
{
  int i=0, ret;

  SUBDBG("PFM_WRITE_PMCS(%d,%p,%d)\n",ctl->ctx_fd, ctl->pc, ctl->out.pfp_pmc_count);
  if(ctl->out.pfp_pmc_count>PFM_MAX_PMCDS){
    for(i=0;i<ctl->out.pfp_pmc_count-PFM_MAX_PMCDS;i+=PFM_MAX_PMCDS){
      if ((ret = pfm_write_pmcs(ctl->ctx_fd, ctl->pc+i, PFM_MAX_PMCDS))){
        DEBUGCALL(DEBUG_SUBSTRATE,dump_pmc(ctl));
        PAPIERROR("pfm_write_pmcs(%d,%p,%d): %s",ctl->ctx_fd,ctl->pc,ctl->out.pfp_pmc_count, strerror(ret));
        return(PAPI_ESYS);
      }
    }
    DEBUGCALL(DEBUG_SUBSTRATE,dump_pmc(ctl));
  }
  if ((ret = pfm_write_pmcs(ctl->ctx_fd, ctl->pc+i, ctl->out.pfp_pmc_count-i))){
      DEBUGCALL(DEBUG_SUBSTRATE,dump_pmc(ctl));
      PAPIERROR("pfm_write_pmcs(%d,%p,%d): %s",ctl->ctx_fd,ctl->pc,ctl->out.pfp_pmc_count, strerror(ret));
      return(PAPI_ESYS);
  }
  DEBUGCALL(DEBUG_SUBSTRATE,dump_pmc(ctl));

  return PAPI_OK;
}

int _papi_pfm_write_pmds(pfm_context_t * ctx, pfm_control_state_t * ctl)
{
  int i=0, ret;

  SUBDBG("PFM_WRITE_PMDS(%d,%p,%d)\n",ctl->ctx_fd, ctl->pd, ctl->in.pfp_event_count);
  if(ctl->in.pfp_event_count>PFM_MAX_PMCDS){
    for(i=0;i<ctl->in.pfp_event_count-PFM_MAX_PMCDS;i+=PFM_MAX_PMCDS){
      if ((ret = pfm_write_pmds(ctl->ctx_fd, ctl->pd+i, PFM_MAX_PMCDS))){
        DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
        PAPIERROR("pfm_write_pmds(%d,%p,%d): errno=%d %s",ctl->ctx_fd,ctl->pd,ctl->in.pfp_event_count, errno, strerror(ret));
	    perror("pfm_write_pmds");
        return(PAPI_ESYS);
      }
    }
    DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
  }
  if ((ret = pfm_write_pmds(ctl->ctx_fd, ctl->pd+i, ctl->in.pfp_event_count-i))){
      DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
      PAPIERROR("pfm_write_pmds(%d,%p,%d): errno=%d %s",ctl->ctx_fd,ctl->pd,ctl->in.pfp_event_count, errno, strerror(ret));
	  perror("pfm_write_pmds");
      return(PAPI_ESYS);
  }
  DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));

  return PAPI_OK;
}

int _papi_pfm_read_pmds(pfm_context_t * ctx, pfm_control_state_t * ctl)
{
  int i=0, ret;

  SUBDBG("PFM_READ_PMDS(%d,%p,%d)\n",ctl->ctx_fd, ctl->pd, ctl->in.pfp_event_count);
  if(ctl->in.pfp_event_count>PFM_MAX_PMCDS){
    for(i=0;i<ctl->in.pfp_event_count-PFM_MAX_PMCDS;i+=PFM_MAX_PMCDS){
      if ((ret = pfm_read_pmds(ctl->ctx_fd, ctl->pd+i, PFM_MAX_PMCDS))){
        DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
        PAPIERROR("pfm_read_pmds(%d,%p,%d): %s",ctl->ctx_fd,ctl->pd,ctl->in.pfp_event_count, strerror(ret));
        return((errno == EBADF) ? PAPI_ECLOST : PAPI_ESYS);
      }
    }
    DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
  }
  if ((ret = pfm_read_pmds(ctl->ctx_fd, ctl->pd+i, ctl->in.pfp_event_count-i))){
      DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));
      PAPIERROR("pfm_read_pmds(%d,%p,%d): %s",ctl->ctx_fd,ctl->pd,ctl->in.pfp_event_count, strerror(ret));
      return((errno == EBADF) ? PAPI_ECLOST : PAPI_ESYS);
  }
  DEBUGCALL(DEBUG_SUBSTRATE,dump_pmd(ctl));

  return PAPI_OK;
}


/* This routine effectively does argument checking as the real magic will happen
   in compute_kernel_args. This just gets the value back from the kernel. */

static int check_multiplex_timeout(int ctx_fd, unsigned long *timeout_ns)
{
  int ret;
  pfarg_setdesc_t set[2];

  memset(set,0,sizeof(pfarg_setdesc_t)*2);
  set[1].set_id = 1;
  set[1].set_flags = PFM_SETFL_TIME_SWITCH;
  set[1].set_timeout = *timeout_ns;
  SUBDBG("Multiplexing interval requested is %llu ns.\n",(unsigned long long)set[1].set_timeout);
      
  /* Create a test eventset */

  SUBDBG("PFM_CREATE_EVTSETS(%d,%p,1)\n",ctx_fd,&set[1]);
  if ((ret = pfm_create_evtsets(ctx_fd, &set[1], 1)) != PFMLIB_SUCCESS)
    {
      DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(&set[1],1));
      PAPIERROR("pfm_create_evtsets(%d,%p,%d): %s", ctx_fd, &set[1], 1, strerror(ret));
      return(PAPI_ESYS);
    }      

  SUBDBG("Multiplexing interval returned is %llu ns.\n",(unsigned long long)set[1].set_timeout);
  *timeout_ns = set[1].set_timeout;
  
  /* Delete the second eventset */

  pfm_delete_evtsets(ctx_fd,&set[1],1);

  return(PAPI_OK);
}

/* The below function is stolen from libpfm from Stephane Eranian */
static int
detect_timeout_and_unavail_pmu_regs(pfmlib_regmask_t *r_pmcs, pfmlib_regmask_t *r_pmds, unsigned long *timeout_ns)
{
  pfarg_ctx_t ctx;
  pfarg_setinfo_t	setf;
  int ret, i, j, myfd;

	memset(r_pmcs, 0, sizeof(*r_pmcs));
	memset(r_pmds, 0, sizeof(*r_pmds));

	memset(&ctx, 0, sizeof(ctx));
	memset(&setf, 0, sizeof(setf));
	/*
	 * if no context descriptor is passed, then create
	 * a temporary context
	 */
	SUBDBG("PFM_CREATE_CONTEXT(%p,%p,%p,%d)\n",&ctx,NULL,NULL,0);
	myfd = pfm_create_context(&ctx, NULL, NULL, 0);
	if (myfd == -1)
	  {
	    PAPIERROR("detect_unavail_pmu_regs:pfm_create_context(): %s", strerror(errno));
	    return(PAPI_ESYS);
	  }
	SUBDBG("PFM_CREATE_CONTEXT returned fd %d\n",myfd);
	/*
	 * retrieve available register bitmasks from set0
	 * which is guaranteed to exist for every context
	 */
	ret = pfm_getinfo_evtsets(myfd, &setf, 1);
	if (ret != PFMLIB_SUCCESS) 
	  {
	    PAPIERROR("pfm_getinfo_evtsets(): %s", strerror(ret));
	    return(PAPI_ESYS);
	  }
      DEBUGCALL(DEBUG_SUBSTRATE,dump_setinfo(&setf,1));
      if (r_pmcs)
		for(i=0; i < PFM_PMC_BV; i++) {
			for(j=0; j < 64; j++) {
				if ((setf.set_avail_pmcs[i] & (1ULL << j)) == 0)
					pfm_regmask_set(r_pmcs, (i<<6)+j);
			}
		}
	if (r_pmds)
		for(i=0; i < PFM_PMD_BV; i++) {
			for(j=0; j < 64; j++) {
				if ((setf.set_avail_pmds[i] & (1ULL << j)) == 0)
					pfm_regmask_set(r_pmds, (i<<6)+j);
			}
		}
	check_multiplex_timeout(myfd,timeout_ns);
#ifndef WIN32
	i = close(myfd);
#endif
	SUBDBG("CLOSE fd %d returned %d\n",myfd,i);
	return PAPI_OK;
}

/* BEGIN COMMON CODE */

#if defined(WIN32)
static void decode_vendor_string(char *s, int *vendor)
{
  if (strcasecmp(s,"GenuineIntel") == 0)
    *vendor = PAPI_VENDOR_INTEL;
  else if ((strcasecmp(s,"AMD") == 0) || (strcasecmp(s,"AuthenticAMD") == 0))
    *vendor = PAPI_VENDOR_AMD;
  else if (strcasecmp(s,"IBM") == 0)
    *vendor = PAPI_VENDOR_IBM;
  else if (strcasecmp(s,"MIPS") == 0)
    *vendor = PAPI_VENDOR_MIPS;
  else if (strcasecmp(s,"SiCortex") == 0)
    *vendor = PAPI_VENDOR_SICORTEX;
  else if (strcasecmp(s,"Cray") == 0)
    *vendor = PAPI_VENDOR_CRAY;
  else
    *vendor = PAPI_VENDOR_UNKNOWN;
}
#endif

#if defined(mips) || defined(__sparc__)
static char *search_cpu_info(FILE * f, char *search_str, char *line)
{
   /* This code courtesy of our friends in Germany. Thanks Rudolph Berrendorf! */
   /* See the PCL home page for the German version of PAPI. */

   char *s;

   while (fgets(line, 256, f) != NULL) {
      if (strstr(line, search_str) != NULL) {
         /* ignore all characters in line up to : */
         for (s = line; *s && (*s != ':'); ++s);
         if (*s)
            return (s);
      }
   }
   return (NULL);
}
#endif

#ifdef WIN32
static int get_cpu_info(PAPI_hw_info_t *hw_info)
{
   int retval = PAPI_OK;
   char maxargs[PAPI_HUGE_STR_LEN];
   char *s;
   char model[48];
   int i;
   for (i = 0; i < 3; ++i)
     __cpuid(&model[i * 16], 0x80000002 + i);
   for (i = 0; i < 48; ++i)
     model[i] = tolower(model[i]);
   if ((s = strstr(model, "mhz")) != NULL)
   {
     --s;
     while (isspace(*s) || isdigit(*s) || *s == '.' && s >= model)
       --s;
     ++s;
     hw_info->mhz = (float)atof(s);
   }
   else if ((s = strstr(model, "ghz")) != NULL)
   {
     --s;
     while (isspace(*s) || isdigit(*s) || *s == '.' && s >= model)
       --s;
     ++s;
     hw_info->mhz = (float)atof(s) * 1000;
   }
   else
     return PAPI_EBUG;

   hw_info->clock_mhz = hw_info->mhz;

   __pfm_getcpuinfo_attr("vendor_id", hw_info->vendor_string, sizeof(hw_info->vendor_string));

   if (strlen(hw_info->vendor_string))
     decode_vendor_string(hw_info->vendor_string, &hw_info->vendor);
   __cpuid(maxargs, 1);
   hw_info->revision = *(uint32_t *)maxargs & 0xf;
   strcpy(hw_info->model_string, model);
   return (retval);
}
#endif

#if defined(__i386__)||defined(__x86_64__)
static int x86_get_memory_info(PAPI_hw_info_t *hw_info)
{
   int retval = PAPI_OK;

   extern int x86_cache_info(PAPI_mh_info_t * mh_info);

   switch (hw_info->vendor) 
     {
     case PAPI_VENDOR_AMD:
     case PAPI_VENDOR_INTEL:
       retval = x86_cache_info(&hw_info->mem_hierarchy);
       break;
     default:
       PAPIERROR("Unknown vendor in memory information call for x86.");
       return(PAPI_ESBSTR);
     }
   return retval;
}
#endif

/* 2.6.19 has this:
VmPeak:     4588 kB
VmSize:     4584 kB
VmLck:         0 kB
VmHWM:      1548 kB
VmRSS:      1548 kB
VmData:      312 kB
VmStk:        88 kB
VmExe:       684 kB
VmLib:      1360 kB
VmPTE:        20 kB
*/

int _papi_pfm_get_dmem_info(PAPI_dmem_info_t *d)
{
  char fn[PATH_MAX], tmp[PATH_MAX];
  FILE *f;
  int ret;
  long long vmpk = 0, sz = 0, lck = 0, res = 0, shr = 0, stk = 0, txt = 0, dat = 0, dum = 0, lib = 0, hwm = 0, pte = 0;

  sprintf(fn,"/proc/%ld/status",(long)getpid());
  f = fopen(fn,"r");
  if (f == NULL)
    {
      PAPIERROR("fopen(%s): %s\n",fn,strerror(errno));
      return PAPI_ESBSTR;
    }
  while (1)
    {
      if (fgets(tmp,PATH_MAX,f) == NULL)
	break;
      if (strspn(tmp,"VmPeak:") == strlen("VmPeak:"))
	{
	  sscanf(tmp+strlen("VmPeak:"),"%lld",&vmpk);
	  d->peak = vmpk;
	  continue;
	}
      if (strspn(tmp,"VmSize:") == strlen("VmSize:"))
	{
	  sscanf(tmp+strlen("VmSize:"),"%lld",&sz);
	  d->size = sz;
	  continue;
	}
      if (strspn(tmp,"VmLck:") == strlen("VmLck:"))
	{
	  sscanf(tmp+strlen("VmLck:"),"%lld",&lck);
	  d->locked = lck;
	  continue;
	}
      if (strspn(tmp,"VmHWM:") == strlen("VmHWM:"))
	{
	  sscanf(tmp+strlen("VmHWM:"),"%lld",&hwm);
	  d->high_water_mark = hwm;
	  continue;
	}
      if (strspn(tmp,"VmRSS:") == strlen("VmRSS:"))
	{
	  sscanf(tmp+strlen("VmRSS:"),"%lld",&res);
	  d->resident = res;
	  continue;
	}
      if (strspn(tmp,"VmData:") == strlen("VmData:"))
	{
	  sscanf(tmp+strlen("VmData:"),"%lld",&dat);
	  d->heap = dat;
	  continue;
	}
      if (strspn(tmp,"VmStk:") == strlen("VmStk:"))
	{
	  sscanf(tmp+strlen("VmStk:"),"%lld",&stk);
	  d->stack = stk;
	  continue;
	}
      if (strspn(tmp,"VmExe:") == strlen("VmExe:"))
	{
	  sscanf(tmp+strlen("VmExe:"),"%lld",&txt);
	  d->text = txt;
	  continue;
	}
      if (strspn(tmp,"VmLib:") == strlen("VmLib:"))
	{
	  sscanf(tmp+strlen("VmLib:"),"%lld",&lib);
	  d->library = lib;
	  continue;
	}
      if (strspn(tmp,"VmPTE:") == strlen("VmPTE:"))
	{
	  sscanf(tmp+strlen("VmPTE:"),"%lld",&pte);
	  d->pte = pte;
	  continue;
	}
    }
  fclose(f);

  sprintf(fn,"/proc/%ld/statm",(long)getpid());
  f = fopen(fn,"r");
  if (f == NULL)
    {
      PAPIERROR("fopen(%s): %s\n",fn,strerror(errno));
      return PAPI_ESBSTR;
    }
  ret = fscanf(f,"%lld %lld %lld %lld %lld %lld %lld",&dum,&dum,&shr,&dum,&dum,&dat,&dum);
  if (ret != 7)
    {
      PAPIERROR("fscanf(7 items): %d\n",ret);
      return PAPI_ESBSTR;
    }
  d->pagesize = getpagesize() / 1024;
  d->shared = (shr * d->pagesize)/1024;
  fclose(f);

  return PAPI_OK;
}

#if defined(__ia64__)
static int get_number( char *buf ){
   char numbers[] = "0123456789";
   int num;
   char *tmp, *end;

   tmp = strpbrk(buf, numbers);
   if ( tmp != NULL ){
	end = tmp;
	while(isdigit(*end)) end++;
	*end='\0';
        num = atoi(tmp);
        return(num);
    }

   PAPIERROR("Number could not be parsed from %s",buf);
   return(-1);
}

static void fline ( FILE *fp, char *rline ) {
  char *tmp,*end,c;

  tmp = rline;
  end = &rline[1023];
   
  memset(rline, '\0', 1024);

  do {
    if ( feof(fp))  return;
    c = getc(fp);
  } while (isspace(c) || c == '\n' || c == '\r');

  ungetc( c, fp);

  for(;;) {
    if ( feof(fp) ) {
       return;
    }
    c = getc( fp);
    if ( c == '\n' || c == '\r' )
      break;
    *tmp++ = c;
    if ( tmp == end ) {
       *tmp = '\0';
       return;
    }
  }
  return;
} 

static int ia64_get_memory_info(PAPI_hw_info_t *hw_info)
{
   int retval = 0;
   FILE *f;
   int clevel = 0, cindex = -1;
   char buf[1024];
   int num, i, j;
   PAPI_mh_info_t *meminfo = &hw_info->mem_hierarchy;
   PAPI_mh_level_t *L = hw_info->mem_hierarchy.level;

   f = fopen("/proc/pal/cpu0/cache_info","r");

   if (!f)
     { PAPIERROR("fopen(/proc/pal/cpu0/cache_info) returned < 0"); return(PAPI_ESYS); }

   while (!feof(f)) {
      fline(f, buf);
      if ( buf[0] == '\0' ) break;
      if (  !strncmp(buf, "Data Cache", 10) ) {
         cindex = 1;
         clevel = get_number( buf );
         L[clevel - 1].cache[cindex].type = PAPI_MH_TYPE_DATA;
      }
      else if ( !strncmp(buf, "Instruction Cache", 17) ) {
         cindex = 0;
         clevel = get_number( buf );
         L[clevel - 1].cache[cindex].type = PAPI_MH_TYPE_INST;
      }
      else if ( !strncmp(buf, "Data/Instruction Cache", 22)) {
         cindex = 0;
         clevel = get_number( buf );
         L[clevel - 1].cache[cindex].type = PAPI_MH_TYPE_UNIFIED;
      }
      else {
         if ( (clevel == 0 || clevel > 3) && cindex >= 0)
	   { PAPIERROR("Cache type could not be recognized, please send /proc/pal/cpu0/cache_info"); return(PAPI_EBUG); }

         if ( !strncmp(buf, "Size", 4) ) {
            num = get_number( buf );
            L[clevel - 1].cache[cindex].size = num;
         }
         else if ( !strncmp(buf, "Associativity", 13) ) {
            num = get_number( buf );
            L[clevel - 1].cache[cindex].associativity = num;
         }
         else if ( !strncmp(buf, "Line size", 9) ) {
            num = get_number( buf );
            L[clevel - 1].cache[cindex].line_size = num;
            L[clevel - 1].cache[cindex].num_lines = L[clevel - 1].cache[cindex].size/num;
         }
      }
   } 

   fclose(f);

   f = fopen("/proc/pal/cpu0/vm_info","r");
   /* No errors on fopen as I am not sure this is always on the systems */
   if ( f != NULL ) {
      cindex = -1;
      clevel = 0;
      while (!feof(f)) {
         fline(f, buf);
         if ( buf[0] == '\0' ) break;
         if (  !strncmp(buf, "Data Translation", 16) ) {
            cindex = 1;
	         clevel = get_number( buf );
            L[clevel - 1].tlb[cindex].type = PAPI_MH_TYPE_DATA;
         }
         else if ( !strncmp(buf, "Instruction Translation", 23) ){
            cindex = 0;
            clevel = get_number( buf );
            L[clevel - 1].tlb[cindex].type = PAPI_MH_TYPE_INST;
         }
         else {
	         if ( (clevel == 0 || clevel > 2) && cindex >= 0)
		   { PAPIERROR("TLB type could not be recognized, send /proc/pal/cpu0/vm_info"); return(PAPI_EBUG); }

	         if ( !strncmp(buf, "Number of entries", 17) ){
	            num = get_number( buf );
               L[clevel - 1].tlb[cindex].num_entries = num;
	         }
  	         else if ( !strncmp(buf, "Associativity", 13) ) {
	            num = get_number( buf );
               L[clevel - 1].tlb[cindex].associativity = num;
	         }
         }
      } 
      fclose(f);
   }

   /* Compute and store the number of levels of hierarchy actually used */
   for (i=0; i<PAPI_MH_MAX_LEVELS; i++) {
      for (j=0; j<2; j++) {
         if (L[i].tlb[j].type != PAPI_MH_TYPE_EMPTY ||
            L[i].cache[j].type != PAPI_MH_TYPE_EMPTY)
            meminfo->levels = i+1;
      }
   }
   return retval;
}
#endif

#if defined(mips)
/* system type             : MIPS Malta
processor               : 0
cpu model               : MIPS 20Kc V2.0  FPU V2.0
BogoMIPS                : 478.20
wait instruction        : no
microsecond timers      : yes
tlb_entries             : 48 64K pages
icache size             : 32K sets 256 ways 4 linesize 32
dcache size             : 32K sets 256 ways 4 linesize 32
scache....
default cache policy    : cached write-back
extra interrupt vector  : yes
hardware watchpoint     : yes
ASEs implemented        : mips3d
VCED exceptions         : not available
VCEI exceptions         : not available
*/

static int mips_get_cache(char *entry, int *sizeB, int *assoc, int *lineB)
{
  int retval, dummy;

  retval = sscanf(entry,"%dK sets %d ways %d linesize %d",sizeB,&dummy,assoc,lineB);
  *sizeB *= 1024;

  if (retval != 4)
    PAPIERROR("Could not get 4 integers from %s\nPlease send this line to ptools-perfapi@cs.utk.edu",entry);

  SUBDBG("Got cache %d, %d, %d\n",*sizeB,*assoc,*lineB);      
  return(PAPI_OK);
}

static int mips_get_policy(char *s, int *cached, int *policy)
{
  if (strstr(s,"cached"))
    *cached = 1;
  if (strstr(s,"write-back"))
    *policy = PAPI_MH_TYPE_WB | PAPI_MH_TYPE_LRU;
  if (strstr(s,"write-through"))
    *policy = PAPI_MH_TYPE_WT | PAPI_MH_TYPE_LRU;

  if (*policy == 0)
    PAPIERROR("Could not get cache policy from %s\nPlease send this line to ptools-perfapi@cs.utk.edu",s);

  SUBDBG("Got policy 0x%x, cached 0x%x\n",*policy,*cached);
  return(PAPI_OK);
}

static int mips_get_tlb(char *s, int *u, int *size2)
{
  int retval;
  
  retval = sscanf(s,"%d %dK",u,size2);
  *size2 *= 1024;

  if (retval <= 0)
    PAPIERROR("Could not get tlb entries from %s\nPlease send this line to ptools-perfapi@cs.utk.edu",s);
  else if (retval >= 1)
    {
      if (*size2 == 0)
	*size2 = getpagesize();
    }
  SUBDBG("Got tlb %d %d pages\n",*u,*size2);
  return(PAPI_OK);
}

static int mips_get_memory_info(PAPI_hw_info_t *hw_info)
{
  char *s;
  int retval = PAPI_OK;
  int i = 0, cached = 0, policy = 0, num = 0, pagesize = 0, maxlevel = 0;
  int sizeB, assoc, lineB;
  char maxargs[PAPI_HUGE_STR_LEN];
  PAPI_mh_info_t *mh_info = &hw_info->mem_hierarchy;

   FILE *f;

   if ((f = fopen("/proc/cpuinfo", "r")) == NULL)
     { PAPIERROR("fopen(/proc/cpuinfo) errno %d",errno); return(PAPI_ESYS); }

   /* All of this information maybe overwritten by the substrate */

   /* MHZ */

   rewind(f);
   s = search_cpu_info(f, "default cache policy", maxargs);
   if (s && strlen(s))
     {
       mips_get_policy(s+2,&cached,&policy);
       if (cached == 0)
	 {
	   SUBDBG("Uncached default policy detected, reporting zero cache entries.\n");
	   goto nocache;
	 }
     }
   else
     {
       PAPIERROR("Could not locate 'default cache policy' in /proc/cpuinfo\nPlease send the contents of this file to ptools-perfapi@cs.utk.edu");
     }

   rewind(f);
   s = search_cpu_info(f, "icache size", maxargs);
   if (s)
     {
       mips_get_cache(s + 2, &sizeB, &assoc, &lineB);
       mh_info->level[0].cache[i].size = sizeB;
       mh_info->level[0].cache[i].line_size = lineB;
       mh_info->level[0].cache[i].num_lines = sizeB / lineB;
       mh_info->level[0].cache[i].associativity = assoc;
       mh_info->level[0].cache[i].type = PAPI_MH_TYPE_INST | policy;
       i++;
       if (!maxlevel) maxlevel++;
     }
   else
     {
       PAPIERROR("Could not locate 'icache size' in /proc/cpuinfo\nPlease send the contents of this file to ptools-perfapi@cs.utk.edu");
     }       

   rewind(f);
   s = search_cpu_info(f, "dcache size", maxargs);
   if (s)
     {
       mips_get_cache(s + 2, &sizeB, &assoc, &lineB);
       mh_info->level[0].cache[i].size = sizeB;
       mh_info->level[0].cache[i].line_size = lineB;
       mh_info->level[0].cache[i].num_lines = sizeB / lineB;
       mh_info->level[0].cache[i].associativity = assoc;
       mh_info->level[0].cache[i].type = PAPI_MH_TYPE_DATA | policy;
       i++;
       if (!maxlevel) maxlevel++;
     }
   else
     {
       PAPIERROR("Could not locate 'dcache size' in /proc/cpuinfo\nPlease send the contents of this file to ptools-perfapi@cs.utk.edu");
     }       

   rewind(f);
   s = search_cpu_info(f, "scache size", maxargs);
   if (s)
     {
       mips_get_cache(s + 2, &sizeB, &assoc, &lineB);
       mh_info->level[1].cache[0].size = sizeB;
       mh_info->level[1].cache[0].line_size = lineB;
       mh_info->level[1].cache[0].num_lines = sizeB / lineB;
       mh_info->level[1].cache[0].associativity = assoc;
       mh_info->level[1].cache[0].type = PAPI_MH_TYPE_UNIFIED | policy;
       maxlevel++;
     }
   else
     {
#if defined(PFMLIB_MIPS_ICE9A_PMU)&&defined(PFMLIB_MIPS_ICE9A_PMU)
       switch (_perfmon2_pfm_pmu_type)
	 {
	 case PFMLIB_MIPS_ICE9A_PMU:
	 case PFMLIB_MIPS_ICE9B_PMU:
	   mh_info->level[1].cache[0].size = 256*1024;
	   mh_info->level[1].cache[0].line_size = 64;
	   mh_info->level[1].cache[0].num_lines = 256*1024/64;
	   mh_info->level[1].cache[0].associativity = 2;
	   mh_info->level[1].cache[0].type = PAPI_MH_TYPE_UNIFIED;
	   maxlevel++;
	   break;
	 default:
	   break;
	 }
#endif
       /* Hey, it's ok not to have an L2. Slow, but ok. */
     }       


   /* Currently only reports on the JTLB. This is in-fact missing the dual 4-entry uTLB
      that only works on systems with 4K pages. */

 nocache:

   rewind(f);
   s = search_cpu_info(f, "tlb_entries", maxargs);
   if (s && strlen(s))
     {
       int i = 0;
       switch (_perfmon2_pfm_pmu_type)
	 {
	 case PFMLIB_MIPS_5KC_PMU:
#if defined(PFMLIB_MIPS_ICE9A_PMU)&&defined(PFMLIB_MIPS_ICE9A_PMU)
	 case PFMLIB_MIPS_ICE9A_PMU:
	 case PFMLIB_MIPS_ICE9B_PMU:
#endif
	   mh_info->level[i].tlb[0].num_entries = 4;
	   mh_info->level[i].tlb[0].associativity = 4;
	   mh_info->level[i].tlb[0].type = PAPI_MH_TYPE_INST;
	   mh_info->level[i].tlb[1].num_entries = 4;
	   mh_info->level[i].tlb[1].associativity = 4;
	   mh_info->level[i].tlb[1].type = PAPI_MH_TYPE_DATA;
	   i = 1;
	 default:
	   break;
	 }
     
       mips_get_tlb(s+2,&num,&pagesize);
       mh_info->level[i].tlb[0].num_entries = num;
       mh_info->level[i].tlb[0].associativity = num;
       mh_info->level[i].tlb[0].type = PAPI_MH_TYPE_UNIFIED;
       if (maxlevel < i+i)
	 maxlevel = i+1;
     }
   else
     {
       PAPIERROR("Could not locate 'tlb_entries' in /proc/cpuinfo\nPlease send the contents of this file to ptools-perfapi@cs.utk.edu");
     }

   fclose(f);

   mh_info->levels = maxlevel;
   return retval;
}
#endif

#if defined(__powerpc__)

PAPI_mh_info_t sys_mem_info[5] = {
  {3,
    {	 
      { // level 1 begins
        { // tlb's begin
          {PAPI_MH_TYPE_UNIFIED, 1024, 4}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_INST, 65536, 128, 512, 1}, 
          {PAPI_MH_TYPE_DATA, 32768, 128, 256, 2}
        }
      }, 
      {	// level 2 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 1474560, 128, 11520, 8}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
      {	// level 3 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 33554432, 512, 65536, 8}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
    }
  }, // POWER4 end
  {2, // 970 begin
    {	 
      { // level 1 begins
        { // tlb's begin
          {PAPI_MH_TYPE_UNIFIED, 1024, 4}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_INST, 65536, 128, 512, 1}, 
          {PAPI_MH_TYPE_DATA, 32768, 128, 256, 2}
        }
      }, 
      {	// level 2 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 524288, 128, 4096, 8}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
    }
  }, // 970 end
  {3,
    {	 
      { // level 1 begins
        { // tlb's begin
          {PAPI_MH_TYPE_UNIFIED, 1024, 4}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_INST, 65536, 128, 512, 2}, 
          {PAPI_MH_TYPE_DATA, 32768, 128, 256, 4}
        }
      }, 
      {	// level 2 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 1966080, 128, 15360, 10}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
      {	// level 3 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 37748736, 256, 147456, 12}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
    }
  }, // POWER5 end
  {3,
    {	 
      {	// level 1 begins
        { // tlb's begin
          /// POWER6 has an ERAT (Effective to Real Address
          /// Translation) instead of a TLB.  For the purposes of this
          /// data, we will treat it like a TLB.
          {PAPI_MH_TYPE_INST, 128, 2}, 
          {PAPI_MH_TYPE_DATA, 128, 128}
        },
        {	// caches begin
          {PAPI_MH_TYPE_INST, 65536, 128, 512, 4}, 
          {PAPI_MH_TYPE_DATA, 65536, 128, 512, 8}
        }
      }, 
      { // level 2 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 4194304, 128, 16384, 8}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
      {	// level 3 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          /// POWER6 has a 2 slice L3 cache.  Each slice is 16MB, so
          /// combined they are 32MB and usable by each core.  For
          /// this reason, we will treat it as a single 32MB cache.
          {PAPI_MH_TYPE_UNIFIED, 33554432, 128, 262144, 16}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
    }
  },	// POWER6 end
  // This section is a placeholder for Power7 data, and is currently
  // just a copy of the Power6 data.  When Power7 goes public, this
  // should be updated.
  {3,
    {	 
      {	// level 1 begins
        { // tlb's begin
          /// POWER7 has an ERAT (Effective to Real Address
          /// Translation) instead of a TLB.  For the purposes of this
          /// data, we will treat it like a TLB.
          {PAPI_MH_TYPE_INST, 128, 2}, 
          {PAPI_MH_TYPE_DATA, 128, 128}
        },
        {	// caches begin
          {PAPI_MH_TYPE_INST, 65536, 128, 512, 4}, 
          {PAPI_MH_TYPE_DATA, 65536, 128, 512, 8}
        }
      }, 
      { // level 2 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          {PAPI_MH_TYPE_UNIFIED, 4194304, 128, 16384, 8}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
      {	// level 3 begins
        { // tlb's begin
          {PAPI_MH_TYPE_EMPTY, -1, -1},
          {PAPI_MH_TYPE_EMPTY, -1, -1}
        },
        { // caches begin
          /// POWER7 has a 2 slice L3 cache.  Each slice is 16MB, so
          /// combined they are 32MB and usable by each core.  For
          /// this reason, we will treat it as a single 32MB cache.
          {PAPI_MH_TYPE_UNIFIED, 33554432, 128, 262144, 16}, 
          {PAPI_MH_TYPE_EMPTY, -1, -1, -1, -1}
        }
      },	
    }
  }	// POWER7 end
};

#define SPRN_PVR 0x11F /* Processor Version Register */
#define PVR_PROCESSOR_SHIFT 16
static unsigned int mfpvr(void)
{
    unsigned long pvr;

    asm("mfspr          %0,%1" : "=r"(pvr) : "i"(SPRN_PVR));
    return pvr;

}

int ppc64_get_memory_info(PAPI_hw_info_t * hw_info)
{
  unsigned int pvr = mfpvr() >> PVR_PROCESSOR_SHIFT;

  int index;
  switch (pvr) {
  case 0x35: /* POWER4 */
  case 0x38: /* POWER4p */
    index = 0;
    break;
  case 0x39: /* PPC970 */
  case 0x3C: /* PPC970FX */
  case 0x44: /* PPC970MP */
  case 0x45: /* PPC970GX */
    index = 1;
    break;
  case 0x3A: /* POWER5 */
  case 0x3B: /* POWER5+ */
    index = 2;
    break;
  case 0x3E: /* POWER6 */
    index = 3;
    break;
  case 0x3F: /* POWER7 */
    index = 4;
    break;
  default:
    index = -1;
    break;
  }
   
  if (index != -1)
    {
      int cache_level;
      PAPI_mh_info_t sys_mh_inf = sys_mem_info[index];
      PAPI_mh_info_t * mh_inf = &hw_info->mem_hierarchy;
      mh_inf->levels = sys_mh_inf.levels;
      PAPI_mh_level_t * level = mh_inf->level;
      PAPI_mh_level_t sys_mh_level;
      for (cache_level = 0; cache_level < sys_mh_inf.levels; cache_level++)
        {
          sys_mh_level = sys_mh_inf.level[cache_level];
          int cache_idx;
          for (cache_idx = 0; cache_idx < 2; cache_idx++)
            {
              // process TLB info
              PAPI_mh_tlb_info_t curr_tlb = sys_mh_level.tlb[cache_idx];
              int type = curr_tlb.type;
              if (type != PAPI_MH_TYPE_EMPTY)
                {
                  level[cache_level].tlb[cache_idx].type = type;
                  level[cache_level].tlb[cache_idx].associativity = curr_tlb.associativity;
                  level[cache_level].tlb[cache_idx].num_entries = curr_tlb.num_entries;
                }
            }
          for (cache_idx = 0; cache_idx < 2; cache_idx++)
            {
              // process cache info
              PAPI_mh_cache_info_t curr_cache = sys_mh_level.cache[cache_idx];
              int type = curr_cache.type;
              if (type != PAPI_MH_TYPE_EMPTY)
                {
                  level[cache_level].cache [cache_idx].type = type;
                  level[cache_level].cache[cache_idx].associativity = curr_cache.associativity;
                  level[cache_level].cache[cache_idx].size = curr_cache.size;
                  level[cache_level].cache[cache_idx].line_size = curr_cache.line_size;
                  level[cache_level].cache[cache_idx].num_lines = curr_cache.num_lines;
                }
            }
        }
    }
  return 0;
}
#endif

#if defined(__sparc__)
static int sparc_sysfs_cpu_attr(char *name, char **result)
{
	const char *path_base = "/sys/devices/system/cpu/";
	char path_buf[PATH_MAX];
	char val_buf[32];
	DIR *sys_cpu;

	sys_cpu = opendir(path_base);
	if (sys_cpu) {
		struct dirent *cpu;

		while ((cpu = readdir(sys_cpu)) != NULL) {
			int fd;

			if (strncmp("cpu", cpu->d_name, 3))
				continue;
			strcpy(path_buf, path_base);
			strcat(path_buf, cpu->d_name);
			strcat(path_buf, "/");
			strcat(path_buf, name);

			fd = open(path_buf, O_RDONLY);
			if (fd < 0)
				continue;

			if (read(fd, val_buf, 32) < 0)
				continue;
			close(fd);

			*result = strdup(val_buf);
			return 0;
		}
	}
	return -1;
}

static int sparc_cpu_attr(char *name, unsigned long long *val)
{
	char *buf;
	int r;

	r = sparc_sysfs_cpu_attr(name, &buf);
	if (r == -1)
		return -1;

	sscanf(buf, "%llu", val);

	free(buf);

	return 0;
}

static int sparc_get_memory_info(PAPI_hw_info_t *hw_info)
{
	unsigned long long cache_size, cache_line_size;
	unsigned long long cycles_per_second;
	char maxargs[PAPI_HUGE_STR_LEN];
	PAPI_mh_tlb_info_t *tlb;
	PAPI_mh_level_t *level;
	char *s, *t;
	FILE *f;

	/* First, fix up the cpu vendor/model/etc. values */
	strcpy(hw_info->vendor_string, "Sun");
	hw_info->vendor = PAPI_VENDOR_SUN;

	f = fopen("/proc/cpuinfo", "r");
	if (!f)
		return PAPI_ESYS;

	rewind(f);
	s = search_cpu_info(f, "cpu", maxargs);
	if (!s) {
		fclose(f);
		return PAPI_ESYS;
	}
	
	t = strchr(s + 2, '\n');
	if (!t) {
		fclose(f);
		return PAPI_ESYS;
	}

	*t = '\0';
	strcpy(hw_info->model_string, s + 2);
	
	fclose(f);

	if (sparc_sysfs_cpu_attr("clock_tick", &s) == -1)
		return PAPI_ESYS;

	sscanf(s, "%llu", &cycles_per_second);
	free(s);

	hw_info->mhz = cycles_per_second / 1000000;
	hw_info->clock_mhz = hw_info->mhz;

	/* Now fetch the cache info */
	hw_info->mem_hierarchy.levels = 3;

	level = &hw_info->mem_hierarchy.level[0];

	sparc_cpu_attr("l1_icache_size", &cache_size);
	sparc_cpu_attr("l1_icache_line_size", &cache_line_size);
	level[0].cache[0].type = PAPI_MH_TYPE_INST;
	level[0].cache[0].size = cache_size;
	level[0].cache[0].line_size = cache_line_size;
	level[0].cache[0].num_lines = cache_size / cache_line_size;
	level[0].cache[0].associativity = 1;

	sparc_cpu_attr("l1_dcache_size", &cache_size);
	sparc_cpu_attr("l1_dcache_line_size", &cache_line_size);
	level[0].cache[1].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_WT;
	level[0].cache[1].size = cache_size;
	level[0].cache[1].line_size = cache_line_size;
	level[0].cache[1].num_lines = cache_size / cache_line_size;
	level[0].cache[1].associativity = 1;

	sparc_cpu_attr("l2_cache_size", &cache_size);
	sparc_cpu_attr("l2_cache_line_size", &cache_line_size);
	level[1].cache[0].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_WB;
	level[1].cache[0].size = cache_size;
	level[1].cache[0].line_size = cache_line_size;
	level[1].cache[0].num_lines = cache_size / cache_line_size;
	level[1].cache[0].associativity = 1;

	tlb = &hw_info->mem_hierarchy.level[0].tlb[0];
	switch (_perfmon2_pfm_pmu_type) {
	case PFMLIB_SPARC_ULTRA12_PMU:
		tlb[0].type = PAPI_MH_TYPE_INST | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[0].num_entries = 64;
		tlb[0].associativity = SHRT_MAX;
		tlb[1].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[1].num_entries = 64;
		tlb[1].associativity = SHRT_MAX;
		break;

	case PFMLIB_SPARC_ULTRA3_PMU:
	case PFMLIB_SPARC_ULTRA3I_PMU:
	case PFMLIB_SPARC_ULTRA3PLUS_PMU:
	case PFMLIB_SPARC_ULTRA4PLUS_PMU:
	  level[0].cache[0].associativity = 4;
	  level[0].cache[1].associativity = 4;
	  level[1].cache[0].associativity = 4;
	  
	  tlb[0].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_PSEUDO_LRU;
	  tlb[0].num_entries = 16;
	  tlb[0].associativity = SHRT_MAX;
	  tlb[1].type = PAPI_MH_TYPE_INST | PAPI_MH_TYPE_PSEUDO_LRU;
	  tlb[1].num_entries = 16;
	  tlb[1].associativity = SHRT_MAX;
	  tlb[2].type = PAPI_MH_TYPE_DATA;
	  tlb[2].num_entries = 1024;
	  tlb[2].associativity = 2;
	  tlb[3].type = PAPI_MH_TYPE_INST;
	  tlb[3].num_entries = 128;
	  tlb[3].associativity = 2;
	  break;

	case PFMLIB_SPARC_NIAGARA1:
		level[0].cache[0].associativity = 4;
		level[0].cache[1].associativity = 4;
		level[1].cache[0].associativity = 12;

		tlb[0].type = PAPI_MH_TYPE_INST | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[0].num_entries = 64;
		tlb[0].associativity = SHRT_MAX;
		tlb[1].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[1].num_entries = 64;
		tlb[1].associativity = SHRT_MAX;
		break;

	case PFMLIB_SPARC_NIAGARA2:
		level[0].cache[0].associativity = 8;
		level[0].cache[1].associativity = 4;
		level[1].cache[0].associativity = 16;

		tlb[0].type = PAPI_MH_TYPE_INST | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[0].num_entries = 64;
		tlb[0].associativity = SHRT_MAX;
		tlb[1].type = PAPI_MH_TYPE_DATA | PAPI_MH_TYPE_PSEUDO_LRU;
		tlb[1].num_entries = 128;
		tlb[1].associativity = SHRT_MAX;
		break;
	}

	return 0;
}
#endif

int _papi_pfm_get_memory_info(PAPI_hw_info_t * hwinfo, int unused)
{
  int retval = PAPI_OK;

#if defined(mips)
  mips_get_memory_info(hwinfo);
#elif defined(__i386__)||defined(__x86_64__)
  x86_get_memory_info(hwinfo);
#elif defined(__ia64__)
  ia64_get_memory_info(hwinfo);
#elif defined(__powerpc__)
  ppc64_get_memory_info(hwinfo);
#elif defined(__sparc__)
  sparc_get_memory_info(hwinfo);
#else
#error "No support for this architecture. Please modify perfmon.c"
#endif

  return(retval);
}

int _papi_pfm_update_shlib_info(void)
{
#ifndef WIN32
   char fname[PAPI_HUGE_STR_LEN];
   char buf[PAPI_HUGE_STR_LEN+PAPI_HUGE_STR_LEN], perm[5], dev[16];
   char mapname[PAPI_HUGE_STR_LEN], lastmapname[PAPI_HUGE_STR_LEN];
   unsigned long begin = 0, end = 0, size = 0, inode = 0, foo = 0;
   unsigned long t_index = 0, d_index = 0, b_index = 0, counting = 1;
   PAPI_address_map_t *tmp = NULL;
   FILE *f;

   memset(fname,0x0,sizeof(fname));
   memset(buf,0x0,sizeof(buf));
   memset(perm,0x0,sizeof(perm));
   memset(dev,0x0,sizeof(dev));
   memset(mapname,0x0,sizeof(mapname));
   memset(lastmapname,0x0,sizeof(lastmapname));

   sprintf(fname, "/proc/%ld/maps", (long) _papi_hwi_system_info.pid);
   f = fopen(fname, "r");
                                                                                
   if (!f)
     {
         PAPIERROR("fopen(%s) returned < 0", fname);
         return(PAPI_OK);
     }
                                                                                
 again:
   while (!feof(f)) {
      begin = end = size = inode = foo = 0;
      if (fgets(buf, sizeof(buf), f) == 0)
         break;
      /* If mapname is null in the string to be scanned, we need to detect that */
      if (strlen(mapname))
        strcpy(lastmapname,mapname);
      else
        lastmapname[0] = '\0';
      /* If mapname is null in the string to be scanned, we need to detect that */
      mapname[0] = '\0';
      sscanf(buf, "%lx-%lx %4s %lx %s %ld %s", &begin, &end, perm,
             &foo, dev, &inode, mapname);
      size = end - begin;
                                                                                
      /* the permission string looks like "rwxp", where each character can
       * be either the letter, or a hyphen.  The final character is either
       * p for private or s for shared. */
                                                                                
      if (counting)
        {
          if ((perm[2] == 'x') && (perm[0] == 'r') && (inode != 0))
            {
              if  (strcmp(_papi_hwi_system_info.exe_info.fullname,mapname) == 0)                {
                  _papi_hwi_system_info.exe_info.address_info.text_start = (caddr_t) begin;
                  _papi_hwi_system_info.exe_info.address_info.text_end =
                    (caddr_t) (begin + size);
                }
              t_index++;
            }
          else if ((perm[0] == 'r') && (perm[1] == 'w') && (inode != 0) && (strcmp(_papi_hwi_system_info.exe_info.fullname,mapname) == 0))
            {
              _papi_hwi_system_info.exe_info.address_info.data_start = (caddr_t) begin;
              _papi_hwi_system_info.exe_info.address_info.data_end =
                (caddr_t) (begin + size);
              d_index++;
            }
          else if ((perm[0] == 'r') && (perm[1] == 'w') && (inode == 0) && (strcmp(_papi_hwi_system_info.exe_info.fullname,lastmapname) == 0))
            {
              _papi_hwi_system_info.exe_info.address_info.bss_start = (caddr_t) begin;
              _papi_hwi_system_info.exe_info.address_info.bss_end =
                (caddr_t) (begin + size);
              b_index++;
            }
        }
      else if (!counting)
        {
          if ((perm[2] == 'x') && (perm[0] == 'r') && (inode != 0))
            {
              if (strcmp(_papi_hwi_system_info.exe_info.fullname,mapname) != 0)
                {
              t_index++;
                  tmp[t_index-1 ].text_start = (caddr_t) begin;
                  tmp[t_index-1 ].text_end = (caddr_t) (begin + size);
                  strncpy(tmp[t_index-1 ].name, mapname, PAPI_MAX_STR_LEN);
                }
            }
          else if ((perm[0] == 'r') && (perm[1] == 'w') && (inode != 0))
            {
              if ( (strcmp(_papi_hwi_system_info.exe_info.fullname,mapname) != 0)
               && (t_index >0 ) && (tmp[t_index-1 ].data_start == 0))
                {
                  tmp[t_index-1 ].data_start = (caddr_t) begin;
                  tmp[t_index-1 ].data_end = (caddr_t) (begin + size);
                }
            }
          else if ((perm[0] == 'r') && (perm[1] == 'w') && (inode == 0))
            {
              if ((t_index > 0 ) && (tmp[t_index-1].bss_start == 0))
                {
                  tmp[t_index-1].bss_start = (caddr_t) begin;
                  tmp[t_index-1].bss_end = (caddr_t) (begin + size);
                }
            }
        }
   }

   if (counting) {
      /* When we get here, we have counted the number of entries in the map
         for us to allocate */
                                                                                
      tmp = (PAPI_address_map_t *) papi_calloc(t_index, sizeof(PAPI_address_map_t));
      if (tmp == NULL)
        { PAPIERROR("Error allocating shared library address map"); return(PAPI_ENOMEM); }
      t_index = 0;
      rewind(f);
      counting = 0;
      goto again;
   } else {
      if (_papi_hwi_system_info.shlib_info.map)
         papi_free(_papi_hwi_system_info.shlib_info.map);
      _papi_hwi_system_info.shlib_info.map = tmp;
      _papi_hwi_system_info.shlib_info.count = t_index;
                                                                                
      fclose(f);
   }
#endif /* ! WIN32 */
   return (PAPI_OK);
}
           
#ifdef WIN32
static char *basename(const char *path)
{
  const char *s = path, *last = NULL;
  while (*s != '\0')
    if (*s == '\\')
      last = s;

  return last ? last + 1 : path;
}
#endif

static int get_system_info(papi_mdi_t *mdi)
{
   int retval;
   /* Software info */

   /* Path and args */

#ifdef WIN32
   SYSTEM_INFO si;
   HANDLE hModule;
   int len;

   _papi_hwi_system_info.pid = getpid();

   hModule = GetModuleHandle(NULL);   // current process
   len = GetModuleFileName(hModule, mdi->exe_info.fullname, PAPI_MAX_STR_LEN);
   if (len)
     strcpy(mdi->exe_info.address_info.name, mdi->exe_info.fullname);
   else
     return(PAPI_ESYS);
#else
   char maxargs[PAPI_HUGE_STR_LEN];
   pid_t pid;

   pid = getpid();
   if (pid < 0)
     { PAPIERROR("getpid() returned < 0"); return(PAPI_ESYS); }
   mdi->pid = pid;

   sprintf(maxargs, "/proc/%d/exe", (int) pid);
   if (readlink(maxargs, mdi->exe_info.fullname, PAPI_HUGE_STR_LEN) < 0)
     { PAPIERROR("readlink(%s) returned < 0", maxargs); return(PAPI_ESYS); }

   /* Careful, basename can modify it's argument */

   strcpy(maxargs,mdi->exe_info.fullname);
   strcpy(mdi->exe_info.address_info.name, basename(maxargs));
#endif
   SUBDBG("Executable is %s\n", mdi->exe_info.address_info.name);
   SUBDBG("Full Executable is %s\n", mdi->exe_info.fullname);

   /* Executable regions, may require reading /proc/pid/maps file */

   retval = _papi_pfm_update_shlib_info();
   SUBDBG("Text: Start %p, End %p, length %d\n",
          mdi->exe_info.address_info.text_start,
          mdi->exe_info.address_info.text_end,
          (int)(mdi->exe_info.address_info.text_end -
          mdi->exe_info.address_info.text_start));
   SUBDBG("Data: Start %p, End %p, length %d\n",
          mdi->exe_info.address_info.data_start,
          mdi->exe_info.address_info.data_end,
          (int)(mdi->exe_info.address_info.data_end -
          mdi->exe_info.address_info.data_start));
   SUBDBG("Bss: Start %p, End %p, length %d\n",
          mdi->exe_info.address_info.bss_start,
          mdi->exe_info.address_info.bss_end,
          (int)(mdi->exe_info.address_info.bss_end -
          mdi->exe_info.address_info.bss_start));

   /* PAPI_preload_option information */

   strcpy(mdi->preload_info.lib_preload_env, "LD_PRELOAD");
   mdi->preload_info.lib_preload_sep = ' ';
   strcpy(mdi->preload_info.lib_dir_env, "LD_LIBRARY_PATH");
   mdi->preload_info.lib_dir_sep = ':';

   /* Hardware info */
#ifdef WIN32
  GetSystemInfo(&si);
  mdi->hw_info.ncpu = mdi->hw_info.totalcpus = si.dwNumberOfProcessors;
  mdi->hw_info.nnodes = 1;
#endif

  retval = get_cpu_info(&mdi->hw_info);
  if (retval)
    return(retval);

  retval = _papi_pfm_get_memory_info(&mdi->hw_info,mdi->hw_info.model);
  if (retval)
    return(retval);

   SUBDBG("Found %d %s(%d) %s(%d) CPU's at %f Mhz, clock %d Mhz.\n",
          mdi->hw_info.totalcpus,
          mdi->hw_info.vendor_string,
          mdi->hw_info.vendor,
          mdi->hw_info.model_string,
          mdi->hw_info.model,
	  mdi->hw_info.mhz,
	  mdi->hw_info.clock_mhz);
                                                                               
   return (PAPI_OK);
}

inline_static pid_t mygettid(void)
{
#ifdef SYS_gettid
  return(syscall(SYS_gettid));
#elif defined(__NR_gettid)
  return(syscall(__NR_gettid));
#elif defined WIN32
  return GetCurrentThreadId();
#else
  return(syscall(1105));  
#endif
}


inline static int compute_kernel_args(hwd_control_state_t * ctl0)
{
  pfm_control_state_t *ctl = (pfm_control_state_t *)ctl0;
  pfmlib_input_param_t *inp = &ctl->in;
  pfmlib_output_param_t *outp = &ctl->out;
  pfmlib_input_param_t tmpin;
  pfmlib_output_param_t tmpout;
#if 0
  /* This will be used to fixup the overflow and sample args after re-allocation */
  pfarg_pmd_t oldpd;
#endif
  pfarg_pmd_t *pd = ctl->pd;
  pfarg_pmc_t *pc = ctl->pc;
  pfarg_setdesc_t *sets = ctl->set;
  pfarg_setinfo_t *setinfos = ctl->setinfo;
  int *num_sets = &ctl->num_sets;
  int set = 0, donepc = 0, donepd = 0, ret, i, j;
  int togo = inp->pfp_event_count, dispatch_count = inp->pfp_event_count, done = 0;
  
  /* Save old PD array so we can reconstruct certain flags. This can be removed
     when we have higher level code call set_profile,set_overflow etc when there
     is hardware (substrate) support, but this change won't happen for PAPI 3.5 */

  SUBDBG("entry multiplexed %d, pfp_event_count %d, num_cntrs %d, num_sets %d\n",ctl->multiplexed, inp->pfp_event_count, MY_VECTOR.cmp_info.num_cntrs,*num_sets);
  if ((ctl->multiplexed) && (inp->pfp_event_count > MY_VECTOR.cmp_info.num_cntrs))
    {
	dispatch_count = MY_VECTOR.cmp_info.num_cntrs;
    }

  while (togo)
    {
    again:
      memset(&tmpin,0x0,sizeof(tmpin));
      memset(&tmpout,0x0,sizeof(tmpout));
      
      SUBDBG("togo %d, done %d, dispatch_count %d, num_cntrs %d\n",togo,done,dispatch_count,MY_VECTOR.cmp_info.num_cntrs);
      tmpin.pfp_event_count = dispatch_count;
      tmpin.pfp_dfl_plm = inp->pfp_dfl_plm;

      /* Make sure we tell dispatch that these PMC's are not available */
      memcpy(&tmpin.pfp_unavail_pmcs,&_perfmon2_pfm_unavailable_pmcs,sizeof(_perfmon2_pfm_unavailable_pmcs));

      for (i=0,j=done;i<dispatch_count;i++,j++)
	{
	  memcpy(tmpin.pfp_events+i,inp->pfp_events+j,sizeof(pfmlib_event_t));
	}

      if ((ret = pfm_dispatch_events(&tmpin, NULL, &tmpout, NULL)) != PFMLIB_SUCCESS)
	{
	  if (ctl->multiplexed) 
	    {
	      dispatch_count--;
	      if (dispatch_count == 0)
		{
		  PAPIERROR("pfm_dispatch_events(): %s", pfm_strerror(ret));
		  return(PAPI_ECNFLCT);
		}
	      SUBDBG("Dispatch failed because of counter conflict, trying again with %d counters.\n",dispatch_count);
	      goto again;
	    }
	  PAPIERROR("pfm_dispatch_events(): %s", pfm_strerror(ret));
	  return(PAPI_ECNFLCT);
	}
  
  /*
    * Now prepare the argument to initialize the PMDs and PMCS.
    * We must pfp_pmc_count to determine the number of PMC to intialize.
    * We must use pfp_event_count to determine the number of PMD to initialize.
    * Some events causes extra PMCs to be used, so  pfp_pmc_count may be >= pfp_event_count.
    *
    * This step is new compared to libpfm-2.x. It is necessary because the library no
    * longer knows about the kernel data structures.
    */

      for (i=0; i < tmpout.pfp_pmc_count; i++, donepc++) 
	{
	  pc[donepc].reg_num   = tmpout.pfp_pmcs[i].reg_num;
	  pc[donepc].reg_value = tmpout.pfp_pmcs[i].reg_value;
	  pc[donepc].reg_set = set;
	  SUBDBG("PC%d (i%d) is reg num %d, value %llx, set %d\n",donepc,i,pc[donepc].reg_num,(unsigned long long)pc[donepc].reg_value,pc[donepc].reg_set);
	}
   
      /* figure out pmd mapping from output pmc */

#if defined(HAVE_PFM_REG_EVT_IDX)
      for (i=0, j=0; i < tmpin.pfp_event_count; i++, donepd++) 
	{
	  pd[donepd].reg_num = tmpout.pfp_pmcs[j].reg_pmd_num;
	  pd[donepd].reg_set = set;
	  SUBDBG("PD%d (i%d,j%d) is reg num %d, set %d\n",donepd,i,j,pd[donepd].reg_num,pd[donepd].reg_set);

	  /* Skip over entries that map to the same PMD, 
	     PIV has 2 PMCS for every PMD */

	  for(; j < tmpout.pfp_pmc_count; j++)  
	    if (tmpout.pfp_pmcs[j].reg_evt_idx != i) break;
	}
#else
      for (i=0; i < tmpout.pfp_pmd_count; i++, donepd++) 
        {
          pd[donepd].reg_num = tmpout.pfp_pmds[i].reg_num;
	  pd[donepd].reg_set = set;
	  SUBDBG("PD%d (i%d) is reg num %d, set %d\n",donepd,i,pd[donepd].reg_num,pd[donepd].reg_set);
        }
#endif
      
      togo -= dispatch_count;
      done += dispatch_count;
      if (togo > MY_VECTOR.cmp_info.num_cntrs)
	dispatch_count = MY_VECTOR.cmp_info.num_cntrs;
      else
	dispatch_count = togo;

      setinfos[set].set_id = set;
      sets[set].set_id = set;
      set++;
    }

  *num_sets = set;
  outp->pfp_pmc_count = donepc;

  if (ctl->multiplexed && (set > 1))
    {
      for (i=0;i<set;i++) {
	sets[i].set_flags = PFM_SETFL_TIME_SWITCH;
	sets[i].set_timeout = ctl->multiplexed;
      }
    }
  SUBDBG("exit multiplexed %d (ns switch time), pfp_pmc_count %d, num_sets %d\n",ctl->multiplexed, outp->pfp_pmc_count, *num_sets);
  return(PAPI_OK);
}

int tune_up_fd(int ctx_fd)
{
#ifndef WIN32
  int ret;

  /* set close-on-exec to ensure we will be getting the PFM_END_MSG, i.e.,
   * fd not visible to child. */
  ret = fcntl(ctx_fd, F_SETFD, FD_CLOEXEC);
  if (ret == -1)
    {
      PAPIERROR("cannot fcntl(FD_CLOEXEC) on %d: %s", ctx_fd,strerror(errno));
      return(PAPI_ESYS);
    }
  /* setup asynchronous notification on the file descriptor */
  ret = fcntl(ctx_fd, F_SETFL, fcntl(ctx_fd, F_GETFL, 0) | O_ASYNC);
  if (ret == -1)
    {
      PAPIERROR("cannot fcntl(O_ASYNC) on %d: %s", ctx_fd,strerror(errno));
      return(PAPI_ESYS);
    }
  /* get ownership of the descriptor */
  ret = fcntl(ctx_fd, F_SETOWN, mygettid());
  if (ret == -1)
    {
      PAPIERROR("cannot fcntl(F_SETOWN) on %d: %s", ctx_fd,strerror(errno));
      return(PAPI_ESYS);
    }
  /*
   * when you explicitely declare that you want a particular signal,
   * even with you use the default signal, the kernel will send more
   * information concerning the event to the signal handler.
   *
   * In particular, it will send the file descriptor from which the
   * event is originating which can be quite useful when monitoring
   * multiple tasks from a single thread.
   */
  ret = fcntl(ctx_fd, F_SETSIG, MY_VECTOR.cmp_info.hardware_intr_sig);
  if (ret == -1)
    {
      PAPIERROR("cannot fcntl(F_SETSIG,%d) on %d: %s", MY_VECTOR.cmp_info.hardware_intr_sig, ctx_fd, strerror(errno));
      return(PAPI_ESYS);
    }
#endif /* ! WIN32 */
  return(PAPI_OK);
}

#ifndef WIN32
static int attach(hwd_control_state_t *ctl, unsigned long tid)
{
  pfarg_ctx_t *newctx = (pfarg_ctx_t *)malloc(sizeof(pfarg_ctx_t));
  pfarg_load_t *load_args = (pfarg_load_t *)malloc(sizeof(pfarg_load_t));
  int ret;

  if ((newctx == NULL) || (load_args == NULL))
    return(PAPI_ENOMEM);
  memset(newctx,0x0,sizeof(*newctx));
  memset(load_args,0,sizeof(*load_args));

  /* Make sure the process exists and is being ptraced() */

  ret = ptrace(PTRACE_ATTACH, tid, NULL, NULL);
  if (ret == 0)
    {
      ptrace(PTRACE_DETACH, tid, NULL, NULL);
      PAPIERROR("Process/thread %d is not being ptraced",tid);
      free(newctx); free(load_args);
      return(PAPI_EINVAL);
    }
  /* If we get here, then we should hope that the process is being
     ptraced, if not, then we probably can't attach to it. */

  if ((ret == -1) && (errno != EPERM))
    {
      PAPIERROR("Process/thread %d cannot be ptraced: %s",tid,strerror(errno));
      free(newctx);
      free(load_args);
      return(PAPI_EINVAL);
    }

  SUBDBG("PFM_CREATE_CONTEXT(%p,%p,%p,%d)\n",newctx,NULL,NULL,0);
  if ((ret = pfm_create_context(newctx, NULL, NULL, 0)) == -1)
    {
      PAPIERROR("attach:pfm_create_context(): %s", strerror(errno));
      free(newctx); 
      free(load_args);
      return(PAPI_ESYS);
    }
  SUBDBG("PFM_CREATE_CONTEXT returned fd %d\n",ret);
  tune_up_fd(ret);

  ((pfm_control_state_t *)ctl)->ctx_fd = ret;
  ((pfm_control_state_t *)ctl)->ctx = newctx;
  load_args->load_pid = tid;
  ((pfm_control_state_t *)ctl)->load = load_args;

  return(PAPI_OK);
}

static int detach(hwd_context_t *ctx, hwd_control_state_t *ctl)
{
  int i;

  i = close(((pfm_control_state_t *)ctl)->ctx_fd);
  SUBDBG("CLOSE fd %d returned %d\n",((pfm_control_state_t *)ctl)->ctx_fd,i);

  /* Restore to main threads context */
  free(((pfm_control_state_t *)ctl)->ctx);
  ((pfm_control_state_t *)ctl)->ctx = &((pfm_context_t *)ctx)->ctx;
  ((pfm_control_state_t *)ctl)->ctx_fd = ((pfm_context_t *)ctx)->ctx_fd;
  free(((pfm_control_state_t *)ctl)->load);
  ((pfm_control_state_t *)ctl)->load = &((pfm_context_t *)ctx)->load;

  return(PAPI_OK);
}
#endif /* ! WIN32 */

inline static int set_domain(hwd_control_state_t * ctl0, int domain)
{
  pfm_control_state_t *ctl = (pfm_control_state_t *) ctl0;
  int mode = 0, did = 0;
  pfmlib_input_param_t *inp = &ctl->in;

   if (domain & PAPI_DOM_USER) {
      did = 1;
      mode |= PFM_PLM3;
   }

   if (domain & PAPI_DOM_KERNEL) {
     did = 1;
     mode |= PFM_PLM0;
   }

   if (domain & PAPI_DOM_SUPERVISOR) {
      did = 1;
      mode |= PFM_PLM1;
   }

   if (domain & PAPI_DOM_OTHER) {
      did = 1;
      mode |= PFM_PLM2;
   }

   if (!did)
      return (PAPI_EINVAL);

   inp->pfp_dfl_plm = mode;

   return(compute_kernel_args(ctl));
}

inline static int set_granularity(hwd_control_state_t * this_state, int domain)
{
   switch (domain) {
   case PAPI_GRN_PROCG:
   case PAPI_GRN_SYS:
   case PAPI_GRN_SYS_CPU:
   case PAPI_GRN_PROC:
      return(PAPI_ESBSTR);
   case PAPI_GRN_THR:
      break;
   default:
      return (PAPI_EINVAL);
   }
   return (PAPI_OK);
}

/* This function should tell your kernel extension that your children
   inherit performance register information and propagate the values up
   upon child exit and parent wait. */

inline static int set_inherit(int arg)
{
   return (PAPI_ESBSTR);
}

static int get_string_from_file(char *file, char *str, int len)
{
  FILE *f = fopen(file,"r");
  char buf[PAPI_HUGE_STR_LEN];
  if (f == NULL)
    {
      PAPIERROR("fopen(%s): %s", file, strerror(errno));
      return(PAPI_ESYS);
    }
  if (fscanf(f,"%s\n",buf) != 1)
    {
      PAPIERROR("fscanf(%s, %%s\\n): Unable to scan 1 token", file);
      fclose(f);
      return(PAPI_ESBSTR);
    }
  strncpy(str,buf,(len > PAPI_HUGE_STR_LEN ? PAPI_HUGE_STR_LEN : len));
  fclose(f);
  return(PAPI_OK);
}

int _papi_pfm_init_substrate(int cidx)
{
  int i, retval;
  unsigned int ncnt;
  unsigned int version;
  char pmu_name[PAPI_MIN_STR_LEN];
#ifdef DEBUG
  pfmlib_options_t pfmlib_options;
#endif
  char buf[PAPI_HUGE_STR_LEN];

/*#ifndef PAPI_NO_VECTOR
  // Setup the vector entries that the OS knows about 
  retval = _papi_hwi_setup_vector_table( vtable, _linux_pfm_table);
  if ( retval != PAPI_OK ) return(retval);
  // And the vector entries for native event control 
  retval = _papi_hwi_setup_vector_table( vtable, _papi_pfm_event_vectors);
  if ( retval != PAPI_OK ) return(retval);
#endif
*/
      /* The following checks the PFMLIB version 
	  against the perfmon2 kernel version... */
   strncpy(MY_VECTOR.cmp_info.support_version,buf,sizeof(MY_VECTOR.cmp_info.support_version));

#ifdef WIN32
   strcpy(_papi_hwi_system_info.sub_info.kernel_version, "PFM for Windows");
   OpenWinPMCDriver();
#else
   retval = get_string_from_file("/sys/kernel/perfmon/version",MY_VECTOR.cmp_info.kernel_version,sizeof(MY_VECTOR.cmp_info.kernel_version));
   if (retval != PAPI_OK)
      return(retval);
#ifdef PFM_VERSION
   sprintf(buf, "%d.%d", PFM_VERSION_MAJOR(PFM_VERSION), PFM_VERSION_MINOR(PFM_VERSION));
   SUBDBG("Perfmon2 library versions...kernel: %s library: %s\n", MY_VECTOR.cmp_info.kernel_version, buf);
   if (strcmp (MY_VECTOR.cmp_info.kernel_version, buf) != 0) {
	   /* do a little exception processing; 81 is compatible with 80 */
	   if (!((PFM_VERSION_MINOR(PFM_VERSION) == 81)
		   && (strncmp (MY_VECTOR.cmp_info.kernel_version, "2.8", 3) == 0))) {
		  PAPIERROR("Version mismatch of libpfm: compiled %s vs. installed %s\n",
				  buf, MY_VECTOR.cmp_info.kernel_version);
		   return (PAPI_ESBSTR);
	   }
   }
#endif
#endif
   /* The following checks the version of the PFM library 
	  against the version PAPI linked to... */
   SUBDBG("pfm_initialize()\n");
   if ((retval = pfm_initialize()) != PFMLIB_SUCCESS)
     {
       PAPIERROR("pfm_initialize(): %s", pfm_strerror(retval));
       return (PAPI_ESBSTR);
     }

   SUBDBG("pfm_get_version(%p)\n",&version);
   if (pfm_get_version(&version) != PFMLIB_SUCCESS) {
       PAPIERROR("pfm_get_version(%p): %s", version, pfm_strerror(retval));
       return (PAPI_ESBSTR);
   }

   sprintf(MY_VECTOR.cmp_info.support_version, "%d.%d", PFM_VERSION_MAJOR(version), PFM_VERSION_MINOR(version));

   if (PFM_VERSION_MAJOR(version) != PFM_VERSION_MAJOR(PFMLIB_VERSION)) {
      PAPIERROR("Version mismatch of libpfm: compiled %x vs. installed %x\n",
              PFM_VERSION_MAJOR(PFMLIB_VERSION), PFM_VERSION_MAJOR(version));
      return (PAPI_ESBSTR);
   }

  /* Load the module, find out if any PMC's/PMD's are off limits */

   {

     /* Perfmon2 timeouts are based on the clock tick, we need to check
	them otherwise it will complain at us when we multiplex */
    
	   unsigned long min_timeout_ns;
#ifdef _WIN32
	 min_timeout_ns = 10000000;
#else
	 struct timespec ts;
     if (syscall(__NR_clock_getres,CLOCK_REALTIME,&ts) == -1) { 
       PAPIERROR("Could not detect proper HZ rate, multiplexing may fail\n");
       min_timeout_ns = 10000000;
     } else {
       min_timeout_ns = ts.tv_nsec;
     }
#endif

     /* This will fail if we've done timeout detection wrong */
     retval = detect_timeout_and_unavail_pmu_regs(&_perfmon2_pfm_unavailable_pmcs,
						  &_perfmon2_pfm_unavailable_pmds,
						  &min_timeout_ns);

     MY_VECTOR.cmp_info.itimer_ns = 10000000;
     /* This field represents the minimum timer resolution. Anything lower
	is not possible. Anything higher and it must be a multiple of this */
     MY_VECTOR.cmp_info.itimer_res_ns = min_timeout_ns;

     if (retval != PAPI_OK)
       return(retval);
   }

  /* Always initialize globals dynamically to handle forks properly. */

  _perfmon2_pfm_pmu_type = -1;

   /* Opened once for all threads. */
   SUBDBG("pfm_get_pmu_type(%p)\n",&_perfmon2_pfm_pmu_type);
   if (pfm_get_pmu_type(&_perfmon2_pfm_pmu_type) != PFMLIB_SUCCESS)
     {
       PAPIERROR("pfm_get_pmu_type(%p): %s", _perfmon2_pfm_pmu_type, pfm_strerror(retval));
       return (PAPI_ESBSTR);
     }

   pmu_name[0] = '\0';
   if (pfm_get_pmu_name(pmu_name,PAPI_MIN_STR_LEN) != PFMLIB_SUCCESS)
     {
       PAPIERROR("pfm_get_pmu_name(%p,%d): %s",pmu_name,PAPI_MIN_STR_LEN,pfm_strerror(retval));
       return (PAPI_ESBSTR);
     }
   SUBDBG("PMU is a %s, type %d\n",pmu_name,_perfmon2_pfm_pmu_type);

#ifdef DEBUG
   memset(&pfmlib_options, 0, sizeof(pfmlib_options));
   if (ISLEVEL(DEBUG_SUBSTRATE)) {
      pfmlib_options.pfm_debug = 1;
      pfmlib_options.pfm_verbose = 1;
   }
   SUBDBG("pfm_set_options(%p)\n",&pfmlib_options);
   if (pfm_set_options(&pfmlib_options))
     {
       PAPIERROR("pfm_set_options(%p): %s", &pfmlib_options, pfm_strerror(retval));
       return (PAPI_ESBSTR);
     }
#endif

   /* Fill in sub_info */

  SUBDBG("pfm_get_num_events(%p)\n",&ncnt);
  if ((retval = pfm_get_num_events(&ncnt)) != PFMLIB_SUCCESS)
    {
      PAPIERROR("pfm_get_num_events(%p): %s\n", &ncnt, pfm_strerror(retval));
      return(PAPI_ESBSTR);
    }
  SUBDBG("pfm_get_num_events: %d\n", ncnt);
  MY_VECTOR.cmp_info.num_native_events = ncnt;
  strcpy(MY_VECTOR.cmp_info.name, "$Id: perfmon.c,v 1.97.4.1 2010/01/27 22:26:42 jagode Exp $");          
  strcpy(MY_VECTOR.cmp_info.version, "$Revision: 1.97.4.1 $");  
  sprintf(buf,"%08x",version);

  pfm_get_num_counters((unsigned int *)&MY_VECTOR.cmp_info.num_cntrs);
  SUBDBG("pfm_get_num_counters: %d\n", MY_VECTOR.cmp_info.num_cntrs);
  retval = get_system_info(&_papi_hwi_system_info);
  if (retval)
    return (retval);
  if ((_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_MIPS)||(_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_SICORTEX))
    MY_VECTOR.cmp_info.available_domains |= PAPI_DOM_KERNEL|PAPI_DOM_SUPERVISOR|PAPI_DOM_OTHER;
  else
    if (_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_IBM)
      {
        /* powerpc */
        MY_VECTOR.cmp_info.available_domains |=
          PAPI_DOM_KERNEL|PAPI_DOM_SUPERVISOR;
        if (strcmp(_papi_hwi_system_info.hw_info.model_string, "POWER6") == 0)
          {
            MY_VECTOR.cmp_info.default_domain =
              PAPI_DOM_USER|PAPI_DOM_KERNEL|PAPI_DOM_SUPERVISOR;
          }
      }
    else
      MY_VECTOR.cmp_info.available_domains |= PAPI_DOM_KERNEL;    

  if (_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_SUN)
    {
      switch (_perfmon2_pfm_pmu_type) {
#ifdef PFMLIB_SPARC_ULTRA12_PMU
      case PFMLIB_SPARC_ULTRA12_PMU:
      case PFMLIB_SPARC_ULTRA3_PMU:
      case PFMLIB_SPARC_ULTRA3I_PMU:
      case PFMLIB_SPARC_ULTRA3PLUS_PMU:
      case PFMLIB_SPARC_ULTRA4PLUS_PMU:
#endif
	break;

      default:
	MY_VECTOR.cmp_info.available_domains |=
	  PAPI_DOM_SUPERVISOR;
	break;
      }
    }

  if (_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_CRAY)
    {
	MY_VECTOR.cmp_info.available_domains |= PAPI_DOM_OTHER;
    }

  if ((_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_INTEL) ||
      (_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_AMD))
    {
      MY_VECTOR.cmp_info.fast_counter_read = 1;
      MY_VECTOR.cmp_info.fast_real_timer = 1;
      MY_VECTOR.cmp_info.cntr_umasks = 1;
    }
  if (_papi_hwi_system_info.hw_info.vendor == PAPI_VENDOR_SICORTEX) {
      MY_VECTOR.cmp_info.fast_real_timer = 1;
      MY_VECTOR.cmp_info.cntr_umasks = 1;
  }    
#ifdef WIN32
  /****WIN32: can we figure out how to get Windows to support hdw interrupts on
    counter overflow? The hardware supports it; does the OS? */
  MY_VECTOR.cmp_info.hardware_intr = 0;
#else
  MY_VECTOR.cmp_info.hardware_intr = 1;
  MY_VECTOR.cmp_info.hardware_intr_sig = SIGRTMIN+2;
#endif
  MY_VECTOR.cmp_info.attach = 1;
  MY_VECTOR.cmp_info.attach_must_ptrace = 1;
  MY_VECTOR.cmp_info.kernel_multiplex = 1;
  MY_VECTOR.cmp_info.kernel_profile = 1;
  MY_VECTOR.cmp_info.profile_ear = 1;  
  MY_VECTOR.cmp_info.num_mpx_cntrs = PFMLIB_MAX_PMDS;
  MY_VECTOR.cmp_info.clock_ticks = sysconf(_SC_CLK_TCK);

#ifdef WIN32
  /* FIX: For now, use the pmu_type from Perfmon */
  _papi_hwi_system_info.hw_info.model = _perfmon2_pfm_pmu_type;
#endif

   /* Setup presets */
   retval = _papi_pfm_setup_presets(pmu_name, _perfmon2_pfm_pmu_type);
   if (retval)
     return(retval);

#ifdef WIN32
  for (i = 0; i < PAPI_MAX_LOCK; i++)
     InitializeCriticalSection(&_papi_hwd_lock_data[i]);
#else
   for (i = 0; i < PAPI_MAX_LOCK; i++)
      _papi_hwd_lock_data[i] = MUTEX_OPEN;
#endif
   return (PAPI_OK);
}

int _papi_pfm_shutdown_substrate()
{
#ifdef WIN32
  int i;
  CloseWinPMCDriver();
  for (i = 0; i < PAPI_MAX_LOCK; i++)
    DeleteCriticalSection(&_papi_hwd_lock_data[i]);
#endif
  return PAPI_OK;
}

#if defined(USE_PROC_PTTIMER)
static int init_proc_thread_timer(hwd_context_t *thr_ctx)
{
  char buf[LINE_MAX];
  int fd;
  sprintf(buf,"/proc/%d/task/%d/stat",getpid(),mygettid());
  fd = open(buf,O_RDONLY);
  if (fd == -1)
    {
      PAPIERROR("open(%s)",buf);
      return(PAPI_ESYS);
    }
  thr_ctx->stat_fd = fd;
  return(PAPI_OK);
}
#endif

int _papi_sub_pfm_init(hwd_context_t * thr_ctx)
{
  pfarg_load_t load_args;
  pfarg_ctx_t newctx;
  int ret, ctx_fd;

#if defined(USE_PROC_PTTIMER)
  ret = init_proc_thread_timer(thr_ctx);
  if (ret != PAPI_OK)
    return(ret);
#endif

  memset(&newctx, 0, sizeof(newctx));
  memset(&load_args, 0, sizeof(load_args));

  if ((ret = pfm_create_context(&newctx, NULL, NULL, 0)) == -1)
    {
      PAPIERROR("_papi_pfm_init:pfm_create_context(): %s", strerror(errno));
      return(PAPI_ESYS);
    }
  SUBDBG("PFM_CREATE_CONTEXT returned fd %d\n",ret);
  tune_up_fd(ret);
  ctx_fd = ret;
  
  memcpy(&((pfm_context_t *)thr_ctx)->ctx,&newctx,sizeof(newctx));
  ((pfm_context_t *)thr_ctx)->ctx_fd = ctx_fd;
  load_args.load_pid = mygettid();
  memcpy(&((pfm_context_t *)thr_ctx)->load,&load_args,sizeof(load_args));

  return(PAPI_OK);
}

long long _papi_pfm_get_real_usec(void) {
  long long retval;
#if defined(HAVE_CLOCK_GETTIME_REALTIME)
   {
     struct timespec foo;
     syscall(__NR_clock_gettime,HAVE_CLOCK_GETTIME_REALTIME,&foo);
     retval = (long long)foo.tv_sec*(long long)1000000;
     retval += (long long)(foo.tv_nsec/1000);
   }
#elif defined(HAVE_GETTIMEOFDAY)
   {
     struct timeval buffer;
     gettimeofday(&buffer,NULL);
     retval = (long long)buffer.tv_sec*(long long)1000000;
     retval += (long long)(buffer.tv_usec);
   }
#else
  retval = get_cycles()/(long long)_papi_hwi_system_info.hw_info.mhz;
#endif
  return(retval);
}
                                                                                
long long _papi_pfm_get_real_cycles(void) {
  long long retval;
#if defined(HAVE_GETTIMEOFDAY)||defined(__powerpc__)||(defined(mips)&&!defined(HAVE_CYCLE))
  retval = _papi_pfm_get_real_usec()*(long long)_papi_hwi_system_info.hw_info.mhz;
#else
  retval = get_cycles();
#endif
  return(retval);
}

long long _papi_pfm_get_virt_usec(const hwd_context_t * zero)
{
   long long retval;
#if defined(USE_PROC_PTTIMER)
   {
     char buf[LINE_MAX];
     long long utime, stime;
     int rv, cnt = 0, i = 0;

again:
     rv = read(((pfm_context_t *)zero)->stat_fd,buf,LINE_MAX*sizeof(char));
     if (rv == -1)
       {
	 if (errno == EBADF)
	   {
	     int ret = init_proc_thread_timer(zero);
	     if (ret != PAPI_OK)
	       return(ret);
	     goto again;
	   }
	 PAPIERROR("read()");
	 return(PAPI_ESYS);
       }
     lseek(((pfm_context_t *)zero)->stat_fd,0,SEEK_SET);

     buf[rv] = '\0';
     SUBDBG("Thread stat file is:%s\n",buf);
     while ((cnt != 13) && (i < rv))
       {
	 if (buf[i] == ' ')
	   { cnt++; }
	 i++;
       }
     if (cnt != 13)
       {
	 PAPIERROR("utime and stime not in thread stat file?");
	 return(PAPI_ESBSTR);
       }
     
     if (sscanf(buf+i,"%llu %llu",&utime,&stime) != 2)
       {
	 PAPIERROR("Unable to scan two items from thread stat file at 13th space?");
	 return(PAPI_ESBSTR);
       }
     retval = (utime+stime)*(long long)1000000/MY_VECTOR.cmp_info.clock_ticks;
   }
#elif defined(HAVE_CLOCK_GETTIME_THREAD)
   {
     struct timespec foo;
     syscall(__NR_clock_gettime,HAVE_CLOCK_GETTIME_THREAD,&foo);
     retval = (long long)foo.tv_sec*(long long)1000000;
     retval += (long long)foo.tv_nsec/1000;
   }
#elif defined(HAVE_PER_THREAD_TIMES)
   {
     struct tms buffer;
     times(&buffer);
     SUBDBG("user %d system %d\n",(int)buffer.tms_utime,(int)buffer.tms_stime);
     retval = (long long)((buffer.tms_utime+buffer.tms_stime)*1000000/MY_VECTOR.cmp_info.clock_ticks);
     /* NOT CLOCKS_PER_SEC as in the headers! */
   }
#elif defined(HAVE_PER_THREAD_GETRUSAGE)
   {
     struct rusage buffer;
     getrusage(RUSAGE_SELF,&buffer);
     SUBDBG("user %d system %d\n",(int)buffer.tms_utime,(int)buffer.tms_stime);
     retval = (long long)(buffer.ru_utime.tv_sec + buffer.ru_stime.tv_sec)*(long long)1000000;
     retval += (long long)(buffer.ru_utime.tv_usec + buffer.ru_stime.tv_usec);
   }
#elif defined WIN32
   HANDLE p;
   BOOL ret;
   FILETIME Creation, Exit, Kernel, User;
   long long virt;

   p = GetCurrentProcess();
   ret = GetProcessTimes(p, &Creation, &Exit, &Kernel, &User);
   if (ret) {
	   virt = (((long long)(Kernel.dwHighDateTime + User.dwHighDateTime))<<32)
	     + Kernel.dwLowDateTime + User.dwLowDateTime;
	   return(virt/1000);
   }
   else return(PAPI_ESBSTR);
#else
#error "No working per thread virtual timer"
#endif
   return (retval);
}

long long _papi_pfm_get_virt_cycles(const hwd_context_t * zero)
{
   return (_papi_pfm_get_virt_usec(zero)*(long long)_papi_hwi_system_info.hw_info.mhz);
}

/* reset the hardware counters */
int _papi_pfm_reset(hwd_context_t *ctx, hwd_control_state_t *ctl)
{
  int i, ret;

  /* Read could have clobbered the values */
  for (i=0; i < ((pfm_control_state_t *)ctl)->in.pfp_event_count; i++) 
    {
      if (((pfm_control_state_t *)ctl)->pd[i].reg_flags & PFM_REGFL_OVFL_NOTIFY)
	((pfm_control_state_t *)ctl)->pd[i].reg_value = ((pfm_control_state_t *)ctl)->pd[i].reg_long_reset;
      else
	((pfm_control_state_t *)ctl)->pd[i].reg_value = 0ULL;
    }

  ret = _papi_pfm_write_pmds((pfm_context_t *)ctx, (pfm_control_state_t *)ctl);
  if(ret!=PAPI_OK)
    return PAPI_ESYS;

  return (PAPI_OK);
}

/* write(set) the hardware counters */
int _papi_pfm_write(hwd_context_t *ctx, hwd_control_state_t *ctl, long long *from)
{
  int i, ret;

  /* Read could have clobbered the values */
  for (i=0; i < ((pfm_control_state_t *)ctl)->in.pfp_event_count; i++) 
    {
      if (((pfm_control_state_t *)ctl)->pd[i].reg_flags & PFM_REGFL_OVFL_NOTIFY)
	((pfm_control_state_t *)ctl)->pd[i].reg_value = from[i] + ((pfm_control_state_t *)ctl)->pd[i].reg_long_reset;
      else
	((pfm_control_state_t *)ctl)->pd[i].reg_value = from[i];
    }

  ret = _papi_pfm_write_pmds((pfm_context_t *)ctx, (pfm_control_state_t *)ctl);
  if(ret!=PAPI_OK)
    return PAPI_ESYS;


  return (PAPI_OK);
}

int _papi_pfm_read(hwd_context_t * ctx0, hwd_control_state_t * ctl0,
                   long long ** events, int flags)
{
  int i, ret;
  long long tot_runs = 0LL;
  pfm_control_state_t *ctl = (pfm_control_state_t *) ctl0;
  pfm_context_t *ctx = (pfm_context_t *)ctx0;

  ret = _papi_pfm_read_pmds(ctx, ctl);
  if(ret!=PAPI_OK)
    return PAPI_ESYS;

  /* Copy the values over */

  for (i=0; i < ctl->in.pfp_event_count; i++) 
    {
      if (ctl->pd[i].reg_flags & PFM_REGFL_OVFL_NOTIFY)
	ctl->counts[i] = ctl->pd[i].reg_value - ctl->pd[i].reg_long_reset;
      else
	ctl->counts[i] = ctl->pd[i].reg_value;
      SUBDBG("PMD[%d] = %lld (LLD),%llu (LLU)\n",i,(unsigned long long)ctl->counts[i],(unsigned long long)ctl->pd[i].reg_value);
    }
  *events = ctl->counts;

  /* If we're not multiplexing, bail now */

  if (ctl->num_sets == 1)
    return(PAPI_OK);

  /* If we're multiplexing, get the scaling information */

  SUBDBG("PFM_GETINFO_EVTSETS(%d,%p,%d)\n",ctl->ctx_fd, ctl->setinfo, ctl->num_sets);
  if ((ret = pfm_getinfo_evtsets(ctl->ctx_fd, ctl->setinfo, ctl->num_sets)))
    {
      DEBUGCALL(DEBUG_SUBSTRATE,dump_setinfo(ctl->setinfo,ctl->num_sets));
      PAPIERROR("pfm_getinfo_evtsets(%d,%p,%d): %s",ctl->ctx_fd, ctl->setinfo, ctl->num_sets, strerror(ret));
      *events = NULL;
      return(PAPI_ESYS);
    }
  DEBUGCALL(DEBUG_SUBSTRATE,dump_setinfo(ctl->setinfo,ctl->num_sets));

  /* Add up the number of total runs */
  
  for (i=0;i<ctl->num_sets;i++)
    tot_runs += ctl->setinfo[i].set_runs;

  /* Now scale the values */

  for (i=0; i < ctl->in.pfp_event_count; i++) 
    {
      SUBDBG("Counter %d is in set %d ran %llu of %llu times, old count %lld.\n",
	     i,
	     ctl->pd[i].reg_set,
	     (unsigned long long)ctl->setinfo[ctl->pd[i].reg_set].set_runs,
	     (unsigned long long)tot_runs,
	     ctl->counts[i]);
      if (ctl->setinfo[ctl->pd[i].reg_set].set_runs)
	ctl->counts[i] = (ctl->counts[i]*tot_runs)/ctl->setinfo[ctl->pd[i].reg_set].set_runs;
      else
	{
	  ctl->counts[i] = 0;
	  SUBDBG("Set %lld didn't run!!!!\n",(unsigned long long)ctl->pd[i].reg_set);
	}
      SUBDBG("Counter %d, new count %lld.\n",i,ctl->counts[i]);
    }

   return PAPI_OK;
}

#if defined(__crayxt)
int _papi_hwd_start_create_context = 0;	/* CrayPat checkpoint support */
#endif /* XT */

int _papi_pfm_start(hwd_context_t * ctx0, hwd_control_state_t * ctl0)
{
  int i, ret; 
  pfm_control_state_t *ctl = (pfm_control_state_t *) ctl0;
  pfm_context_t *ctx = (pfm_context_t *)ctx0;

#if defined(__crayxt)
  if (_papi_hwd_start_create_context) {
    pfarg_ctx_t tmp;

    memset (&tmp, 0, sizeof(tmp));
    if ((ret = pfm_create_context(&tmp, NULL, NULL, 0)) == -1)
    {
      PAPIERROR("_papi_hwd_init:pfm_create_context(): %s", strerror(errno));
      return(PAPI_ESYS);
    }
    tune_up_fd(ret);
    ctl->ctx_fd = ctx->ctx_fd = ret;
  }
#endif /* XT */

  if (ctl->num_sets > 1)
    {
      SUBDBG("PFM_CREATE_EVTSETS(%d,%p,%d)\n",ctl->ctx_fd,ctl->set,ctl->num_sets);
      if ((ret = pfm_create_evtsets(ctl->ctx_fd,ctl->set,ctl->num_sets)) != PFMLIB_SUCCESS)
	{
	  DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(ctl->set,ctl->num_sets));
	  PAPIERROR("pfm_create_evtsets(%d,%p,%d): errno=%d  %s", ctl->ctx_fd,ctl->set,ctl->num_sets, errno, strerror(ret));
	  perror("pfm_create_evtsets");
	  return(PAPI_ESYS);
	}
      DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(ctl->set,ctl->num_sets));
    }

  /*
   * Now program the registers
   *
   * We don't use the same variable to indicate the number of elements passed to
   * the kernel because, as we said earlier, pc may contain more elements than
   * the number of events (pmd) we specified, i.e., contains more than counting
   * monitors.
   */

  ret = _papi_pfm_write_pmcs(ctx, ctl);
  if(ret!=PAPI_OK)
    return PAPI_ESYS;
 
  /* Set counters to zero as per PAPI_start man page, unless it is set to overflow */

  for (i=0; i < ctl->in.pfp_event_count; i++) 
    if (!(ctl->pd[i].reg_flags & PFM_REGFL_OVFL_NOTIFY))
      ctl->pd[i].reg_value = 0ULL; 

  /*
   * To be read, each PMD must be either written or declared
   * as being part of a sample (reg_smpl_pmds)
   */

  ret = _papi_pfm_write_pmds(ctx, ctl);
  if(ret!=PAPI_OK)
    return PAPI_ESYS;

  SUBDBG("PFM_LOAD_CONTEXT(%d,%p(%u))\n",ctl->ctx_fd,ctl->load,ctl->load->load_pid);
  if ((ret = pfm_load_context(ctl->ctx_fd,ctl->load)))
    {
      PAPIERROR("pfm_load_context(%d,%p(%u)): %s", ctl->ctx_fd,ctl->load,ctl->load->load_pid, strerror(ret));
      return PAPI_ESYS;
    }

  SUBDBG("PFM_START(%d,%p)\n",ctl->ctx_fd, NULL);
  if ((ret = pfm_start(ctl->ctx_fd, NULL)))
    {
      PAPIERROR("pfm_start(%d): %s", ctl->ctx_fd, strerror(ret));
      return(PAPI_ESYS);
    }
   return PAPI_OK;
}

int _papi_pfm_stop(hwd_context_t * ctx0, hwd_control_state_t * ctl0)
{
  int ret;
  pfm_control_state_t *ctl = (pfm_control_state_t *) ctl0;
//  pfm_context_t *ctx = (pfm_context_t *)ctx0;

  SUBDBG("PFM_STOP(%d)\n",ctl->ctx_fd);
  if ((ret = pfm_stop(ctl->ctx_fd)))
    {
      /* If this thread is attached to another thread, and that thread
	 has exited, we can safely discard the error here. */

      if ((ret == PFMLIB_ERR_NOTSUPP) && (ctl->load->load_pid != mygettid()))
	return(PAPI_OK);

      PAPIERROR("pfm_stop(%d): %s", ctl->ctx_fd, strerror(ret));
      return(PAPI_ESYS);
    }

  SUBDBG("PFM_UNLOAD_CONTEXT(%d) (tid %u)\n",ctl->ctx_fd,ctl->load->load_pid);
  if ((ret = pfm_unload_context(ctl->ctx_fd)))
    {
      PAPIERROR("pfm_unload_context(%d): %s", ctl->ctx_fd, strerror(ret));
      return PAPI_ESYS;
    }

  if (ctl->num_sets > 1)
    {
      static pfarg_setdesc_t set = { 0, 0, 0, 0 };
      /* Delete the high sets */
      SUBDBG("PFM_DELETE_EVTSETS(%d,%p,%d)\n",ctl->ctx_fd,&ctl->set[1],ctl->num_sets-1);
      if ((ret = pfm_delete_evtsets(ctl->ctx_fd,&ctl->set[1],ctl->num_sets-1)) != PFMLIB_SUCCESS)
	{
	  DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(&ctl->set[1],ctl->num_sets-1));
	  PAPIERROR("pfm_delete_evtsets(%d,%p,%d): %s", ctl->ctx_fd,&ctl->set[1],ctl->num_sets-1, strerror(ret));
	  return(PAPI_ESYS);
	}
      DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(&ctl->set[1],ctl->num_sets-1));
      /* Reprogram the 0 set */
      SUBDBG("PFM_CREATE_EVTSETS(%d,%p,%d)\n",ctl->ctx_fd,&set,1);
      if ((ret = pfm_create_evtsets(ctl->ctx_fd,&set,1)) != PFMLIB_SUCCESS)
	{
	  DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(&set,1));
	  PAPIERROR("pfm_create_evtsets(%d,%p,%d): %s", ctl->ctx_fd,&set,ctl->num_sets, strerror(ret));
	  return(PAPI_ESYS);
	}
      DEBUGCALL(DEBUG_SUBSTRATE,dump_sets(&set,1));
    }

   return PAPI_OK;
}

inline_static int round_requested_ns(int ns)
{
  if (ns <= MY_VECTOR.cmp_info.itimer_res_ns) {
    return MY_VECTOR.cmp_info.itimer_res_ns;
  } else {
    int leftover_ns = ns % MY_VECTOR.cmp_info.itimer_res_ns;
    return (ns - leftover_ns + MY_VECTOR.cmp_info.itimer_res_ns);
  }
}

int _papi_pfm_ctl(hwd_context_t * ctx, int code, _papi_int_option_t * option)
{
  switch (code) {
  case PAPI_MULTIPLEX:
    {
      option->multiplex.ns = round_requested_ns(option->multiplex.ns);
      ((pfm_control_state_t *)(option->multiplex.ESI->ctl_state))->multiplexed = option->multiplex.ns;
      return(PAPI_OK);
    }
#ifndef WIN32
  case PAPI_ATTACH:
    return(attach((pfm_control_state_t *)(option->attach.ESI->ctl_state), option->attach.tid));
  case PAPI_DETACH:
    return(detach(ctx, (pfm_control_state_t *)(option->attach.ESI->ctl_state)));
#endif
  case PAPI_DOMAIN:
    return(set_domain((pfm_control_state_t *)(option->domain.ESI->ctl_state), option->domain.domain));
  case PAPI_GRANUL:
    return (set_granularity
	    ((pfm_control_state_t *)(option->granularity.ESI->ctl_state), option->granularity.granularity));
#if 0
  case PAPI_DATA_ADDRESS:
    ret=set_default_domain((pfm_control_state_t *)(option->address_range.ESI->ctl_state), option->address_range.domain);
    if(ret != PAPI_OK) return(ret);
    set_drange(ctx, (pfm_control_state_t *)(option->address_range.ESI->ctl_state), option);
    return (PAPI_OK);
  case PAPI_INSTR_ADDRESS:
    ret=set_default_domain((pfm_control_state_t *)(option->address_range.ESI->ctl_state), option->address_range.domain);
    if(ret != PAPI_OK) return(ret);
    set_irange(ctx, (pfm_control_state_t *)(option->address_range.ESI->ctl_state), option);
    return (PAPI_OK);
#endif

#ifndef WIN32   // todo:WIN32
  case PAPI_DEF_ITIMER:
    {
      /* flags are currently ignored, eventually the flags will be able
	 to specify whether or not we use POSIX itimers (clock_gettimer) */
      if ((option->itimer.itimer_num == ITIMER_REAL) &&
	  (option->itimer.itimer_sig != SIGALRM))
	return PAPI_EINVAL;
      if ((option->itimer.itimer_num == ITIMER_VIRTUAL) &&
	  (option->itimer.itimer_sig != SIGVTALRM))
	return PAPI_EINVAL;
      if ((option->itimer.itimer_num == ITIMER_PROF) &&
	  (option->itimer.itimer_sig != SIGPROF))
	return PAPI_EINVAL;
      if (option->itimer.ns > 0)
	option->itimer.ns = round_requested_ns(option->itimer.ns);
      /* At this point, we assume the user knows what he or
	 she is doing, they maybe doing something arch specific */
      return PAPI_OK;
    }
#endif
  case PAPI_DEF_MPX_NS:
    { 
      option->multiplex.ns = round_requested_ns(option->multiplex.ns);
      return(PAPI_OK);
    }
  case PAPI_DEF_ITIMER_NS:
    { 
      option->itimer.ns = round_requested_ns(option->itimer.ns);
      return(PAPI_OK);
    }
  default:
    return (PAPI_ENOSUPP);
  }
}

int _papi_pfm_shutdown(hwd_context_t * ctx0)
{
  pfm_context_t *ctx = (pfm_context_t *)ctx0;
  int ret;
#if defined(USE_PROC_PTTIMER)
  close(ctx->stat_fd);
#endif

#ifndef WIN32
  ret = close(ctx->ctx_fd);
  SUBDBG("CLOSE fd %d returned %d\n",ctx->ctx_fd,ret);
#endif
  return (PAPI_OK);
}

/* This will need to be modified for the Pentium IV */

static inline int find_profile_index(EventSetInfo_t *ESI, int pmd, int *flags, unsigned int *native_index, int *profile_index)
{
  int pos, esi_index, count;
  pfm_control_state_t *ctl = (pfm_control_state_t *)ESI->ctl_state;
  pfarg_pmd_t *pd;
  pfarg_pmc_t *pc;
  int i;

  pd = ctl->pd;
  pc = ctl->pc;
  
  /* Find virtual PMD index, the one we actually read from the physical PMD number that
     overflowed. This index is the one related to the profile buffer. */

  for (i=0;i<ctl->in.pfp_event_count;i++)
    {
      if (pd[i].reg_num == pmd)
	{
	  SUBDBG("Physical PMD %d is Virtual PMD %d\n",pmd,i);
	  pmd = i;
	  break;
	}
    }

#ifndef _WIN32
  SUBDBG("(%p,%d,%p)\n",ESI,pmd,index);
#endif
  for (count = 0; count < ESI->profile.event_counter; count++) 
    {
      /* Find offset of PMD that gets read from the kernel */
      esi_index = ESI->profile.EventIndex[count];
      pos = ESI->EventInfoArray[esi_index].pos[0];
      SUBDBG("Examining event at ESI index %d, PMD position %d\n",esi_index,pos);
      // PMU_FIRST_COUNTER
      if (pos == pmd) 
	{
	  *profile_index = count;
	  *native_index = ESI->NativeInfoArray[pos].ni_event & PAPI_NATIVE_AND_MASK;
	  *flags = ESI->profile.flags;
	  SUBDBG("Native event %d is at profile index %d, flags %d\n",*native_index,*profile_index,*flags);
	  return(PAPI_OK);
	}
    }
  
  PAPIERROR("wrong count: %d vs. ESI->profile.event_counter %d", count, ESI->profile.event_counter);
  return(PAPI_EBUG);
}

#if defined(__ia64__)
static inline int is_montecito_and_dear(unsigned int native_index)
{
  if (_perfmon2_pfm_pmu_type == PFMLIB_MONTECITO_PMU)
    {
      if (pfm_mont_is_dear(native_index))
	return(1);
    }
  return(0);
}
static inline int is_montecito_and_iear(unsigned int native_index)
{
  if (_perfmon2_pfm_pmu_type == PFMLIB_MONTECITO_PMU)
    {
      if (pfm_mont_is_iear(native_index))
	return(1);
    }
  return(0);
}
static inline int is_itanium2_and_dear(unsigned int native_index)
{
  if (_perfmon2_pfm_pmu_type == PFMLIB_ITANIUM2_PMU)
    {
      if (pfm_ita2_is_dear(native_index))
	return(1);
    }
  return(0);
}
static inline int is_itanium2_and_iear(unsigned int native_index)
{
  if (_perfmon2_pfm_pmu_type == PFMLIB_ITANIUM2_PMU)
    {
      if (pfm_ita2_is_iear(native_index))
	return(1);
    }
  return(0);
}
#endif

#define BPL (sizeof(uint64_t)<<3)
#define LBPL	6
static inline void pfm_bv_set(uint64_t *bv, uint16_t rnum)
{
	bv[rnum>>LBPL] |= 1UL << (rnum&(BPL-1));
}

static inline int setup_ear_event(unsigned int native_index, pfarg_pmd_t *pd, int flags)
{
#if defined(__ia64__)
  if (_perfmon2_pfm_pmu_type == PFMLIB_MONTECITO_PMU)
    {
      if (pfm_mont_is_dear(native_index)) /* 2,3,17 */
	{
	  pfm_bv_set(pd[0].reg_smpl_pmds, 32);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 33);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 36);
	  pfm_bv_set(pd[0].reg_reset_pmds, 36);
	  return(1);
	}
      else if (pfm_mont_is_iear(native_index))  /* O,1 MK */
	{
	  pfm_bv_set(pd[0].reg_smpl_pmds, 34);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 35);
	  pfm_bv_set(pd[0].reg_reset_pmds, 34);
	  return(1);
	}
      return(0);
    }
  else if (_perfmon2_pfm_pmu_type == PFMLIB_ITANIUM2_PMU)
    {
      if (pfm_mont_is_dear(native_index)) /* 2,3,17 */
	{
	  pfm_bv_set(pd[0].reg_smpl_pmds, 2);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 3);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 17);
	  pfm_bv_set(pd[0].reg_reset_pmds, 17);
	  return(1);
	}
      else if (pfm_mont_is_iear(native_index))  /* O,1 MK */
	{
	  pfm_bv_set(pd[0].reg_smpl_pmds, 0);
	  pfm_bv_set(pd[0].reg_smpl_pmds, 1);
	  pfm_bv_set(pd[0].reg_reset_pmds, 0);
	  return(1);
	}
      return(0);    
    }
#endif
  return(0);
}

static inline int process_smpl_entry(unsigned int native_pfm_index, int flags, pfm_dfl_smpl_entry_t **ent, caddr_t *pc)
{
  SUBDBG("process_smpl_entry(%d,%d,%p,%p)\n",native_pfm_index,flags,ent,pc);

#ifdef __ia64__
  /* Fixup EAR stuff here */      
  if (is_montecito_and_dear(native_pfm_index))
    {
      pfm_mont_pmd_reg_t data_addr; 
      pfm_mont_pmd_reg_t latency;
      pfm_mont_pmd_reg_t load_addr;
      unsigned long newent;
      
      if ((flags & (PAPI_PROFIL_DATA_EAR|PAPI_PROFIL_INST_EAR)) == 0)
	goto safety;

      /* Skip the header */
      ++(*ent);
      
      // PMD32 has data address on Montecito
      // PMD33 has latency on Montecito
      // PMD36 has instruction address on Montecito
      data_addr = *(pfm_mont_pmd_reg_t *)*ent;
      latency = *(pfm_mont_pmd_reg_t *)((unsigned long)*ent + sizeof(data_addr));
      load_addr = *(pfm_mont_pmd_reg_t *)((unsigned long)*ent + sizeof(data_addr)+sizeof(latency));
      
      SUBDBG("PMD[32]: 0x%016llx\n",(unsigned long long)data_addr.pmd_val);
      SUBDBG("PMD[33]: 0x%016llx\n",(unsigned long long)latency.pmd_val);
      SUBDBG("PMD[36]: 0x%016llx\n",(unsigned long long)load_addr.pmd_val);

      if ((!load_addr.pmd36_mont_reg.dear_vl) || (!load_addr.pmd33_mont_reg.dear_stat))
	{
	  SUBDBG("Invalid DEAR sample found, dear_vl = %d, dear_stat = 0x%x\n",
		 load_addr.pmd36_mont_reg.dear_vl,load_addr.pmd33_mont_reg.dear_stat);
	bail1:
	  newent = (unsigned long)*ent;
	  newent += 3*sizeof(pfm_mont_pmd_reg_t);
	  *ent = (pfm_dfl_smpl_entry_t *)newent;
	  return 0;
	}

      if (flags & PAPI_PROFIL_DATA_EAR)
	*pc = (caddr_t)data_addr.pmd_val;
      else if (flags & PAPI_PROFIL_INST_EAR)
	{
	  unsigned long tmp = ((load_addr.pmd36_mont_reg.dear_iaddr + (unsigned long)load_addr.pmd36_mont_reg.dear_bn) << 4) | (unsigned long)load_addr.pmd36_mont_reg.dear_slot;
	  *pc = (caddr_t)tmp;
	}
      else
	{
	  PAPIERROR("BUG!");
	  goto bail1;
	}

      newent = (unsigned long)*ent;
      newent += 3*sizeof(pfm_mont_pmd_reg_t);
      *ent = (pfm_dfl_smpl_entry_t *)newent;
      return 0;
    }
  else if (is_montecito_and_iear(native_pfm_index))
    {
      pfm_mont_pmd_reg_t latency;
      pfm_mont_pmd_reg_t icache_line_addr;
      unsigned long newent;
      
      if ((flags & PAPI_PROFIL_INST_EAR) == 0)
	goto safety;

      /* Skip the header */
      ++(*ent);
      
      // PMD34 has data address on Montecito
      // PMD35 has latency on Montecito
      icache_line_addr = *(pfm_mont_pmd_reg_t *)*ent;
      latency = *(pfm_mont_pmd_reg_t *)((unsigned long)*ent + sizeof(icache_line_addr));
     
      SUBDBG("PMD[34]: 0x%016llx\n",(unsigned long long)icache_line_addr.pmd_val);
      SUBDBG("PMD[35]: 0x%016llx\n",(unsigned long long)latency.pmd_val);

      if ((icache_line_addr.pmd34_mont_reg.iear_stat & 0x1) == 0)
	{
	  SUBDBG("Invalid IEAR sample found, iear_stat = 0x%x\n",icache_line_addr.pmd34_mont_reg.iear_stat);
	bail2:
	  newent = (unsigned long)*ent;
	  newent += 2*sizeof(pfm_mont_pmd_reg_t);
	  *ent = (pfm_dfl_smpl_entry_t *)newent;
	  return(0);
	}

      if (flags & PAPI_PROFIL_INST_EAR)
	{
	  unsigned long tmp = icache_line_addr.pmd34_mont_reg.iear_iaddr << 5;
	  *pc = (caddr_t)tmp;
	}
      else 
      	{
	  PAPIERROR("BUG!");
	  goto bail2;
	}

      newent = (unsigned long)*ent;
      newent += 2*sizeof(pfm_mont_pmd_reg_t);
      *ent = (pfm_dfl_smpl_entry_t *)newent;
      return 0;
    }
  else if (is_itanium2_and_dear(native_pfm_index))
    {
      pfm_ita2_pmd_reg_t data_addr; 
      pfm_ita2_pmd_reg_t latency;
      pfm_ita2_pmd_reg_t load_addr;
      unsigned long newent;
      
      if ((flags & (PAPI_PROFIL_DATA_EAR|PAPI_PROFIL_INST_EAR)) == 0)
	goto safety;

      /* Skip the header */
      ++(*ent);
      
      // PMD2 has data address on Itanium 2
      // PMD3 has latency on Itanium 2
      // PMD17 has instruction address on Itanium 2
      data_addr = *(pfm_ita2_pmd_reg_t *)*ent;
      latency = *(pfm_ita2_pmd_reg_t *)((unsigned long)*ent + sizeof(data_addr));
      load_addr = *(pfm_ita2_pmd_reg_t *)((unsigned long)*ent + sizeof(data_addr)+sizeof(latency));
      
      SUBDBG("PMD[2]: 0x%016llx\n",(unsigned long long)data_addr.pmd_val);
      SUBDBG("PMD[3]: 0x%016llx\n",(unsigned long long)latency.pmd_val);
      SUBDBG("PMD[17]: 0x%016llx\n",(unsigned long long)load_addr.pmd_val);

      if ((!load_addr.pmd17_ita2_reg.dear_vl) || (!load_addr.pmd3_ita2_reg.dear_stat))
	{
	  SUBDBG("Invalid DEAR sample found, dear_vl = %d, dear_stat = 0x%x\n",load_addr.pmd17_ita2_reg.dear_vl,load_addr.pmd3_ita2_reg.dear_stat);
	bail3:
	  newent = (unsigned long)*ent;
	  newent += 3*sizeof(pfm_mont_pmd_reg_t);
	  *ent = (pfm_dfl_smpl_entry_t *)newent;
	  return 0;
	}

      if (flags & PAPI_PROFIL_DATA_EAR)
	*pc = (caddr_t)data_addr.pmd_val;
      else if (flags & PAPI_PROFIL_INST_EAR)
	{
	  unsigned long tmp = ((load_addr.pmd17_ita2_reg.dear_iaddr + (unsigned long)load_addr.pmd17_ita2_reg.dear_bn) << 4) | (unsigned long)load_addr.pmd17_ita2_reg.dear_slot;
	  *pc = (caddr_t)tmp;
	}
      else
	{
	  PAPIERROR("BUG!");
	  goto bail3;
	}
      
      newent = (unsigned long)*ent;
      newent += 3*sizeof(pfm_ita2_pmd_reg_t);
      *ent = (pfm_dfl_smpl_entry_t *)newent;
      return 0;
    }
  else if (is_itanium2_and_iear(native_pfm_index))
    {
      pfm_ita2_pmd_reg_t latency;
      pfm_ita2_pmd_reg_t icache_line_addr;
      unsigned long newent;
      
      if ((flags & PAPI_PROFIL_INST_EAR) == 0)
	goto safety;

      /* Skip the header */
      ++(*ent);
      
      // PMD0 has address on Itanium 2
      // PMD1 has latency on Itanium 2
      icache_line_addr = *(pfm_ita2_pmd_reg_t *)*ent;
      latency = *(pfm_ita2_pmd_reg_t *)((unsigned long)*ent + sizeof(icache_line_addr));
     
      SUBDBG("PMD[0]: 0x%016llx\n",(unsigned long long)icache_line_addr.pmd_val);
      SUBDBG("PMD[1]: 0x%016llx\n",(unsigned long long)latency.pmd_val);

      if ((icache_line_addr.pmd0_ita2_reg.iear_stat & 0x1) == 0) 
	{
	  SUBDBG("Invalid IEAR sample found, iear_stat = 0x%x\n",icache_line_addr.pmd0_ita2_reg.iear_stat);
	bail4:
	  newent = (unsigned long)*ent;
	  newent += 2*sizeof(pfm_mont_pmd_reg_t);
	  *ent = (pfm_dfl_smpl_entry_t *)newent;
	  return(0);
	}

      if (flags & PAPI_PROFIL_INST_EAR)
	{
	  unsigned long tmp = icache_line_addr.pmd0_ita2_reg.iear_iaddr << 5;
	  *pc = (caddr_t)tmp;
	}
      else 
	{
	  PAPIERROR("BUG!");
	  goto bail4;
	}

      newent = (unsigned long)*ent;
      newent += 2*sizeof(pfm_ita2_pmd_reg_t);
      *ent = (pfm_dfl_smpl_entry_t *)newent;
      return 0;
    }
#if 0
  (is_btb(native_pfm_index))
    {
      // PMD48-63,39 on Montecito
      // PMD8-15,16 on Itanium 2
    }
#endif
  else
    safety:
#endif
    {
      *pc = (caddr_t)((size_t)((*ent)->ip));
      ++(*ent);
      return(0);
    }
}

static inline int
process_smpl_buf(int num_smpl_pmds, int entry_size, ThreadInfo_t **thr)
{
  int cidx = MY_VECTOR.cmp_info.CmpIdx;
  pfm_dfl_smpl_entry_t *ent;
  size_t pos;
  uint64_t entry, count;
  pfm_dfl_smpl_hdr_t *hdr = ((pfm_context_t *)(*thr)->context[cidx])->smpl_buf;
  int ret, profile_index, flags;
  unsigned int native_pfm_index;
  caddr_t pc = NULL;
  long long weight;

  DEBUGCALL(DEBUG_SUBSTRATE,dump_smpl_hdr(hdr));
  count = hdr->hdr_count;
  ent   = (pfm_dfl_smpl_entry_t *)(hdr+1);
  pos   = (unsigned long)ent;
  entry = 0;
  
  SUBDBG("This buffer has %llu samples in it.\n",(unsigned long long)count);
  while(count--) 
    {
      SUBDBG("Processing sample entry %llu\n",(unsigned long long)entry);
      DEBUGCALL(DEBUG_SUBSTRATE,dump_smpl(ent));

      /* Find the index of the profile buffers if we are profiling on many events */

      ret = find_profile_index((*thr)->running_eventset[cidx],ent->ovfl_pmd,&flags,&native_pfm_index,&profile_index);
      if (ret != PAPI_OK)
	return(ret);

      weight = process_smpl_entry(native_pfm_index,flags,&ent,&pc);

      _papi_hwi_dispatch_profile((*thr)->running_eventset[cidx], pc,
				 weight, profile_index);
 
      entry++;
    }
  return(PAPI_OK);
}

#ifndef WIN32
/* This function  used when hardware overflows ARE working 
    or when software overflows are forced					*/

void _papi_pfm_dispatch_timer(int n, hwd_siginfo_t * info, void *uc)
{
    _papi_hwi_context_t ctx;
#ifdef HAVE_PFM_MSG_TYPE
    pfm_msg_t msg;
#else
    pfarg_msg_t msg;
#endif
    int ret, wanted_fd, fd = info->si_fd;
    caddr_t address;
    ThreadInfo_t *thread = _papi_hwi_lookup_thread();
    int cidx = MY_VECTOR.cmp_info.CmpIdx;

    if (thread == NULL) {
        PAPIERROR("thread == NULL in _papi_pfm_dispatch_timer!");
	if (n == MY_VECTOR.cmp_info.hardware_intr_sig) { ret = read(fd, &msg, sizeof(msg)); pfm_restart(fd); }
        return;
    }

    if (thread->running_eventset[cidx] == NULL) {
        PAPIERROR("thread->running_eventset == NULL in _papi_pfm_dispatch_timer!");
	if (n == MY_VECTOR.cmp_info.hardware_intr_sig) { ret = read(fd, &msg, sizeof(msg)); pfm_restart(fd); }
	return;
    }

    if (thread->running_eventset[cidx]->overflow.flags == 0) {
        PAPIERROR("thread->running_eventset->overflow.flags == 0 in _papi_pfm_dispatch_timer!");
	if (n == MY_VECTOR.cmp_info.hardware_intr_sig) { ret = read(fd, &msg, sizeof(msg)); pfm_restart(fd); }
	return;
    }
      
    ctx.si = info;
    ctx.ucontext = (hwd_ucontext_t *)uc;

    if (thread->running_eventset[cidx]->overflow.flags & PAPI_OVERFLOW_FORCE_SW) {
      address = GET_OVERFLOW_ADDRESS(ctx);
      _papi_hwi_dispatch_overflow_signal((void *) &ctx, address, NULL, 
					 0, 0, &thread, cidx);
    }
    else {
      if (thread->running_eventset[cidx]->overflow.flags == PAPI_OVERFLOW_HARDWARE) {
	wanted_fd = ((pfm_control_state_t *)(thread->running_eventset[cidx]->ctl_state))->ctx_fd;
      } else {
	wanted_fd = ((pfm_context_t *)thread->context[cidx])->ctx_fd;
      }
      if (wanted_fd != fd) {
	SUBDBG("expected fd %d, got %d in _papi_hwi_dispatch_timer!",wanted_fd,fd);
	if (n == MY_VECTOR.cmp_info.hardware_intr_sig) { ret = read(fd, &msg, sizeof(msg)); pfm_restart(fd); }
	return;
      }
 retry:
        ret = read(fd, &msg, sizeof(msg));
        if (ret == -1) {
            if (errno == EINTR) {
                SUBDBG("read(%d) interrupted, retrying\n", fd);
                goto retry;
            }
            else {
                PAPIERROR("read(%d): errno %d", fd, errno); 
            }
        }
        else if (ret != sizeof(msg)) {
            PAPIERROR("read(%d): short %d vs. %d bytes", fd, ret, sizeof(msg)); 
            ret = -1;
        }
   
        if (msg.type != PFM_MSG_OVFL) {
            PAPIERROR("unexpected msg type %d",msg.type);
            ret = -1;
        }

#if 0
	if (msg.pfm_ovfl_msg.msg_ovfl_tid != mygettid()) {
	  PAPIERROR("unmatched thread id %lx vs. %lx",msg.pfm_ovfl_msg.msg_ovfl_tid,mygettid());
	  ret = -1;
	}
#endif

		if (ret != -1) {
			if ((thread->running_eventset[cidx]->state & PAPI_PROFILING) &&
				!(thread->running_eventset[cidx]->profile.flags & PAPI_PROFIL_FORCE_SW))
				process_smpl_buf(0, sizeof(pfm_dfl_smpl_entry_t), &thread);
			else {
				/* PAPI assumes that the overflow vector contains the register index of the
				overflowing native event. That is generally true, but Stephane used some
				tricks to offset the fixed counters on Core2 (Core? i7?) by 16. This hack
				corrects for that hack in a (hopefully) transparent manner */
				unsigned long i, vector = msg.pfm_ovfl_msg.msg_ovfl_pmds[0];
				pfm_control_state_t *ctl = (pfm_control_state_t *)thread->running_eventset[cidx]->ctl_state;
				for (i=0;i<ctl->in.pfp_event_count;i++) {
					/* We're only comparing to pmds[0]. A more robust implementation would
					compare to pmds[0-3]. The bit mask must be converted to an index
					for the comparison to work */
					if(ctl->pd[i].reg_num == ffsl(msg.pfm_ovfl_msg.msg_ovfl_pmds[0]) - 1) {
						/* if a match is found, convert the index back to a bitmask */
						vector = 1 << i;
						break;
					}
				}
				_papi_hwi_dispatch_overflow_signal((void *) &ctx,
					(caddr_t)((size_t)msg.pfm_ovfl_msg.msg_ovfl_ip),
					NULL, vector, 0, &thread, cidx);
			}
		}

		if ((ret = pfm_restart(fd))) {
			PAPIERROR("pfm_restart(%d): %s", fd, strerror(ret));
		}
	}
}

int _papi_pfm_stop_profiling(ThreadInfo_t * thread, EventSetInfo_t * ESI)
{
  /* Process any remaining samples in the sample buffer */
  return(process_smpl_buf(0, sizeof(pfm_dfl_smpl_entry_t), &thread));
}

int _papi_pfm_set_profile(EventSetInfo_t * ESI, int EventIndex, int threshold)
{
  int cidx = MY_VECTOR.cmp_info.CmpIdx;
  pfm_control_state_t *ctl = (pfm_control_state_t *)(ESI->ctl_state);
  pfm_context_t *ctx = (pfm_context_t *)(ESI->master->context[cidx]);
  pfarg_ctx_t newctx;
  void *buf_addr = NULL;
  pfm_dfl_smpl_arg_t buf_arg;
  pfm_dfl_smpl_hdr_t *hdr;
  int i, ret, ctx_fd;

  memset(&newctx, 0, sizeof(newctx));
  
  if (threshold == 0)
    {
      SUBDBG("MUNMAP(%p,%lld)\n",ctx->smpl_buf,(unsigned long long)ctx->smpl.buf_size);
      munmap(ctx->smpl_buf,ctx->smpl.buf_size);

      i = close(ctl->ctx_fd);
      SUBDBG("CLOSE fd %d returned %d\n",ctl->ctx_fd,i);

      /* Thread has master context */

      ctl->ctx_fd = ctx->ctx_fd;
      ctl->ctx = &ctx->ctx;
      memset(&ctx->smpl,0,sizeof(buf_arg));
      ctx->smpl_buf = NULL;
      ret = _papi_pfm_set_overflow(ESI,EventIndex,threshold);
//#warning "This should be handled somewhere else"
      ESI->state &= ~(PAPI_OVERFLOWING);
      ESI->overflow.flags &= ~(PAPI_OVERFLOW_HARDWARE);

      return(ret);
    }

  memset(&buf_arg, 0, sizeof(buf_arg));
  buf_arg.buf_size = 2*getpagesize();

  SUBDBG("PFM_CREATE_CONTEXT(%p,%s,%p,%d)\n",&newctx, PFM_DFL_SMPL_NAME, &buf_arg, (int)sizeof(buf_arg));
  if ((ret = pfm_create_context(&newctx, PFM_DFL_SMPL_NAME, &buf_arg, sizeof(buf_arg))) == -1)
    {
      DEBUGCALL(DEBUG_SUBSTRATE,dump_smpl_arg(&buf_arg));
      PAPIERROR("_papi_hwd_set_profile:pfm_create_context(): %s", strerror(errno));
      return(PAPI_ESYS);
    }
  ctx_fd = ret;
  SUBDBG("PFM_CREATE_CONTEXT returned fd %d\n",ctx_fd);
  tune_up_fd(ret);

  SUBDBG("MMAP(NULL,%lld,%d,%d,%d,0)\n",(unsigned long long)buf_arg.buf_size,PROT_READ,MAP_PRIVATE,ctx_fd);
  buf_addr = mmap(NULL, (size_t)buf_arg.buf_size, PROT_READ, MAP_PRIVATE, ctx_fd, 0);
  if (buf_addr == MAP_FAILED)
    {
      PAPIERROR("mmap(NULL,%d,%d,%d,%d,0): %s",buf_arg.buf_size,PROT_READ,MAP_PRIVATE,ctx_fd,strerror(errno));
      close(ctx_fd);
      return(PAPI_ESYS);
    }
  SUBDBG("Sample buffer is located at %p\n",buf_addr);

  hdr = (pfm_dfl_smpl_hdr_t *)buf_addr;
  SUBDBG("hdr_cur_offs=%llu version=%u.%u\n",
	 (unsigned long long)hdr->hdr_cur_offs,
	 PFM_VERSION_MAJOR(hdr->hdr_version),
	 PFM_VERSION_MINOR(hdr->hdr_version));
  
  if (PFM_VERSION_MAJOR(hdr->hdr_version) < 1)
    {
      PAPIERROR("invalid buffer format version %d",PFM_VERSION_MAJOR(hdr->hdr_version));
      munmap(buf_addr,buf_arg.buf_size);
      close(ctx_fd);
      return(PAPI_ESBSTR);
    }

  ret = _papi_pfm_set_overflow(ESI,EventIndex,threshold);
  if (ret != PAPI_OK)
    {
      munmap(buf_addr,buf_arg.buf_size);
      close(ctx_fd);
      return(ret);
    }
  
  /* Look up the native event code */

  if (ESI->profile.flags & (PAPI_PROFIL_DATA_EAR|PAPI_PROFIL_INST_EAR))
    {
      pfarg_pmd_t *pd;
      int pos, native_index;
      pd = ctl->pd;
      pos = ESI->EventInfoArray[EventIndex].pos[0];
      native_index = ((pfm_register_t *)(ESI->NativeInfoArray[pos].ni_bits))->event;
      setup_ear_event(native_index,&pd[pos],ESI->profile.flags);
    }

  if (ESI->profile.flags & PAPI_PROFIL_RANDOM) 
    {
      pfarg_pmd_t *pd;
      int pos;
      pd = ctl->pd;
      pos = ESI->EventInfoArray[EventIndex].pos[0];
      pd[pos].reg_random_seed = 5;
      pd[pos].reg_random_mask = 0xff;
    }

  /* Now close our context it is safe */

  // close(ctx->ctx_fd);

  /* Copy the new data to the threads context control block */

  ctl->ctx_fd = ctx_fd;
  memcpy(&ctx->smpl,&buf_arg,sizeof(buf_arg));
  ctx->smpl_buf = buf_addr;

  return(PAPI_OK);
}

#endif

int _papi_pfm_set_overflow(EventSetInfo_t * ESI, int EventIndex, int threshold)
{
   pfm_control_state_t *this_state = (pfm_control_state_t *)(ESI->ctl_state);
   int j, retval = PAPI_OK, *pos;

   /* Which counter are we on, this looks suspicious because of the pos[0],
      but this could be because of derived events. We should do more here
      to figure out exactly what the position is, because the event may
      actually have more than one position. */
   
   pos = ESI->EventInfoArray[EventIndex].pos;
   j = pos[0];
   SUBDBG("Hardware counter %d used in overflow, threshold %d\n", j, threshold);

   if (threshold == 0) 
     {
       /* If this counter isn't set to overflow */
       
       if ((this_state->pd[j].reg_flags & PFM_REGFL_OVFL_NOTIFY) == 0)
	 return(PAPI_EINVAL);
       
       /* Remove the signal handler */
       
       retval = _papi_hwi_stop_signal(MY_VECTOR.cmp_info.hardware_intr_sig);
       if (retval != PAPI_OK)
	 return(retval);

       /* Disable overflow */

       this_state->pd[j].reg_flags ^= PFM_REGFL_OVFL_NOTIFY;

	/*
	 * we may want to reset the other PMDs on
	 * every overflow. If we do not set
	 * this, the non-overflowed counters
	 * will be untouched.

	 if (inp.pfp_event_count > 1)
	 this_state->pd[j].reg_reset_pmds[0] ^= 1UL << counter_to_reset */

       /* Clear the overflow period */

      this_state->pd[j].reg_value = 0;
      this_state->pd[j].reg_long_reset = 0;
      this_state->pd[j].reg_short_reset = 0;
      this_state->pd[j].reg_random_seed = 0;
      this_state->pd[j].reg_random_mask = 0;
     } 
   else
     {
       /* Enable the signal handler */

       retval = _papi_hwi_start_signal(MY_VECTOR.cmp_info.hardware_intr_sig, 1, MY_VECTOR.cmp_info.CmpIdx);
       if (retval != PAPI_OK)
	 return(retval);

       /* Set it to overflow */

       this_state->pd[j].reg_flags |= PFM_REGFL_OVFL_NOTIFY;
       
	/*
	 * we may want to reset the other PMDs on
	 * every overflow. If we do not set
	 * this, the non-overflowed counters
	 * will be untouched.

	 if (inp.pfp_event_count > 1)
	 this_state->pd[j].reg_reset_pmds[0] |= 1UL << counter_to_reset */
       
       /* Set the overflow period */

       this_state->pd[j].reg_value = - (unsigned long long) threshold + 1;
       this_state->pd[j].reg_short_reset = - (unsigned long long) threshold + 1;
       this_state->pd[j].reg_long_reset = - (unsigned long long) threshold + 1;
     }
   return (retval);
}

int _papi_pfm_init_control_state(hwd_control_state_t *ctl0)
{
  pfm_control_state_t *ctl = (pfm_control_state_t *)ctl0;
  pfmlib_input_param_t *inp = &ctl->in;
  pfmlib_output_param_t *outp = &ctl->out;
  pfarg_pmd_t *pd = ctl->pd;
  pfarg_pmc_t *pc = ctl->pc;
  pfarg_setdesc_t *set = ctl->set;
  pfarg_setinfo_t *setinfo = ctl->setinfo;

  memset(inp,0,sizeof(*inp));
  memset(outp,0,sizeof(*inp));
  memset(pc,0,sizeof(ctl->pc));
  memset(pd,0,sizeof(ctl->pd));
  memset(set,0,sizeof(ctl->set));
  memset(setinfo,0,sizeof(ctl->setinfo));
  /* Will be filled by update now...until this gets another arg */
  ctl->ctx = NULL;
  ctl->ctx_fd = -1;
  ctl->load = NULL;
  set_domain(ctl,MY_VECTOR.cmp_info.default_domain);
  return(PAPI_OK);
}

int _papi_pfm_allocate_registers(EventSetInfo_t * ESI)
{
  int i, j;
  for (i = 0; i < ESI->NativeCount; i++) 
    {
      if (_papi_pfm_ntv_code_to_bits(ESI->NativeInfoArray[i].ni_event,ESI->NativeInfoArray[i].ni_bits) != PAPI_OK)
	goto bail;
    }
  return(1);
 bail:
  for (j = 0; j < i; j++) 
    memset(ESI->NativeInfoArray[j].ni_bits,0x0,sizeof(pfm_register_t));
  return(0);
}

/* This function clears the current contents of the control structure and 
   updates it with whatever resources are allocated for all the native events
   in the native info structure array. */

int _papi_pfm_update_control_state(hwd_control_state_t *ctl0,
                                   NativeInfo_t *native, int count, hwd_context_t * ctx0) {
  pfm_control_state_t *ctl = (pfm_control_state_t *)ctl0;
  pfm_context_t *ctx = (pfm_context_t *)ctx0;
  int i = 0, ret;
  int last_reg_set = 0, reg_set_done = 0, offset = 0;
  pfmlib_input_param_t tmpin,*inp = &ctl->in;
  pfmlib_output_param_t tmpout,*outp = &ctl->out;
  pfarg_pmd_t *pd = ctl->pd;

  if (count == 0)
    {
      SUBDBG("Called with count == 0\n");
      inp->pfp_event_count = 0;
      outp->pfp_pmc_count = 0;
      memset(inp->pfp_events,0x0,sizeof(inp->pfp_events));
      return(PAPI_OK);
    }
  
  memcpy(&tmpin,inp,sizeof(tmpin));
  memcpy(&tmpout,outp,sizeof(tmpout));

  for (i=0;i<count;i++)
    {
      SUBDBG("Stuffing native event index %d (code 0x%x) into input structure.\n",
	     i,((pfm_register_t *)native[i].ni_bits)->event);
      memcpy(inp->pfp_events+i,native[i].ni_bits,sizeof(pfmlib_event_t));
    }
  inp->pfp_event_count = count;
      
  /* let the library figure out the values for the PMCS */
  
  ret = compute_kernel_args(ctl);
  if (ret != PAPI_OK)
    {
      /* Restore values */
      memcpy(inp,&tmpin,sizeof(tmpin));
      memcpy(outp,&tmpout,sizeof(tmpout));
      return(ret);
    }
  
  /* Update the native structure, because the allocation is done here. */
  
  last_reg_set = pd[0].reg_set;
  for (i=0;i<count;i++)
    {
      if (pd[i].reg_set != last_reg_set)
	{
	  offset += reg_set_done;
	  reg_set_done = 0;
	}
      reg_set_done++;

      native[i].ni_position = i;
      SUBDBG("native event index %d (code 0x%x) is at PMD offset %d\n", i, ((pfm_register_t *)native[i].ni_bits)->event, native[i].ni_position);
    }
      
   /* If structure has not yet been filled with a context, fill it
      from the thread's context. This should happen in init_control_state
      when we give that a *ctx argument */

  if (ctl->ctx == NULL)
    {
      ctl->ctx = &ctx->ctx;
      ctl->ctx_fd = ctx->ctx_fd;
      ctl->load = &ctx->load;
    }

   return (PAPI_OK);
}

#ifdef WIN32
void CALLBACK _papi_hwd_timer_callback(UINT wTimerID, UINT msg, 
        DWORD dwUser, DWORD dw1, DWORD dw2) 
{
    _papi_hwi_context_t ctx;
    CONTEXT	context;	// processor specific context structure
    HANDLE	threadHandle;
    BOOL	error;
    ThreadInfo_t *master = NULL;
    int isHardware=0;

#define OVERFLOW_MASK 0
#define GEN_OVERFLOW 1

   ctx.ucontext = &context;

   // dwUser is the threadID passed by timeSetEvent
    // NOTE: This call requires W2000 or later
    threadHandle = OpenThread(THREAD_GET_CONTEXT, FALSE, dwUser);

    // retrieve the contents of the control registers only
    context.ContextFlags = CONTEXT_CONTROL;
    error = GetThreadContext(threadHandle, &context);
    CloseHandle(threadHandle);

    // pass a void pointer to cpu register data here
    _papi_hwi_dispatch_overflow_signal((void *)&ctx, GET_OVERFLOW_ADDRESS((&ctx)), &isHardware, OVERFLOW_MASK, GEN_OVERFLOW, &master); 
}
#endif

papi_vector_t _papi_pfm_vector = {
    .cmp_info = {
	/* default component information (unspecified values are initialized to 0) */
	.default_domain =	PAPI_DOM_USER,
	.available_domains =	PAPI_DOM_USER|PAPI_DOM_KERNEL,
	.default_granularity =	PAPI_GRN_THR,
	.available_granularities = PAPI_GRN_THR,

    .hardware_intr = 1,
    .attach = 1,
    .attach_must_ptrace = 1,
    .kernel_multiplex = 1,
    .kernel_profile = 1,
    .profile_ear = 1,  
    .num_mpx_cntrs = PFMLIB_MAX_PMDS,
    .hardware_intr_sig = PAPI_INT_SIGNAL,
 
	/* component specific cmp_info initializations */
	.fast_real_timer =	1,
	.fast_virtual_timer =	0,
	.attach =		0,
	.attach_must_ptrace =	0,
    .itimer_sig = PAPI_INT_MPX_SIGNAL,
    .itimer_num = PAPI_INT_ITIMER,
    .itimer_ns = PAPI_INT_MPX_DEF_US * 1000,
    .itimer_res_ns = 1,
    },

    /* sizes of framework-opaque component-private structures */
    .size = {
	.context =		sizeof(pfm_context_t),
	.control_state =	sizeof(pfm_control_state_t),
	.reg_value =		sizeof(pfm_register_t),
	.reg_alloc =		sizeof(pfm_reg_alloc_t),
    },
    /* function pointers in this component */
    .init_control_state =	_papi_pfm_init_control_state,
    .start =			_papi_pfm_start,
    .stop =			_papi_pfm_stop,
    .read =			_papi_pfm_read,
    .shutdown =		        _papi_pfm_shutdown,
    .shutdown_substrate =       _papi_pfm_shutdown_substrate,
    .ctl =			_papi_pfm_ctl,
    .update_control_state =	_papi_pfm_update_control_state,
    .update_shlib_info =	_papi_pfm_update_shlib_info,
    .set_domain =		set_domain,
    .reset =			_papi_pfm_reset,
    .set_overflow =		_papi_pfm_set_overflow,
    .set_profile =		_papi_pfm_set_profile,
    .stop_profiling =		_papi_pfm_stop_profiling,
    .init_substrate =		_papi_pfm_init_substrate,
    .dispatch_timer =		_papi_pfm_dispatch_timer,
    .get_real_usec =		_papi_pfm_get_real_usec,
    .get_real_cycles =		_papi_pfm_get_real_cycles,
    .get_virt_cycles =		_papi_pfm_get_virt_cycles,
    .get_virt_usec =		_papi_pfm_get_virt_usec,
    .get_memory_info =	_papi_pfm_get_memory_info,
    .init =		_papi_sub_pfm_init,
    .get_dmem_info =	_papi_pfm_get_dmem_info,
    .allocate_registers =	_papi_pfm_allocate_registers,
    .write =	_papi_pfm_write,
    .ntv_enum_events =	_papi_pfm_ntv_enum_events,
	.ntv_name_to_code = _papi_pfm_ntv_name_to_code,
    .ntv_code_to_name =	_papi_pfm_ntv_code_to_name,
    .ntv_code_to_descr =	_papi_pfm_ntv_code_to_descr,
    .ntv_code_to_bits =	_papi_pfm_ntv_code_to_bits,
    .ntv_bits_to_info =	_papi_pfm_ntv_bits_to_info,
    /* from OS */
};

