#include<stdio.h>
#include<lib.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>

int main(int argc, char* argv[]){
	int cnc = atoi(argv[1]);
	int m2 = atoi(argv[2]);
	int max = atoi(argv[3]);
	int outa,pid = getpid(),empty = 0,full=0,mutex=0;
	FILE *semfile,*out;
	semfile = fopen("semfile","r");
	if(semfile == NULL){
		fprintf(stderr,"producer: cannot open semfile");
		return -1;
	}
	fscanf(semfile, "%d", &empty);
	fscanf(semfile, "%d", &full);
	fscanf(semfile, "%d", &mutex);
	fclose(semfile);
	for(int i = 0;i<cnc;i++){
		semdown(full);
		semdown(mutex);
		out = fopen("PCout","r+");
		fscanf(out,"%d", &outa);
		outa = (outa+1)%max;
		fprintf(out,"%d\n", outa);
		fclose(out);
		semup(mutex);
		semup(empty);
		sleep(m2);
	}

	return 0;
}


		

