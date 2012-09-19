#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

int iproc, nproc;

/* Return the current time in seconds, using a double precision number. */
double When()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

int broadcast(void* buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
	int d, dimensions, me, mask;
	MPI_Status status;
	MPI_Comm tempComm;
	//me = iproc;
	
	me = (iproc ^ root);		// translate everyone's numbers
	//MPI_Comm_split(MPI_COMM_WORLD,0,me,&tempComm);	// make the translation in the comm
	
	
	dimensions = (int)(log(nproc)/log(2));
	if (dimensions < (log(nproc)/log(2))) {
		dimensions++;
	}
	//dimensions = 2;
	//fprintf(stderr,"\nDimensions: %d",dimensions);
	//fprintf(stderr,"\nNProc: %d",nproc);
	//fprintf(stderr,"\nLog(nproc)/log(2): %f)",log(nproc)/log(2));
	//fprintf(stderr,"\nLog(nproc)/log(2) as int: %d", (int)(log(nproc)/log(2)));
	
	//mask = dimensions;
	mask = (int)pow(2,dimensions) - 1;
	
	for (d = 0; d < dimensions; d++) {
		
		
		//fprintf(stderr,"\n%d's mask & me: %d",me,mask & me);
		if ((root & (int)pow(2,d)) == (int)pow(2,d)) {
		
			
			mask = mask ^ (int)pow(2,d);
			
			if ((mask & me) == 0) {		// should i be involved?
			
				//fprintf(stderr,"\n%d's me(%d) & (int)pow(2,d)[%d]: %d",iproc,me,(int)pow(2,d),me & (int)pow(2,d));
				if ((iproc ^ (int)pow(2,d)) < nproc) {	// does my comm partner exist?
			
					if ((me & (int)pow(2,d)) == 0) {
						//fprintf(stderr,"(%d) as %d sending %d to (%d) as %d\n",iproc,me,((int*)buf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Send(buf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD);
					}
					else {
						//fprintf(stderr,"(%d) as %d waiting to receive from (%d) as %d\n",iproc,me,iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Recv(buf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD,&status);
						//fprintf(stderr,"(%d) as %d received %d from (%d) as %d\n",iproc,me,((int*)buf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
					}
				}
			
			}
			else {
				//fprintf(stderr,"\n%d(%d) failed mask & me test: %d",iproc,me,mask & me);
			}
		}
		//mask = mask ^ (int)pow(2,d);
	}
	
	//mask = (int)pow(2,dimensions) - 1;		// don't reset the mask between loops, keep the same gates open
	
	for (d = 0; d < dimensions; d++) {
		
		//fprintf(stderr,"\n%d's mask & me: %d",me,mask & me);
		if ((root & (int)pow(2,d)) == 0) {
			mask = mask ^ (int)pow(2,d);
		
			if ((mask & me) == 0) {		// should i be involved?
			
				//fprintf(stderr,"\n%d's me(%d) & (int)pow(2,d)[%d]: %d",iproc,me,(int)pow(2,d),me & (int)pow(2,d));
				if ((iproc ^ (int)pow(2,d)) < nproc) {	// does my comm partner exist?
				
					if ((me & (int)pow(2,d)) == 0) {
						//fprintf(stderr,"(%d) as %d sending %d to (%d) as %d\n",iproc,me,((int*)buf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Send(buf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD);
					}
					else {
						//fprintf(stderr,"(%d) as %d waiting to receive from (%d) as %d\n",iproc,me,iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Recv(buf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD,&status);
						//fprintf(stderr,"(%d) as %d received %d from (%d) as %d\n",iproc,me,((int*)buf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
					}
				}
			
			}
			else {
				//fprintf(stderr,"\n%d(%d) failed mask & me test: %d",iproc,me,mask & me);
			}
		}
		//mask = mask ^ (int)pow(2,d);
	}
	
	
}


int main(int argc, char** argv) {
	int i,root,size;
	int* pass;
	double starttime, finishtime, runtime;
	
	//starttime = When();
	
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &iproc);
	
	
	root = atoi(argv[1]);
	size = atoi(argv[2]);
	
	//fprintf(stderr,"root is %d\n",root);
	//fprintf(stderr,"size is %d\n",size);
	
	
	//fprintf(stderr,"\nI am %d\n",iproc);
	
	pass = (int*)malloc(size * sizeof(int));
	for (i = 0; i < size; i++) {
		pass[i] = iproc;
	}
	if (size > 1) {
		pass[size - 1] = 7;
	}
	//pass[0] = iproc;
	
	starttime = When();
	
	broadcast(pass, size, MPI_INT, root, MPI_COMM_WORLD);
	
	//fprintf(stderr,"I am %d and this is the message I got: %d,%d\n",iproc,pass[0],pass[size - 1]);
	
	if (iproc == root) {
		finishtime = When();
		runtime = finishtime - starttime;
		fprintf(stdout,"%d says time taken: %f\n", iproc,runtime);
	}
	
	free(pass);
	
	MPI_Finalize();
		
}

