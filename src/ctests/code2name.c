/* This file performs the following test: event_code_to_name */

#include "papi_test.h"

static void test_continue(char *file, int line, char *call, int retval)
{
      char errstring[PAPI_MAX_STR_LEN];
      PAPI_perror(retval, errstring, PAPI_MAX_STR_LEN);
      printf("Expected error in %s: %s\n", call, errstring);
}

int main(int argc, char **argv)
{
  int retval;
  unsigned int code = PAPI_TOT_CYC;
  char event_name[PAPI_MAX_STR_LEN];
  const PAPI_hw_info_t *hwinfo = NULL;
  const PAPI_component_info_t *cmp_info;

  tests_quiet(argc, argv);     /* Set TESTS_QUIET variable */

  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT)
    test_fail(__FILE__, __LINE__, "PAPI_library_init", retval);
  
  retval = papi_print_header ("Test case code2name.c: Check limits and indexing of event tables.\n", 0, &hwinfo);
  if (retval != PAPI_OK) test_fail(__FILE__, __LINE__, "PAPI_get_hardware_info", 2);

  printf("Looking for PAPI_TOT_CYC...\n");
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  printf("Found |%s|\n", event_name);
  
  code = PAPI_FP_OPS;
  printf("Looking for highest defined preset event (PAPI_FP_OPS): 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  printf("Found |%s|\n", event_name);
  
  code = PAPI_PRESET_MASK | (PAPI_MAX_PRESET_EVENTS - 1);
  printf("Looking for highest allocated preset event: 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_continue(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  else printf("Found |%s|\n", event_name);
  
  code = PAPI_PRESET_MASK | PAPI_NATIVE_AND_MASK;
  printf("Looking for highest possible preset event: 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_continue(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  else printf("Found |%s|\n", event_name);

  /* Find the first defined native event */
  code = PAPI_NATIVE_MASK;
  printf("Looking for first native event: 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  printf("Found |%s|\n", event_name);

  /* Find the last defined native event */
  cmp_info = PAPI_get_component_info(0);
  if (cmp_info == NULL)
    test_fail(__FILE__, __LINE__, "PAPI_get_component_info", PAPI_ESBSTR);
  code = (cmp_info->num_native_events - 1) | PAPI_NATIVE_MASK;
  printf("Looking for last native event: 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  printf("Found |%s|\n", event_name);

  /* Highly doubtful we have this many natives */
  /* Turn on all bits *except* PRESET bit and COMPONENT bits */
  code = PAPI_PRESET_AND_MASK & PAPI_COMPONENT_AND_MASK;
  printf("Looking for highest definable native event: 0x%x...\n", code);
  retval = PAPI_event_code_to_name(code, event_name);
  if (retval != PAPI_OK)
    test_continue(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);
  else printf("Found |%s|\n", event_name);
  if ((retval == PAPI_ENOEVNT) || (retval == PAPI_OK))
   test_pass(__FILE__, 0, 0);
   
   test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", PAPI_EBUG);

   exit(1);
}

