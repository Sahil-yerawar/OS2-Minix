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
#define NUM_SEMS       100
#define MAX_SEM_PLIST  1000
#define NO_SEM        -1
#define INIT_SEMVAL    1000
#define MAX_SEMVAL     1000000
#define INT_LIMIT      0x7fffffff
#define ESEMVAL        0x8000000
#define NO_PROC_NR     0
#define RETERR         0


//FORWARD _PROTOTYPE( int sem_exists, (int sem));
//FORWARD _PROTOTYPE( void init_sems, (void));


struct semaphore{
	int id,value;
	int plist[MAX_SEM_PLIST];
	int begin;
	int end;
	int pcount;
}semarray[NUM_SEMS];
int semcount = 0;

 void init_sems (void){
	int i;
	for(i = 0; i< NUM_SEMS; ++i){
		semarray[i].id = NO_SEM;
		semarray[i].value = 0;
		semarray[i].pcount = 0;
		semarray[i].plist[0] = NO_PROC_NR;
		semarray[i].begin = 0;
		semarray[i].end = 0;
	}
	semcount = 0;
}

 int sem_exists(sem){
	if(sem <= 0 || sem > INT_LIMIT - 1) return 0;

	if(semcount < 1) return 0;

	int i;
	for(i = 0; i < NUM_SEMS;++i){
		if(semarray[i].id == sem) return 1;
	}
	return 0;
}

 int do_semval(void){
	int exitstatus = ESEMVAL;
	int sem = m_in.m1_i1;

	if(sem <= 0 || sem > INT_LIMIT - 1 || semcount < 1) return ESEMVAL;
        int index;
	index = (sem%NUM_SEMS) -1;
	if(index < 0) index = NUM_SEMS - 1;
	int i;
	for(i = 0; i < NUM_SEMS && exitstatus == ESEMVAL;++i){
		if(semarray[index].id == sem) exitstatus = semarray[index].value;
		else if(index == (NUM_SEMS-1)) index = 0;
		else ++index;
	}

	if(exitstatus ==  ESEMVAL) return ESEMVAL;

	return exitstatus;
}


 int do_seminit(void){
	if(semcount == 0) init_sems();

	int exitstatus = RETERR;

	int sem = m_in.m1_i1;
	int value = m_in.m1_i2;

	if(semcount >= NUM_SEMS){
		m_in.m_type = EAGAIN;
		return RETERR;
	}

	if(sem < 0 || sem > INT_LIMIT -1 || value > INIT_SEMVAL || value < -INIT_SEMVAL){
		m_in.m_type = EINVAL;
		return RETERR;
	}

		
	if(sem != 0 && sem_exists(sem)){
		m_in.m_type = EEXIST;
		return RETERR;
	}

	int index;
	if(sem == 0){
		int i;
		for(i = 0;i<NUM_SEMS && exitstatus == RETERR;++i){
			if(semarray[i].id == NO_SEM){
				index = i;
				sem = i+1;

				while(sem_exists(sem)) sem += 100;
				++semcount;
				exitstatus = sem;

				semarray[i].id = sem;
				semarray[i].value = value;

				semarray[i].pcount = 0;
				semarray[i].plist[0] = NO_PROC_NR;
				semarray[i].begin = 0;
				semarray[i].end = 0;
			}
		}
	}
	else{
		index = ((sem % NUM_SEMS)-1);
		if(index < 0) index = NUM_SEMS - 1;
		int j;
		for(j=0;j<NUM_SEMS && exitstatus == RETERR;++j){
			if(semarray[index].id == NO_SEM){
				++semcount;
				exitstatus = sem;

				semarray[index].id = sem;
				semarray[index].value = value;

				semarray[index].pcount = 0;
				semarray[index].plist[0] = NO_PROC_NR;
				semarray[index].begin = 0;
				semarray[index].end = 0;
			}
			else if(index == (NUM_SEMS-1))index = 0;
			else ++index;
		}
	}
	return exitstatus;
}

 int do_semup(void){
	int exitstatus = RETERR;
	int sem = m_in.m1_i1;

	if(sem <= 0 || sem > INT_LIMIT -1 || semcount < 1){
		m_in.m_type = EINVAL;
		return RETERR;
	}
	int index;
	index = (sem % NUM_SEMS) - 1;
	if (index < 0) index = NUM_SEMS - 1;
	int i;
	for(i = 0; i< NUM_SEMS && exitstatus == RETERR;++i){
		if(semarray[index].id == sem) exitstatus = 1;
		else if (index == (NUM_SEMS-1))index = 0;
		else ++index;
	}

	if(exitstatus == RETERR){
		return RETERR;
	}

	if(semarray[index].value == MAX_SEMVAL){
		m_in.m_type = EOVERFLOW;
		return RETERR;
	}

	++(semarray[index].value);
	if(semarray[index].pcount != 0){
		int next = semarray[index].begin;
		int proc_nr = semarray[index].plist[next];

		if(proc_nr <= NO_PROC_NR || proc_nr >= NR_PROCS) return RETERR;
		if(next == semarray[index].end){
			semarray[index].plist[next] = NO_PROC_NR;
			semarray[index].begin = 0;
			semarray[index].end =0;
		}
		else if(next == (MAX_SEM_PLIST -1)){
			semarray[index].plist[MAX_SEM_PLIST-1] = NO_PROC_NR;
			semarray[index].begin = 0;
			
		}
		else{
			semarray[index].plist[next] = NO_PROC_NR;
			++next;
			semarray[index].begin = next;
		}

		--(semarray[index].pcount);

		register struct mproc *rmp = &mproc[proc_nr];
		setreply (proc_nr, exitstatus);
	}
	return exitstatus;
}


 int do_semdown (void){
	int exitstatus = RETERR;

	int sem = m_in.m1_i1;

	if(sem<=0 || sem > INT_LIMIT - 1 ||semcount < 1){
		m_in.m_type = EINVAL;
		return RETERR;
	}
	int index;
	index = (sem % NUM_SEMS)-1;
	if(index < 0) index = NUM_SEMS - 1;
	int i;
	for(i = 0; i< NUM_SEMS && exitstatus == RETERR; ++i){
		if(semarray[index].id == sem) exitstatus = 1;
		else if (index == (NUM_SEMS - 1)) index = 0;
		else ++index;
	}

	if(exitstatus == RETERR){
		return RETERR;
	}

	if(semarray[index].value == -MAX_SEMVAL){
		m_in.m_type = EOVERFLOW;
		return RETERR;
	}

	if(semarray[index].pcount == MAX_SEM_PLIST){
		m_in.m_type = EOVERFLOW;
		return RETERR;
	}
	--(semarray[index].value);

	if(semarray[index].value < 0){
		int last = semarray[index].end;
		int proc_nr = who_p;

		if(semarray[index].pcount == 0){
			semarray[index].plist[last] = proc_nr;
		}
		else if(last == (MAX_SEM_PLIST-1)){
		 	last = 0;
			semarray[index].end = last;
			semarray[index].plist[last] = proc_nr;
		}
		else{
			++last;
			semarray[index].end = last;
			semarray[index].plist[last] = proc_nr;
		}

		++(semarray[index].pcount);

		return (SUSPEND);
	}
	return exitstatus;
}


int do_semfree (void){
	int exitstatus = RETERR;
	int sem = m_in.m1_i1;
	if (sem <= 0 || sem > INT_LIMIT -1){
		m_in.m_type = EINVAL;
		return RETERR;
	}

	int index;
	index = (sem%NUM_SEMS) -1;
	if (index < 0) index = NUM_SEMS-1;
	int i;
	for(i=0;i<NUM_SEMS && exitstatus == RETERR; ++i){
		if(semarray[index].id == sem) exitstatus = 1;
		else if (index == (NUM_SEMS - 1))index = 0;
		else ++index;
	}

	if(exitstatus == RETERR){
		m_in.m_type = EEXIST;
		return RETERR;
	}

	if(semarray[index].pcount != 0){
		m_in.m_type = EBUSY;
		return RETERR;
	}

	--semcount;
	semarray[index].id = NO_SEM;
	semarray[index].value = 0;
	semarray[index].pcount = 0;
	semarray[index].plist[0] = NO_PROC_NR;
	semarray[index].begin = 0;
	semarray[index].pcount = 0;

	return exitstatus;
}



