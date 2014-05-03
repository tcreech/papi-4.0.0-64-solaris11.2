/* 
* File:    p3_ath_event_tables.c
* CVS:     $Id: p3_ath_event_tables.h,v 1.2 2009/09/10 20:19:43 terpstra Exp $
* Author:  Dan Terpstra; refactored from p3_events.c by Joseph Thomas
*          terpstra@cs.utk.edu
* Mods:    <your name here>
*          <your email address>
*/

/* Note:  AMD MOESI bits are programmatically defined for those events
	  that can support them. You can find those events in this file
	  by searching for HAS_MOESI.  Events containing all possible 
	  combinations of these bits can be formed by appending the 
	  proper letters to the end of the event name, e.g. L2_LD_MESI
	  or L2_ST_MI. Some of these bit combinations are used in the 
	  preset tables. In these cases they are explicitly defined.
	  Others can be defined as needed. Otherwise the user can access
	  these native events using _papi_hwd_name_to_code() with the 
	  proper bit characters appended to the event name.
*/ 

/* This AMD Athlon enumeration table is used to define the location
   in the native event tables.  Each even has a unique name so as
   to not interfere with location of other events in other native
   tables.  The preset tables use these enumerations to lookup
   native events.
*/

enum {
   PNE_ATH_SEG_REG_LOADS = 0x40000000,
   PNE_ATH_ST_ACTIVE_IS,
   PNE_ATH_DC_ACCESSES,
   PNE_ATH_DC_MISSES,

   PNE_ATH_L2_DC_REFILLS,
   PNE_ATH_SYS_DC_REFILLS,
   PNE_ATH_DC_WB,

   PNE_ATH_L1_DTLB_MISSES_AND_L2_DTLB_HITS,
   PNE_ATH_L1_AND_L2_DTLB_MISSES,
   PNE_ATH_MISALIGNED_DATA_REFERENCES,
   PNE_ATH_DRAM_SYS_REQS,
   PNE_ATH_SYS_REQS_SEL_TYPE,
   PNE_ATH_SNOOP_HITS,
   PNE_ATH_ECC_ERR,
   PNE_ATH_INTERNAL_CACHE_INV,
   PNE_ATH_TOT_CYC,
   PNE_ATH_L2_REQ,
   PNE_ATH_FILL_REQ_STALLS,
   PNE_ATH_IC_FETCHES,
   PNE_ATH_IC_MISSES,
   PNE_ATH_L2_IC_REFILLS,
   PNE_ATH_SYS_IC_REFILLS,
   PNE_ATH_L1_ITLB_MISSES,
   PNE_ATH_L2_ITLB_MISSES,
   PNE_ATH_SNOOP_RESYNCS,
   PNE_ATH_IFETCH_STALLS,
   PNE_ATH_RET_STACK_HITS,
   PNE_ATH_RET_STACK_OVERFLOW,
   PNE_ATH_RET_INSTRUCTIONS,
   PNE_ATH_RET_OPS,
   PNE_ATH_RET_BR,
   PNE_ATH_RET_BR_MIS,
   PNE_ATH_RET_TAKEN_BR,
   PNE_ATH_RET_TAKEN_BR_MIS,
   PNE_ATH_RET_FAR_CONTROL_TRANSFERS,
   PNE_ATH_RET_RESYNC_BR,
   PNE_ATH_RET_NEAR_RETURNS,
   PNE_ATH_RET_NEAR_RETURNS_MIS,
   PNE_ATH_RET_INDIRECT_BR_MIS,
   PNE_ATH_INTS_MASKED_CYC,
   PNE_ATH_INTS_MASKED_WHILE_PENDING_CYC,
   PNE_ATH_TAKEN_HARDWARE_INTS,
   PNE_ATH_INS_DECODER_EMPTY,
   PNE_ATH_DISPATCH_STALLS,
   PNE_ATH_BRANCH_ABORTS,
   PNE_ATH_SERIALIZE,
   PNE_ATH_SEG_LOAD_STALLS,
   PNE_ATH_ICU_FULL,
   PNE_ATH_RES_STATIONS_FULL,
   PNE_ATH_FPU_FULL,
   PNE_ATH_LS_FULL,
   PNE_ATH_ALL_QUIET_STALL,
   PNE_ATH_TRANS_OR_BRANCH_PENDING,
   PNE_ATH_BP_DR0,
   PNE_ATH_BP_DR1,
   PNE_ATH_BP_DR2,
   PNE_ATH_BP_DR3,
   PNE_ATH_LAST_NATIVE_EVENT
};
/* These are special Athlon events with MOESI bits set as used in the preset table */
#define PNE_ATH_L2_DC_REFILLS_M     (PNE_ATH_L2_DC_REFILLS  | MOESI_M)
#define PNE_ATH_L2_DC_REFILLS_MI    (PNE_ATH_L2_DC_REFILLS  | MOESI_M | MOESI_I)
#define PNE_ATH_L2_DC_REFILLS_OES   (PNE_ATH_L2_DC_REFILLS  | MOESI_O | MOESI_E | MOESI_S)
#define PNE_ATH_L2_DC_REFILLS_MOES  (PNE_ATH_L2_DC_REFILLS  | MOESI_M | MOESI_O | MOESI_E | MOESI_S)
#define PNE_ATH_SYS_DC_REFILLS_M    (PNE_ATH_SYS_DC_REFILLS | MOESI_M)
#define PNE_ATH_SYS_DC_REFILLS_MOES (PNE_ATH_SYS_DC_REFILLS | MOESI_M | MOESI_O | MOESI_E | MOESI_S)
#define PNE_ATH_SYS_DC_REFILLS_OES  (PNE_ATH_SYS_DC_REFILLS | MOESI_O | MOESI_E | MOESI_S)


