/* 
* File:    p3_core_event_tables.c
* CVS:     $Id: p3_core_event_tables.h,v 1.3 2009/09/10 20:19:43 terpstra Exp $
* Author:  Dan Terpstra; refactored from p3_events.c by Joseph Thomas
*          terpstra@cs.utk.edu
* Mods:    <your name here>
*          <your email address>
*/

/* Note:  Intel MESI bits are programmatically defined for those events
	  that can support them. You can find those events in this file
	  by searching for HAS_MESI. Events containing all possible
	  combinations of these bits can be formed by appending the 
	  proper letters to the end of the event name, e.g. L2_LD_MESI
	  or L2_ST_MI. Some of these bit combinations are used in the 
	  preset tables. In these cases they are explicitly defined.
	  Others can be defined as needed. Otherwise the user can access
	  these native events using _papi_hwd_name_to_code() with the 
	  proper bit characters appended to the event name.
*/ 

/* This Pentium Core enumeration table is used to define the location
   in the native event tables.  Each even has a unique name so as
   to not interfere with location of other events in other native
   tables.  The preset tables use these enumerations to lookup
   native events.
*/


/* Pentium Core enumerations */
enum {
   PNE_CORE_UNHLTCORE_CYCLES = 0x40000000,
   PNE_CORE_UNHLTREF_CYCLES,
   PNE_CORE_INSTR_RETIRED,
   PNE_CORE_LLC_REFERENCE,
   PNE_CORE_LLC_MISSES,
   PNE_CORE_BR_RETIRED,
   PNE_CORE_MISPREDBR_RETIRED,
   PNE_CORE_LD_BLOCKS,
   PNE_CORE_SD_DRAINS,
   PNE_CORE_MISALIGN_MEM_REF,
   PNE_CORE_SEG_REG_LOADS,
   PNE_CORE_SSE_PREFNTA_RET,
   PNE_CORE_SSE_PREFT1_RET,
   PNE_CORE_SSE_PREFT2_RET,
   PNE_CORE_SSE_NTSTORES_RET,
   PNE_CORE_FP_COMPS_OP_EXE,
// 0x40000010
   PNE_CORE_FP_ASSIST,
   PNE_CORE_MUL,
   PNE_CORE_DIV,
   PNE_CORE_CYCLES_DIV_BUSY,
   PNE_CORE_L2_ADS_SELF,
   PNE_CORE_L2_ADS_BOTH_CORES,
   PNE_CORE_DBUS_BUSY_SELF,
   PNE_CORE_DBUS_BUSY_BOTH_CORES,
   PNE_CORE_DBUS_BUSY_RD_SELF,
   PNE_CORE_DBUS_BUSY_RD_BOTH_CORES,
   PNE_CORE_L2_LINES_IN_SELF_ANY,
   PNE_CORE_L2_LINES_IN_SELF_DEMAND,
   PNE_CORE_L2_LINES_IN_SELF_PREFETCH,
   PNE_CORE_L2_LINES_IN_BOTH_CORES_ANY,
   PNE_CORE_L2_LINES_IN_BOTH_CORES_DEMAND,
   PNE_CORE_L2_LINES_IN_BOTH_CORES_PREFETCH,
// 0x40000020
   PNE_CORE_L2_M_LINES_IN_SELF,
   PNE_CORE_L2_M_LINES_IN_BOTH_CORES,
   PNE_CORE_L2_LINES_OUT_SELF_ANY,
   PNE_CORE_L2_LINES_OUT_SELF_DEMAND,
   PNE_CORE_L2_LINES_OUT_SELF_PREFETCH,
   PNE_CORE_L2_LINES_OUT_BOTH_CORES_ANY,
   PNE_CORE_L2_LINES_OUT_BOTH_CORES_DEMAND,
   PNE_CORE_L2_LINES_OUT_BOTH_CORES_PREFETCH,
   PNE_CORE_L2_M_LINES_OUT_SELF_ANY,
   PNE_CORE_L2_M_LINES_OUT_SELF_DEMAND,
   PNE_CORE_L2_M_LINES_OUT_SELF_PREFETCH,
   PNE_CORE_L2_M_LINES_OUT_BOTH_CORES_ANY,
   PNE_CORE_L2_M_LINES_OUT_BOTH_CORES_DEMAND,
   PNE_CORE_L2_M_LINES_OUT_BOTH_CORES_PREFETCH,
   PNE_CORE_L2_IFETCH_SELF,
   PNE_CORE_L2_IFETCH_BOTH_CORES,
// 0x40000030
   PNE_CORE_L2_LD_SELF_ANY,
   PNE_CORE_L2_LD_SELF_DEMAND,
   PNE_CORE_L2_LD_SELF_PREFETCH,
   PNE_CORE_L2_LD_BOTH_CORES_ANY,
   PNE_CORE_L2_LD_BOTH_CORES_DEMAND,
   PNE_CORE_L2_LD_BOTH_CORES_PREFETCH,
   PNE_CORE_L2_ST_SELF,
   PNE_CORE_L2_ST_BOTH_CORES,
   PNE_CORE_L2_RQSTS_SELF_ANY,
   PNE_CORE_L2_RQSTS_SELF_DEMAND,
   PNE_CORE_L2_RQSTS_SELF_PREFETCH,
   PNE_CORE_L2_RQSTS_BOTH_CORES_ANY,
   PNE_CORE_L2_RQSTS_BOTH_CORES_DEMAND,
   PNE_CORE_L2_RQSTS_BOTH_CORES_PREFETCH,
   PNE_CORE_L2_REJECT_CYCLES_SELF_ANY,
   PNE_CORE_L2_REJECT_CYCLES_SELF_DEMAND,
// 0x40000040
   PNE_CORE_L2_REJECT_CYCLES_SELF_PREFETCH,
   PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_ANY,
   PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_DEMAND,
   PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_PREFETCH,
   PNE_CORE_L2_NO_REQUEST_CYCLES_SELF,
   PNE_CORE_L2_NO_REQUEST_CYCLES_BOTH_CORES,
   PNE_CORE_EST_TRANS_ALL,
   PNE_CORE_EST_TRANS_ALL_FREQUENCY,
   PNE_CORE_THERMAL_TRIP,
   PNE_CORE_NONHLT_REF_CYCLES,
   PNE_CORE_SERIAL_EXECUTION_CYCLES,
   PNE_CORE_DCACHE_CACHE_LD,
   PNE_CORE_DCACHE_CACHE_ST,
   PNE_CORE_DCACHE_CACHE_LOCK,
   PNE_CORE_DATA_MEM_REF,
   PNE_CORE_DATA_MEM_CACHE_REF,
// 0x40000050
   PNE_CORE_DCACHE_REPL,
   PNE_CORE_DCACHE_M_REPL,
   PNE_CORE_DCACHE_M_EVICT,
   PNE_CORE_DCACHE_PEND_MISS,
   PNE_CORE_DTLB_MISS,
   PNE_CORE_SSE_PREFNTA_MISS,
   PNE_CORE_SSE_PREFT1_MISS,
   PNE_CORE_SSE_PREFT2_MISS,
   PNE_CORE_SSE_NTSTORES_MISS,
   PNE_CORE_L1_PREF_REQ,
   PNE_CORE_BUS_REQ_OUTSTANDING_SELF,
   PNE_CORE_BUS_REQ_OUTSTANDING_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_REQ_OUTSTANDING_ALL_AGENTS,
   PNE_CORE_BUS_BNR_CLOCKS,
   PNE_CORE_BUS_DRDY_CLOCKS_THIS_AGENT,
   PNE_CORE_BUS_DRDY_CLOCKS_ALL_AGENTS,
// 0x40000060
   PNE_CORE_BUS_LOCKS_CLOCKS_SELF,
   PNE_CORE_BUS_LOCKS_CLOCKS_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_LOCKS_CLOCKS_ALL_AGENTS,
   PNE_CORE_BUS_DATA_RCV,
   PNE_CORE_BUS_TRANS_BRD_SELF,
   PNE_CORE_BUS_TRANS_BRD_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_BRD_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_RFO_SELF,
   PNE_CORE_BUS_TRANS_RFO_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_RFO_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_IFETCH_SELF,
   PNE_CORE_BUS_TRANS_IFETCH_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_IFETCH_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_INVAL_SELF,
   PNE_CORE_BUS_TRANS_INVAL_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_INVAL_ALL_AGENTS,
// 0x40000070
   PNE_CORE_BUS_TRANS_PWR_SELF,
   PNE_CORE_BUS_TRANS_PWR_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_PWR_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_P_SELF,
   PNE_CORE_BUS_TRANS_P_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_P_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_IO_SELF,
   PNE_CORE_BUS_TRANS_IO_BOTH_CORES_THIS_AGENT,
   PNE_CORE_BUS_TRANS_IO_ALL_AGENTS,
   PNE_CORE_BUS_TRANS_DEF,
   PNE_CORE_BUS_TRANS_WB,
   PNE_CORE_BUS_TRANS_BURST,
   PNE_CORE_BUS_TRANS_MEM,
   PNE_CORE_BUS_TRANS_ANY,
   PNE_CORE_BUS_SNOOPS,
   PNE_CORE_DCU_SNOOP_TO_SHARE,
// 0x40000080
   PNE_CORE_BUS_NOT_IN_USE,
   PNE_CORE_BUS_SNOOP_STALL,
   PNE_CORE_ICACHE_READS,
   PNE_CORE_ICACHE_MISSES,
   PNE_CORE_ITLB_MISSES,
   PNE_CORE_IFU_MEM_STALL,
   PNE_CORE_ILD_STALL,
   PNE_CORE_BR_INST_EXEC,
   PNE_CORE_BR_MISSP_EXEC,
   PNE_CORE_BR_BAC_MISSP_EXEC,
   PNE_CORE_BR_CND_EXEC,
   PNE_CORE_BR_CND_MISSP_EXEC,
   PNE_CORE_BR_IND_EXEC,
   PNE_CORE_BR_IND_MISSP_EXEC,
   PNE_CORE_BR_RET_EXEC,
   PNE_CORE_BR_RET_MISSP_EXEC,
// 0x40000090
   PNE_CORE_BR_RET_BAC_MISSP_EXEC,
   PNE_CORE_BR_CALL_EXEC,
   PNE_CORE_BR_CALL_MISSP_EXEC,
   PNE_CORE_BR_IND_CALL_EXEC,
   PNE_CORE_RESOURCE_STALL,
   PNE_CORE_MMX_INSTR_EXEC,
   PNE_CORE_SIMD_INT_SAT_EXEC,
   PNE_CORE_SIMD_INT_PMUL_EXEC,
   PNE_CORE_SIMD_INT_PSFT_EXEC,
   PNE_CORE_SIMD_INT_PCK_EXEC,
   PNE_CORE_SIMD_INT_UPCK_EXEC,
   PNE_CORE_SIMD_INT_PLOG_EXEC,
   PNE_CORE_SIMD_INT_PARI_EXEC,
   PNE_CORE_INSTR_RET,
   PNE_CORE_FP_COMP_INSTR_RET,
   PNE_CORE_UOPS_RET,
// 0x400000A0
   PNE_CORE_SMC_DETECTED,
   PNE_CORE_BR_INSTR_RET,
   PNE_CORE_BR_MISPRED_RET,
   PNE_CORE_CYCLES_INT_MASKED,
   PNE_CORE_CYCLES_INT_PEDNING_MASKED,
   PNE_CORE_HW_INT_RX,
   PNE_CORE_BR_TAKEN_RET,
   PNE_CORE_BR_MISPRED_TAKEN_RET,
   PNE_CORE_MMX_FP_TRANS,
   PNE_CORE_FP_MMX_TRANS,
   PNE_CORE_MMX_ASSIST,
   PNE_CORE_MMX_INSTR_RET,
   PNE_CORE_INSTR_DECODED,
   PNE_CORE_ESP_UOPS,
   PNE_CORE_SIMD_FP_SP_RET,
   PNE_CORE_SIMD_FP_SP_S_RET,
// 0x400000B0
   PNE_CORE_SIMD_FP_DP_P_RET,
   PNE_CORE_SIMD_FP_DP_S_RET,
   PNE_CORE_SIMD_INT_128_RET,
   PNE_CORE_SIMD_FP_SP_P_COMP_RET,
   PNE_CORE_SIMD_FP_SP_S_COMP_RET,
   PNE_CORE_SIMD_FP_DP_P_COMP_RET,
   PNE_CORE_SIMD_FP_DP_S_COMP_RET,
   PNE_CORE_FUSED_UOPS_RET,
   PNE_CORE_FUSED_LD_UOPS_RET,
   PNE_CORE_FUSED_ST_UOPS_RET,
   PNE_CORE_UNFUSION,
   PNE_CORE_BR_INSTR_DECODED,
   PNE_CORE_BTB_MISSES,
   PNE_CORE_BR_BOGUS,
   PNE_CORE_BACLEARS,
   PNE_CORE_PREF_RQSTS_UP,
// 0x400000C0
   PNE_CORE_PREF_RQSTS_DN,
   PNE_CORE_LAST_NATIVE_EVENT /* used to initialize sub_info.num_native_events */
};

