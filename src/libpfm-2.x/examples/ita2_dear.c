/*
 * ita2_dear.c - example of how use the D-EAR with the Itanium 2 PMU
 *
 * Copyright (C) 2003 Hewlett-Packard Co
 * Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.  
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of libpfm, a performance monitoring support library for
 * applications on Linux/ia64.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_itanium2.h>

#define NUM_PMCS PMU_MAX_PMCS
#define NUM_PMDS PMU_MAX_PMDS

#define SMPL_PERIOD	(400)

/*
 * We use a small buffer size to exercise the overflow handler
 */
#define SMPL_BUF_NENTRIES	64

#define M_PMD(x)		(1UL<<(x))
#define DEAR_REGS_MASK		(M_PMD(2)|M_PMD(3)|M_PMD(17))

static void *smpl_vaddr;
static unsigned long smpl_regs; /* just 64 registers for now */
static pfmlib_param_t evt;

long
do_test(unsigned long loop)
{
	long sum  = 0;

	pfm_start();
	while(loop--) sum += getpid();
	pfm_stop();
	return sum;
}


static void fatal_error(char *fmt,...) __attribute__((noreturn));

static void
fatal_error(char *fmt, ...) 
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}

/*
 * print content of sampling buffer
 *
 * XXX: using stdio to print from a signal handler is not safe with multi-threaded
 * applications
 */
#define safe_printf	printf

int
process_smpl_buffer(void)
{
	perfmon_smpl_hdr_t *hdr = (perfmon_smpl_hdr_t *)smpl_vaddr;
	perfmon_smpl_entry_t *ent;
	unsigned long pos;
	unsigned long smpl_entry = 0;
	pfm_ita2_reg_t *reg;
	int i, ret;

	/*
	 * Make sure the kernel uses the format we understand
	 */
	if (PFM_VERSION_MAJOR(hdr->hdr_version) != PFM_VERSION_MAJOR(PFM_SMPL_VERSION)) {
		fatal_error("Perfmon v%u.%u sampling format is not supported\n", 
				PFM_VERSION_MAJOR(hdr->hdr_version),
				PFM_VERSION_MINOR(hdr->hdr_version));
	}

	pos = (unsigned long)(hdr+1);

	/*
	 * walk through all the entries recored in the buffer
	 */
	for(i=0; i < hdr->hdr_count; i++) {

		ret = 0;

		ent = (perfmon_smpl_entry_t *)pos;
		/*
		 * print entry header
		 */
		safe_printf("Entry %ld PID:%d CPU:%d STAMP:0x%lx IIP:0x%016lx\n",
			smpl_entry++,
			ent->pid,
			ent->cpu,
			ent->stamp,
			ent->ip);


		/*
		 * point to first recorded register (always contiguous with entry header)
		 */
		reg = (pfm_ita2_reg_t*)(ent+1);
		safe_printf("PMD2 : 0x%016lx\n", reg->reg_val);

		reg++;

		safe_printf("PMD3 : 0x%016lx, valid %c "
			    ", latency %u, overflow %c\n", 
			    reg->reg_val,
			    reg->pmd3_ita2_reg.dear_stat ? 'Y' : 'N',
			    reg->pmd3_ita2_reg.dear_latency,
			    reg->pmd3_ita2_reg.dear_overflow ? 'Y' : 'N');

		reg++;

		safe_printf("PMD17: 0x%016lx, valid %c, bundle %d, address 0x%016lx\n",
				reg->reg_val,
				reg->pmd17_ita2_reg.dear_vl ? 'Y': 'N',
				reg->pmd17_ita2_reg.dear_bn,
				((reg->pmd17_ita2_reg.dear_iaddr+reg->pmd17_ita2_reg.dear_bn) << 4) |
				(unsigned long)reg->pmd17_ita2_reg.dear_slot);

		/*
		 * move to next entry
		 */
		pos += hdr->hdr_entry_size;

	}
	return 0;
}

