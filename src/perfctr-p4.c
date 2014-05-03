/* 
* File:    p4.c
* CVS:     $Id: perfctr-p4.c,v 1.106 2010/01/08 16:10:55 terpstra Exp $
* Author:  Philip Mucci
*          mucci@cs.utk.edu
* Mods:    Kevin London 
*          london@cs.utk.edu
* Mods:    Dan Terpstra 
*          terpstra@cs.utk.edu
*          Modified to use libpfm and papi_pfm_events.c for native event encoding
* Mods:    <your name here>
*          <your email address>
*/

// NOTE: papi_avail doesn't seem to show derived events for P4...
// This must be because P4 only has DERIVED_CMPD and they don't show up ??

#include "papi.h"
#include "papi_internal.h"
#include "perfctr-p4.h"

/* Prototypes for entry points found in linux.c and linux-memory.c */
extern int _linux_init_substrate(int);
extern int _linux_ctl(hwd_context_t * ctx, int code, _papi_int_option_t * option);
extern void _linux_dispatch_timer(int signal, siginfo_t * si, void *context);
extern int _linux_get_memory_info(PAPI_hw_info_t * hw_info, int cpu_type);
extern int _linux_update_shlib_info(void);
extern int _linux_get_system_info(void);
extern int _linux_get_dmem_info(PAPI_dmem_info_t *d);
extern int _linux_init(hwd_context_t * ctx);
extern long long _linux_get_real_usec(void);
extern long long _linux_get_real_cycles(void);
extern long long _linux_get_virt_cycles(const hwd_context_t * ctx);
extern long long _linux_get_virt_usec(const hwd_context_t * ctx);
extern int _linux_shutdown(hwd_context_t * ctx);

#ifdef PERFCTR_PFM_EVENTS
/* Cleverly remap definitions of ntv routines from p3 to pfm */
#define _p4_ntv_enum_events _papi_pfm_ntv_enum_events
#define _p4_ntv_name_to_code _papi_pfm_ntv_name_to_code
#define _p4_ntv_code_to_name _papi_pfm_ntv_code_to_name
#define _p4_ntv_code_to_descr _papi_pfm_ntv_code_to_descr
#define _p4_ntv_code_to_bits _papi_pfm_ntv_code_to_bits
#define _p4_ntv_bits_to_info _papi_pfm_ntv_bits_to_info
/* Cleverly add an entry that doesn't exist for non-pfm */
int _p4_ntv_name_to_code(char *name, unsigned int *event_code);
#else
/* this routine doesn't exist if not pfm */
#define _p4_ntv_name_to_code NULL
#endif

/* Prototypes for entry points found in either p3_events or papi_pfm_events */
extern int _p4_ntv_enum_events(unsigned int *EventCode, int modifer);
extern int _p4_ntv_code_to_name(unsigned int EventCode, char * name, int len);
extern int _p4_ntv_code_to_descr(unsigned int EventCode, char * name, int len);
extern int _p4_ntv_code_to_bits(unsigned int EventCode, hwd_register_t *bits);
extern int _p4_ntv_bits_to_info(hwd_register_t *bits, char *names, unsigned int *values,
                          int name_len, int count);


extern papi_vector_t MY_VECTOR;

#if defined(PERFCTR26) || defined (PERFCTR25)
#define evntsel_aux             p4.escr
#endif

/*******************************/
/* BEGIN EXTERNAL DECLARATIONS */
/*******************************/

extern int _papi_pfm_setup_presets(char *name, int type);
extern int _papi_pfm_init();

/*****************************/
/* END EXTERNAL DECLARATIONS */
/*****************************/

/****************************/
/* BEGIN LOCAL DECLARATIONS */
/****************************/
static int _papi_hwd_fixup_fp(void);
static int _papi_hwd_fixup_vec(void);

/**************************/
/* END LOCAL DECLARATIONS */
/**************************/

int setup_p4_presets(int cputype)
{
    int retval;

   /* load the baseline event map for all Pentium 4s */
   if((retval = _papi_pfm_init()) != PAPI_OK) return (retval);
   _papi_pfm_setup_presets("Intel Pentium4", 0); /* base events */

   /* fix up the floating point and vector ops */
   if((retval = _papi_hwd_fixup_fp()) != PAPI_OK) return (retval);
   if ((retval = _papi_hwd_fixup_vec()) != PAPI_OK) return (retval);

   /* install L3 cache events iff 3 levels of cache exist */
   if (_papi_hwi_system_info.hw_info.mem_hierarchy.levels == 3)
      _papi_pfm_setup_presets("Intel Pentium4 L3", 0);

   /* overload with any model dependent events */
   if (cputype == PERFCTR_X86_INTEL_P4) {
     /* do nothing besides the base map */
   }
   else if (cputype == PERFCTR_X86_INTEL_P4M2) {
   }
#ifdef PERFCTR_X86_INTEL_P4M3
   else if (cputype == PERFCTR_X86_INTEL_P4M3) {
   }
#endif
   else {
      PAPIERROR(MODEL_ERROR);
      return(PAPI_ESBSTR);
   }
   return (PAPI_OK);
}

/* This used to be init_config, static to the substrate.
   Now its exposed to the hwi layer and called when an EventSet is allocated.
*/
static int _p4_init_control_state(hwd_control_state_t * ptr)
{
   int def_mode = 0, i;
   if (MY_VECTOR.cmp_info.default_domain & PAPI_DOM_USER)
      def_mode |= ESCR_T0_USR;
   if (MY_VECTOR.cmp_info.default_domain & PAPI_DOM_KERNEL)
     def_mode |= ESCR_T0_OS;

   for(i = 0; i < MY_VECTOR.cmp_info.num_cntrs; i++) {
      ptr->control.cpu_control.evntsel_aux[i] |= def_mode;
   }
   ptr->control.cpu_control.tsc_on = 1;
   ptr->control.cpu_control.nractrs = 0;
   ptr->control.cpu_control.nrictrs = 0;

#ifdef VPERFCTR_CONTROL_CLOEXEC
	ptr->control.flags = VPERFCTR_CONTROL_CLOEXEC;
	SUBDBG("close on exec\t\t\t%u\n", ptr->control.flags);
#endif

#if 0
   ptr->interval_usec = sampling_interval;
   ptr->nrcpus = all_cpus;
#endif
   return(PAPI_OK);
}

