#include<stdio.h>
#include<lib.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>

int main(int argc, char* argv[]){
	int cnp = atoi(argv[1]);
	int m1 = atoi(argv[2]);
	int max = atoi(argv[3]);
	int ina,pid = getpid(),empty = 0,full=0,mutex=0;
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
	for(int i = 0;i<cnp;i++){
		semdown(empty);
		semdown(mutex);
		in = fopen("PCin","r+");
		fscanf(in,"%d", &ina);
		ina = (ina+1)%max;
		fprintf(in,"%d\n", ina);
		fclose(in);
		semup(mutex);
		semup(full);
		sleep(m1);
	}

	return 0;
}


		

