#include "pm.h"
#include "param.h"
#include "glo.h"
#include "mproc.h"
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//constants
#define MAX_SEMAPHORES       		 100								
#define MAX_PROCESSES_PER_SEMAPHORE  1000							
#define NO_SEMAPHORE        		-1
#define SEMINIT_VALUE    			 1000
#define SEMMAX_VALUE     			 1000000
#define MAX_INT      				 0x7fffffff						//maximum value for an int
#define ESEMVAL        				 0x8000000						//semaphore value error
#define NO_PROC_NR     				 0								//label to define no process on that index in the array
#define RETURN_ERROR         		 0								//label for return error	


//FORWARD _PROTOTYPE( int exist_sem, (int sem));
//FORWARD _PROTOTYPE( void sem_initialize, (void));

/**********************define semaphore structure****************************/
struct semaphore{
	int s_id,s_value;
	int plist[MAX_PROCESSES_PER_SEMAPHORE];
	int s_start;
	int s_end;
	int process_count;
}array_semaphores[MAX_SEMAPHORES];
int semaphore_count = 0;

 void sem_initialize (void){
	int i;
	for(i = 0; i< MAX_SEMAPHORES; ++i){
		array_semaphores[i].s_id = NO_SEMAPHORE;
		array_semaphores[i].s_value = 0;
		array_semaphores[i].process_count = 0;
		array_semaphores[i].process_list[0] = NO_PROC_NR;
		array_semaphores[i].s_start = 0;
		array_semaphores[i].s_end = 0;
	}
	semaphore_count = 0;
}

 int exist_sem(sem){
	if(sem <= 0 || sem > MAX_INT - 1) return 0;

	if(semaphore_count < 1) return 0;

	int i;
	for(i = 0; i < MAX_SEMAPHORES;++i){
		if(array_semaphores[i].s_id == sem) return 1;
	}
	return 0;
}

 int do_semval(void){
	int exitstatus = ESEMVAL;
	int sem = m_in.m1_i1;

	if(sem <= 0 || sem > MAX_INT - 1 || semaphore_count < 1) return ESEMVAL;
        int index;
	index = (sem%MAX_SEMAPHORES) -1;
	if(index < 0) index = MAX_SEMAPHORES - 1;
	int i;
	for(i = 0; i < MAX_SEMAPHORES && exitstatus == ESEMVAL;++i){
		if(array_semaphores[index].s_id == sem) exitstatus = array_semaphores[index].s_value;
		else if(index == (MAX_SEMAPHORES-1)) index = 0;
		else ++index;
	}

	if(exitstatus ==  ESEMVAL) return ESEMVAL;

	return exitstatus;
}


 int do_seminit(void){
	if(semaphore_count == 0) sem_initialize();

	int exitstatus = RETURN_ERROR;

	int sem = m_in.m1_i1;
	int value = m_in.m1_i2;

	if(semaphore_count >= MAX_SEMAPHORES){
		m_in.m_type = EAGAIN;
		return RETURN_ERROR;
	}

	if(sem < 0 || sem > MAX_INT -1 || value > SEMINIT_VALUE || value < -SEMINIT_VALUE){
		m_in.m_type = EINVAL;
		return RETURN_ERROR;
	}

		
	if(sem != 0 && exist_sem(sem)){
		m_in.m_type = EEXIST;
		return RETURN_ERROR;
	}

	int index;
	if(sem == 0){
		int i;
		for(i = 0;i<MAX_SEMAPHORES && exitstatus == RETURN_ERROR;++i){
			if(array_semaphores[i].s_id == NO_SEMAPHORE){
				index = i;
				sem = i+1;

				while(exist_sem(sem)) sem += 100;
				++semaphore_count;
				exitstatus = sem;

				array_semaphores[i].s_id = sem;
				array_semaphores[i].s_value = value;

				array_semaphores[i].process_count = 0;
				array_semaphores[i].process_list[0] = NO_PROC_NR;
				array_semaphores[i].s_start = 0;
				array_semaphores[i].s_end = 0;
			}
		}
	}
	else{
		index = ((sem % MAX_SEMAPHORES)-1);
		if(index < 0) index = MAX_SEMAPHORES - 1;
		int j;
		for(j=0;j<MAX_SEMAPHORES && exitstatus == RETURN_ERROR;++j){
			if(array_semaphores[index].s_id == NO_SEMAPHORE){
				++semaphore_count;
				exitstatus = sem;

				array_semaphores[index].s_id = sem;
				array_semaphores[index].s_value = value;

				array_semaphores[index].process_count = 0;
				array_semaphores[index].process_list[0] = NO_PROC_NR;
				array_semaphores[index].s_start = 0;
				array_semaphores[index].s_end = 0;
			}
			else if(index == (MAX_SEMAPHORES-1))index = 0;
			else ++index;
		}
	}
	return exitstatus;
}

 int do_semrelease(void){
	int exitstatus = RETURN_ERROR;
	int sem = m_in.m1_i1;

	if(sem <= 0 || sem > MAX_INT -1 || semaphore_count < 1){
		m_in.m_type = EINVAL;
		return RETURN_ERROR;
	}
	int index;
	index = (sem % MAX_SEMAPHORES) - 1;
	if (index < 0) index = MAX_SEMAPHORES - 1;
	int i;
	for(i = 0; i< MAX_SEMAPHORES && exitstatus == RETURN_ERROR;++i){
		if(array_semaphores[index].s_id == sem) exitstatus = 1;
		else if (index == (MAX_SEMAPHORES-1))index = 0;
		else ++index;
	}

	if(exitstatus == RETURN_ERROR){
		return RETURN_ERROR;
	}

	if(array_semaphores[index].s_value == SEMMAX_VALUE){
		m_in.m_type = EOVERFLOW;
		return RETURN_ERROR;
	}

	++(array_semaphores[index].s_value);
	if(array_semaphores[index].process_count != 0){
		int next = array_semaphores[index].s_start;
		int proc_nr = array_semaphores[index].process_list[next];

		if(proc_nr <= NO_PROC_NR || proc_nr >= NR_PROCS) return RETURN_ERROR;
		if(next == array_semaphores[index].s_end){
			array_semaphores[index].process_list[next] = NO_PROC_NR;
			array_semaphores[index].s_start = 0;
			array_semaphores[index].s_end =0;
		}
		else if(next == (MAX_PROCESSES_PER_SEMAPHORE -1)){
			array_semaphores[index].process_list[MAX_PROCESSES_PER_SEMAPHORE-1] = NO_PROC_NR;
			array_semaphores[index].s_start = 0;
			
		}
		else{
			array_semaphores[index].process_list[next] = NO_PROC_NR;
			++next;
			array_semaphores[index].s_start = next;
		}

		--(array_semaphores[index].process_count);

		register struct mproc *rmp = &mproc[proc_nr];
		setreply (proc_nr, exitstatus);
	}
	return exitstatus;
}


 int do_semacquire (void){
	int exitstatus = RETURN_ERROR;

	int sem = m_in.m1_i1;

	if(sem<=0 || sem > MAX_INT - 1 ||semaphore_count < 1){
		m_in.m_type = EINVAL;
		return RETURN_ERROR;
	}
	int index;
	index = (sem % MAX_SEMAPHORES)-1;
	if(index < 0) index = MAX_SEMAPHORES - 1;
	int i;
	for(i = 0; i< MAX_SEMAPHORES && exitstatus == RETURN_ERROR; ++i){
		if(array_semaphores[index].s_id == sem) exitstatus = 1;
		else if (index == (MAX_SEMAPHORES - 1)) index = 0;
		else ++index;
	}

	if(exitstatus == RETURN_ERROR){
		return RETURN_ERROR;
	}

	if(array_semaphores[index].s_value == -SEMMAX_VALUE){
		m_in.m_type = EOVERFLOW;
		return RETURN_ERROR;
	}

	if(array_semaphores[index].process_count == MAX_PROCESSES_PER_SEMAPHORE){
		m_in.m_type = EOVERFLOW;
		return RETURN_ERROR;
	}
	--(array_semaphores[index].s_value);

	if(array_semaphores[index].s_value < 0){
		int last = array_semaphores[index].s_end;
		int proc_nr = who_p;

		if(array_semaphores[index].process_count == 0){
			array_semaphores[index].process_list[last] = proc_nr;
		}
		else if(last == (MAX_PROCESSES_PER_SEMAPHORE-1)){
		 	last = 0;
			array_semaphores[index].s_end = last;
			array_semaphores[index].process_list[last] = proc_nr;
		}
		else{
			++last;
			array_semaphores[index].s_end = last;
			array_semaphores[index].process_list[last] = proc_nr;
		}

		++(array_semaphores[index].process_count);

		return (SUSPs_end);
	}
	return exitstatus;
}