#ifdef DEBUG
void print_control(const struct perfctr_cpu_control *control)
{
   unsigned int i;

   SUBDBG("Control used:\n");
   SUBDBG("tsc_on\t\t\t0x%x\n", control->tsc_on);
   SUBDBG("nractrs\t\t\t0x%x\n", control->nractrs);
   SUBDBG("nrictrs\t\t\t0x%x\n", control->nrictrs);
   for (i = 0; i < (control->nractrs + control->nrictrs); ++i) {
      if (control->pmc_map[i] >= 18) {
         SUBDBG("pmc_map[%u]\t\t0x%08X\n", i, control->pmc_map[i]);
      } else {
         SUBDBG("pmc_map[%u]\t\t%u\n", i, control->pmc_map[i]);
      }
      SUBDBG("evntsel[%u]\t\t0x%08X\n", i, control->evntsel[i]);
#if defined(__i386__) || defined(__x86_64__)
      if (control->evntsel_aux[i])
         SUBDBG("evntsel_aux[%u]\t\t0x%08X\n", i, control->evntsel_aux[i]);
#endif
      if (control->ireset[i])
         SUBDBG("ireset[%u]\t\t%d\n", i, control->ireset[i]);
   }
#if defined(__i386__) || defined(__x86_64__)
   if (control->p4.pebs_enable)
      SUBDBG("pebs_enable\t0x%08X\n", control->p4.pebs_enable);
   if (control->p4.pebs_matrix_vert)
      SUBDBG("pebs_matrix_vert\t0x%08X\n", control->p4.pebs_matrix_vert);
#endif
}
#endif


static int _p4_start(hwd_context_t * ctx, hwd_control_state_t * state)
{
   int error;
#ifdef DEBUG
   print_control(&state->control.cpu_control);
#endif
   if (state->rvperfctr != NULL) 
     {
       if((error = rvperfctr_control(state->rvperfctr, &state->control)) < 0) 
	 {
	   SUBDBG("rvperfctr_control returns: %d\n", error);
	   PAPIERROR(RCNTRL_ERROR); 
	   return(PAPI_ESYS); 
	 }
       return (PAPI_OK);
     }
   error = vperfctr_control(ctx->perfctr, &state->control);
   if (error < 0) {
      SUBDBG("vperfctr_control returns: %d\n", error);
      { PAPIERROR(VCNTRL_ERROR); return(PAPI_ESYS); }
   }
#if 0
   if (gperfctr_control(ctx->perfctr, &state->control) < 0)
     { PAPIERROR(GCNTRL_ERROR); return(PAPI_ESYS); }
#endif

   return (PAPI_OK);
}

static int _p4_stop(hwd_context_t * ctx, hwd_control_state_t * state)
{
   if( state->rvperfctr != NULL ) {
     if(rvperfctr_stop((struct rvperfctr*)ctx->perfctr) < 0)
       { PAPIERROR( RCNTRL_ERROR); return(PAPI_ESYS); }
     return (PAPI_OK);
   }
   if (vperfctr_stop(ctx->perfctr) < 0)
     { PAPIERROR(VCNTRL_ERROR); return(PAPI_ESYS); }
#if 0
   if (gperfctr_stop(ctx->perfctr) < 0)
     { PAPIERROR(GCNTRL_ERROR); return(PAPI_ESYS); }
#endif

   return (PAPI_OK);
}

static int _p4_read(hwd_context_t * ctx, hwd_control_state_t * spc,
                   long long ** dp, int flags)
{
 
   if ( flags & PAPI_PAUSED ) {
     vperfctr_read_state(ctx->perfctr, &spc->state, NULL);
   }  
   else {
      SUBDBG("vperfctr_read_ctrs\n");
      if( spc->rvperfctr != NULL ) {
        rvperfctr_read_ctrs( spc->rvperfctr, &spc->state );
      } else {
        vperfctr_read_ctrs(ctx->perfctr, &spc->state);
        }
   }
      *dp = (long long *) spc->state.pmc;
#ifdef DEBUG
   {
      if (ISLEVEL(DEBUG_SUBSTRATE)) {
         int i;
         for (i = 0; i < spc->control.cpu_control.nractrs; i++) {
            SUBDBG("raw val hardware index %d is %lld\n", i,
                   (long long) spc->state.pmc[i]);
         }
      }
   }
#endif
   return (PAPI_OK);
}

#ifdef DEBUG

#if 0
static void print_bits(_p4_register_t * b)
{
   SUBDBG("  counter[0,1]: 0x%x, 0x%x\n", b->counter[0], b->counter[1]);
   SUBDBG("  escr[0,1]: 0x%x, 0x%x\n", b->escr[0], b->escr[1]);
   SUBDBG("  cccr: 0x%x,  event: 0x%x\n", b->cccr, b->event);
   SUBDBG("  pebs_enable: 0x%x,  pebs_matrix_vert: 0x%x,  ireset: 0x%x\n", b->pebs_enable,
          b->pebs_matrix_vert, b->ireset);
}
#endif

