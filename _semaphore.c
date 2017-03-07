#include<sys/cdefs.h>
#include"namespace.h"
#include<lib.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<minix/callnr.h>

PUBLIC int semvalue (int sem){
	message m;
	m.m1_i1 = sem;
	return (_syscall(PM_PROC_NR, SEMVAL, &m));
}

PUBLIC int seminit (int sem, int val){
	message m;
	m.m1_i1 = sem;
	m.m1_i2 = val;
	return (_syscall (PM_PROC_NR, SEMINIT, &m));
}

PUBLIC int semdown (int sem){
	message m;
	m.m1_i1 = sem;
	return (_syscall (PM_PROC_NR, SEMDOWN, &m));
}

PUBLIC int semup(int sem){
	message m;
	m.m1_i1  =sem;
	return (_syscall (PM_PROC_NR, SEMUP, &m));
}

PUBLIC int semfree (int sem){
	message m;
	m.m1_i1 = sem;
	return (_syscall (PM_PROC_NR, SEMFREE, &m));
}
