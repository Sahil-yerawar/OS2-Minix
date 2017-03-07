#include<stdio.h>
#include<lib.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/time.h>

int main(){
	FILE *in,*out,*semfile;
	int max,np,nc,cnp,cnc,m1,m2;
	int full = 0,empty = 0,mutex = 0;
	pid_t pid_p[max],pid_c[max];

	scanf("%d %d %d %d %d %d %d", &max, &np, &nc, &cnp, &cnc, &m1, &m2);
	empty = seminit(0,max);
	if(empty < 0){
		switch (errno){
			case EEXIST:
				fprintf(stderr, "prod_cons.c: empty semaphore already exists\n");
				return -1;
				break;
			case EAGAIN:
				fprintf(stderr, "prod_cons.c: cannot create empty semaphore due to max limit\n");
				return -1;
				break;
			case EINVAL:
				fprintf(stderr, "prod_cons.c: empty id is negative or value not in range\n");
				return -1;
				break;
			case -EEXIST:
				fprintf(stderr, "prod_cons.c: empty semaphore already exists\n");
				return -1;
				break;
			case -EAGAIN:
				fprintf(stderr,"prod_cons.c: cannot create empty semaphore due to max limit\n");
				return -1;
				break;
			case -EINVAL:
				fprintf(stderr, "prod_cons.c: empty id is negative or value not in range\n");
				return -1;
				break;
			default:
				fprintf(stderr, "prod_cons.c: Unknown error retuned on creation of mutex semaphore\n");
				return -1;
				break;
			}
		}
	full = seminit(0,0);
	if(full < 0){
		switch(errno){
			case EEXIST:
				fprintf(stderr,"prod_cons.c: full semaphore already exists\n");
				return -1;
				break;
			case EAGAIN:
				fprintf(stderr,"prod_cons.c:cannot create full semaphore due to max limit\n");
				return -1;
				break;
			case EINVAL:
				fprintf(stderr,"prod_cons.c: full id is negative or value not in range\n");
				return -1;
				break;
			case -EEXIST:
				fprintf(stderr,"prod_cons.c: full semaphore already exists\n");
				return -1;
				break;
			case -EAGAIN:
				fprintf(stderr,"prod_cons.c: cannot create full semaphore due to max limit\n");
				return -1;
				break;
			case -EINVAL:
				fprintf(stderr,"prod_cons.c: full id is negative or value not in range\n");
				return -1;
				break;
			default:
				fprintf(stderr, "prod_cons.c: Unknown error returned on creation of full semaphore\n");
				return -1;
				break;
			}
		}
	mutex = seminit(0,1);
	if(mutex < 0){
		switch(errno){
			case EEXIST:
				fprintf(stderr,"prod_cons.c: mutex semaphore already exists\n");
				return -1;
				break;
			case EAGAIN:
				fprintf(stderr, "prod_cons.c: cannot create mutex semaphore due to max limit\n");
				return -1;
				break;

			case EINVAL:
				fprintf(stderr,"prod_cons.c: mutex id is negative or value not in range\n");
				return -1;
				break;
			case -EEXIST:
				fprintf(stderr, "prod_cons.c: mutex semaphore already exists\n");
				return -1;
				break;
			case -EAGAIN:
				fprintf(stderr,"prod_cons.c: cannot create mutex semaphore due to max limit\n");
				return -1;
				break;
			case -EINVAL;
				fprintf(stderr,"prod_cons,c: mutex id is negative or value not in range\n");
				return -1;
				break;
			default:
				fprintf(stderr,"prod_cons.c: Unknown error returned on creation of mutex semaphore\n");
				return -1;
				break;
			}
		}
	in = fopen("PCin","w");
	fprintf(in,"0\n");
	fclose(in);

	out = fopen("PCout","w");
	fprintf(out,"0\n");
	fclose(out);
	
	semfile = fopen("semfile", "w");
	fprintf(semfile,"%d ", empty);
	fprintf(semfile,"%d ", full);
	fprintf(semfile,"%d\n" mutex);
	fclose(semfile);


	char cnpa[20],cnca[20],m1a[20],m2a[20],maxa[20];
	snprintf(cnpa,20,"%d",cnp);
	snprintf(cnca,20,"%d",cnc);
	snprintf(m1a,20,"%d",m1);
	snprintf(m2a,20,"%d",m2);
	snprintf(maxa,20,"%d",max);

	for(int i = 0; i<np;i++){
		pid_p[i] = fork();
		if(pid_p[i]<0){
			printf("Fork failed");
			return 1;
		}
		else if(pid_p[i] == 0){
			execl("./producer", "producer",cnpa,m1a,maxa,(char*)NULL);
		}
	}

	for(int i = 0; i<nc;i++){
		pid_c[i] = fork();
		if(pid_c[i] < 0){
			printf("Fork Failed");
			return 1;
		}
		else if(pid_c[i] == 0){
			execl("./consumer","consumer",cnca,m2a,maxa,(char*)NULL);
		}
	}

	for(int i = 0;i<np;i++){
		wait(NULL);
	}
	for(int i = 0; i<nc;i++){
		wait(NULL);
	}

	return 0;
}