static void print_alloc(P4_reg_alloc_t * a)
{
   SUBDBG("P4_reg_alloc:\n");
//    print_bits(&(a->ra_bits));
   SUBDBG("  selector: 0x%x\n", a->ra_selector);
   SUBDBG("  rank: 0x%x\n", a->ra_rank);
   SUBDBG("  escr: 0x%x 0x%x\n", a->ra_escr[0], a->ra_escr[1]);
}

#endif

/* This function examines the event to determine
    if it can be mapped to counter ctr. 
    Returns true if it can, false if it can't.
*/
static int _p4_bpt_map_avail(hwd_reg_alloc_t * dst, int ctr)
{
   return (dst->ra_selector & (1 << ctr));
}

/* This function forces the event to
    be mapped to only counter ctr. 
    Returns nothing.
*/
static void _p4_bpt_map_set(hwd_reg_alloc_t * dst, int ctr)
{
   dst->ra_selector = (1 << ctr);
   dst->ra_rank = 1;
   /* Pentium 4 requires that both an escr and a counter are selected.
      Find which counter mask contains this counter.
      Set the opposite escr to empty (-1) */
   if (dst->ra_bits.counter[0] & dst->ra_selector)
      dst->ra_escr[1] = -1;
   else
      dst->ra_escr[0] = -1;
}

/* This function examines the event to determine
    if it has a single exclusive mapping. 
    Returns true if exlusive, false if non-exclusive.
*/
static int _p4_bpt_map_exclusive(hwd_reg_alloc_t * dst)
{
   return (dst->ra_rank == 1);
}

/* This function compares the dst and src events
    to determine if any resources are shared. Typically the src event
    is exclusive, so this detects a conflict if true.
    Returns true if conflict, false if no conflict.
*/
static int _p4_bpt_map_shared(hwd_reg_alloc_t * dst, hwd_reg_alloc_t * src)
{
   int retval1, retval2;
   /* Pentium 4 needs to check for conflict of both counters and esc registers */
             /* selectors must share bits */
   retval1 = ((dst->ra_selector & src->ra_selector) ||
             /* or escrs must equal each other and not be set to -1 */
             ((dst->ra_escr[0] == src->ra_escr[0]) && (dst->ra_escr[0] != -1)) ||
             ((dst->ra_escr[1] == src->ra_escr[1]) && (dst->ra_escr[1] != -1)));
   /* Pentium 4 also needs to check for conflict on pebs registers */
               /* pebs enables must both be non-zero */
   retval2 = (((dst->ra_bits.pebs_enable && src->ra_bits.pebs_enable) &&
               /* and not equal to each other */
               (dst->ra_bits.pebs_enable != src->ra_bits.pebs_enable)) ||
               /* same for pebs_matrix_vert */
              ((dst->ra_bits.pebs_matrix_vert && src->ra_bits.pebs_matrix_vert) &&
               (dst->ra_bits.pebs_matrix_vert != src->ra_bits.pebs_matrix_vert)));
   if (retval2) SUBDBG("pebs conflict!\n");
   return(retval1 | retval2);
}

/* This function removes shared resources available to the src event
    from the resources available to the dst event,
    and reduces the rank of the dst event accordingly. Typically,
    the src event will be exclusive, but the code shouldn't assume it.
    Returns nothing.
*/
static void _p4_bpt_map_preempt(hwd_reg_alloc_t * dst, hwd_reg_alloc_t * src)
{
   int i;
   unsigned shared;

   /* On Pentium 4, shared resources include escrs, counters, and pebs registers
      There is only one pair of pebs registers, so if two events use them differently
      there is an immediate conflict, and the dst rank is forced to 0.
   */
#ifdef DEBUG
   SUBDBG("src, dst\n");
   print_alloc(src);
   print_alloc(dst);
#endif

   /* check for a pebs conflict */
       /* pebs enables must both be non-zero */
   i = (((dst->ra_bits.pebs_enable && src->ra_bits.pebs_enable) &&
         /* and not equal to each other */
         (dst->ra_bits.pebs_enable != src->ra_bits.pebs_enable)) ||
         /* same for pebs_matrix_vert */
        ((dst->ra_bits.pebs_matrix_vert && src->ra_bits.pebs_matrix_vert) &&
         (dst->ra_bits.pebs_matrix_vert != src->ra_bits.pebs_matrix_vert)));
   if (i) {
      SUBDBG("pebs conflict! clearing selector\n");
      dst->ra_selector = 0;
      return;
   } else {
      /* remove counters referenced by any shared escrs */
      if ((dst->ra_escr[0] == src->ra_escr[0]) && (dst->ra_escr[0] != -1)) {
         dst->ra_selector &= ~dst->ra_bits.counter[0];
         dst->ra_escr[0] = -1;
      }
      if ((dst->ra_escr[1] == src->ra_escr[1]) && (dst->ra_escr[1] != -1)) {
         dst->ra_selector &= ~dst->ra_bits.counter[1];
         dst->ra_escr[1] = -1;
      }

      /* remove any remaining shared counters */
      shared = (dst->ra_selector & src->ra_selector);
      if (shared)
         dst->ra_selector ^= shared;
   }
   /* recompute rank */
   for (i = 0, dst->ra_rank = 0; i < MAX_COUNTERS; i++)
      if (dst->ra_selector & (1 << i))
         dst->ra_rank++;
#ifdef DEBUG
   SUBDBG("new dst\n");
   print_alloc(dst);
#endif
}

/* This function updates the selection status of 
    the dst event based on information in the src event.
    Returns nothing.
*/
static void _p4_bpt_map_update(hwd_reg_alloc_t * dst, hwd_reg_alloc_t * src)
{
   dst->ra_selector = src->ra_selector;
   dst->ra_escr[0] = src->ra_escr[0];
   dst->ra_escr[1] = src->ra_escr[1];
}


/* Register allocation */

