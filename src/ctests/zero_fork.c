/* 
* File:    zero_fork.c
* CVS:     $Id: zero_fork.c,v 1.9 2009/11/11 16:00:44 bsheely Exp $
* Author:  Philip Mucci
*          mucci@cs.utk.edu
* Mods:    <your name here>
*          <your email address>
*/

/* This file performs the following test: start, stop and timer
functionality for a parent and a forked child. */

#include "papi_test.h"
#include <sys/wait.h>

int EventSet1=PAPI_NULL;
int PAPI_event, mask1;
int num_events1 = 2;
long long elapsed_us, elapsed_cyc;
long long **values;
char event_name[PAPI_MAX_STR_LEN];
int retval, num_tests = 1;
const PAPI_hw_info_t *hw_info = NULL;

void process_init(void)
{
   printf("Process %d \n", (int) getpid());
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT)
      test_fail(__FILE__, __LINE__, "PAPI_library_init", retval);

   hw_info = PAPI_get_hardware_info();
   if (hw_info == NULL)
     test_fail(__FILE__, __LINE__, "PAPI_get_hardware_info", 2);
 
    /* add PAPI_TOT_CYC and one of the events in PAPI_FP_INS, PAPI_FP_OPS or
      PAPI_TOT_INS, depends on the availability of the event on the
      platform */
   EventSet1 = add_two_events(&num_events1, &PAPI_event, hw_info, &mask1);

   values = allocate_test_space(num_tests, num_events1);

   retval = PAPI_event_code_to_name(PAPI_event, event_name);
   if (retval != PAPI_OK)
      test_fail(__FILE__, __LINE__, "PAPI_event_code_to_name", retval);


   elapsed_us = PAPI_get_real_usec();

   elapsed_cyc = PAPI_get_real_cyc();

   retval = PAPI_start(EventSet1);
   if (retval != PAPI_OK)
      test_fail(__FILE__, __LINE__, "PAPI_start", retval);
}

void process_fini(void)
{
   retval = PAPI_stop(EventSet1, values[0]);
   if (retval != PAPI_OK)
      test_fail(__FILE__, __LINE__, "PAPI_stop", retval);

   elapsed_us = PAPI_get_real_usec() - elapsed_us;

   elapsed_cyc = PAPI_get_real_cyc() - elapsed_cyc;

   remove_test_events(&EventSet1, mask1);

      printf("Process %d %-12s : \t%lld\n", (int)getpid(), event_name,
             (values[0])[0]);
      printf("Process %d PAPI_TOT_CYC : \t%lld\n", (int)getpid(), (values[0])[1]);
      printf("Process %d Real usec    : \t%lld\n", (int)getpid(), elapsed_us);
      printf("Process %d Real cycles  : \t%lld\n", (int)getpid(), elapsed_cyc);

   free_test_space(values, num_tests);

}

int main(int argc, char **argv)
{
   int flops1;
   int retval;

   tests_quiet(argc, argv);     /* Set TESTS_QUIET variable */
# if (defined(__ALPHA) && defined(__osf__))
   test_skip(__FILE__, __LINE__, "main: fork not supported.", 0);
#endif

   printf("This tests if PAPI_library_init(),2*fork(),PAPI_library_init() works.\n");
   /* Initialize PAPI for this process */
   process_init();
   flops1 = 1000000;
   if (fork() == 0)
     {
       /* Initialize PAPI for the child process */
       process_init();
       /* Let the child process do work */
       do_flops(flops1);
       /* Measure the child process */
       process_fini();
       exit(0);
     }
   flops1 = 2000000;
   if (fork() == 0)
     {
       /* Initialize PAPI for the child process */
       process_init();
       /* Let the child process do work */
       do_flops(flops1);
       /* Measure the child process */
       process_fini();
       exit(0);
     }
   /* Let this process do work */
   flops1 = 4000000;
   do_flops(flops1);

   /* Wait for child to finish */
   wait(&retval);
   /* Wait for child to finish */
   wait(&retval);

   /* Measure this process */
   process_fini();

   test_pass(__FILE__, NULL, 0);
   exit(1);
}
