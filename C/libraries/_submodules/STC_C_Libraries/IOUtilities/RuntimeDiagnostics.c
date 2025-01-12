#include "RuntimeDiagnostics.h"

unsigned int enable_debug = 0;	// default value => disable debugging (false)

// enable debug if env var exists, otherwise disable
void init_debug_env(const char* system_debug_varname) {
	if (getenv("PROC_DEBUG_STATS") != NULL || getenv(system_debug_varname) != NULL) {  // nosemgrep  // value isn't used, its safe
		enable_debug = 1;
	} else {
		enable_debug = 0;
	}
}

#if defined(_WIN32) || defined(_WIN64)
// get CPU time for the current process
double get_cpu_time() {
	FILETIME a, b, c, d;
	if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
		//  Returns total user time.
		//  Can be tweaked to include kernel times as well.
		return
			(double)(d.dwLowDateTime |
				((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
	} else {
		//  Handle error
		return 0;
	}
}

// get CPU time for current thread (helpful for parallel processing)
double get_cpu_time_thread() {
	FILETIME a, b, c, d;
	if (GetThreadTimes(GetCurrentThread(), &a, &b, &c, &d) != 0) {
		//  Returns total user time.
		//  Can be tweaked to include kernel times as well.
		return
			(double)(d.dwLowDateTime |
				((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
	} else {
		//  Handle error
		return 0;
	}
}

/* print various statistics on current memory usage */
void mem_usage(char* banner) {
	if (!enable_debug)
		return;

	double bytes_per_kb = 1000;
	double bytes_per_gb = 1.0e9;

	printf("MEMORY USAGE: %s\n", banner);

	/* SYSTEM WIDE MEMORY */
	PERFORMANCE_INFORMATION pi;
	if(GetPerformanceInfo(&pi, sizeof(pi))){
		printf("~~System wide~~\n");

		printf("Total     [V]: %16.0f kB (%.2f GB)\n", 
			(double)(pi.CommitLimit * pi.PageSize) / bytes_per_kb, 
			(double)(pi.CommitLimit * pi.PageSize) / bytes_per_gb
		);
		printf("Available [V]: %16.0f kB (%.2f GB)\n", 
			(double)((pi.CommitLimit - pi.CommitTotal) * pi.PageSize) / bytes_per_kb, 
			(double)((pi.CommitLimit - pi.CommitTotal) * pi.PageSize) / bytes_per_gb
		);
		printf("Total     [P]: %16.0f kB (%.2f GB)\n", 
			(double)(pi.PhysicalTotal * pi.PageSize) / bytes_per_kb, 
			(double)(pi.PhysicalTotal * pi.PageSize) / bytes_per_gb
		);
		printf("Available [P]: %16.0f kB (%.2f GB)\n", 
			(double)(pi.PhysicalAvailable * pi.PageSize) / bytes_per_kb, 
			(double)(pi.PhysicalAvailable * pi.PageSize) / bytes_per_gb
		);
	}else{
		printf("Call to GetPerformanceInfo() failed\n");
	}

	/* PROCESS SPECIFIC MEMORY */
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS_EX*)&pmc, sizeof(pmc))) {
		printf("~~Process specific~~\n");

		printf("Private Bytes: %16.0f kB (%.2f GB)\n",
			(double)pmc.PrivateUsage / bytes_per_kb,
			(double)pmc.PrivateUsage / bytes_per_gb
		);
		printf("Working Set  : %16.0f kB (%.2f GB)\n",
			(double)pmc.WorkingSetSize / bytes_per_kb,
			(double)pmc.WorkingSetSize / bytes_per_gb
		);
		printf("Page Faults  : %3lu\n",
			pmc.PageFaultCount
		);
	}
	else {
		printf("Call to GetProcessMemoryInfo() failed\n");
	}
	printf("\n");
}


#else 
/* return current execution time
	wall, cpu, or cpu-thread time returned depending on `clock_id`
		CLOCK_PROCESS_CPUTIME_ID		-> CPU for all threads
		CLOCK_THREAD_CPUTIME_ID			-> CPU for current thread
		CLOCK_REALTIME					-> wall time
*/
double get_exec_time(clockid_t clock_id) {
	struct timespec st;
	clock_gettime(clock_id, &st);

	double cur_time = (double)st.tv_sec;
	cur_time += (double)st.tv_nsec / 1000000000;
	return cur_time;
}

/*  MEMORY USAGE FUNCTIONS
	Most of this code was derived from 
		https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process */
int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);  // nosemgrep  // non-critical diagnostic code
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);  // nosemgrep  // non-critical best-effort diagnostic code, its safe
    return i;
}