static int _p4_allocate_registers(EventSetInfo_t * ESI)
{
   int i, j, natNum;
   hwd_reg_alloc_t event_list[MAX_COUNTERS], *e;
   hwd_register_t *ptr;


   /* not yet successfully mapped, but have enough slots for events */

   /* Initialize the local structure needed 
      for counter allocation and optimization. */
   natNum = ESI->NativeCount;
   SUBDBG("native event count: %d\n", natNum);
   for (i = 0; i < natNum; i++) {
      /* dereference event_list so code is easier to read */
      e = &event_list[i];

      /* retrieve the mapping information about this native event */
      _papi_pfm_ntv_code_to_bits(ESI->NativeInfoArray[i].ni_event, &e->ra_bits);

      /* combine counter bit masks for both esc registers into selector */
      e->ra_selector = e->ra_bits.counter[0] | e->ra_bits.counter[1];
      /* calculate native event rank, which is number of counters it can live on */
      e->ra_rank = 0;
      for (j = 0; j < MAX_COUNTERS; j++) {
         if (e->ra_selector & (1 << j)) {
            e->ra_rank++;
         }
      }

      /* set the bits for the two esc registers this event can live on */
      e->ra_escr[0] = e->ra_bits.escr[0];
      e->ra_escr[1] = e->ra_bits.escr[1];

#ifdef DEBUG
      SUBDBG("i: %d\n", i);
      print_alloc(e);
#endif
   }

   if (_papi_hwi_bipartite_alloc(event_list, natNum, ESI->CmpIdx)) { /* successfully mapped */
      for (i = 0; i < natNum; i++) {
#ifdef DEBUG
         SUBDBG("i: %d\n", i);
         print_alloc(&event_list[i]);
#endif
         /* Copy all info about this native event to the NativeInfo struct */
         ptr = ESI->NativeInfoArray[i].ni_bits;
         *ptr = event_list[i].ra_bits;

         /* The selector contains the counter bit position. Turn it into a number
            and store it in the first counter value, zeroing the second. */
         ptr->counter[0] = ffs(event_list[i].ra_selector) - 1;
         ptr->counter[1] = 0;

         /* Array order on perfctr is event ADD order, not counter #... */
         ESI->NativeInfoArray[i].ni_position = i;
      }
      return 1;
   }

   return (PAPI_OK);
}


static void clear_cs_events(hwd_control_state_t * this_state)
{
   int i,j;

   /* total counters is sum of accumulating (nractrs) and interrupting (nrictrs) */
   j = this_state->control.cpu_control.nractrs + this_state->control.cpu_control.nrictrs;

   /* Remove all counter control command values from eventset. */
   for (i = 0; i < j; i++) {
      SUBDBG("Clearing pmc event entry %d\n", i);
      this_state->control.cpu_control.pmc_map[i] = 0;
      this_state->control.cpu_control.evntsel[i] = 0;
      this_state->control.cpu_control.evntsel_aux[i] = 
         this_state->control.cpu_control.evntsel_aux[i] & (ESCR_T0_OS | ESCR_T0_USR);
      this_state->control.cpu_control.ireset[i] = 0;
   }

   /* Clear pebs stuff */
   this_state->control.cpu_control.p4.pebs_enable = 0;
   this_state->control.cpu_control.p4.pebs_matrix_vert = 0;

   /* clear both a and i counter counts */
   this_state->control.cpu_control.nractrs = 0;
   this_state->control.cpu_control.nrictrs = 0;

#ifdef DEBUG
   print_control(&this_state->control.cpu_control);
#endif
}


/* This function clears the current contents of the control structure and updates it 
   with whatever resources are allocated for all the native events 
   in the native info structure array. */
static int _p4_update_control_state(hwd_control_state_t * this_state,
                                   NativeInfo_t * native, int count, hwd_context_t *ctx)
{
   int i, retval = PAPI_OK;

   hwd_register_t *bits;
   struct perfctr_cpu_control *cpu_control = &this_state->control.cpu_control;

   /* clear out the events from the control state */
   clear_cs_events(this_state);

   /* fill the counters we're using */
   for (i = 0; i < count; i++) {
      /* dereference the mapping information about this native event */
      bits = native[i].ni_bits;

      /* Add counter control command values to eventset */

      cpu_control->pmc_map[i] = bits->counter[0];
      cpu_control->evntsel[i] = bits->cccr;
      cpu_control->ireset[i] = bits->ireset;
      cpu_control->pmc_map[i] |= FAST_RDPMC;
      cpu_control->evntsel_aux[i] |= bits->event;

    /* pebs_enable and pebs_matrix_vert are shared registers used for replay_events.
      Replay_events count L1 and L2 cache events. There is only one of each for 
	   the entire eventset. Therefore, there can be only one unique replay_event 
	   per eventset. This means L1 and L2 can't be counted together. Which stinks.
      This conflict should be trapped in the allocation scheme, but we'll test for it
      here too, just in case.
    */
      if (bits->pebs_enable) {
	 /* if pebs_enable isn't set, just copy */
         if (cpu_control->p4.pebs_enable == 0) {
            cpu_control->p4.pebs_enable = bits->pebs_enable;
	 /* if pebs_enable conflicts, flag an error */
         } else if (cpu_control->p4.pebs_enable != bits->pebs_enable) {
            SUBDBG("WARNING: P4_update_control_state -- pebs_enable conflict!");
	         retval = PAPI_ECNFLCT;
         }
	 /* if pebs_enable == bits->pebs_enable, do nothing */
      }
      if (bits->pebs_matrix_vert) {
	 /* if pebs_matrix_vert isn't set, just copy */
         if (cpu_control->p4.pebs_matrix_vert == 0) {
	         cpu_control->p4.pebs_matrix_vert = bits->pebs_matrix_vert;
	 /* if pebs_matrix_vert conflicts, flag an error */
         } else if (cpu_control->p4.pebs_matrix_vert != bits->pebs_matrix_vert) {
            SUBDBG("WARNING: P4_update_control_state -- pebs_matrix_vert conflict!");
 	         retval = PAPI_ECNFLCT;
         }
	 /* if pebs_matrix_vert == bits->pebs_matrix_vert, do nothing */
     }
   }
   this_state->control.cpu_control.nractrs = count;

   /* Make sure the TSC is always on */
   this_state->control.cpu_control.tsc_on = 1;

#ifdef DEBUG
   print_control(&this_state->control.cpu_control);
#endif
   return (retval);
}


