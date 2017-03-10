#include<stdio.h>
#include<time.h>
#include<lib.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

struct memory1{
	int in;
	int out;
	int buffer[200];
};
double exponential(double mean){
	double r = (double)rand()/(double)1000000000;
	double lambda = (double)1/mean; 
	return lambda*exp((-1)*lambda*r);
}
int main(int argc, char* argv[]){
	int cnp = atoi(argv[1]);
	double m1 = atoi(argv[2]);
	int max = atoi(argv[3]);
	int ina,pid = getpid(),empty = 0,full=0,mutex=0;
	time_t systime;
	unsigned long micros = 0;
	clock_t start, end;
	key_t shmkey;
	int shmid,test=0;
	struct memory1* shmptr;

	shmkey = ftok(".",'x');
	shmid = shmget(shmkey, sizeof(struct memory1), 0666);
	if (shmid < 0){
		printf("*** shmget error (consumer) ***\n");
		exit(1);
	
	}
	shmptr = (struct memory1 * )shmat(shmid,NULL, 0);
	if((int)shmptr == -1){
		printf("*** shmat error(consumer) ***\n");
		exit(1);
	}
	FILE *semfile,*in;
	semfile = fopen("semfile","r");
	if(semfile == NULL){
		fprintf(stderr,"producer: cannot open semfile");
		return -1;
	}
	fscanf(semfile, "%d", &empty);
	fscanf(semfile, "%d", &full);
	fscanf(semfile, "%d", &mutex);
	fclose(semfile);
	start  = clock();
	for(int i = 0;i<cnp;i++){
		semdown(empty);
		semdown(mutex);
		/*in = fopen("PCin","r");
		fscanf(in,"%d", &ina);
		fclose(in);*/
		shmptr->buffer[shmptr->in]=i;
		shmptr->in = (shmptr->in + 1)%max;
		systime = time(NULL);
		struct tm* l= localtime(&systime);
		end = clock();
		micros = end - start;
		printf("%dth item by producer %d from buffer location %d @ time:%02d:%02d:%02d\n", i, pid, shmptr->in,l->tm_hour,l->tm_min,l->tm_sec);
	 	//printf("%dth item by producer %d from buffer location %d @ time:%03lu\n", i, pid, shmptr->out, micros);
		
		fflush(stdout);

		/*in = fopen("PCin","w");
		fprintf(in,"%d\n", ina);
		fclose(in);*/
		semup(mutex);
		semup(full);
		//sleep(m1);
		sleep((int)(exponential((double)m1)*10000));
	}

	return 0;
}


		