int do_semfree (void){
	int exitstatus = RETURN_ERROR;
	int sem = m_in.m1_i1;
	if (sem <= 0 || sem > MAX_INT -1){
		m_in.m_type = EINVAL;
		return RETURN_ERROR;
	}

	int index;
	index = (sem%MAX_SEMAPHORES) -1;
	if (index < 0) index = MAX_SEMAPHORES-1;
	int i;
	for(i=0;i<MAX_SEMAPHORES && exitstatus == RETURN_ERROR; ++i){
		if(array_semaphores[index].s_id == sem) exitstatus = 1;
		else if (index == (MAX_SEMAPHORES - 1))index = 0;
		else ++index;
	}

	if(exitstatus == RETURN_ERROR){
		m_in.m_type = EEXIST;
		return RETURN_ERROR;
	}

	if(array_semaphores[index].process_count != 0){
		m_in.m_type = EBUSY;
		return RETURN_ERROR;
	}

	--semaphore_count;
	array_semaphores[index].s_id = NO_SEMAPHORE;
	array_semaphores[index].s_value = 0;
	array_semaphores[index].process_count = 0;
	array_semaphores[index].process_list[0] = NO_PROC_NR;
	array_semaphores[index].s_start = 0;
	array_semaphores[index].process_count = 0;

	return exitstatus;
}