static int _p4_set_domain(hwd_control_state_t * cntrl, int domain)
{
   int i, did = 0;
    
     /* Clear the current domain set for this event set */
     /* We don't touch the Enable bit in this code but  */
     /* leave it as it is */
   for(i = 0; i < MY_VECTOR.cmp_info.num_cntrs; i++) {
      cntrl->control.cpu_control.evntsel_aux[i] &= ~(ESCR_T0_OS|ESCR_T0_USR);
   }
   if(domain & PAPI_DOM_USER) {
      did = 1;
      for(i = 0; i < MY_VECTOR.cmp_info.num_cntrs; i++) {
         cntrl->control.cpu_control.evntsel_aux[i] |= ESCR_T0_USR;
      }
   }
   if(domain & PAPI_DOM_KERNEL) {
      did = 1;
      for(i = 0; i < MY_VECTOR.cmp_info.num_cntrs; i++) {
         cntrl->control.cpu_control.evntsel_aux[i] |= ESCR_T0_OS;
      }
   }
   if(!did)
      return(PAPI_EINVAL);
   else
      return(PAPI_OK);
}

static int _p4_reset(hwd_context_t *ctx, hwd_control_state_t *cntrl)
{
   /* this is what I gleaned from PAPI 2.3.4... is it right??? dkt */
   return (_p4_start(ctx, cntrl));
}


/* Perfctr requires that interrupting counters appear at the end of the pmc list.
   In the case a user wants to interrupt on a counter in an evntset that is not 
   among the last events, we need to move the perfctr virtual events around to 
   make it last. This function swaps two perfctr events, and then adjusts the
   position entries in both the NativeInfoArray and the EventInfoArray to keep
   everything consistent.
*/

static void swap_events(EventSetInfo_t * ESI, struct vperfctr_control *contr, int cntr1,
                        int cntr2)
{
   unsigned int ui;
   int si, i, j;

   for (i = 0; i < ESI->NativeCount; i++) {
      if (ESI->NativeInfoArray[i].ni_position == cntr1)
         ESI->NativeInfoArray[i].ni_position = cntr2;
      else if (ESI->NativeInfoArray[i].ni_position == cntr2)
         ESI->NativeInfoArray[i].ni_position = cntr1;
   }

   for (i = 0; i < ESI->NumberOfEvents; i++) {
      for (j = 0; ESI->EventInfoArray[i].pos[j] >= 0; j++) {
         if (ESI->EventInfoArray[i].pos[j] == cntr1)
            ESI->EventInfoArray[i].pos[j] = cntr2;
         else if (ESI->EventInfoArray[i].pos[j] == cntr2)
            ESI->EventInfoArray[i].pos[j] = cntr1;
      }
   }

   ui = contr->cpu_control.pmc_map[cntr1];
   contr->cpu_control.pmc_map[cntr1] = contr->cpu_control.pmc_map[cntr2];
   contr->cpu_control.pmc_map[cntr2] = ui;

   ui = contr->cpu_control.evntsel[cntr1];
   contr->cpu_control.evntsel[cntr1] = contr->cpu_control.evntsel[cntr2];
   contr->cpu_control.evntsel[cntr2] = ui;

   ui = contr->cpu_control.evntsel_aux[cntr1];
   contr->cpu_control.evntsel_aux[cntr1] = contr->cpu_control.evntsel_aux[cntr2];
   contr->cpu_control.evntsel_aux[cntr2] = ui;

   si = contr->cpu_control.ireset[cntr1];
   contr->cpu_control.ireset[cntr1] = contr->cpu_control.ireset[cntr2];
   contr->cpu_control.ireset[cntr2] = si;
}

static int _p4_set_overflow(EventSetInfo_t * ESI, int EventIndex, int threshold)
{
   hwd_control_state_t *this_state = ESI->ctl_state;
   struct vperfctr_control *contr = &this_state->control;
   int i, ncntrs, nricntrs = 0, nracntrs = 0, retval = 0;

   OVFDBG("EventIndex=%d\n", EventIndex);

#ifdef DEBUG
   /* The correct event to overflow is EventIndex */
   print_control(&this_state->control.cpu_control);
#endif

   ncntrs = MY_VECTOR.cmp_info.num_cntrs;
   i = ESI->EventInfoArray[EventIndex].pos[0];
   if (i >= ncntrs) 
     {
       PAPIERROR("Selector id %d is larger than ncntrs %d", i, ncntrs);
       return PAPI_EINVAL;
     }

   if (threshold != 0) {        /* Set an overflow threshold */

      if ((retval = _papi_hwi_start_signal(MY_VECTOR.cmp_info.hardware_intr_sig,NEED_CONTEXT, MY_VECTOR.cmp_info.CmpIdx)) != PAPI_OK)
	      return(retval);

      /* overflow interrupt occurs on the NEXT event after overflow occurs
         thus we subtract 1 from the threshold. */
      contr->cpu_control.ireset[i] = (-threshold + 1);
      contr->cpu_control.evntsel[i] |= CCCR_OVF_PMI_T0;
      contr->cpu_control.nrictrs++;
      contr->cpu_control.nractrs--;
      nricntrs = contr->cpu_control.nrictrs;
      nracntrs = contr->cpu_control.nractrs;
      contr->si_signo = MY_VECTOR.cmp_info.hardware_intr_sig;

      /* move this event to the bottom part of the list if needed */
      if (i < nracntrs) {
         swap_events(ESI, contr, i, nracntrs);
      }
      OVFDBG("Modified event set\n");
   } else {
      if (contr->cpu_control.evntsel[i] & CCCR_OVF_PMI_T0) {
         contr->cpu_control.ireset[i] = 0;
         contr->cpu_control.evntsel[i] &= (~CCCR_OVF_PMI_T0);
         contr->cpu_control.nrictrs--;
         contr->cpu_control.nractrs++;
      }
      nricntrs = contr->cpu_control.nrictrs;
      nracntrs = contr->cpu_control.nractrs;

      /* move this event to the top part of the list if needed */
      if (i >= nracntrs) {
         swap_events(ESI, contr, i, nracntrs - 1);
      }
      if (!nricntrs)
         contr->si_signo = 0;

      OVFDBG("Modified event set\n");

      retval = _papi_hwi_stop_signal(MY_VECTOR.cmp_info.hardware_intr_sig);
   }
#ifdef DEBUG
   print_control(&this_state->control.cpu_control);
#endif
   OVFDBG("End of call. Exit code: %d\n", retval);
   return (retval);
}

