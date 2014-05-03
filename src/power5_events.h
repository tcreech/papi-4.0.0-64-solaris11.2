/****************************/
/* THIS IS OPEN SOURCE CODE */
/****************************/

#ifndef _PAPI_POWER5_EVENTS_H
#define _PAPI_POWER5_EVENTS_H

/* 
* File:    power5_events.h
* CVS:     
*          
* This file MUST be kept synchronised with the events file.
*
*/
#include "papiStdEventDefs.h"

#define GROUP_INTS 5
#define PAPI_MAX_NATIVE_EVENTS 512
#define MAX_GROUPS (GROUP_INTS * 32)
#define MAX_NATNAME_MAP_INDEX 466


enum native_name {
PNE_PM_0INST_CLB_CYC = PAPI_NATIVE_MASK,
	PNE_PM_1INST_CLB_CYC,
	PNE_PM_1PLUS_PPC_CMPL,
	PNE_PM_2INST_CLB_CYC,
	PNE_PM_3INST_CLB_CYC,
	PNE_PM_4INST_CLB_CYC,
	PNE_PM_5INST_CLB_CYC,
	PNE_PM_6INST_CLB_CYC,
	PNE_PM_BRQ_FULL_CYC,
	PNE_PM_BR_UNCOND,
	PNE_PM_CLB_FULL_CYC,
	PNE_PM_CR_MAP_FULL_CYC,
	PNE_PM_CYC,
	PNE_PM_DATA_FROM_L2,
	PNE_PM_DATA_FROM_L25_SHR,
	PNE_PM_DATA_FROM_L275_MOD,
	PNE_PM_DATA_FROM_L3,
	PNE_PM_DATA_FROM_L35_SHR,
	PNE_PM_DATA_FROM_L375_MOD,
	PNE_PM_DATA_FROM_RMEM,
	PNE_PM_DATA_TABLEWALK_CYC,
	PNE_PM_DSLB_MISS,
	PNE_PM_DTLB_MISS,
	PNE_PM_DTLB_MISS_16M,
	PNE_PM_DTLB_MISS_4K,
	PNE_PM_DTLB_REF_16M,
	PNE_PM_DTLB_REF_4K,
	PNE_PM_FAB_CMD_ISSUED,
	PNE_PM_FAB_DCLAIM_ISSUED,
	PNE_PM_FAB_HOLDtoNN_EMPTY,
	PNE_PM_FAB_HOLDtoVN_EMPTY,
	PNE_PM_FAB_M1toP1_SIDECAR_EMPTY,
	PNE_PM_FAB_P1toM1_SIDECAR_EMPTY,
	PNE_PM_FAB_PNtoNN_DIRECT,
	PNE_PM_FAB_PNtoVN_DIRECT,
	PNE_PM_FPR_MAP_FULL_CYC,
	PNE_PM_FPU0_1FLOP,
	PNE_PM_FPU0_DENORM,
	PNE_PM_FPU0_FDIV,
	PNE_PM_FPU0_FMA,
	PNE_PM_FPU0_FSQRT,
	PNE_PM_FPU0_FULL_CYC,
	PNE_PM_FPU0_SINGLE,
	PNE_PM_FPU0_STALL3,
	PNE_PM_FPU0_STF,
	PNE_PM_FPU1_1FLOP,
	PNE_PM_FPU1_DENORM,
	PNE_PM_FPU1_FDIV,
	PNE_PM_FPU1_FMA,
	PNE_PM_FPU1_FSQRT,
	PNE_PM_FPU1_FULL_CYC,
	PNE_PM_FPU1_SINGLE,
	PNE_PM_FPU1_STALL3,
	PNE_PM_FPU1_STF,
	PNE_PM_FPU_DENORM,
	PNE_PM_FPU_FDIV,
	PNE_PM_FPU_1FLOP,
	PNE_PM_FPU_FULL_CYC,
	PNE_PM_FPU_SINGLE,
	PNE_PM_FXU_IDLE,
	PNE_PM_GCT_NOSLOT_CYC,
	PNE_PM_GCT_FULL_CYC,
	PNE_PM_GCT_USAGE_00to59_CYC,
	PNE_PM_GRP_BR_REDIR,
	PNE_PM_GRP_BR_REDIR_NONSPEC,
	PNE_PM_GRP_DISP_REJECT,
	PNE_PM_GRP_DISP_VALID,
	PNE_PM_GRP_IC_MISS,
	PNE_PM_GRP_IC_MISS_BR_REDIR_NONSPEC,
	PNE_PM_GRP_IC_MISS_NONSPEC,
	PNE_PM_GRP_MRK,
	PNE_PM_IC_PREF_REQ,
	PNE_PM_IERAT_XLATE_WR,
	PNE_PM_INST_CMPL,
	PNE_PM_INST_DISP,
	PNE_PM_INST_FETCH_CYC,
	PNE_PM_INST_FROM_L2,
	PNE_PM_INST_FROM_L25_SHR,
	PNE_PM_INST_FROM_L3,
	PNE_PM_INST_FROM_L35_SHR,
	PNE_PM_ISLB_MISS,
	PNE_PM_ITLB_MISS,
	PNE_PM_L2SA_MOD_TAG,
	PNE_PM_L2SA_RCLD_DISP,
	PNE_PM_L2SA_RCLD_DISP_FAIL_RC_FULL,
	PNE_PM_L2SA_RCST_DISP,
	PNE_PM_L2SA_RCST_DISP_FAIL_RC_FULL,
	PNE_PM_L2SA_RC_DISP_FAIL_CO_BUSY,
	PNE_PM_L2SA_SHR_MOD,
	PNE_PM_L2SA_ST_REQ,
	PNE_PM_L2SB_MOD_TAG,
	PNE_PM_L2SB_RCLD_DISP,
	PNE_PM_L2SB_RCLD_DISP_FAIL_RC_FULL,
	PNE_PM_L2SB_RCST_DISP,
	PNE_PM_L2SB_RCST_DISP_FAIL_RC_FULL,
	PNE_PM_L2SB_RC_DISP_FAIL_CO_BUSY,
	PNE_PM_L2SB_SHR_MOD,
	PNE_PM_L2SB_ST_REQ,
	PNE_PM_L2SC_MOD_TAG,
	PNE_PM_L2SC_RCLD_DISP,
	PNE_PM_L2SC_RCLD_DISP_FAIL_RC_FULL,
	PNE_PM_L2SC_RCST_DISP,
	PNE_PM_L2SC_RCST_DISP_FAIL_RC_FULL,
	PNE_PM_L2SC_RC_DISP_FAIL_CO_BUSY,
	PNE_PM_L2SC_SHR_MOD,
	PNE_PM_L2SC_ST_REQ,
	PNE_PM_L3SA_ALL_BUSY,
	PNE_PM_L3SA_MOD_TAG,
	PNE_PM_L3SA_REF,
	PNE_PM_L3SB_ALL_BUSY,
	PNE_PM_L3SB_MOD_TAG,
	PNE_PM_L3SB_REF,
	PNE_PM_L3SC_ALL_BUSY,
	PNE_PM_L3SC_MOD_TAG,
	PNE_PM_L3SC_REF,
	PNE_PM_LARX_LSU0,
	PNE_PM_LR_CTR_MAP_FULL_CYC,
	PNE_PM_LSU0_BUSY_REJECT,
	PNE_PM_LSU0_DERAT_MISS,
	PNE_PM_LSU0_FLUSH_LRQ,
	PNE_PM_LSU0_FLUSH_SRQ,
	PNE_PM_LSU0_FLUSH_ULD,
	PNE_PM_LSU0_FLUSH_UST,
	PNE_PM_LSU0_REJECT_ERAT_MISS,
	PNE_PM_LSU0_REJECT_LMQ_FULL,
	PNE_PM_LSU0_REJECT_RELOAD_CDF,
	PNE_PM_LSU0_REJECT_SRQ_LHS,
	PNE_PM_LSU0_SRQ_STFWD,
	PNE_PM_LSU1_BUSY_REJECT,
	PNE_PM_LSU1_DERAT_MISS,
	PNE_PM_LSU1_FLUSH_LRQ,
	PNE_PM_LSU1_FLUSH_SRQ,
	PNE_PM_LSU1_FLUSH_ULD,
	PNE_PM_LSU1_FLUSH_UST,
	PNE_PM_LSU1_REJECT_ERAT_MISS,
	PNE_PM_LSU1_REJECT_LMQ_FULL,
	PNE_PM_LSU1_REJECT_RELOAD_CDF,
	PNE_PM_LSU1_REJECT_SRQ_LHS,
	PNE_PM_LSU1_SRQ_STFWD,
	PNE_PM_LSU_BUSY_REJECT,
	PNE_PM_LSU_FLUSH_LRQ_FULL,
	PNE_PM_LSU_FLUSH_SRQ,
	PNE_PM_LSU_FLUSH_ULD,
	PNE_PM_LSU_LRQ_S0_ALLOC,
	PNE_PM_LSU_LRQ_S0_VALID,
	PNE_PM_LSU_REJECT_ERAT_MISS,
	PNE_PM_LSU_REJECT_SRQ_LHS,
	PNE_PM_LSU_SRQ_S0_ALLOC,
	PNE_PM_LSU_SRQ_S0_VALID,
	PNE_PM_LSU_SRQ_STFWD,
	PNE_PM_MEM_FAST_PATH_RD_CMPL,
	PNE_PM_MEM_HI_PRIO_PW_CMPL,
	PNE_PM_MEM_HI_PRIO_WR_CMPL,
	PNE_PM_MEM_PWQ_DISP,
	PNE_PM_MEM_PWQ_DISP_BUSY2or3,
	PNE_PM_MEM_READ_CMPL,
	PNE_PM_MEM_RQ_DISP,
	PNE_PM_MEM_RQ_DISP_BUSY8to15,
	PNE_PM_MEM_WQ_DISP_BUSY1to7,
	PNE_PM_MEM_WQ_DISP_WRITE,
	PNE_PM_MRK_DATA_FROM_L2,
	PNE_PM_MRK_DATA_FROM_L25_SHR,
	PNE_PM_MRK_DATA_FROM_L275_MOD,
	PNE_PM_MRK_DATA_FROM_L3,
	PNE_PM_MRK_DATA_FROM_L35_SHR,
	PNE_PM_MRK_DATA_FROM_L375_MOD,
	PNE_PM_MRK_DATA_FROM_RMEM,
	PNE_PM_MRK_DTLB_MISS_16M,
	PNE_PM_MRK_DTLB_MISS_4K,
	PNE_PM_MRK_DTLB_REF_16M,
	PNE_PM_MRK_DTLB_REF_4K,
	PNE_PM_MRK_GRP_DISP,
	PNE_PM_MRK_GRP_ISSUED,
	PNE_PM_MRK_IMR_RELOAD,
	PNE_PM_MRK_LD_MISS_L1,
	PNE_PM_MRK_LD_MISS_L1_LSU0,
	PNE_PM_MRK_LD_MISS_L1_LSU1,
	PNE_PM_MRK_STCX_FAIL,
	PNE_PM_MRK_ST_CMPL,
	PNE_PM_MRK_ST_MISS_L1,
	PNE_PM_PMC4_OVERFLOW,
	PNE_PM_PMC5_OVERFLOW,
	PNE_PM_PTEG_FROM_L2,
	PNE_PM_PTEG_FROM_L25_SHR,
	PNE_PM_PTEG_FROM_L275_MOD,
	PNE_PM_PTEG_FROM_L3,
	PNE_PM_PTEG_FROM_L35_SHR,
	PNE_PM_PTEG_FROM_L375_MOD,
	PNE_PM_PTEG_FROM_RMEM,
	PNE_PM_RUN_CYC,
	PNE_PM_SNOOP_DCLAIM_RETRY_QFULL,
	PNE_PM_SNOOP_PW_RETRY_RQ,
	PNE_PM_SNOOP_RD_RETRY_QFULL,
	PNE_PM_SNOOP_RD_RETRY_RQ,
	PNE_PM_SNOOP_RETRY_1AHEAD,
	PNE_PM_SNOOP_TLBIE,
	PNE_PM_SNOOP_WR_RETRY_RQ,
	PNE_PM_STCX_FAIL,
	PNE_PM_STCX_PASS,
	PNE_PM_SUSPENDED,
	PNE_PM_TB_BIT_TRANS,
	PNE_PM_THRD_ONE_RUN_CYC,
	PNE_PM_THRD_PRIO_1_CYC,
	PNE_PM_THRD_PRIO_2_CYC,
	PNE_PM_THRD_PRIO_3_CYC,
	PNE_PM_THRD_PRIO_4_CYC,
	PNE_PM_THRD_PRIO_5_CYC,
	PNE_PM_THRD_PRIO_6_CYC,
	PNE_PM_THRD_PRIO_7_CYC,
	PNE_PM_TLB_MISS,
	PNE_PM_XER_MAP_FULL_CYC,
	PNE_PM_INST_FROM_L2MISS,
	PNE_PM_BR_PRED_TA,
	PNE_PM_CMPLU_STALL_DCACHE_MISS,
	PNE_PM_CMPLU_STALL_FDIV,
	PNE_PM_CMPLU_STALL_FXU,
	PNE_PM_CMPLU_STALL_LSU,
	PNE_PM_DATA_FROM_L25_MOD,
	PNE_PM_DATA_FROM_L35_MOD,
	PNE_PM_DATA_FROM_LMEM,
	PNE_PM_FPU_FSQRT,
	PNE_PM_FPU_FMA,
	PNE_PM_FPU_STALL3,
	PNE_PM_FPU_STF,
	PNE_PM_FXU_BUSY,
	PNE_PM_FXU_FIN,
	PNE_PM_GCT_NOSLOT_IC_MISS,
	PNE_PM_GCT_USAGE_60to79_CYC,
	PNE_PM_GRP_DISP,
	PNE_PM_HV_CYC,
	PNE_PM_INST_FROM_L1,
	PNE_PM_INST_FROM_L25_MOD,
	PNE_PM_INST_FROM_L35_MOD,
	PNE_PM_INST_FROM_LMEM,
	PNE_PM_LSU_DERAT_MISS,
	PNE_PM_LSU_FLUSH_LRQ,
	PNE_PM_LSU_FLUSH_UST,
	PNE_PM_LSU_LMQ_SRQ_EMPTY_CYC,
	PNE_PM_LSU_REJECT_LMQ_FULL,
	PNE_PM_LSU_REJECT_RELOAD_CDF,
	PNE_PM_MRK_BRU_FIN,
	PNE_PM_MRK_DATA_FROM_L25_MOD,
	PNE_PM_MRK_DATA_FROM_L25_SHR_CYC,
	PNE_PM_MRK_DATA_FROM_L275_SHR_CYC,
	PNE_PM_MRK_DATA_FROM_L2_CYC,
	PNE_PM_MRK_DATA_FROM_L35_MOD,
	PNE_PM_MRK_DATA_FROM_L35_SHR_CYC,
	PNE_PM_MRK_DATA_FROM_L375_SHR_CYC,
	PNE_PM_MRK_DATA_FROM_L3_CYC,
	PNE_PM_MRK_DATA_FROM_LMEM,
	PNE_PM_MRK_GRP_BR_REDIR,
	PNE_PM_MRK_ST_GPS,
	PNE_PM_PMC1_OVERFLOW,
	PNE_PM_PTEG_FROM_L25_MOD,
	PNE_PM_PTEG_FROM_L35_MOD,
	PNE_PM_PTEG_FROM_LMEM,
	PNE_PM_SLB_MISS,
	PNE_PM_GCT_EMPTY_CYC,
	PNE_PM_THRD_GRP_CMPL_BOTH_CYC,
	PNE_PM_BR_ISSUED,
	PNE_PM_BR_MPRED_CR,
	PNE_PM_BR_MPRED_TA,
	PNE_PM_BR_PRED_CR,
	PNE_PM_CRQ_FULL_CYC,
	PNE_PM_DATA_FROM_L275_SHR,
	PNE_PM_DATA_FROM_L375_SHR,
	PNE_PM_DC_INV_L2,
	PNE_PM_DC_PREF_DST,
	PNE_PM_DC_PREF_STREAM_ALLOC,
	PNE_PM_EE_OFF,
	PNE_PM_EE_OFF_EXT_INT,
	PNE_PM_FAB_CMD_RETRIED,
	PNE_PM_FAB_DCLAIM_RETRIED,
	PNE_PM_FAB_M1toVNorNN_SIDECAR_EMPTY,
	PNE_PM_FAB_P1toVNorNN_SIDECAR_EMPTY,
	PNE_PM_FAB_PNtoNN_SIDECAR,
	PNE_PM_FAB_PNtoVN_SIDECAR,
	PNE_PM_FAB_VBYPASS_EMPTY,
	PNE_PM_FLUSH_BR_MPRED,
	PNE_PM_FLUSH_IMBAL,
	PNE_PM_FLUSH,
	PNE_PM_FLUSH_SB,
	PNE_PM_FLUSH_SYNC,
	PNE_PM_FPU0_FEST,
	PNE_PM_FPU0_FIN,
	PNE_PM_FPU0_FMOV_FEST,
	PNE_PM_FPU0_FPSCR,
	PNE_PM_FPU0_FRSP_FCONV,
	PNE_PM_FPU1_FEST,
	PNE_PM_FPU1_FIN,
	PNE_PM_FPU1_FMOV_FEST,
	PNE_PM_FPU1_FRSP_FCONV,
	PNE_PM_FPU_FMOV_FEST,
	PNE_PM_FPU_FRSP_FCONV,
	PNE_PM_FXLS0_FULL_CYC,
	PNE_PM_FXLS1_FULL_CYC,
	PNE_PM_FXU0_BUSY_FXU1_IDLE,
	PNE_PM_FXU0_FIN,
	PNE_PM_FXU1_FIN,
	PNE_PM_GCT_NOSLOT_SRQ_FULL,
	PNE_PM_GCT_USAGE_80to99_CYC,
	PNE_PM_GPR_MAP_FULL_CYC,
	PNE_PM_GRP_CMPL,
	PNE_PM_GRP_DISP_BLK_SB_CYC,
	PNE_PM_GRP_DISP_SUCCESS,
	PNE_PM_IC_DEMAND_L2_BHT_REDIRECT,
	PNE_PM_IC_DEMAND_L2_BR_REDIRECT,
	PNE_PM_IC_PREF_INSTALL,
	PNE_PM_INST_FROM_L275_SHR,
	PNE_PM_INST_FROM_L375_SHR,
	PNE_PM_INST_FROM_PREF,
	PNE_PM_L1_DCACHE_RELOAD_VALID,
	PNE_PM_L1_PREF,
	PNE_PM_L1_WRITE_CYC,
	PNE_PM_L2SA_MOD_INV,
	PNE_PM_L2SA_RCLD_DISP_FAIL_ADDR,
	PNE_PM_L2SA_RCLD_DISP_FAIL_OTHER,
	PNE_PM_L2SA_RCST_DISP_FAIL_ADDR,
	PNE_PM_L2SA_RCST_DISP_FAIL_OTHER,
	PNE_PM_L2SA_RC_DISP_FAIL_CO_BUSY_ALL,
	PNE_PM_L2SA_SHR_INV,
	PNE_PM_L2SA_ST_HIT,
	PNE_PM_L2SB_MOD_INV,
	PNE_PM_L2SB_RCLD_DISP_FAIL_ADDR,
	PNE_PM_L2SB_RCLD_DISP_FAIL_OTHER,
	PNE_PM_L2SB_RCST_DISP_FAIL_ADDR,
	PNE_PM_L2SB_RCST_DISP_FAIL_OTHER,
	PNE_PM_L2SB_RC_DISP_FAIL_CO_BUSY_ALL,
	PNE_PM_L2SB_SHR_INV,
	PNE_PM_L2SB_ST_HIT,
	PNE_PM_L2SC_MOD_INV,
	PNE_PM_L2SC_RCLD_DISP_FAIL_ADDR,
	PNE_PM_L2SC_RCLD_DISP_FAIL_OTHER,
	PNE_PM_L2SC_RCST_DISP_FAIL_ADDR,
	PNE_PM_L2SC_RCST_DISP_FAIL_OTHER,
	PNE_PM_L2SC_RC_DISP_FAIL_CO_BUSY_ALL,
	PNE_PM_L2SC_SHR_INV,
	PNE_PM_L2SC_ST_HIT,
	PNE_PM_L2_PREF,
	PNE_PM_L3SA_HIT,
	PNE_PM_L3SA_MOD_INV,
	PNE_PM_L3SA_SHR_INV,
	PNE_PM_L3SA_SNOOP_RETRY,
	PNE_PM_L3SB_HIT,
	PNE_PM_L3SB_MOD_INV,
	PNE_PM_L3SB_SHR_INV,
	PNE_PM_L3SB_SNOOP_RETRY,
	PNE_PM_L3SC_HIT,
	PNE_PM_L3SC_MOD_INV,
	PNE_PM_L3SC_SHR_INV,
	PNE_PM_L3SC_SNOOP_RETRY,
	PNE_PM_LD_MISS_L1,
	PNE_PM_LD_MISS_L1_LSU0,
	PNE_PM_LD_MISS_L1_LSU1,
	PNE_PM_LD_REF_L1_LSU0,
	PNE_PM_LD_REF_L1_LSU1,
	PNE_PM_LSU0_LDF,
	PNE_PM_LSU0_NCLD,
	PNE_PM_LSU1_LDF,
	PNE_PM_LSU1_NCLD,
	PNE_PM_LSU_FLUSH,
	PNE_PM_LSU_FLUSH_SRQ_FULL,
	PNE_PM_LSU_LMQ_FULL_CYC,
	PNE_PM_LSU_LMQ_LHR_MERGE,
	PNE_PM_LSU_LMQ_S0_ALLOC,
	PNE_PM_LSU_LMQ_S0_VALID,
	PNE_PM_LSU_LRQ_FULL_CYC,
	PNE_PM_DC_PREF_STREAM_ALLOC_BLK,
	PNE_PM_LSU_SRQ_FULL_CYC,
	PNE_PM_LSU_SRQ_SYNC_CYC,
	PNE_PM_LWSYNC_HELD,
	PNE_PM_MEM_LO_PRIO_PW_CMPL,
	PNE_PM_MEM_LO_PRIO_WR_CMPL,
	PNE_PM_MEM_PW_CMPL,
	PNE_PM_MEM_PW_GATH,
	PNE_PM_MEM_RQ_DISP_BUSY1to7,
	PNE_PM_MEM_SPEC_RD_CANCEL,
	PNE_PM_MEM_WQ_DISP_BUSY8to15,
	PNE_PM_MEM_WQ_DISP_DCLAIM,
	PNE_PM_MRK_DATA_FROM_L275_SHR,
	PNE_PM_MRK_DATA_FROM_L375_SHR,
	PNE_PM_MRK_DSLB_MISS,
	PNE_PM_MRK_DTLB_MISS,
	PNE_PM_MRK_FPU_FIN,
	PNE_PM_MRK_INST_FIN,
	PNE_PM_MRK_L1_RELOAD_VALID,
	PNE_PM_MRK_LSU0_FLUSH_LRQ,
	PNE_PM_MRK_LSU0_FLUSH_SRQ,
	PNE_PM_MRK_LSU0_FLUSH_UST,
	PNE_PM_MRK_LSU0_FLUSH_ULD,
	PNE_PM_MRK_LSU1_FLUSH_LRQ,
	PNE_PM_MRK_LSU1_FLUSH_SRQ,
	PNE_PM_MRK_LSU1_FLUSH_ULD,
	PNE_PM_MRK_LSU1_FLUSH_UST,
	PNE_PM_MRK_LSU_FLUSH_LRQ,
	PNE_PM_MRK_LSU_FLUSH_UST,
	PNE_PM_MRK_LSU_SRQ_INST_VALID,
	PNE_PM_MRK_ST_CMPL_INT,
	PNE_PM_PMC2_OVERFLOW,
	PNE_PM_PMC6_OVERFLOW,
	PNE_PM_PTEG_FROM_L275_SHR,
	PNE_PM_PTEG_FROM_L375_SHR,
	PNE_PM_SNOOP_PARTIAL_RTRY_QFULL,
	PNE_PM_SNOOP_PW_RETRY_WQ_PWQ,
	PNE_PM_SNOOP_RD_RETRY_WQ,
	PNE_PM_SNOOP_WR_RETRY_QFULL,
	PNE_PM_SNOOP_WR_RETRY_WQ,
	PNE_PM_STOP_COMPLETION,
	PNE_PM_ST_MISS_L1,
	PNE_PM_ST_REF_L1,
	PNE_PM_ST_REF_L1_LSU0,
	PNE_PM_ST_REF_L1_LSU1,
	PNE_PM_CLB_EMPTY_CYC,
	PNE_PM_THRD_L2MISS_BOTH_CYC,
	PNE_PM_THRD_PRIO_DIFF_0_CYC,
	PNE_PM_THRD_PRIO_DIFF_1or2_CYC,
	PNE_PM_THRD_PRIO_DIFF_3or4_CYC,
	PNE_PM_THRD_PRIO_DIFF_5or6_CYC,
	PNE_PM_THRD_PRIO_DIFF_minus1or2_CYC,
	PNE_PM_THRD_PRIO_DIFF_minus3or4_CYC,
	PNE_PM_THRD_PRIO_DIFF_minus5or6_CYC,
	PNE_PM_THRD_SEL_OVER_CLB_EMPTY,
	PNE_PM_THRD_SEL_OVER_GCT_IMBAL,
	PNE_PM_THRD_SEL_OVER_ISU_HOLD,
	PNE_PM_THRD_SEL_OVER_L2MISS,
	PNE_PM_THRD_SEL_T0,
	PNE_PM_THRD_SEL_T1,
	PNE_PM_THRD_SMT_HANG,
	PNE_PM_THRESH_TIMEO,
	PNE_PM_TLBIE_HELD,
	PNE_PM_DATA_FROM_L2MISS,
	PNE_PM_MRK_DATA_FROM_L2MISS,
	PNE_PM_PTEG_FROM_L2MISS,
	PNE_PM_0INST_FETCH,
	PNE_PM_BR_PRED_CR_TA,
	PNE_PM_CMPLU_STALL_DIV,
	PNE_PM_CMPLU_STALL_ERAT_MISS,
	PNE_PM_CMPLU_STALL_FPU,
	PNE_PM_CMPLU_STALL_REJECT,
	PNE_PM_EXT_INT,
	PNE_PM_FPU_FEST,
	PNE_PM_FPU_FIN,
	PNE_PM_FXLS_FULL_CYC,
	PNE_PM_FXU1_BUSY_FXU0_IDLE,
	PNE_PM_GCT_NOSLOT_BR_MPRED,
	PNE_PM_INST_FROM_L275_MOD,
	PNE_PM_INST_FROM_L375_MOD,
	PNE_PM_INST_FROM_RMEM,
	PNE_PM_LD_REF_L1,
	PNE_PM_LSU_LDF,
	PNE_PM_LSU_SRQ_EMPTY_CYC,
	PNE_PM_MRK_CRU_FIN,
	PNE_PM_MRK_DATA_FROM_L25_MOD_CYC,
	PNE_PM_MRK_DATA_FROM_L275_MOD_CYC,
	PNE_PM_MRK_DATA_FROM_L35_MOD_CYC,
	PNE_PM_MRK_DATA_FROM_L375_MOD_CYC,
	PNE_PM_MRK_DATA_FROM_LMEM_CYC,
	PNE_PM_MRK_DATA_FROM_RMEM_CYC,
	PNE_PM_MRK_GRP_CMPL,
	PNE_PM_MRK_GRP_IC_MISS,
	PNE_PM_MRK_GRP_TIMEO,
	PNE_PM_MRK_LSU_FIN,
	PNE_PM_MRK_LSU_FLUSH_SRQ,
	PNE_PM_MRK_LSU_FLUSH_ULD,
	PNE_PM_PMC3_OVERFLOW,
	PNE_PM_WORK_HELD,
	NATNAME_GUARD,
};

#endif
