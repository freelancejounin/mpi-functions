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

int reduction(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
	int d, dimensions, me, mask, i;
	MPI_Status status;
	MPI_Comm tempComm;
	//me = iproc;
	
	
	for (i = 0; i < nproc; i++) {
		((int*)sendbuf)[i] = ((int*)recvbuf)[i];
	}
	
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
	//mask = (int)pow(2,dimensions) - 1 - 1;
	
	
	
	mask = 0;
	
	for (d = 0; d < dimensions; d++) {
		
		if ((root & (int)pow(2,d)) == 0) {
		
		//fprintf(stderr,"\n%d as (%d)'s mask & me: %d",iproc,me,mask & me);

		
			if ((mask & me) == 0) {		// should i be involved?
			
				//fprintf(stderr,"\n%d's me(%d) & (int)pow(2,d)[%d]: %d",iproc,me,(int)pow(2,d),me & (int)pow(2,d));
				if ((iproc ^ (int)pow(2,d)) < nproc) {	// does my comm partner exist?
			
					if ((me & (int)pow(2,d)) != 0) {
						//fprintf(stderr,"(%d) as %d sending %d to (%d) as %d\n",iproc,me,((int*)sendbuf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Send(sendbuf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD);
					}
					else {
						//fprintf(stderr,"(%d) as %d waiting to receive from (%d) as %d\n",iproc,me,iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Recv(recvbuf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD,&status);
						//fprintf(stderr,"(%d) as %d received %d from (%d) as %d\n",iproc,me,((int*)recvbuf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						for (i = 0; i < count; i++) {
							((int*)sendbuf)[i] += ((int*)recvbuf)[i];
						}
						//fprintf(stderr,"(%d) as %d, answer thus far: %d\n",iproc,me,((int*)sendbuf)[0]);
						//fflush(stderr);
					}
				}
			
			}
			else {
				//fprintf(stderr,"\n%d(%d) failed mask & me test: %d",iproc,me,mask & me);
			}
			//mask = mask ^ (int)pow(2,d+1);
		
			mask = mask ^ (int)pow(2,d);
		}
	}
	
	//mask = 0;		// resetting causes death for: 56 nodes, reduce to 47, size 1
	
	for (d = 0; d < dimensions; d++) {
		
		if ((root & (int)pow(2,d)) == (int)pow(2,d)) {
			
			//fprintf(stderr,"\n%d as (%d)'s mask & me: %d",iproc,me,mask & me);
			
			
			if ((mask & me) == 0) {		// should i be involved?
				
				//fprintf(stderr,"\n%d's me(%d) & (int)pow(2,d)[%d]: %d",iproc,me,(int)pow(2,d),me & (int)pow(2,d));
				if ((iproc ^ (int)pow(2,d)) < nproc) {	// does my comm partner exist?
					
					if ((me & (int)pow(2,d)) != 0) {
						//fprintf(stderr,"(%d) as %d sending %d to (%d) as %d\n",iproc,me,((int*)sendbuf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Send(sendbuf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD);
					}
					else {
						//fprintf(stderr,"(%d) as %d waiting to receive from (%d) as %d\n",iproc,me,iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						MPI_Recv(recvbuf,count,datatype,iproc ^ (int)pow(2,d),0,MPI_COMM_WORLD,&status);
						//fprintf(stderr,"(%d) as %d received %d from (%d) as %d\n",iproc,me,((int*)recvbuf)[0],iproc ^ (int)pow(2,d),me ^ (int)pow(2,d));
						//fflush(stderr);
						for (i = 0; i < count; i++) {
							((int*)sendbuf)[i] += ((int*)recvbuf)[i];
						}
						//fprintf(stderr,"(%d) as %d, answer thus far: %d\n",iproc,me,((int*)sendbuf)[0]);
						//fflush(stderr);
					}
				}
				
			}
			else {
				//fprintf(stderr,"\n%d(%d) failed mask & me test: %d",iproc,me,mask & me);
			}
			//mask = mask ^ (int)pow(2,d+1);
		
			mask = mask ^ (int)pow(2,d);
		}
	}
	
	
		
	
}


int main(int argc, char** argv) {
	int i,root,size;
	int* senbuf;
	int* ansbuf;
	
	double starttime, finishtime, runtime;
	
	//starttime = When();
	
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &iproc);
	
	if (argc < 3)
		exit(0);
	
	root = atoi(argv[1]);
	size = atoi(argv[2]);

	//fprintf(stderr,"root is %d\n",root);
	//fprintf(stderr,"size is %d\n",size);
	
	senbuf = (int*)malloc(size * sizeof(int));
	ansbuf = (int*)malloc(size * sizeof(int));
	for (i = 0; i < size; i++) {
		senbuf[i] = 1;
		ansbuf[i] = 1;
	}
	if (size > 1) {
		senbuf[size - 1] = 2;
		ansbuf[size - 1] = 2;
	}
	
	
	//fprintf(stderr,"\nI am %d\n",iproc);
	
	starttime = When();
	
	reduction(senbuf, ansbuf, size, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);
	
	//fprintf(stderr,"I am %d and this is the message I got: %d\n",iproc,senbuf[0]);

	
	if (iproc == root) {
		finishtime = When();
		runtime = finishtime - starttime;
		fprintf(stdout,"%d says time taken: %f with answer: %d,%d\n", iproc,runtime,senbuf[0],senbuf[size - 1]);
	}
	
	free(senbuf);
	free(ansbuf);
	
	
	MPI_Finalize();
	
	
}