static void copy_value(unsigned int val, char *nam, char *names, unsigned int *values, int len)
{
   *values = val;
   strncpy(names, nam, len);
   names[len-1] = 0;
}

/**************************************************************/
    /* perfctr-p4      */
/* these define cccr and escr register bits, and the p4 event structure */
#include "perfmon/pfmlib_pentium4.h"
#include "../lib/pfmlib_pentium4_priv.h"

extern pentium4_escr_reg_t pentium4_escrs[];
extern pentium4_cccr_reg_t pentium4_cccrs[];
extern pentium4_event_t pentium4_events[];

extern inline int _pfm_decode_native_event(unsigned int EventCode, unsigned int *event, unsigned int *umask);
extern inline unsigned int _pfm_convert_umask(unsigned int event, unsigned int umask);

/* convert a collection of pfm mask bits into an array of pfm mask indices */
/* lifted wholesale from papi_pfm_events because I didn't want to publish it */
static inline int prepare_umask(unsigned int foo,unsigned int *values)
{
  unsigned int tmp = foo, i, j = 0;

  SUBDBG("umask 0x%x\n",tmp);
  while ((i = ffs(tmp)))
    {
      tmp = tmp ^ (1 << (i-1));
      values[j] = i - 1;
      SUBDBG("umask %d is %d\n",j,values[j]);
      j++;
    }
  return(j);
}


/* This array provides values for the PEBS_ENABLE and PEBS_MATRIX_VERT
	registers to support a series of metric for replay_event.
	The first two entries are dummies; the remaining 9 correspond to 
	virtual bit masks in the replay_event definition and map onto Intel
	documentation.
*/

#define P4_REPLAY_REAL_MASK 0x00000003
#define P4_REPLAY_VIRT_MASK 0x00000FFC

static pentium4_replay_regs_t p4_replay_regs[]={
/* 0 */ {.enb		= 0,			/* dummy */
		 .mat_vert	= 0,
		},
/* 1 */ {.enb		= 0,			/* dummy */
		 .mat_vert	= 0,
		},
/* 2 */ {.enb		= 0x01000001,	/* 1stL_cache_load_miss_retired */
		 .mat_vert	= 0x00000001,
		},
/* 3 */ {.enb		= 0x01000002,	/* 2ndL_cache_load_miss_retired */
		 .mat_vert	= 0x00000001,
		},
/* 4 */ {.enb		= 0x01000004,	/* DTLB_load_miss_retired */
		 .mat_vert	= 0x00000001,
		},
/* 5 */ {.enb		= 0x01000004,	/* DTLB_store_miss_retired */
		 .mat_vert	= 0x00000002,
		},
/* 6 */ {.enb		= 0x01000004,	/* DTLB_all_miss_retired */
		 .mat_vert	= 0x00000003,
		},
/* 7 */ {.enb		= 0x01018001,	/* Tagged_mispred_branch */
		 .mat_vert	= 0x00000010,
		},
/* 8 */ {.enb		= 0x01000200,	/* MOB_load_replay_retired */
		 .mat_vert	= 0x00000001,
		},
/* 9 */ {.enb		= 0x01000400,	/* split_load_retired */
		 .mat_vert	= 0x00000001,
		},
/* 10 */ {.enb		= 0x01000400,	/* split_store_retired */
		 .mat_vert	= 0x00000002,
		},
};

/* this maps the arbitrary pmd index in libpfm/pentium4_events.h to the intel documentation */
static int pfm2intel[] = {0, 1, 4, 5, 8, 9, 12, 13, 16, 2, 3, 6, 7, 10, 11, 14, 15, 17 };

