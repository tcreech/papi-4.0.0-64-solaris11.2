/* 
 * Copyright (C) 2001-2002 Hewlett-Packard Co
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

/*
 * This file will eventually be part of the GNU C library (glibc),
 * in <sys/perfmon.h>. For now it is shipped with libpfm and
 * should be installed in /usr/include/perfmon to avoid future conflict.
 */
#ifndef _PERFMON_PERFMON_H_
#define _PERFMON_PERFMON_H_

/*
 * perfmon comamnds supported on all CPU models
 */
#define PFM_WRITE_PMCS		0x01
#define PFM_WRITE_PMDS		0x02
#define PFM_READ_PMDS		0x03
#define PFM_STOP		0x04
#define PFM_START		0x05
#define PFM_ENABLE		0x06
#define PFM_DISABLE		0x07
#define PFM_CREATE_CONTEXT	0x08
#define PFM_DESTROY_CONTEXT	0x09
#define PFM_RESTART		0x0a
#define PFM_PROTECT_CONTEXT	0x0b
#define PFM_GET_FEATURES	0x0c
#define PFM_DEBUG		0x0d
#define PFM_UNPROTECT_CONTEXT	0x0e
#define PFM_GET_PMC_RESET_VAL	0x0f

/*
 * CPU model specific commands (may not be supported on all models)
 */
#define PFM_WRITE_IBRS		0x20
#define PFM_WRITE_DBRS		0x21

/*
 * context flags
 */
#define PFM_FL_INHERIT_NONE	 0x00	/* never inherit a context across fork (default) */
#define PFM_FL_INHERIT_ONCE	 0x01	/* clone pfm_context only once across fork() */
#define PFM_FL_INHERIT_ALL	 0x02	/* always clone pfm_context across fork() */
#define PFM_FL_NOTIFY_BLOCK    	 0x04	/* block task on user level notifications */
#define PFM_FL_SYSTEM_WIDE	 0x08	/* create a system wide context */
#define PFM_FL_EXCL_IDLE	 0x20	/* exclude idle task from system wide session */
#define PFM_FL_UNSECURE		 0x40   /* allow unsecure monitoring for non self-monitoring task */

/*
 * PMC flags
 */
#define PFM_REGFL_OVFL_NOTIFY	0x1	/* send notification on overflow */
#define PFM_REGFL_RANDOM	0x2	/* randomize sampling periods    */

/*
 * PMD/PMC/IBR/DBR return flags (ignored on input)
 *
 * Those flags are used on output and must be checked in case EAGAIN is returned
 * by any of the calls using a pfarg_reg_t or pfarg_dbreg_t structure.
 */
#define PFM_REG_RETFL_NOTAVAIL	(1U<<31) /* set if register is implemented but not available */
#define PFM_REG_RETFL_EINVAL	(1U<<30) /* set if register entry is invalid */
#define PFM_REG_RETFL_MASK	(PFM_REG_RETFL_NOTAVAIL|PFM_REG_RETFL_EINVAL)

#define PFM_REG_HAS_ERROR(flag)	(((flag) & PFM_REG_RETFL_MASK) != 0)

/*
 * Request structure used to define a context
 */
typedef struct {
	unsigned long ctx_smpl_entries;	/* how many entries in sampling buffer */
	unsigned long ctx_smpl_regs[4];	/* which pmds to record on overflow */

	pid_t	      ctx_notify_pid;	/* which process to notify on overflow */
	int	      ctx_flags;	/* noblock/block, inherit flags */
	void	      *ctx_smpl_vaddr;	/* returns address of BTB buffer */

	unsigned long ctx_cpu_mask;	/* on which CPU to enable perfmon (systemwide) */

	unsigned long reserved[8];	/* for future use */
} pfarg_context_t;

/*
 * Request structure used to write/read a PMC or PMD
 */
