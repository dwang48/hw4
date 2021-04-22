#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <cstdlib>
/* compuate global residual, assuming ghost values are updated */
double compute_residual(double *lu, int lN, double invhsq){
  int i,j;
  double gres = 0.0, lres = 0.0, tmp, tmp_sqr;
  for (i = 1; i <= lN; i++){
	for(j=1;j<=lN;j++){
    	tmp = ((4.0*lu[i+(lN+2)*j] - lu[i-1+(lN+2)*j] - lu[i+1+(lN+2)*j] - lu[i+(lN+2)*(j-1)] - lu[i+(lN+2)*(j+1)]) * invhsq - 1);
    	tmp_sqr = tmp*tmp;
    	lres+=tmp_sqr;
	}
  }
  /* use allreduce for convenience; a reduce would also be sufficient */
  MPI_Allreduce(&lres, &gres, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  return sqrt(gres);
}


int main(int argc, char * argv[]){
  int rank, iter, max_iters, i, p, N, lN;
  MPI_Status status, status1;
  MPI_Status status2,status3;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  /* get name of host running MPI process */
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  printf("Rank %d/%d running on %s.\n", rank, p, processor_name);

  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &max_iters);

  int lblocks=int(sqrt(p));
  if(rank==0){ printf("Processors in one side:%d\n",lblocks);}

  /* compute number of unknowns handled by each process */
  lN = N / p;
  if ((N%p!=0) && rank==0 ) {
    printf("N: %d, local N: %d\n", N, lN);
    printf("Exiting. N must be a multiple of p\n");
    MPI_Abort(MPI_COMM_WORLD, 0);
  }
  /* timing */
  MPI_Barrier(MPI_COMM_WORLD);
  double tt = MPI_Wtime();

  //Variable Allocation
  
  double *rbuffs=(double*) calloc(sizeof(double), lN);
  double *rbuffr=(double*) calloc(sizeof(double), lN);
  double *lbuffs=(double*) calloc(sizeof(double), lN);
  double *lbuffr=(double*) calloc(sizeof(double), lN);
  double *ubuffs=(double*) calloc(sizeof(double), lN);
  double *ubuffr=(double*) calloc(sizeof(double), lN);
  double *bbuffs=(double*) calloc(sizeof(double), lN);
  double *bbuffr=(double*) calloc(sizeof(double), lN);
  double * lu    = (double *) calloc(sizeof(double), (lN + 2)*(lN + 2));
  double * lunew = (double *) calloc(sizeof(double), (lN + 2)*(lN + 2));
  double * lutemp;


  double gres,gres0,tol,h,h_square,invhsq;
  h = 1.0 / (N + 1);
  h_square = h * h;
  invhsq = 1./h_square;
  tol = 1e-6;

  /* initial residual */
  gres0 = compute_residual(lu, lN, invhsq);
  gres = gres0;

  
  for (iter = 0; iter < max_iters && gres/gres0 > tol; iter++) {
    /* Jacobi step for local points */
    for (long i=1;i<=lN;i++){
        for (long j=1;j<=lN;j++){
            lunew[i+(lN+2)*j]=0.25*(h_square+lu[i-1+(lN+2)*j]+lu[i+(lN+2)*(j-1)]+lu[i+1+(lN+2)*j]+lu[i+(lN+2)*(j+1)]);
        }
    }


    /* communicate ghost values */

	if (rank%lblocks!=0) {
   	  for (long j=0;j<lN;j++){
	  	lbuffs[j]=lunew[1+(lN+2)*(j+1)];
	  }

	  MPI_Send(lbuffs, lN, MPI_DOUBLE, rank-1, 123, MPI_COMM_WORLD);
      MPI_Recv(lbuffr, lN, MPI_DOUBLE, rank-1, 124, MPI_COMM_WORLD, &status1);

	  for (long j=0;j<lN;j++){
	  	lunew[0+(lN+2)*(j+1)]=lbuffr[j];
	  }
	}

	if (rank%lblocks!=lblocks-1) {
	  for (long j=0;j<lN;j++){
	  	rbuffs[j]=lunew[lN+(lN+2)*(j+1)];
	  }

      MPI_Send(rbuffs, lN, MPI_DOUBLE, rank+1, 124, MPI_COMM_WORLD);
      MPI_Recv(rbuffr, lN, MPI_DOUBLE, rank+1, 123, MPI_COMM_WORLD, &status);

	  for (long j=0;j<lN;j++){
	  	lunew[lN+1+(lN+2)*(j+1)]=rbuffr[j];
	  }
    }

    if (rank<p-lblocks) {
   	  for (long j=0;j<lN;j++){
	  	ubuffs[j]=lunew[j+1+(lN+2)*(lN)];
	  }

	  MPI_Send(ubuffs, lN, MPI_DOUBLE, rank+lblocks, 126, MPI_COMM_WORLD);
      MPI_Recv(ubuffr, lN, MPI_DOUBLE, rank+lblocks, 125, MPI_COMM_WORLD, &status2);

	  for (long j=0;j<lN;j++){
	  	lunew[j+1+(lN+2)*(lN+1)]=ubuffr[j];
	  }
	}

    if (rank>=lblocks) {
   	  for (long j=0;j<lN;j++){
	  	bbuffs[j]=lunew[j+1+(lN+2)*1];
	  }

	  MPI_Send(bbuffs, lN, MPI_DOUBLE, rank-lblocks, 125, MPI_COMM_WORLD);
      MPI_Recv(bbuffr, lN, MPI_DOUBLE, rank-lblocks, 126, MPI_COMM_WORLD, &status3);

	  for (long j=0;j<lN;j++){
	  	lunew[j+1+(lN+2)*0]=bbuffr[j];
	  }
	}



    /* copy newu to u using pointer flipping */
    lutemp = lu; lu = lunew; lunew = lutemp;
    if (iter%10==0) {
      gres = compute_residual(lu, lN, invhsq);
      if (rank==0) {
	printf("Number of Iterations:%d\nResidual:%g\n", iter, gres);
      }
    }
  }

  /* Clean up */
  free(lu);
  free(lunew);
  free(lbuffs);
  free(lbuffr);
  free(rbuffs);
  free(rbuffr);
  free(ubuffs);
  free(ubuffr);
  free(bbuffs);
  free(bbuffr);

  /* timing */
  MPI_Barrier(MPI_COMM_WORLD);
  double elapsed = MPI_Wtime() - tt;
  if (rank==0) {
    printf("Time elapsed is %f seconds.\n", elapsed);
  }
  MPI_Finalize();
  return 0;
}
