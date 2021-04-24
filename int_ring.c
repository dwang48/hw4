//use mpirun -np p ./int_ring N to run

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>

#include <unistd.h>
#include "util.h"
#include <mpi.h>

int main (int argc, char **argv)
{
  
  int i, P, N, M, target, rank, source;      
  int label = 99;             
  int *message;                                                 
  MPI_Status status;                                        

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if(argc > 1 && atoi(argv[1]) > 0){
  	N=atoi(argv[1]);
  }else{
  	N=1;
  };     

  if(argc > 2 && atoi(argv[2]) > 0){
  	M=atoi(argv[2]);
  }else{
  	M=1;
  };

  message =  (int *) malloc(sizeof(int) * M);
  message[0]=0;
  if (P < 2) { 
  		MPI_Abort(MPI_COMM_WORLD,1); 
  	}       

  

  

  if (rank == 0){
      int total = N * P*(P-1)/2;
      timestamp_type tic, toc;
      static double tictoc;
      get_timestamp(&tic);
	  printf("Message Length: %d \n", M);
	  printf("Message Size: %f MB \n\n", M*sizeof(int)/1024./1024);
      printf("\nNumber of Processors: %d \nNumber of Loops: %d \n", P, N);
      
      

      source = P-1;
      target = 1;
      MPI_Send(message, M, MPI_INT, target, label, MPI_COMM_WORLD);

      for (i=0; i<N-1; i++)
      {
          MPI_Recv(message, M, MPI_INT, source, label, MPI_COMM_WORLD, &status);
          MPI_Send(message, M, MPI_INT, target, label, MPI_COMM_WORLD);
      }

      MPI_Recv(message, M, MPI_INT, source, label, MPI_COMM_WORLD, &status);
      get_timestamp(&toc);
      tictoc = timestamp_diff_in_seconds(tic, toc);

      printf("(Source,Target):(%d, %d), Message: %d\n", source, rank, message[0]);
      printf((message[0]==total)? "Success\n\n" : "Failure %d\n\n", total);
      printf("Time  %f seconds\n", tictoc);
      printf("Latency %f seconds\n\n", tictoc/N/P);
  }
  else
  {   
  	  target = (rank+1)%P;
      source = rank-1;
      

      for (i=0; i<N; i++)
      {
          MPI_Recv(message, M, MPI_INT, source, label, MPI_COMM_WORLD, &status);
          if (i == 0)
          {
              printf("(Source,Target):(%d, %d), Message: %d\n", source, rank, message[0]);
              printf((rank==P-1 && N>1)? "Cycling ...\n": "" );
          }
          message[0] += rank;
          MPI_Send(message, M, MPI_INT, target, label, MPI_COMM_WORLD);
      }
  }

  free(message);
  MPI_Finalize();
  return 0;
}
