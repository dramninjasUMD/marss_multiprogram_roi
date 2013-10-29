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

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h> //sem_t
#include <unistd.h> //sysconf()
#include <fcntl.h>  //sem_open()
#include <sys/stat.h> // O_* constants
#include <assert.h>
#include "affinity.h" // roi_*()

#include "multiprogram_daemon.h"


int main(int argc, const char **argv) {
	sem_unlink(waiting_sem_name); 
	sem_unlink(go_sem_name);
	sem_unlink(running_sem_name); 
	sem_t *wait; 
	sem_t *go; 
	int default_mode = O_CREAT | O_EXCL;
	long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
	unsigned initial_value = (unsigned)nprocs;
	initial_value =2;
	printf("Waiting for %u processes\n",initial_value); 
	wait = sem_open(waiting_sem_name, default_mode, 0644, initial_value); 
	if (wait == SEM_FAILED) {
		perror("Failed to get semaphore");
		sem_close(wait); 
		exit(-1); 
	}

	go = sem_open(go_sem_name, default_mode, 0644, 0); 
	if (go == SEM_FAILED) {
		perror("Failed to get semaphore");
		sem_close(go); 
		exit(-1); 
	}

	sem_t *running = sem_open(running_sem_name, default_mode, 0644, initial_value); 
	if (go == SEM_FAILED) {
		perror("Failed to get semaphore");
		sem_close(go); 
		exit(-1); 
	}
	
	printf("Waiting for child processes to reach ROI\n"); 
	int waiting_value=-1;
	int old_value=1;
	while (waiting_value != 0) {
		// poll
		sem_getvalue(wait, &waiting_value); 
		if (old_value != waiting_value) {
			printf("value changed to %d\n",waiting_value); 
			old_value=waiting_value;
		}
	}

	printf("Everyone is done! starting up\n");
	roi_start();
	sem_post(go); 

	sem_unlink(waiting_sem_name); 
	sem_unlink(go_sem_name);
	sem_unlink(running_sem_name); 

	return 0;
};