static void
overflow_handler(int n, struct pfm_siginfo *info, struct sigcontext *sc)
{
	unsigned long mask =info->sy_pfm_ovfl[0];

	/*
	 * Check to see if we received a spurious SIGPROF, i.e., one not
	 * generated by the perfmon subsystem.
	 */
	if (info->sy_code != PROF_OVFL) {
		printf("Received spurious SIGPROF si_code=%d\n", info->sy_code);
		return;
	} 
	/*
	 * Each bit set in the overflow mask represents an overflowed counter.
	 *
	 * Here we check that the overflow was caused by our first counter.
	 */
	if ((mask & (1UL<< evt.pfp_pc[0].reg_num)) == 0) {
		printf("Something is wrong, unexpected mask 0x%lx\n", mask);
		exit(1);
	}

	printf("Notification received\n");
	process_smpl_buffer();
	/*
	 * And resume monitoring
	 */
	if (perfmonctl(getpid(), PFM_RESTART,NULL, 0) == -1) {
		perror("PFM_RESTART");
		exit(1);
	}
	/* Here we have the PMU enabled and are capturing events */
}


int
main(void)
{
	int ret;
	int type = 0, ev;
	pid_t pid = getpid();
	pfmlib_ita2_param_t ita2_param;
	pfarg_reg_t pd[NUM_PMDS];
	pfarg_context_t ctx[1];
	pfmlib_options_t pfmlib_options;
	struct sigaction act;

	/*
	 * Initialize pfm library (required before we can use it)
	 */
	if (pfm_initialize() != PFMLIB_SUCCESS) {
		fatal_error("Can't initialize library\n");
	}

	/*
	 * Let's make sure we run this on the right CPU
	 */
	pfm_get_pmu_type(&type);
	if (type != PFMLIB_ITANIUM2_PMU) {
		char *model; 
		pfm_get_pmu_name(&model);
		fatal_error("this program does not work with %s PMU\n", model);
	}

	/*
	 * Install the overflow handler (SIGPROF)
	 */
	memset(&act, 0, sizeof(act));
	act.sa_handler = (sig_t)overflow_handler;
	sigaction (SIGPROF, &act, 0);


	/*
	 * pass options to library (optional)
	 */
	memset(&pfmlib_options, 0, sizeof(pfmlib_options));
	pfmlib_options.pfm_debug = 0; /* set to 1 for debug */
	pfmlib_options.pfm_verbose = 0; /* set to 1 for debug */
	pfm_set_options(&pfmlib_options);



	memset(pd, 0, sizeof(pd));
	memset(ctx, 0, sizeof(ctx));

	/*
	 * prepare parameters to library. we don't use any Itanium
	 * specific features here. so the pfp_model is NULL.
	 */
	memset(&evt,0, sizeof(evt));
	memset(&ita2_param,0, sizeof(ita2_param));


	/*
	 * because we use a model specific feature, we must initialize the
	 * model specific pfmlib parameter structure and link it to the
	 * common structure.
	 * The magic number is a simple mechanism used by the library to check
	 * that the model specific data structure is decent. You must set it manually
	 * otherwise the model specific feature won't work.
	 */
	ita2_param.pfp_magic = PFMLIB_ITA2_PARAM_MAGIC;
	evt.pfp_model        = &ita2_param;

	/*
	 * To count the number of occurence of this instruction, we must
	 * program a counting monitor with the IA64_TAGGED_INST_RETIRED_PMC8
	 * event.
	 */
	if (pfm_find_event_byname("DATA_EAR_CACHE_LAT4", &ev) != PFMLIB_SUCCESS) {
		fatal_error("cannot find event DATA_EAR_CACHE_LAT4\n");
	}

	/*
	 * set the (global) privilege mode:
	 * 	PFM_PLM3 : user level only
	 */
	evt.pfp_dfl_plm   = PFM_PLM3|PFM_PLM0; 

	/*
	 * how many counters we use
	 */
	evt.pfp_event_count = 1;

	/*
	 * propagate the event descriptor
	 */
	evt.pfp_events[0].event = ev;

	/*
	 * indicate that we configure DEAR
	 */
	ita2_param.pfp_ita2_dear.ear_used   = 1;

	/*
	 * determine EAR mode (CACHE, TLB, ALAT)
	 */
	pfm_ita2_get_ear_mode(ev, &ita2_param.pfp_ita2_dear.ear_mode);

	/*
	 * configure privilege level mask and instruction set for EAR
	 * leaving it to zero will use the global default set in pfp_dfl_plm
	 */
	ita2_param.pfp_ita2_dear.ear_plm = PFM_PLM3|PFM_PLM0;
	ita2_param.pfp_ita2_dear.ear_ism = PFMLIB_ITA2_ISM_IA64; /* ia64 only */

	/*
	 * extract the umask (encodes the latency)
	 */
	pfm_ita2_get_event_umask(ev, &ita2_param.pfp_ita2_dear.ear_umask);
	
	/*
	 * let the library figure out the values for the PMCS
	 */
	if ((ret=pfm_dispatch_events(&evt)) != PFMLIB_SUCCESS) {
		fatal_error("cannot configure events: %s\n", pfm_strerror(ret));
	}
	/*
	 * for this example, we will get notified ONLY when the sampling
	 * buffer is full. The monitoring is not to be inherited
	 * in derived tasks
	 */
	ctx[0].ctx_flags        = PFM_FL_INHERIT_NONE;
	ctx[0].ctx_notify_pid   = getpid();
	ctx[0].ctx_smpl_entries = SMPL_BUF_NENTRIES;
	ctx[0].ctx_smpl_regs[0] = smpl_regs = DEAR_REGS_MASK;

	/*
	 * now create the context for self monitoring/per-task
	 */
	if (perfmonctl(pid, PFM_CREATE_CONTEXT, ctx, 1) == -1 ) {
		if (errno == ENOSYS) {
			fatal_error("Your kernel does not have performance monitoring support!\n");
		}
		fatal_error("Can't create PFM context %s\n", strerror(errno));
	}

	printf("Sampling buffer mapped at %p\n", ctx[0].ctx_smpl_vaddr);

	smpl_vaddr = ctx[0].ctx_smpl_vaddr;

	/* 
	 * Must be done before any PMD/PMD calls (unfreeze PMU). Initialize
	 * PMC/PMD to safe values. psr.up is cleared.
	 */
	if (perfmonctl(pid, PFM_ENABLE, NULL, 0) == -1) {
		fatal_error("perfmonctl error PFM_ENABLE errno %d\n",errno);
	}

	/*
	 * indicate we want notification when buffer is full
	 */
	evt.pfp_pc[0].reg_flags |= PFM_REGFL_OVFL_NOTIFY;

	/*
	 * Now prepare the argument to initialize the PMD and the sampling period
	 */
	pd[0].reg_num         = evt.pfp_pc[0].reg_num;
	pd[0].reg_value       = (~0UL) - SMPL_PERIOD +1;
	pd[0].reg_long_reset  = (~0UL) - SMPL_PERIOD +1;
	pd[0].reg_short_reset = (~0UL) - SMPL_PERIOD +1;

	/*
	 * Now program the registers
	 *
	 * We don't use the save variable to indicate the number of elements passed to
	 * the kernel because, as we said earlier, pc may contain more elements than
	 * the number of events we specified, i.e., contains more thann coutning monitors.
	 */
	if (perfmonctl(pid, PFM_WRITE_PMCS, evt.pfp_pc, evt.pfp_pc_count) == -1) {
		fatal_error("perfmonctl error PFM_WRITE_PMCS errno %d\n",errno);
	}
	if (perfmonctl(pid, PFM_WRITE_PMDS, pd, 1) == -1) {
		fatal_error("perfmonctl error PFM_WRITE_PMDS errno %d\n",errno);
	}

	/*
	 * Let's roll now.
	 */
	do_test(100000);

	/*
	 * We must call the processing routine to cover the last entries recorded
	 * in the sampling buffer, i.e. which may not be full
	 */
	process_smpl_buffer();

	/* 
	 * let's stop this now
	 */
	if (perfmonctl(pid, PFM_DESTROY_CONTEXT, NULL, 0) == -1) {
		fatal_error("perfmonctl error PFM_DESTROY errno %d\n",errno);
	}
	return 0;
}