/* PAPI preset events are defined in the tables below.
   Each entry consists of a PAPI name, derived info, and up to four
   native event indeces as defined above.
*/

hwi_search_t _papi_hwd_ath_preset_map[] = {
   {PAPI_L1_DCM, {DERIVED_ADD, {PNE_ATH_SYS_DC_REFILLS_MOES, PNE_ATH_L2_DC_REFILLS_MOES, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICM, {0, {PNE_ATH_IC_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCM, {0, {PNE_ATH_SYS_DC_REFILLS_MOES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_ICM, {0, {PNE_ATH_SYS_IC_REFILLS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_TCM, {DERIVED_ADD, {PNE_ATH_SYS_DC_REFILLS_MOES, PNE_ATH_L2_DC_REFILLS_MOES, PNE_ATH_IC_MISSES, PAPI_NULL}, {0,}}},
   {PAPI_L2_TCM,
    {DERIVED_ADD, {PNE_ATH_SYS_DC_REFILLS_MOES, PNE_ATH_SYS_IC_REFILLS, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TLB_DM, {0, {PNE_ATH_L1_AND_L2_DTLB_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TLB_IM, {0, {PNE_ATH_L2_ITLB_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TLB_TL,
    {DERIVED_ADD, {PNE_ATH_L1_AND_L2_DTLB_MISSES, PNE_ATH_L2_ITLB_MISSES, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_LDM, {0, {PNE_ATH_L2_DC_REFILLS_OES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_STM, {0, {PNE_ATH_L2_DC_REFILLS_M, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_LDM, {0, {PNE_ATH_SYS_DC_REFILLS_OES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_STM, {0, {PNE_ATH_SYS_DC_REFILLS_M, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_HW_INT, {0, {PNE_ATH_TAKEN_HARDWARE_INTS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_UCN, {0, {PNE_ATH_RET_FAR_CONTROL_TRANSFERS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_CN, {0, {PNE_ATH_RET_BR, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_TKN, {0, {PNE_ATH_RET_TAKEN_BR, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_NTK, {DERIVED_SUB, {PNE_ATH_RET_BR, PNE_ATH_RET_TAKEN_BR, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_MSP, {0, {PNE_ATH_RET_BR_MIS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_PRC, {DERIVED_SUB, {PNE_ATH_RET_BR, PNE_ATH_RET_BR_MIS, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TOT_INS, {0, {PNE_ATH_RET_INSTRUCTIONS, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_BR_INS, {0, {PNE_ATH_RET_TAKEN_BR, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
//   { PAPI_VEC_INS,           {0, { PNE_ATH_DCU_LINES_IN,PAPI_NULL,PAPI_NULL,PAPI_NULL},{0,}}},
   {PAPI_RES_STL, {0, {PNE_ATH_ALL_QUIET_STALL, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_TOT_CYC, {0, {PNE_ATH_TOT_CYC, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_DCH, {DERIVED_POSTFIX, {PNE_ATH_DC_ACCESSES, PNE_ATH_SYS_DC_REFILLS_MOES, PNE_ATH_L2_DC_REFILLS_MOES, PAPI_NULL}, {"N0|N1|-|N2|-|",}}},
   {PAPI_L2_DCH, {0, {PNE_ATH_L2_DC_REFILLS_MOES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_DCA, {0, {PNE_ATH_DC_ACCESSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCA, {DERIVED_ADD, {PNE_ATH_SYS_DC_REFILLS_MOES, PNE_ATH_L2_DC_REFILLS_MOES, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_DCR, {0, {PNE_ATH_L2_DC_REFILLS_OES, PAPI_NULL, PAPI_NULL, PAPI_NULL},{0,}}},
   {PAPI_L2_DCW, {0, {PNE_ATH_L2_DC_REFILLS_M, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICA, {0, {PNE_ATH_IC_FETCHES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L2_ICA, {0, {PNE_ATH_IC_MISSES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_ICR, {0, {PNE_ATH_IC_FETCHES, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}},
   {PAPI_L1_TCA, {DERIVED_ADD, {PNE_ATH_DC_ACCESSES, PNE_ATH_IC_FETCHES, PAPI_NULL, PAPI_NULL}, {0,}}},
   {0, {0, {PAPI_NULL, PAPI_NULL, PAPI_NULL, PAPI_NULL}, {0,}}}
};

/* The following are the native tables for AMD Athlon.
   They contain the following:
   A short text description of the native event,
   A longer more descriptive text of the native event,
   Information on which counter the native can live,
   and the Native Event Code.
   In some cases, event codes can be programmatically expanded to
   include MOESI bits. These are identified by the addition
   of a HAS_MOESI flag in the selector field.
*/

/* The notes/descriptions of these events have sometimes been truncated */
/* Please see the architecture's manual for any clarifications.         */

const int _papi_hwd_k7_native_count = (PNE_ATH_LAST_NATIVE_EVENT & PAPI_NATIVE_AND_MASK);
native_event_entry_t _papi_hwd_k7_native_map[] = {
   {"SEG_REG_LOADS",
    "Number of segment register loads",
    {ALLCNTRS, 0x20}},
   {"ST_ACTIVE_IS",
    "Number of stores to active instruction stream (self-modifying code occurences)",
    {ALLCNTRS, 0x21}},
   {"DC_ACCESSES",
    "Number of data cache accesses",
    {ALLCNTRS, 0x40}},
   {"DC_MISSES",
    "Number of data cache misses",
    {ALLCNTRS, 0x41}},

   {"L2_DC_REFILLS",
    "Number of data cache lines refilled from L2",
    {ALLCNTRS | HAS_MOESI, 0x42}},
   {"SYS_DC_REFILLS",
    "Number of data cache lines refilled from system",
    {ALLCNTRS | HAS_MOESI, 0x43}},
   {"DC_WB",
    "Number of data cache writebacks",
    {ALLCNTRS | HAS_MOESI, 0x44}},

   {"L1_DTLB_MISSES ANDL2_DTLB_HITS",
    "L1 DTLB misses and L2 DLTB hits",
    {ALLCNTRS, 0x45}},
   {"L1_AND_L2_DTLB_MISSES",
    "L1 and L2 DTLB misses",
    {ALLCNTRS, 0x46}},
   {"MISALIGNED_DATA_REFERENCES",
    "Misaligned data references",
    {ALLCNTRS, 0x47}},
   {"DRAM_SYS_REQS",
    "Number of DRAM system requests",
    {ALLCNTRS, 0x64}},
   {"SYS_REQS_SEL_TYPE",
    "Number of system request with the selected type", /* ??? */
    {ALLCNTRS, 0x65}},
   {"SNOOP_HITS",
    "Number of snoop hits",
    {ALLCNTRS, 0x73}},
   {"ECC_ERR",
    "Number of single bit ecc errors detected or corrected",
    {ALLCNTRS, 0x74}},
   {"INTERNAL_CACHE_INV",
    "Number of internal cache line invalidates",
    {ALLCNTRS, 0x75}},
   {"TOT_CYC",
    "Number of cycles processor is running",
    {ALLCNTRS, 0x76}},
   {"L2_REQ",
    "Number of L2 requests",
    {ALLCNTRS, 0x79}},
   {"FILL_REQ_STALLS",
    "Number of cycles that at least one fill request waited to use the L2",
    {ALLCNTRS, 0x7a}},
   {"IC_FETCHES",
    "Number of instruction cache fetches",
    {ALLCNTRS, 0x80}},
   {"IC_MISSES",
    "Number of instruction cache misses",
    {ALLCNTRS, 0x81}},
   {"L2_IC_REFILLS",
    "Number of instruction cache refills from L2",
    {ALLCNTRS, 0x82}},
   {"SYS_IC_REFILLS",
    "Number of instruction cache refills from system",
    {ALLCNTRS, 0x83}},
   {"L1_ITLB_MISSES",
    "Number of L1 ITLB misses (and L2 ITLB hits.)",
    {ALLCNTRS, 0x84}},
   {"L2_ITLB_MISSES",
    "Number of (L1 and) L2 ITLB misses",
    {ALLCNTRS, 0x85}},
   {"SNOOP_RESYNCS",
    "Number of snoop resyncs",
    {ALLCNTRS, 0x86}},
   {"IFETCH_STALLS",
    "Number of instruction fetch stall cycles",
    {ALLCNTRS, 0x87}},
   {"RET_STACK_HITS",
    "Number of return stack hits",
    {ALLCNTRS, 0x88}},
   {"RET_STACK_OVERFLOW",
    "Return stack overflow",
    {ALLCNTRS, 0x89}},
   {"RET_INSTRUCTIONS",
    "Retired instructions (includes exceptions, interrupts, resyncs.)",
    {ALLCNTRS, 0xc0}},
   {"RET_OPS",
    "Retired ops",
    {ALLCNTRS, 0xc1}},
   {"RET_BR",
    "Retired branches (conditional, unconditional, exceptions, interrupts.)",
    {ALLCNTRS, 0xc2}},
   {"RET_BR_MIS",
    "Retired branches mispredicted",
    {ALLCNTRS, 0xc3}},
   {"RET_TAKEN_BR",
    "Retired taken branches",
    {ALLCNTRS, 0xc4}},
   {"RET_TAKEN_BR_MIS",
    "Retired taken branches mispredicted",
    {ALLCNTRS, 0xc5}},
   {"RET_FAR_CONTROL_TRANSFERS",
    "Retired far control transfers",
    {ALLCNTRS, 0xc6}},
   {"RET_RESYNC_BR",
    "Retired resync branches - only non-control transfer branches counted",
    {ALLCNTRS, 0xc7}},
   {"RET_NEAR_RETURNS",
    "Retired near returns",
    {ALLCNTRS, 0xc8}},
   {"RET_NEAR_RETURNS_MIS",
    "Retired near returns mispredicted",
    {ALLCNTRS, 0xc9}},
   {"RET_INDIRECT_BR_MIS",
    "Retired indirect branches with target mispredicted",
    {ALLCNTRS, 0xca}},
   {"INTS_MASKED_CYC",
    "Interrupts masked cycles (IF=0)",
    {ALLCNTRS, 0xcd}},
   {"INTS_MASKED_WHILE_PENDING_CYC",
    "Interrupts masked while pending cycles (INTR while IF=0)",
    {ALLCNTRS, 0xce}},
   {"TAKEN_HARDWARE_INTS",
    "Number of taken hardware interrupts",
    {ALLCNTRS, 0xcf}},
   {"INS_DECODER_EMPTY",
    "Number of cycles in whih the instruction decoder is empty",
    {ALLCNTRS, 0xd0}},
   {"DISPATCH_STALLS",
    "Number of dispatch stalls",
    {ALLCNTRS, 0xd1}},
   {"BRANCH_ABORTS",
    "Number of branch aborts to retire",
    {ALLCNTRS, 0xd2}},
   {"SERIALIZE",
    "Serialize",
    {ALLCNTRS, 0xd3}},
   {"SEG_LOAD_STALLS",
    "Number of segment load stalls",
    {ALLCNTRS, 0xd4}},
   {"ICU_FULL",
    "Number of cycles in which the ICU is full",
    {ALLCNTRS, 0xd5}},
   {"RES_STATIONS_FULL",
    "Number of cycles in which the reservation station is full",
    {ALLCNTRS, 0xd6}},
   {"FPU_FULL",
    "Number of cycles in which the FPU is full",
    {ALLCNTRS, 0xd7}},
   {"LS_FULL",
    "Number of cycles in which the LS is full",
    {ALLCNTRS, 0xd8}},
   {"ALL_QUIET_STALL",
    "Number of all quiet stalls",
    {ALLCNTRS, 0xd9}},
   {"TRANS_OR_BRANCH_PENDING",
    "Far transfer or resync branch pending",
    {ALLCNTRS, 0xda}},
   {"BP_DR0",
    "Number of breakpoint matches for DR0",
    {ALLCNTRS, 0xdc}},
   {"BP_DR1",
    "Number of breakpoint matches for DR1",
    {ALLCNTRS, 0xdd}},
   {"BP_DR2",
    "Number of breakpoint matches for DR2",
    {ALLCNTRS, 0xde}},
   {"BP_DR3",
    "Number of breakpoint matches for DR3",
    {ALLCNTRS, 0xdf}},
   {"", "", {0, 0}}
};

