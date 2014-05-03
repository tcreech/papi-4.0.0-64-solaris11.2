/* 
* File:    profile.c
* CVS:     $Id: shlib.c,v 1.18 2009/09/10 20:19:49 terpstra Exp $
* Author:  Philip Mucci
*          mucci@cs.utk.edu
* Mods:    <your name here>
*          <your email address>
*/
#include "papi_test.h"

int main(int argc, char **argv)
{
   int retval;
   int i;

   const PAPI_shlib_info_t *shinfo;
   PAPI_address_map_t *map;

   tests_quiet(argc, argv);     /* Set TESTS_QUIET variable */

   if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
      test_fail(__FILE__, __LINE__, "PAPI_library_init", retval);

   if ((shinfo = PAPI_get_shared_lib_info()) == NULL) {
      test_skip(__FILE__, __LINE__, "PAPI_get_shared_lib_info", 1);
   }

   if ((shinfo->count == 0) && (shinfo->map)) {
      test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info", 1);
   }

   map = shinfo->map;
   for (i=0;i<shinfo->count;i++)
     {
       printf("Library: %s\n",map->name);
       printf("Text start: %p, Text end: %p\n",map->text_start,map->text_end);
       printf("Data start: %p, Data end: %p\n",map->data_start,map->data_end);
       printf("Bss start: %p, Bss end: %p\n",map->bss_start,map->bss_end);

       if ((map->name == NULL) || (strlen(map->name) == 0))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
       if ((map->text_start == 0x0) || (map->text_end == 0x0) ||
	   (map->text_start >= map->text_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
/*
       if ((map->data_start == 0x0) || (map->data_end == 0x0) ||
	   (map->data_start >= map->data_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
       if (((map->bss_start) && (!map->bss_end)) ||
	   ((!map->bss_start) && (map->bss_end)) ||
	   (map->bss_start > map->bss_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
*/

       map++;
     }

   sleep(1); /* Needed for debugging, so you can ^Z and stop the process, inspect /proc to see if it's right */

#ifndef NO_DLFCN
   {
     char *libname = 
     "libpapi.so";
     char *libname2 = 
     "libpapi64.so";
     char *_libname;
     void *handle ;
/*
     void *handle = dlopen("libm.so", RTLD_LAZY);
*/
     int (*num_hwctrs)(void);
     int oldcount;
     int my_dlerror = 0;

RETRY:
     if ( !my_dlerror ){
        printf("\nLoading %s with dlopen().\n",libname);
        _libname = libname;
     }
     else{
        printf("\nLoading %s with dlopen().\n",libname2);
        _libname = libname2;
     }

     handle = dlopen (_libname, RTLD_NOW);
     if (!handle) {
	 printf("dlopen: %s\n",dlerror());
          if ( !my_dlerror ){
             my_dlerror = 1;
	     printf("Trying again.");
             goto RETRY;
          }
          printf("Did you forget to set the environmental variable LIBPATH (in AIX) or LD_LIBRARY_PATH (in linux) ?\n");
          test_fail(__FILE__, __LINE__, "dlopen", 1);
     }
     
     printf("Looking up PAPI_num_counters() \n");
     num_hwctrs = ( int (*) (void)) dlsym(handle, "PAPI_num_counters");
     if (num_hwctrs == NULL)  {
       printf("dlsym: %s \n", dlerror());
       test_fail(__FILE__, __LINE__, "dlsym", 1);
     }
     
     printf ("There are %d hardware counters \n", (*num_hwctrs)());
 
   oldcount = shinfo->count;

   if ((shinfo = PAPI_get_shared_lib_info()) == NULL) {
      test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info", 1);
   }

   sleep(1); /* Needed for debugging, so you can ^Z and stop the process, inspect /proc to see if it's right */

   if ((shinfo->count == 0) && (shinfo->map)) {
      test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info", 1);
   }

   if (shinfo->count <= oldcount) {
      test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info", 1);
   }

   map = shinfo->map;
   for (i=0;i<shinfo->count;i++)
     {
	   printf("Library: %s\n",map->name);
	   printf("Text start: %p, Text end: %p\n",map->text_start,map->text_end);
	   printf("Data start: %p, Data end: %p\n",map->data_start,map->data_end);
	   printf("Bss start: %p, Bss end: %p\n",map->bss_start,map->bss_end);

       if ((map->name == NULL) || (strlen(map->name) == 0))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
       if ((map->text_start == 0x0) || (map->text_end == 0x0) ||
	   (map->text_start >= map->text_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
/*
       if ((map->data_start == 0x0) || (map->data_end == 0x0) ||
	   (map->data_start >= map->data_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
       if (((map->bss_start) && (!map->bss_end)) ||
	   ((!map->bss_start) && (map->bss_end)) ||
	   (map->bss_start > map->bss_end))
	 test_fail(__FILE__, __LINE__, "PAPI_get_shared_lib_info",1);
*/

       map++;
     }
   
   sleep(1); /* Needed for debugging, so you can ^Z and stop the process, inspect /proc to see if it's right */

   dlclose(handle);
   }
#endif

   test_pass(__FILE__, NULL, 0);
   exit(0);
}