int _papi_pfm_ntv_code_to_bits(unsigned int EventCode, hwd_register_t *bits)
{
    pentium4_escr_value_t escr_value;
    pentium4_cccr_value_t cccr_value;
    unsigned int umask, num_masks, replay_mask, unit_masks[12];
    unsigned int event, event_mask;
    unsigned int tag_value, tag_enable;

    int i, j, escr, cccr, pmd;

    if (_pfm_decode_native_event(EventCode,&event,&umask) != PAPI_OK)
      return(PAPI_ENOEVNT);

    /* for each allowed escr (1 or 2) find the allowed cccrs.
       for each allowed cccr find the pmd index
       convert to an intel counter number; or it into bits->counter
    */
	for (i = 0; i < MAX_ESCRS_PER_EVENT; i++) {
		bits->counter[i] = 0;
		escr = pentium4_events[event].allowed_escrs[i];
		if (escr < 0) {
			continue;
		}

		bits->escr[i] = escr;
		for (j = 0; j < MAX_CCCRS_PER_ESCR; j++) {
			cccr = pentium4_escrs[escr].allowed_cccrs[j];
			if (cccr < 0) {
				continue;
			}

			pmd = pentium4_cccrs[cccr].pmd;
			bits->counter[i] |= (1 << pfm2intel[pmd]);
		}
    }
    /* if there's only one valid escr, copy the values */
    if (escr < 0) {
	bits->escr[1] = bits->escr[0];
	bits->counter[1] = bits->counter[0];
    }

    /* Calculate the event-mask value. Invalid masks
     * specified by the caller are ignored.
     */
	tag_value = 0;
	tag_enable = 0;
	event_mask = _pfm_convert_umask(event, umask);
	if (event_mask & 0xF0000) {
		tag_enable = 1;
		tag_value = ((event_mask & 0xF0000) >> EVENT_MASK_BITS);
	}
	event_mask &=0x0FFFF; /* mask off possible tag bits */

    /* Set up the ESCR and CCCR register values. */
    escr_value.val = 0;

    escr_value.bits.t1_usr       = 0; /* controlled by kernel */
    escr_value.bits.t1_os        = 0; /* controlled by kernel */
//    escr_value.bits.t0_usr       = (plm & PFM_PLM3) ? 1 : 0;
//    escr_value.bits.t0_os        = (plm & PFM_PLM0) ? 1 : 0;
    escr_value.bits.tag_enable   = tag_enable;
    escr_value.bits.tag_value    = tag_value;
    escr_value.bits.event_mask   = event_mask;
    escr_value.bits.event_select = pentium4_events[event].event_select;
    escr_value.bits.reserved     = 0;

    /* initialize the proper bits in the cccr register */
    cccr_value.val = 0;
    cccr_value.bits.reserved1     = 0;
    cccr_value.bits.enable        = 1;
    cccr_value.bits.escr_select   = pentium4_events[event].escr_select;
    cccr_value.bits.active_thread = 3; /* FIXME: This is set to count when either logical
					*        CPU is active. Need a way to distinguish
					*        between logical CPUs when HT is enabled.
					*        the docs say these bits should always 
					*        be set.                                  */
    cccr_value.bits.compare       = 0; /* FIXME: What do we do with "threshold" settings? */
    cccr_value.bits.complement    = 0; /* FIXME: What do we do with "threshold" settings? */
    cccr_value.bits.threshold     = 0; /* FIXME: What do we do with "threshold" settings? */
    cccr_value.bits.force_ovf     = 0; /* FIXME: Do we want to allow "forcing" overflow
					*        interrupts on all counter increments? */
    cccr_value.bits.ovf_pmi_t0    = 0;
    cccr_value.bits.ovf_pmi_t1    = 0; /* PMI taken care of by kernel typically */
    cccr_value.bits.reserved2     = 0;
    cccr_value.bits.cascade       = 0; /* FIXME: How do we handle "cascading" counters? */
    cccr_value.bits.overflow      = 0;

	/* these flags are always zero, from what I can tell... */
	bits->pebs_enable = 0;	/* flag for PEBS counting */
	bits->pebs_matrix_vert = 0;	/* flag for PEBS_MATRIX_VERT, whatever that is */

	/* ...unless the event is replay_event */
	if (!strcmp(pentium4_events[event].name, "replay_event")) {
		escr_value.bits.event_mask   = event_mask & P4_REPLAY_REAL_MASK;
		num_masks = prepare_umask(umask, unit_masks);
		for (i = 0; i < num_masks; i++) {
			replay_mask = unit_masks[i];
			if (replay_mask > 1 && replay_mask < 11) { /* process each valid mask we find */
				bits->pebs_enable |= p4_replay_regs[replay_mask].enb;
				bits->pebs_matrix_vert |= p4_replay_regs[replay_mask].mat_vert;
			}
		}
	}
	/* store the escr and cccr values */
	bits->event = escr_value.val;
	bits->cccr = cccr_value.val;

	bits->ireset = 0;		/* I don't really know what this does */

	SUBDBG("escr: 0x%lx; cccr:  0x%lx\n", escr_value.val, cccr_value.val);

	return (PAPI_OK);
}

/* This version of bits_to_info is straight from p4_events and is appropriate 
    only for that class of machines. */
int _papi_pfm_ntv_bits_to_info(hwd_register_t *bits, char *names,
                               unsigned int *values, int name_len, int count)
{
   int i = 0;
   copy_value(bits->cccr, "P4 CCCR", &names[i*name_len], &values[i], name_len);
   if (++i == count) return(i);
   copy_value(bits->event, "P4 Event", &names[i*name_len], &values[i], name_len);
   if (++i == count) return(i);
   copy_value(bits->pebs_enable, "P4 PEBS Enable", &names[i*name_len], &values[i], name_len);
   if (++i == count) return(i);
   copy_value(bits->pebs_matrix_vert, "P4 PEBS Matrix Vertical", &names[i*name_len], &values[i], name_len);
   if (++i == count) return(i);
   copy_value(bits->ireset, "P4 iReset", &names[i*name_len], &values[i], name_len);
   return(++i);
}

#if defined(PAPI_PENTIUM4_FP_X87)
   #define P4_FPU " X87"
#elif defined(PAPI_PENTIUM4_FP_X87_SSE_SP)
   #define P4_FPU " X87 SSE_SP"
#elif defined(PAPI_PENTIUM4_FP_SSE_SP_DP)
   #define P4_FPU " SSE_SP SSE_DP"
#else
   #define P4_FPU " X87 SSE_DP"
