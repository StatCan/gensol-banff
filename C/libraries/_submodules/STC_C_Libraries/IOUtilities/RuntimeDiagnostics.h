#ifndef _RUNTIME_DIAGNOSTICS_H_
#define _RUNTIME_DIAGNOSTICS_H_
/* this code allows measurement of execution time and memory usage
	execution time measurements are broken down into 3 categories
		CPU time: time the CPU was processing instructions since timer started
			- for all threads
			- for a single thread
		WALL time: time elapsed since timer started
	Memory usage measurements are somewhat "best effort", sometimes not showing a decrease
	in usage following freeing of memory (particular on Linux).  
*/

#include <stdio.h>

/* enable_debug: global variable declared and defined by RuntimeDiagnostics.c
	To enable printing of TIME measurements and memory usage
	enable_debug must be set to != 0 (i.e. set to TRUE) */
extern unsigned int enable_debug;

void init_debug_env(const char* system_debug_varname);

/* TIME MEASUREMENTS */
// the `CONCAT*` macros are for composing variable names based on macro arguments
#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

/* TIME Measurement macro functions
	Platform specific implementations exist for Windows and Linux
	3 sets of macros measure the
		- Wall time
		- CPU time of all threads in the process
		- CPU time of the current thread
	In each set, the following macros are defined
		- DECLARE:	Create variable for storing time
		- BEGIN:	Record start time into declared variable
		- START: 	Convenience function calls DECLARE and BEGIN
		- STOP: 	Create variable to record stop time
		- DIFF: 	Print time elapsed between start and stop timers
		- STOPDIFF:	Convenience functions calls STOP and then calls DIFF
	The functions take a suffix (unquoted string) as an argument.
		Pass the same suffix to different functions to work on the same timer
		Suffix is used to name variables and in the print statements
*/

#if defined(_WIN32) || defined(_WIN64)
/* CPU TIME : Windows */
#include <Windows.h>
#include <processthreadsapi.h>
#include <time.h>
#include "psapi.h"
double get_cpu_time();
double get_cpu_time_thread();

/* PROCESS CPU time macros: measure CPU time for all threads */
#define TIME_CPU_DECLARE(suffix) double CONCAT(tm_cpu_start_, suffix) = { 0 }; double CONCAT(tm_cpu_end_, suffix) = { 0 };
#define TIME_CPU_BEGIN(suffix) CONCAT(tm_cpu_start_, suffix) = get_cpu_time();
#define TIME_CPU_START(suffix) TIME_CPU_DECLARE(suffix) TIME_CPU_BEGIN(suffix)
#define TIME_CPU_STOP(suffix) CONCAT(tm_cpu_end_, suffix) = get_cpu_time();
#define TIME_CPU_DIFF(suffix) if(enable_debug) printf("\n[C,TIME,CPU , %10s]: %.3f seconds.\n", #suffix, (CONCAT(tm_cpu_end_, suffix) - CONCAT(tm_cpu_start_, suffix)));
#define TIME_CPU_STOPDIFF(suffix) TIME_CPU_STOP(suffix) TIME_CPU_DIFF(suffix)

/* THREAD CPU time macros: measure CPU time for specific thread */
#define TIME_CPU_THREAD_START(suffix) double CONCAT(tm_cpu_thread_start_, suffix) = get_cpu_time_thread();
#define TIME_CPU_THREAD_STOP(suffix) double CONCAT(tm_cpu_thread_end_, suffix) = get_cpu_time_thread();
#define TIME_CPU_THREAD_DIFF(suffix) if(enable_debug) printf("\n[C,TIME,CPUT, %10s]: %.3f seconds.\n", #suffix, (CONCAT(tm_cpu_thread_end_, suffix) - CONCAT(tm_cpu_thread_start_, suffix)));
#define TIME_CPU_THREAD_STOPDIFF(suffix) TIME_CPU_THREAD_STOP(suffix) TIME_CPU_THREAD_DIFF(suffix)