int get_status_value(const char* key){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");  // nosemgrep  // ignore this non-security code
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, key, strlen(key)) == 0){  // nosemgrep  // non-critical diagnostic code
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int print_status_file(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");  // nosemgrep  // ignore this non-security code
    int result = -1;
    char line[1024];

    while (fgets(line, 1024, file) != NULL){
		printf("%s", line);
    }
    fclose(file);
    return result;
}

/* calculate and print various statistics on current memory usage
	This is a relative nightmare to implement compared to Windows 
*/
void mem_usage(char* banner) {
	if (!enable_debug)
		return;

	double bytes_per_kb = 1000;
	double bytes_per_gb = 1.0e9;

	printf("MEMORY USAGE: %s\n", banner);

	/* SYSTEM WIDE MEMORY */
	struct sysinfo memInfo;
	if(sysinfo (&memInfo) == 0){
		printf("~~System wide~~\n");

		// VIRTUAL MEMORY: TOTAL
		long long totalVirtualMem = memInfo.totalram;
		//Add other values in next statement to avoid int overflow on right hand side...
		totalVirtualMem += memInfo.totalswap;
		totalVirtualMem *= memInfo.mem_unit;
		printf("Total     [V]: %16.0f kB (%.2f GB)\n", 
			(double) totalVirtualMem/bytes_per_kb,
			((double)totalVirtualMem/bytes_per_gb)
		);

		// VIRTUAL MEMORY: FREE
		long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
		//Add other values in next statement to avoid int overflow on right hand side...
		virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
		virtualMemUsed *= memInfo.mem_unit;
		long long vm_free = (totalVirtualMem-virtualMemUsed);
		printf("Available [V]: %16.0f kB (%.2f GB)\n", 
			(double) vm_free/bytes_per_kb,
			((double)vm_free/bytes_per_gb)
		);

		// PHYSICAL MEMORY: TOTAL
		long long totalPhysMem = memInfo.totalram;
		//Multiply in next statement to avoid int overflow on right hand side...
		totalPhysMem *= memInfo.mem_unit;
		printf("Total     [P]: %16.0f kB (%.2f GB)\n", 
			(double) totalPhysMem/bytes_per_kb,
			((double)totalPhysMem/bytes_per_gb)
		);

		// PHYSICAL MEMORY: FREE
		long long physMemUsed = memInfo.totalram - memInfo.freeram;
		//Multiply in next statement to avoid int overflow on right hand side...
		physMemUsed *= memInfo.mem_unit;
		long long pm_free = (totalPhysMem-physMemUsed);
		printf("Available [P]: %16.0f kB (%.2f GB)\n",
			 (double) pm_free/bytes_per_kb,
			 ((double)pm_free/bytes_per_gb)
		);

	}

	/* PROCESS SPECIFIC MEMORY 
		I haven't figured this out yet, so I'm printing quite a bit of info.  */
	printf("~~Process specific: PROC~~\n");
	// VIRTUAL MEMORY: CURRENTLY USED
	long long vm_current_use2 = get_status_value("VmSize:");
	printf("Private Bytes: %16.0f kB (%.2f GB)\n", 
		(double) vm_current_use2, 
		((double)vm_current_use2/1.0e6)
	);
	// PHYSICAL MEMORY: CURRENTLY USED
	long long pm_current_use2 = get_status_value("VmRSS:");
	printf("WOrking Set  : %16.0f kB (%.2f GB)\n", 
		(double) pm_current_use2, 
		((double)pm_current_use2/1.0e6)
	);

	printf("~~Process specific: MALLINFO~~\n");
	struct mallinfo foo = mallinfo();
	// VIRTUAL MEMORY: CURRENTLY USED
	long long vm_current_use = foo.arena;
	vm_current_use += foo.hblkhd;
	printf("Private Bytes: %16.0f kB (%.2f GB)\n", 
		(double) vm_current_use/bytes_per_kb, 
		((double)vm_current_use/bytes_per_gb)
	);

	// PHYSICAL MEMORY: CURRENTLY USED
	long long pm_current_use = foo.uordblks;
	pm_current_use += foo.hblkhd;
	printf("Working Set  : %16.0f kB (%.2f GB)\n", 
		(double) pm_current_use/bytes_per_kb,
		((double)pm_current_use/bytes_per_gb)
	);
	//malloc_stats();
	/* EXTRA DETAILS */
	printf("~~Process specific: extra~~\n");
	//printf("arena = %d\n", foo.arena);
	//printf("ordblks = %d\n", foo.ordblks);
	//printf("smblks = %d\n", foo.smblks);
	//printf("hblks = %d\n", foo.hblks);
	printf("hblkhd   =     %16d bytes\n", foo.hblkhd);
	printf("usmblks  =     %16d bytes\n", foo.usmblks);
	printf("fsmblks  =     %16d bytes\n", foo.fsmblks);
	printf("uordblks =     %16d bytes\n", foo.uordblks);
	printf("fordblks =     %16d bytes\n", foo.fordblks);
    printf("\n");
	//printf("keepcost = %d\n", foo.keepcost);

	//malloc_info(0, stdout);
	return;
}
#endif