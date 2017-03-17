# OS2-Minix

Project for CS3523(Spring-17)

Directories for every file to be put into in Minix 3.2.1
/usr/src/lib/libc/sys-minix:
        _semaphore.c
        lockcond.c
        Makefile.inc

/usr/src/servers/pm:
        proto.h
        table.c
        semaphore.c
        Makefile

/usr/src/include/minix:
        callnr.h
        

/usr/include/minix:
        callnr.h


In Makefile and Makefile.inc, the only change is to include semaphore.c and _semaphore.c in the  SRCS field respectively.

The next change is to open unistd.h in /usr/src/include and add the function prototypes for each of the functions  implemented in _semaphore.c . After that, move to /usr/src directory and  enter "make build" in the terminal. It will take around 15 mins to build the whole system again and  make a new  boot image. Reboot the OS.

The rest of the files, are used for testing the semaphores. prod_cons.c is the server program used for  creating np producers and nc consumers and  running them. The producer.c and consumer.c are used to simulate  producer and consumer processes  with the arguments as specified in the question. The parameters m1 and m2 are used by the producer and consumer processes to sleep  for m1 and m2 seconds respectively. The output of the whole  prod_cons.c program is  put into a porc_log.txt using  output pipe command ">".
In all of these three files, make sure to include these libraries
#include<lib.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

The output is tested for  different  values of m1/m2. The description of the output is put in the report. 

How to  run the test cases:-
In minix, only clang compiler is present. 
1)To compile CS15BTECH11044_prod_cons.c, enter "cc CS15BTECH11044_prod_cons.c -o pc" in the terminal.
2)To compile CS15BTECH11044_producer.c, enter "cc CS15BTECH11044_producer.c -o producer -lm" in the terminal. The -lm flag is used for dynamically linking math.h module for clang compiler
3)To compile CS15BTECH11044_consumer.c, enter "cc CS15BTECH11044_consumer.c -o consumer -lm" in the terminal. The -lm flag is used for dynamically linking math.h module for clang compiler       
4)Prepare an input.txt document which has "(max number in buffer) (number of producers) (number of consumers) (loop count for producer process) (loop count for consumer process) (mean time for producer) (mean time for consumer)" as the only content in the files.
5) To run the CS15BTECH11044_prod_cons.c, and to log the outputs in a file, enter "./pc < input.txt > output.txt" in the terminal. Check output.txt to see the results of the producer_consumer problem