typedef struct {
	unsigned int	reg_num;	/* which register */
	unsigned int	reg_flags;	/* PMC: notify/don't notify. PMD/PMC: return flags */
	unsigned long	reg_value;	/* configuration (PMC) or initial value (PMD) */

	unsigned long	reg_long_reset;	/* reset after sampling buffer overflow (large) */
	unsigned long	reg_short_reset;/* reset after counter overflow (small) */

	unsigned long	reg_reset_pmds[4]; /* which other counters to reset on overflow */
	unsigned long	reg_random_seed;   /* seed value when randomization is used */
	unsigned long	reg_random_mask;   /* bitmask used to limit random value */
	unsigned long	reg_last_reset_val;/* bitmask used to limit random value */

	unsigned long   reserved[13];	/* for future use */
} pfarg_reg_t;

typedef struct {
	unsigned int	dbreg_num;	/* which register */
	unsigned int	dbreg_flags;	/* dbregs return flags */
	unsigned long	dbreg_value;	/* configuration (PMC) or initial value (PMD) */
	unsigned long	reserved[6];
} pfarg_dbreg_t;

typedef struct {			
	unsigned int	ft_version;	/* perfmon: major [16-31], minor [0-15] */
	unsigned int	ft_smpl_version;/* sampling format: major [16-31], minor [0-15] */
	unsigned long	reserved[4];	/* for future use */
} pfarg_features_t;

/*
 * This header is at the beginning of the sampling buffer returned to the user.
 * It is exported as Read-Only at this point. It is directly followed by the
 * first record.
 */
typedef struct {
	unsigned int	hdr_version;		/* contains perfmon version (smpl format diffs) */
	unsigned int	reserved;
	unsigned long	hdr_entry_size;		/* size of one entry in bytes */
	unsigned long	hdr_count;		/* how many valid entries */
	unsigned long	hdr_pmds[4];		/* which pmds are recorded */
} perfmon_smpl_hdr_t;

/*
 * Define the version numbers for both perfmon as a whole and the sampling buffer format.
 */
#define PFM_VERSION_MAJ		1U
#define PFM_VERSION_MIN		0U
#define PFM_VERSION		(((PFM_VERSION_MAJ&0xffff)<<16)|(PFM_VERSION_MIN & 0xffff))

#define PFM_SMPL_VERSION_MAJ	1U
#define PFM_SMPL_VERSION_MIN	1U
#define PFM_SMPL_VERSION	(((PFM_SMPL_VERSION_MAJ&0xffff)<<16)|(PFM_SMPL_VERSION_MIN & 0xffff))


#define PFM_VERSION_MAJOR(x)	(((x)>>16) & 0xffff)
#define PFM_VERSION_MINOR(x)	((x) & 0xffff)

/*
 * Entry header in the sampling buffer.  The header is directly followed
 * with the PMDs saved in increasing index order: PMD4, PMD5, .... How
 * many PMDs are present is determined by the user program during
 * context creation.
 *
 * XXX: in this version of the entry, only up to 64 registers can be
 * recorded. This should be enough for quite some time. Always check
 * sampling format before parsing entries!
 *
 * In the case where multiple counters overflow at the same time, the
 * last_reset_value member indicates the initial value of the PMD with
 * the smallest index.  For instance, if PMD2 and PMD5 have overflowed,
 * the last_reset_value member contains the initial value of PMD2.
 */
typedef struct {
	int		pid;		 /* identification of process */
	int		cpu;		 /* which cpu was used */
	unsigned long	last_reset_val;  /* initial value of counter that overflowed */
	unsigned long	stamp;		 /* timestamp */
	unsigned long	ip;		 /* where did the overflow interrupt happened */
	unsigned long	regs;		 /* bitmask of which registers overflowed */
	unsigned long   reserved;	 /* unused */
} perfmon_smpl_entry_t;

extern int perfmonctl(pid_t pid, int cmd, void *arg, int narg);

#endif /* _PERFMON_PERFMON_H_ */
