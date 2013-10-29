/*********************************************************************************
*  Copyright (c) 2013-2011,   Paul Rosenfeld
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include "affinity.h"
#include "ptlcalls.h"

#include <semaphore.h> //sem_t
#include <unistd.h> //sysconf()
#include <fcntl.h>  //sem_open()
#include <sys/stat.h> // O_* constants

#include "ptlcalls.h"
#include "multiprogram_daemon.h"

int testprog_set_affinity(int start_cpu, int end_cpu)
{
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	int i;
	int retval;

	int n_cpus = get_nprocs(); 
	int n_cpus_conf = get_nprocs_conf();

	if (n_cpus != n_cpus_conf) {
		printf("uh, ncpus=%d but ncpus_conf=%d\n", n_cpus, n_cpus_conf); 
	}

	if (start_cpu < 0) start_cpu=0;
	if (end_cpu < 0 || end_cpu > n_cpus_conf) {
		end_cpu = n_cpus_conf;
	}

	for (i=start_cpu; i<end_cpu; i++) {
		CPU_SET(i,&cpu_set);
	}
	if ((retval = sched_setaffinity(0, sizeof(cpu_set), &cpu_set)) == 0) {
		printf("Set affinity for CPUs %d-%d OK\n", start_cpu, end_cpu-1);
	}
	return retval; 
}

void wait_for_start(sem_t *go) {
	int go_value = -1;
	printf("Waiting for go signal\n"); 
	while (go_value != 1) {
		// poll
		sem_getvalue(go, &go_value);
	}
}

void roi_start_multiprogram() {
	sem_t *wait = sem_open(waiting_sem_name, 0, 0644, 0); 
	if (wait == SEM_FAILED) {
		if (errno == ENOENT) {
			perror("Didn't find running multiprogram coordinator, proceeding as usual"); 
			roi_start(); 
		}
	}

	else {
		sem_trywait(wait); 
		sem_t *go = sem_open(go_sem_name, 0, 0644, 0); 
		if (go == SEM_FAILED) {
			perror("'go' semaphore failed"); 
			abort();
		}
		wait_for_start(go); 
	}
}

void roi_start()
{
	printf("starting ROI...\n"); 
	char *checkpoint_name = getenv("CHECKPOINT_NAME"); 
	if (checkpoint_name)
		ptlcall_checkpoint_and_shutdown(checkpoint_name); 
	else
		ptlcall_switch_to_sim(); 
}
void roi_stop()
{
	ptlcall_kill(); 
}