#endif

static int _papi_hwd_fixup_fp(void)
{
   char table_name[PAPI_MIN_STR_LEN] = "Intel Pentium4 FPU";
   char *str = getenv("PAPI_PENTIUM4_FP");

   /* if the env variable isn't set, use the default */
   if ((str == NULL) || (strlen(str) == 0)) {
      strcat(table_name, P4_FPU);
   } else {
       if (strstr(str,"X87"))    strcat(table_name, " X87");
       if (strstr(str,"SSE_SP")) strcat(table_name, " SSE_SP");
       if (strstr(str,"SSE_DP")) strcat(table_name, " SSE_DP");
   }
   if((_papi_pfm_setup_presets(table_name, 0)) != PAPI_OK) {
      PAPIERROR("Improper usage of PAPI_PENTIUM4_FP environment variable.\nUse one or two of X87,SSE_SP,SSE_DP");
      return(PAPI_ESBSTR);
   }
   return(PAPI_OK);
}

#if defined(PAPI_PENTIUM4_VEC_MMX)
   #define P4_VEC "MMX"
#else
   #define P4_VEC "SSE"
#endif

static int _papi_hwd_fixup_vec(void)
{
   char table_name[PAPI_MIN_STR_LEN] = "Intel Pentium4 VEC ";
   char *str = getenv("PAPI_PENTIUM4_VEC");

   /* if the env variable isn't set, use the default */
   if ((str == NULL) || (strlen(str) == 0)) {
      strcat(table_name, P4_VEC);
   } else {
      strcat(table_name, str);
   }
   if((_papi_pfm_setup_presets(table_name, 0)) != PAPI_OK) {
      PAPIERROR("Improper usage of PAPI_PENTIUM4_VEC environment variable.\nUse either SSE or MMX");
      return(PAPI_ESBSTR);
   }
   return(PAPI_OK);
}

int setup_p3_presets(int cputype){
  int retval=PAPI_OK;
  return ( retval ); 
}

static int _p4_stop_profiling(ThreadInfo_t * master, EventSetInfo_t * ESI) {
   return (PAPI_OK);
}

papi_vector_t _p4_vector = {
    .cmp_info = {
	/* default component information (unspecified values are initialized to 0) */
	.num_mpx_cntrs =	PAPI_MPX_DEF_DEG,
	.default_domain =	PAPI_DOM_USER,
	.available_domains =	PAPI_DOM_USER|PAPI_DOM_KERNEL,
	.default_granularity =	PAPI_GRN_THR,
	.available_granularities = PAPI_GRN_THR,
	.itimer_sig = PAPI_INT_MPX_SIGNAL,
    .itimer_num = PAPI_INT_ITIMER,
    .itimer_res_ns = 1,
	.hardware_intr_sig =	PAPI_INT_SIGNAL,

	/* component specific cmp_info initializations */
	.fast_real_timer =	1,
	.fast_virtual_timer =	1,
	.attach =		1,
	.attach_must_ptrace =	1,
    },

    /* sizes of framework-opaque component-private structures */
    .size = {
	.context =		sizeof(P4_perfctr_context_t),
	.control_state =	sizeof(P4_perfctr_control_t),
	.reg_value =		sizeof(P4_register_t),
	.reg_alloc =		sizeof(P4_reg_alloc_t),
    },

    /* function pointers in this component */
    .init_control_state =	_p4_init_control_state,
    .start =			_p4_start,
    .stop =			_p4_stop,
    .read =			_p4_read,
//    .shutdown =			_p4_shutdown,
//    .ctl =			_p4_ctl,
    .bpt_map_avail =		_p4_bpt_map_avail,
    .bpt_map_set =		_p4_bpt_map_set,
    .bpt_map_exclusive =	_p4_bpt_map_exclusive,
    .bpt_map_shared =		_p4_bpt_map_shared,
    .bpt_map_preempt =		_p4_bpt_map_preempt,
    .bpt_map_update =		_p4_bpt_map_update,
    .allocate_registers =	_p4_allocate_registers,
    .update_control_state =	_p4_update_control_state,
    .set_domain =		_p4_set_domain,
    .reset =			_p4_reset,
    .set_overflow =		_p4_set_overflow,
    .stop_profiling =		_p4_stop_profiling,
//    .init_substrate =		_p4_init_substrate,
//    .dispatch_timer =		_p4_dispatch_timer,
//    .get_real_usec =		_p4_get_real_usec,
//    .get_real_cycles =		_p4_get_real_cycles,
//    .get_virt_cycles =		_p4_get_virt_cycles,
//    .get_virt_usec =		_p4_get_virt_usec,

    /* from pfm */
    .ntv_enum_events =		_p4_ntv_enum_events,
    .ntv_name_to_code =		_p4_ntv_name_to_code,
    .ntv_code_to_name =		_p4_ntv_code_to_name,
    .ntv_code_to_descr =	_p4_ntv_code_to_descr,
    .ntv_code_to_bits =		_p4_ntv_code_to_bits,
    .ntv_bits_to_info =		_p4_ntv_bits_to_info,

    /* from OS */
    .update_shlib_info = _linux_update_shlib_info,
    .get_memory_info =	_linux_get_memory_info,
    .get_system_info =	_linux_get_system_info,
    .init_substrate =	_linux_init_substrate,
    .ctl =			_linux_ctl,
    .dispatch_timer =		_linux_dispatch_timer,
    .init =		_linux_init,
    .get_dmem_info =	_linux_get_dmem_info,
    .shutdown =			_linux_shutdown,
    .get_real_usec =		_linux_get_real_usec,
    .get_real_cycles =		_linux_get_real_cycles,
    .get_virt_cycles =		_linux_get_virt_cycles,
    .get_virt_usec =		_linux_get_virt_usec
};