/* These are special Pentium Core events with MESI bits set as used in the preset table */
#define PNE_CORE_L2_IFETCH_SELF_MESI (PNE_CORE_L2_IFETCH_SELF | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_IFETCH_BOTH_CORES_MESI (PNE_CORE_L2_IFETCH_BOTH_CORES | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_SELF_ANY_MESI     (PNE_CORE_L2_LD_SELF_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_SELF_DEMAND_MESI     (PNE_CORE_L2_LD_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_SELF_PREFETCH_MESI     (PNE_CORE_L2_LD_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_BOTH_CORES_ANY_MESI     (PNE_CORE_L2_LD_BOTH_CORES_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_BOTH_CORES_DEMAND_MESI     (PNE_CORE_L2_LD_BOTH_CORES_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_LD_BOTH_CORES_PREFETCH_MESI     (PNE_CORE_L2_LD_BOTH_CORES_PREFETCH | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_ST_SELF_MESI     (PNE_CORE_L2_ST_SELF | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_ST_BOTH_CORES_MESI     (PNE_CORE_L2_ST_BOTH_CORES | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_SELF_ANY_MESI     (PNE_CORE_L2_RQSTS_SELF_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_SELF_ANY_S     (PNE_CORE_L2_RQSTS_SELF_ANY | MOESI_S)
#define PNE_CORE_L2_RQSTS_SELF_DEMAND_MESI     (PNE_CORE_L2_RQSTS_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_SELF_PREFETCH_MESI     (PNE_CORE_L2_RQSTS_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_BOTH_CORES_ANY_MESI     (PNE_CORE_L2_RQSTS_BOTH_CORES_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_BOTH_CORES_DEMAND_MESI     (PNE_CORE_L2_RQSTS_BOTH_CORES_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_RQSTS_BOTH_CORES_PREFETCH_MESI     (PNE_CORE_L2_RQSTS_BOTH_CORES_PREFETCH | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_SELF_ANY_MESI     (PNE_CORE_L2_REJECT_CYCLES_SELF_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_SELF_DEMAND_MESI     (PNE_CORE_L2_REJECT_CYCLES_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_SELF_PREFETCH_MESI     (PNE_CORE_L2_REJECT_CYCLES_SELF_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_ANY_MESI     (PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_ANY | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_DEMAND_MESI     (PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_DEMAND | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)
#define PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_PREFETCH_MESI     (PNE_CORE_L2_REJECT_CYCLES_BOTH_CORES_PREFETCH | MOESI_M_INTEL | MOESI_E | MOESI_S | MOESI_I)