/* WALL TIME */
#define TIME_WALL_DECLARE(suffix) clock_t CONCAT(tm_wall_start_, suffix) = { 0 };  clock_t CONCAT(tm_wall_end_, suffix) = { 0 };
#define TIME_WALL_BEGIN(suffix) CONCAT(tm_wall_start_, suffix) = clock();
#define TIME_WALL_START(suffix) TIME_WALL_DECLARE(suffix) TIME_WALL_BEGIN(suffix);
#define TIME_WALL_STOP(suffix) CONCAT(tm_wall_end_, suffix) = clock();
#define TIME_WALL_DIFF(suffix) if(enable_debug) printf("\n[C,TIME,WALL, %10s]: %.3Lf seconds.\n", #suffix, (long double)((long double)CONCAT(tm_wall_end_, suffix) - (long double)CONCAT(tm_wall_start_, suffix))/ (long double) CLOCKS_PER_SEC);
#define TIME_WALL_STOPDIFF(suffix) TIME_WALL_STOP(suffix) TIME_WALL_DIFF(suffix)

#else 
/* Linux implementation */
#include <time.h>
#include <malloc.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

double get_exec_time(clockid_t);

/* PROCESS CPU time macros: measure CPU time for all threads */
#define TIME_CPU_DECLARE(suffix) 	double 	CONCAT(tm_cpu_start_, suffix) 	= { 0 }; \
									double 	CONCAT(tm_cpu_end_, suffix)		= { 0 };
#define TIME_CPU_BEGIN(suffix) 				CONCAT(tm_cpu_start_, suffix) 	= get_exec_time(CLOCK_PROCESS_CPUTIME_ID);
#define TIME_CPU_STOP(suffix) 				CONCAT(tm_cpu_end_, suffix) 	= get_exec_time(CLOCK_PROCESS_CPUTIME_ID);
#define TIME_CPU_DIFF(suffix) 		if(enable_debug) { \
										printf("\n[C,TIME,CPU , %10s]: %.3f seconds.\n", #suffix, (CONCAT(tm_cpu_end_, suffix) - CONCAT(tm_cpu_start_, suffix))); \
									}
#define TIME_CPU_START(suffix) 		TIME_CPU_DECLARE(suffix) 	TIME_CPU_BEGIN(suffix)
#define TIME_CPU_STOPDIFF(suffix) 	TIME_CPU_STOP(suffix) 		TIME_CPU_DIFF(suffix)

/* THREAD CPU time macros: measure CPU time for specific thread */
#define TIME_CPU_THREAD_START(suffix) 		double CONCAT(tm_cpu_thread_start_, suffix) = get_exec_time(CLOCK_THREAD_CPUTIME_ID);
#define TIME_CPU_THREAD_STOP(suffix) 		double CONCAT(tm_cpu_thread_end_, suffix) = get_exec_time(CLOCK_THREAD_CPUTIME_ID);
#define TIME_CPU_THREAD_DIFF(suffix) 		if(enable_debug) printf("\n[C,TIME,CPUT, %10s]: %.3f seconds.\n", #suffix, (CONCAT(tm_cpu_thread_end_, suffix) - CONCAT(tm_cpu_thread_start_, suffix)));
#define TIME_CPU_THREAD_STOPDIFF(suffix) 	TIME_CPU_THREAD_STOP(suffix) TIME_CPU_THREAD_DIFF(suffix)

/* WALL TIME */
#define TIME_WALL_DECLARE(suffix) 	double 	CONCAT(tm_wall_start_, suffix) 	= { 0 }; \
									double 	CONCAT(tm_wall_end_, suffix) 	= { 0 };
#define TIME_WALL_BEGIN(suffix) 			CONCAT(tm_wall_start_, suffix) 	= get_exec_time(CLOCK_REALTIME);
#define TIME_WALL_STOP(suffix) 				CONCAT(tm_wall_end_, suffix) = get_exec_time(CLOCK_REALTIME);
#define TIME_WALL_DIFF(suffix) 		if(enable_debug) { \
										printf("\n[C,TIME,WALL, %10s]: %.3f seconds.\n", #suffix, (CONCAT(tm_wall_end_, suffix) - CONCAT(tm_wall_start_, suffix))); \
									}
#define TIME_WALL_START(suffix) 	TIME_WALL_DECLARE(suffix) 	TIME_WALL_BEGIN(suffix)
#define TIME_WALL_STOPDIFF(suffix) 	TIME_WALL_STOP(suffix) 		TIME_WALL_DIFF(suffix)

#endif

/* memory usage:
	A single function with platform-specific implementation */
void mem_usage(char* banner);

#endif _RUNTIME_DIAGNOSTICS_H_