/* PAPI preset events are defined in the table below.
   Each entry consists of a PAPI name, derived info, and up to four
   native event indeces as defined above.
*/

const hwi_search_t _papi_hwd_core_preset_map[] = {
   {PAPI_L1_DCM,{0,{PNE_CORE_DCACHE_REPL,PAPI_NULL,PAPI_NULL,PAPI_NULL},{0,}}},
   {PAPI_L1_ICM,{0,{PNE_CORE_L2_IFETCH_SELF_MESI,PAPI_NULL,PAPI_NULL,PAPI_NULL},{0,}}},
   {PAPI_L2_DCM,{DERIVED_SUB,{PNE_CORE_L2_LINES_IN_SELF_ANY,PNE_CORE_BUS_TRANS_IFETCH_SELF,PAPI_NULL,PAPI_NULL},{0,}}},
   {PAPI_L2_ICM, {0, {PNE_CORE_BUS_TRANS_IFETCH_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_TCM, {0, {PNE_CORE_L2_RQSTS_SELF_DEMAND_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCM, {0, {PNE_CORE_L2_LINES_IN_SELF_ANY, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_CA_SHR, {0, {PNE_CORE_L2_RQSTS_SELF_ANY_S, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_CA_CLN, {0, {PNE_CORE_BUS_TRANS_RFO_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
/*   { PAPI_CA_INV, {0, {PNE_PM_BUS_HITM_DRV,PAPI_NULL,PAPI_NULL,PAPI_NULL},{0,}}},   */
   {PAPI_CA_ITV, {0, {PNE_CORE_BUS_TRANS_INVAL_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TLB_IM, {0, {PNE_CORE_ITLB_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TLB_DM, {0, {PNE_CORE_DTLB_MISS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_LDM, {0, {PNE_CORE_L2_LD_SELF_ANY_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_STM, {0, {PNE_CORE_L2_ST_SELF_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_LDM, {DERIVED_SUB, {PNE_CORE_L2_LINES_IN_SELF_ANY, PNE_CORE_L2_M_LINES_IN_SELF, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_STM, {0, {PNE_CORE_L2_M_LINES_IN_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BTAC_M, {0, {PNE_CORE_BTB_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_HW_INT, {0, {PNE_CORE_HW_INT_RX, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_CN, {0, {PNE_CORE_BR_RETIRED, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_TKN, {0, {PNE_CORE_BR_TAKEN_RET, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_NTK,
    {DERIVED_SUB, {PNE_CORE_BR_RETIRED, PNE_CORE_BR_TAKEN_RET, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_MSP, {0, {PNE_CORE_MISPREDBR_RETIRED, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_PRC,
    {DERIVED_SUB, {PNE_CORE_BR_RETIRED, PNE_CORE_MISPREDBR_RETIRED, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TOT_IIS, {0, {PNE_CORE_INSTR_DECODED, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TOT_INS, {0, {PNE_CORE_INSTR_RETIRED, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_FP_INS, {0, {PNE_CORE_FP_COMPS_OP_EXE, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_FP_OPS, {0, {PNE_CORE_FP_COMPS_OP_EXE, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_INS, {0, {PNE_CORE_BR_RETIRED, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
/*   { PAPI_VEC_INS,       {0, {PNE_PM_MMX_INSTR_EXEC,PAPI_NULL,PAPI_NULL,PAPI_NULL},{0,}}},  Says event exists on P2 only  */
   {PAPI_RES_STL, {0, {PNE_CORE_RESOURCE_STALL, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TOT_CYC, {0, {PNE_CORE_UNHLTCORE_CYCLES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_DCH, {DERIVED_SUB, {PNE_CORE_DATA_MEM_REF, PNE_CORE_DCACHE_REPL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_DCA, {0, {PNE_CORE_DATA_MEM_REF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCA, {DERIVED_ADD, {PNE_CORE_L2_LD_SELF_ANY_MESI, PNE_CORE_L2_ST_SELF_MESI, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCR, {0, {PNE_CORE_L2_LD_SELF_ANY_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCW, {0, {PNE_CORE_L2_ST_SELF_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICH, {DERIVED_SUB, {PNE_CORE_BUS_TRANS_IFETCH_SELF, PNE_CORE_L2_IFETCH_SELF_MESI, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_ICH,
    {DERIVED_SUB, {PNE_CORE_L2_IFETCH_SELF_MESI, PNE_CORE_BUS_TRANS_IFETCH_SELF, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICA, {0, {PNE_CORE_BUS_TRANS_IFETCH_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_ICA, {0, {PNE_CORE_L2_IFETCH_SELF_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICR, {0, {PNE_CORE_BUS_TRANS_IFETCH_SELF, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_ICR, {0, {PNE_CORE_L2_IFETCH_SELF_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCH, {DERIVED_SUB, {PNE_CORE_L2_RQSTS_SELF_ANY_MESI, PNE_CORE_L2_LINES_IN_SELF_ANY, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_TCA, {DERIVED_ADD, {PNE_CORE_DATA_MEM_REF, PNE_CORE_BUS_TRANS_IFETCH_SELF, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCA, {0, {PNE_CORE_L2_RQSTS_SELF_ANY_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCR, {DERIVED_ADD, {PNE_CORE_L2_LD_SELF_ANY_MESI, PNE_CORE_L2_IFETCH_SELF_MESI, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCW, {0, {PNE_CORE_L2_ST_SELF_MESI, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_FML_INS, {0, {PNE_CORE_MUL, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_FDV_INS, {0, {PNE_CORE_DIV, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {0, {0, {PAPI_NULL, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}}
};


/* The following is the Intel Pentium Core native table.
   It contains the following:
   A short text description of the native event,
   A longer more descriptive text of the native event,
   Information on which counter the native can live,
   and the Native Event Code.
   In some cases, event codes can be programmatically expanded to
   include MESI bits. These are identified by the addition
   of a HAS_MESI flag in the selector field.
*/

/* The notes/descriptions of these events have sometimes been truncated */
/* Please see the architecture's manual for any clarifications.         */

const int _papi_hwd_core_native_count = (PNE_CORE_LAST_NATIVE_EVENT & PAPI_NATIVE_AND_MASK);
const native_event_entry_t _papi_hwd_core_native_map[] = {
// 0x40000000
   {"UnhltCore_Cycles",
    "Unhalted core cycles",
    {CNTRS12, 0x3c}},
   {"UnhltRef_Cycles",
    "Unhalted reference cycles, Measures bus cycle",
    {CNTRS12, 0x13c}},
   {"Instr_Retired",
    "Instruction retired",
    {CNTRS12, 0xc0}},
   {"LLC_Reference",
    "LL cache references",
    {CNTRS12, 0x4f2e}},
   {"LLC_Misses",
    "LL cache misses",
    {CNTRS12, 0x412e}},
   {"Br_Retired",
    "Branch instruction retired",
    {CNTRS12, 0xc4}},
   {"MispredBr_Retired",
    "Mispredicted Branch Instruction retired",
    {CNTRS12, 0xc5}},
   {"LD_Blocks",
    "Load operations delayed due to store buffer blocks.",
    {CNTRS12, 0x3}},
   {"SD_Drains",
    "Cycles while draining store buffers",
    {CNTRS12, 0x4}},
   {"Misalign_Mem_Ref",
    "Misaligned data memory references",
    {CNTRS12, 0x5}},
   {"Seg_Reg_Loads",
    "Segment register loads",
    {CNTRS12, 0x6}},
   {"SSE_PrefNta_Ret",
    "SSE software prefetch instruction PREFETCHNTA retired",
    {CNTRS12, 0x7}},
   {"SSE_PrefT1_Ret",
    "SSE software prefetch instruction PREFETCHT1 retired",
    {CNTRS12, 0x107}},
   {"SSE_PrefT2_Ret",
    "SSE software prefetch instruction PREFETCHT2 retired",
    {CNTRS12, 0x207}},
   {"SSE_NTStores_Ret",
    "SSE streaming store instruction retired",
    {CNTRS12, 0x307}},
   {"FP_Comps_Op_Exe",
    "FP computational Instruction executed",
    {CNTRS12, 0x10}},
// 0x40000010
   {"FP_Assist",
    "FP exceptions experienced microcode assists",
    {CNTRS12, 0x11}},
   {"Mul",
    "Multiply operations",
    {CNTRS12, 0x12}},
   {"Div",
    "Divide operations",
    {CNTRS12, 0x13}},
   {"Cycles_Div_Busy",
    "Cycles the divider is busy",
    {CNTRS12, 0x14}},
   {"L2_ADS_Self",
    "L2 Address strobes Require corespecificity",
    {CNTRS12, 0x4021}},
   {"L2_ADS_Both_Cores",
    "L2 Address strobes Require corespecificity",
    {CNTRS12, 0xc021}},
   {"Dbus_Busy_Self",
    "Core cycle during which data bus was busy",
    {CNTRS12, 0x4022}},
   {"Dbus_Busy_Both_Cores",
    "Core cycle during which data bus was busy",
    {CNTRS12, 0xc022}},
   {"Dbus_Busy_Rd_Self",
    "Cycles data bus is busy transferring data to a core",
    {CNTRS12, 0x4023}},
   {"Dbus_Busy_Rd_Both_Cores",
    "Cycles data bus is busy transferring data to a core",
    {CNTRS12, 0xc023}},
   {"L2_Lines_In_Self_Any",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x7024}},
   {"L2_Lines_In_Self_Demand",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x4024}},
   {"L2_Lines_In_Self_Prefetch",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x5024}},
   {"L2_Lines_In_Both_Cores_Any",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xf024}},
   {"L2_Lines_In_Both_Cores_Demand",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xc024}},
   {"L2_Lines_In_Both_Cores_Prefetch",
    "L2 cache lines allocated Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xd024}},
// 0x40000020
   {"L2_M_Lines_In_Self",
    "L2 Modified-state cache lines allocated Require corespecificity",
    {CNTRS12, 0x4025}},
   {"L2_M_Lines_In_Both_Cores",
    "L2 Modified-state cache lines allocated Require corespecificity",
    {CNTRS12, 0xc025}},
   {"L2_Lines_Out_Self_Any",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x7026}},
   {"L2_Lines_Out_Self_Demand",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x4026}},
   {"L2_Lines_Out_Self_Prefetch",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0x5026}},
   {"L2_Lines_Out_Both_Cores_Any",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xf026}},
   {"L2_Lines_Out_Both_Cores_Demand",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xc026}},
   {"L2_Lines_Out_Both_Cores_Prefetch",
    "L2 cache lines evicted Require corespecificity and HW prefetch qualification.",
    {CNTRS12, 0xd026}},
   {"L2_M_Lines_Out_Self_Any",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0x7027}},
   {"L2_M_Lines_Out_Self_Demand",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0x4027}},
   {"L2_M_Lines_Out_Self_Prefetch",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0x5027}},
   {"L2_M_Lines_Out_Both_Cores_Any",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0xf027}},
   {"L2_M_Lines_Out_Both_Cores_Demand",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0xc027}},
   {"L2_M_Lines_Out_Both_Cores_Prefetch",
    "L2 Modified-state cache lines evicted",
    {CNTRS12, 0xd027}},
   {"L2_IFetch_Self",
    "Support MESI qualification L2 instruction fetches from instruction fetch unit",
    {CNTRS12 | HAS_MESI, 0x4028}},
   {"L2_IFetch_Both_Cores",
    "Support MESI qualification L2 instruction fetches from instruction fetch unit",
    {CNTRS12 | HAS_MESI, 0xc028}},
// 0x40000030
   {"L2_LD_Self_Any",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0x7029}},
   {"L2_LD_Self_Demand",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0x4029}},
   {"L2_LD_Self_Prefetch",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0x5029}},
   {"L2_LD_Both_Cores_Any",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0xf029}},
   {"L2_LD_Both_Cores_Demand",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0xc029}},
   {"L2_LD_Both_Cores_Prefetch",
    "Support MESI qualification L2 cache reads Require corespecificity.",
    {CNTRS12 | HAS_MESI, 0xd029}},
   {"L2_ST_Self",
    "Support MESI qualification L2 cache writes",
    {CNTRS12 | HAS_MESI, 0x402a}},
   {"L2_ST_Both_Cores",
    "Support MESI qualification L2 cache writes",
    {CNTRS12 | HAS_MESI, 0xc02a}},
   {"L2_Rqsts_Self_Any",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0x702e}},
   {"L2_Rqsts_Self_Demand",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0x402e}},
   {"L2_Rqsts_Self_Prefetch",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0x502e}},
   {"L2_Rqsts_Both_Cores_Any",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0xf02e}},
   {"L2_Rqsts_Both_Cores_Demand",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0xc02e}},
   {"L2_Rqsts_Both_Cores_Prefetch",
    "Support MESI qualification L2 cache reference requests.",
    {CNTRS12 | HAS_MESI, 0xd02e}},
   {"L2_Reject_Cycles_Self_Any",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0x7030}},
   {"L2_Reject_Cycles_Self_Demand",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0x4030}},
// 0x40000040
   {"L2_Reject_Cycles_Self_Prefetch",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0x5030}},
   {"L2_Reject_Cycles_Both_Cores_Any",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0xf030}},
   {"L2_Reject_Cycles_Both_Cores_Demand",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0xc030}},
   {"L2_Reject_Cycles_Both_Cores_Prefetch",
    "Support MESI qualification Cycles L2 is busy and rejecting new requests",
    {CNTRS12 | HAS_MESI, 0xd030}},
   {"L2_No_Request_Cycles_Self",
    "Support MESI qualification Cycles there is no request to access L2",
    {CNTRS12 | HAS_MESI, 0x4032}},
   {"L2_No_Request_Cycles_Both_Cores",
    "Support MESI qualification Cycles there is no request to access L2",
    {CNTRS12 | HAS_MESI, 0xc032}},
   {"EST_Trans_All",
    "Any Intel Enhanced SpeedStep(R) Technology transitions",
    {CNTRS12, 0x3a}},
   {"EST_Trans_Frequency",
    "Any Intel Enhanced SpeedStep(R) Technology frequency transitions",
    {CNTRS12, 0x103a}},
   {"Thermal_Trip",
    "Duration in a thermal trip based on the current core clock Use Edge trigger to count occurrence",
    {CNTRS12, 0xc03b}},
   {"NonHlt_Ref_Cycles",
    "Non-halted bus cycles",
    {CNTRS12, 0x13c}},
   {"Serial_Execution_Cycles",
    "Non-halted bus cycles of this core executing code while the other core is halted",
    {CNTRS12, 0x23c}},
   {"DCache_Cache_LD",
    "Require MESI qualification L1 cacheable data read operations",
    {CNTRS12 | HAS_MESI, 0x40}},
   {"DCache_Cache_ST",
    "Require MESI qualification L1 cacheable data write operations",
    {CNTRS12 | HAS_MESI, 0x41}},
   {"DCache_Cache_Lock",
    "Require MESI qualification L1 cacheable lock read operations to invalid state",
    {CNTRS12 | HAS_MESI, 0x42}},
   {"Data_Mem_Ref",
    "L1 data read and writes of cacheable and non-cacheable types",
    {CNTRS12, 0x143}},
   {"Data_Mem_Cache_Ref",
    "L1 data cacheable read and write operations",
    {CNTRS12, 0x244}},
// 0x40000050
   {"DCache_Repl",
    "L1 data cache line replacements",
    {CNTRS12, 0xf45}},
   {"DCache_M_Repl",
    "L1 data M-state cache line allocated",
    {CNTRS12, 0x46}},
   {"DCache_M_Evict",
    "L1 data M-state cache line evicted",
    {CNTRS12, 0x47}},
   {"DCache_Pend_Miss",
    "Weighted cycles of L1 miss oustanding Use Cmask =1 to count duration",
    {CNTRS12, 0x48}},
   {"Dtlb_Miss",
    "Data references that missed TLB",
    {CNTRS12, 0x49}},
   {"SSE_PrefNta_Miss",
    "PREFETCHNTA missed all caches",
    {CNTRS12, 0x4b}},
   {"SSE_PrefT1_Miss",
    "PREFETCHT1 missed all caches",
    {CNTRS12, 0x14b}},
   {"SSE_PrefT2_Miss",
    "PREFETCHT2 missed all caches",
    {CNTRS12, 0x24b}},
   {"SSE_NTStores_Miss",
    "SSE streaming store instruction missed all caches",
    {CNTRS12, 0x34b}},
   {"L1_Pref_Req",
    "L1 prefetch requests due to DCU cache misses May overcount if a request was resubmitted",
    {CNTRS12, 0x4f}},
   {"Bus_Req_Outstanding_Self",
    "Require corespecificity, and agent specificity Weighted cycles of cacheable bus data read requests.",
    {CNTRS12, 0x4060}},
   {"Bus_Req_Outstanding_Both_Cores_This_Agent",
    "Require corespecificity, and agent specificity Weighted cycles of cacheable bus data read requests.",
    {CNTRS12, 0xc060}},
   {"Bus_Req_Outstanding_All_Agents",
    "Require corespecificity, and agent specificity Weighted cycles of cacheable bus data read requests.",
    {CNTRS12, 0xe060}},
   {"Bus_BNR_Clocks",
    "External bus cycles while BNR asserted",
    {CNTRS12, 0x61}},
   {"Bus_DRDY_Clocks_This_Agent",
    "External bus cycles while DRDY asserted Require agent specificity",
    {CNTRS12, 0x62}},
   {"Bus_DRDY_Clocks_All_Agents",
    "External bus cycles while DRDY asserted Require agent specificity",
    {CNTRS12, 0x2062}},
// 0x40000060
   {"Bus_Locks_Clocks_Self",
    "External bus cycles while bus lock signal asserted Require core specificity",
    {CNTRS12, 0x4063}},
   {"Bus_Locks_Clocks_Both_Cores_This_Agent",
    "External bus cycles while bus lock signal asserted Require core specificity",
    {CNTRS12, 0xc063}},
   {"Bus_Locks_Clocks_All_Agents",
    "External bus cycles while bus lock signal asserted Require core specificity",
    {CNTRS12, 0xe063}},
   {"Bus_Data_Rcv",
    "External bus cycles while bus lock signal asserted",
    {CNTRS12, 0x4064}},
   {"Bus_Trans_Brd_Self",
    "Burst read bus transactions (data or code) Require core specificity",
    {CNTRS12, 0x4065}},
   {"Bus_Trans_Brd_Both_Cores_This_Agent",
    "Burst read bus transactions (data or code) Require core specificity",
    {CNTRS12, 0xc065}},
   {"Bus_Trans_Brd_All_Agents",
    "Burst read bus transactions (data or code) Require core specificity",
    {CNTRS12, 0xe065}},
   {"Bus_Trans_RFO_Self",
    "Completed read for ownership (RFO) transactions.",
    {CNTRS12, 0x4066}},
   {"Bus_Trans_RFO_Both_Cores_This_Agent",
    "Completed read for ownership (RFO) transactions.",
    {CNTRS12, 0xc066}},
   {"Bus_Trans_RFO_All_Agents",
    "Completed read for ownership (RFO) transactions.",
    {CNTRS12, 0xe066}},
   {"Bus_Trans_Ifetch_Self",
    "Completed instruction fetch transactions.",
    {CNTRS12, 0x4068}},
   {"Bus_Trans_Ifetch_Both_Cores_This_Agent",
    "Completed instruction fetch transactions.",
    {CNTRS12, 0xc068}},
   {"Bus_Trans_Ifetch_All_Agents",
    "Completed instruction fetch transactions.",
    {CNTRS12, 0xe068}},
   {"Bus_Trans_Inval_Self",
    "Completed invalidate transactions.",
    {CNTRS12, 0x4069}},
   {"Bus_Trans_Inval_Both_Cores_This_Agent",
    "Completed invalidate transactions.",
    {CNTRS12, 0xc069}},
   {"Bus_Trans_Inval_All_Agents",
    "Completed invalidate transactions.",
    {CNTRS12, 0xe069}},
// 0x40000070
   {"Bus_Trans_Pwr_Self",
    "Completed partial write transactions.",
    {CNTRS12, 0x406a}},
   {"Bus_Trans_Pwr_Both_Cores_This_Agent",
    "Completed partial write transactions.",
    {CNTRS12, 0xc06a}},
   {"Bus_Trans_Pwr_All_Agents",
    "Completed partial write transactions.",
    {CNTRS12, 0xe06a}},
   {"Bus_Trans_P_Self",
    "Completed partial transactions (include partial read + partial write + line write).",
    {CNTRS12, 0x406b}},
   {"Bus_Trans_P_Both_Cores_This_Agent",
    "Completed partial transactions (include partial read + partial write + line write).",
    {CNTRS12, 0xc06b}},
   {"Bus_Trans_P_All_Agents",
    "Completed partial transactions (include partial read + partial write + line write).",
    {CNTRS12, 0xe06b}},
   {"Bus_Trans_IO_Self",
    "Completed I/O transactions (read and write).",
    {CNTRS12, 0x406c}},
   {"Bus_Trans_IO_Both_Cores_This_Agent",
    "Completed I/O transactions (read and write).",
    {CNTRS12, 0xc06c}},
   {"Bus_Trans_IO_All_Agents",
    "Completed I/O transactions (read and write).",
    {CNTRS12, 0xe06c}},
   {"Bus_Trans_Def",
    "Completed defer transactions.",
    {CNTRS12, 0x206d}},
   {"Bus_Trans_WB",
    "Completed writeback transactions from DCU (does not include L2 writebacks).",
    {CNTRS12, 0xc067}},
   {"Bus_Trans_Burst",
    "Completed burst transactions ( full line transactions include reads, write, RFO, and writebacks).",
    {CNTRS12, 0xc06e}},
   {"Bus_Trans_Mem",
    "Completed memory transactions. This includes Bus_Trans_Burst + Bus_Trans_P + Bus_Trans_Inval.",
    {CNTRS12, 0xc06f}},
   {"Bus_Trans_Any",
    "Any completed bus transactions.",
    {CNTRS12, 0xc070}},
   {"Bus_Snoops",
    "External bus cycles while bus lock signal asserted",
    {CNTRS12, 0x77}},
   {"DCU_Snoop_To_Share",
    "DCU snoops to share-state L1 cache line due to L1 misses",
    {CNTRS12, 0x178}},
// 0x40000080
   {"Bus_Not_In_Use",
    "Number of cycles there is no transaction from the core",
    {CNTRS12, 0x7d}},
   {"Bus_Snoop_Stall",
    "Number of bus cycles while bus snoop is stalled",
    {CNTRS12, 0x7e}},
   {"ICache_Reads",
    "Number of instruction fetches from ICache, streaming buffers (both cacheable and uncacheable fetches)",
    {CNTRS12, 0x80}},
   {"ICache_Misses",
    "Number of instruction fetch misses from ICache, streaming buffers.",
    {CNTRS12, 0x81}},
   {"ITLB_Misses",
    "Number of iITLB misses",
    {CNTRS12, 0x85}},
   {"IFU_Mem_Stall",
    "Cycles IFU is stalled while waiting for data from memory",
    {CNTRS12, 0x86}},
   {"ILD_Stall",
    "Number of instruction length decoder stalls (Counts number of LCP stalls)",
    {CNTRS12, 0x87}},
   {"Br_Inst_Exec",
    "Branch instruction executed (includes speculation)",
    {CNTRS12, 0x88}},
   {"Br_Missp_Exec",
    "Branch instruction executed and were mispredicted at execution (includes branches that do not have prediction or mispredicted)",
    {CNTRS12, 0x89}},
   {"Br_BAC_Missp_Exec",
    "Branch instruction executed that were mispredicted at front end..",
    {CNTRS12, 0x8a}},
   {"Br_Cnd_Exec",
    "Conditional branch instruction executed.",
    {CNTRS12, 0x8b}},
   {"Br_Cnd_Missp_Exec",
    "Conditional branch instruction executed that were mispredicted",
    {CNTRS12, 0x8c}},
   {"Br_Ind_Exec",
    "Indirect branch instruction executed.",
    {CNTRS12, 0x8d}},
   {"Br_Ind_Missp_Exec",
    "Indirect branch instruction executed that were mispredicted",
    {CNTRS12, 0x8e}},
   {"Br_Ret_Exec",
    "Return branch instruction executed.",
    {CNTRS12, 0x8f}},
   {"Br_Ret_Missp_Exec",
    "Return branch instruction executed that were mispredicted.",
    {CNTRS12, 0x90}},
// 0x40000090
   {"Br_Ret_BAC_Missp_Exec",
    "Return branch instruction executed that were mispredicted at the Front End.",
    {CNTRS12, 0x91}},
   {"Br_Call_Exec",
    "Return call instruction executed.",
    {CNTRS12, 0x92}},
   {"Br_Call_Missp_Exec",
    "Return call instruction executed that were mispredicted.",
    {CNTRS12, 0x93}},
   {"Br_Ind_Call_Exec",
    "Indirect call branch instruction executed.",
    {CNTRS12, 0x94}},
   // from the Intel System Programming Guide 3B March 06 (253669-019)
   //{"Resource_Stall",
   // "Cycles while there is a resource related stall (renaming, buffer entries) as seen by allocator.",
   // {CNTRS12, 0xa2}},
   // from the Intel System Programming Guide 3B Nov 06 (253669-022US)
   {"Resource_Stalls_Any",
    "This event counts the number of cycles while resource-related stalls occurs for any conditions.",
    {CNTRS12, 0x1fdc}},
   {"MMX_Instr_Exec",
    "Number of MMX instruction executed (does not include MOVQ and MOVD stores).",
    {CNTRS12, 0xb0}},
   {"SIMD_Int_Sat_Exec",
    "Number of SIMD Integer saturating instruction executed.",
    {CNTRS12, 0xb1}},
   {"SIMD_Int_Pmul_Exec",
    "Number of SIMD Integer packed multiply instruction executed.",
    {CNTRS12, 0x1b3}},
   {"SIMD_Int_Psft_Exec",
    "Number of SIMD Integer packed shift instruction executed.",
    {CNTRS12, 0x2b3}},
   {"SIMD_Int_Pck_Exec",
    "Number of SIMD Integer pack operation instruction executed.",
    {CNTRS12, 0x4b3}},
   {"SIMD_Int_Upck_Exec",
    "Number of SIMD Integer unpack instruction executed.",
    {CNTRS12, 0x8b3}},
   {"SIMD_Int_Plog_Exec",
    "Number of SIMD Integer packed logical instruction executed.",
    {CNTRS12, 0x10b3}},
   {"SIMD_Int_Pari_Exec",
    "Number of SIMD Integer packed arithmetic instruction executed.",
    {CNTRS12, 0x20b3}},
   {"Instr_Ret",
    "Number of instruction retired (Macro fused instruction count as 2).",
    {CNTRS12, 0xc0}},
   {"FP_Comp_Instr_Ret",
    "Number of FP compute instruction retired (X87 instruction or instruction that contain X87 operations).",
    {CNTRS12, 0xc1}},
   {"Uops_Ret",
    "Number of micro-ops retired (include fused uops).",
    {CNTRS12, 0xc2}},
// 0x400000A0
   {"SMC_Detected",
    "Number of times self-modifying code condition is detected.",
    {CNTRS12, 0xc3}},
   {"Br_Instr_Ret",
    "Number of branch instruction retired",
    {CNTRS12, 0xc4}},
   {"Br_MisPred_Ret",
    "Number of mispredicted branch instruction retired",
    {CNTRS12, 0xc5}},
   {"Cycles_Int_Masked",
    "Cycles while interrupt is disabled.",
    {CNTRS12, 0xc6}},
   {"Cycles_Int_Pedning_Masked",
    "Cycles while interrupt is disabled and interrupts are pending.",
    {CNTRS12, 0xc7}},
   {"HW_Int_Rx",
    "Number of hardware interrupts received.",
    {CNTRS12, 0xc8}},
   {"Br_Taken_Ret",
    "Number of taken branch instruction retired.",
    {CNTRS12, 0xc9}},
   {"Br_MisPred_Taken_Ret",
    "Number of taken and mispredicted branch instruction retired.",
    {CNTRS12, 0xca}},
   {"MMX_FP_Trans",
    "Number of transitions From MMX to X87.",
    {CNTRS12, 0xcc}},
   {"FP_MMX_Trans",
    "Number of transitions From X87 to MMX.",
    {CNTRS12, 0x1cc}},
   {"MMX_Assist",
    "Number of EMMS executed.",
    {CNTRS12, 0xcd}},
   {"MMX_Instr_Ret",
    "Number of MMX instruction retired.",
    {CNTRS12, 0xce}},
   {"Instr_Decoded",
    "Number of instruction decoded.",
    {CNTRS12, 0xd0}},
   {"ESP_Uops",
    "Number of ESP folding instruction decoded.",
    {CNTRS12, 0xd7}},
   {"SIMD_FP_SP_Ret",
    "Number of SSE/SSE2 single precision instructions retired (packed and scalar).",
    {CNTRS12, 0xd8}},
   {"SIMD_FP_SP_S_Ret",
    "Number of SSE/SSE2 scalar single precision instructions retired.",
    {CNTRS12, 0x1d8}},
// 0x400000B0
   {"SIMD_FP_DP_P_Ret",
    "Number of SSE/SSE2 packed double precision instructions retired.",
    {CNTRS12, 0x2d8}},
   {"SIMD_FP_DP_S_Ret",
    "Number of SSE/SSE2 scalar double precision instructions retired.",
    {CNTRS12, 0x3d8}},
   {"SIMD_Int_128_Ret",
    "Number of SSE2 128 bit integer instructions retired.",
    {CNTRS12, 0x4d8}},
   {"SIMD_FP_SP_P_Comp_Ret",
    "Number of SSE/SSE2 packed single precision compute instructions retired (does not include AND, OR, XOR).",
    {CNTRS12, 0xd9}},
   {"SIMD_FP_SP_S_Comp_Ret",
    "Number of SSE/SSE2 scalar single precision compute instructions retired (does not include AND, OR, XOR).",
    {CNTRS12, 0x1d9}},
   {"SIMD_FP_DP_P_Comp_Ret",
    "Number of SSE/SSE2 packed double precision compute instructions retired (does not include AND, OR, XOR).",
    {CNTRS12, 0x2d9}},
   {"SIMD_FP_DP_S_Comp_Ret",
    "Number of SSE/SSE2 scalar double precision compute instructions retired (does not include AND, OR, XOR).",
    {CNTRS12, 0x3d9}},
   {"Fused_Uops_Ret",
    "All fused uops retired.",
    {CNTRS12, 0xda}},
   {"Fused_Ld_Uops_Ret",
    "Fused load uops retired.",
    {CNTRS12, 0x1da}},
   {"Fused_St_Uops_Ret",
    "Fused store uops retired.",
    {CNTRS12, 0x2da}},
   {"Unfusion",
    "Number of unfusion events in the ROB (due to exception).",
    {CNTRS12, 0xdb}},
   {"Br_Instr_Decoded",
    "Branch instruction decoded.",
    {CNTRS12, 0xe0}},
   {"BTB_Misses",
    "Number of branches the BTB did not produce a prediction",
    {CNTRS12, 0xe2}},
   {"Br_Bogus",
    "Number of bogus branches",
    {CNTRS12, 0xe4}},
   {"BAClears",
    "Number of BAClears asserted",
    {CNTRS12, 0xe6}},
   {"Pref_Rqsts_Up",
    "Number of hardware prefetch request issued in forward streams",
    {CNTRS12, 0xf0}},
// 0x400000C0
   {"Pref_Rqsts_Dn",
    "Number of hardware prefetch request issued in backward streams",
    {CNTRS12, 0xf8}},
   {"", "", {0, 0}}
};

