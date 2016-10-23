#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A sense-reversing centralized barrier

    shared count : integer := P
    shared sense : Boolean := true
    processor private local_sense : Boolean := true

    procedure central_barrier
        local_sense := not local_sense // each processor toggles its own sense
	if fetch_and_decrement (&count) = 1
	    count := P
	    sense := local_sense // last processor toggles global sense
        else
           repeat until sense = local_sense
*/


static MPI_Status status;
static int P;

void gtmpi_init(int num_threads){ P = num_threads; }

void gtmpi_barrier(){
  int vpid, i;

  MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
  
  if (vpid != 0) {
    MPI_Isend(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
    MPI_Recv(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
  } else {
    for(i = 1; i < P; i++)
      MPI_Recv(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
    for(i = 1; i < P; i++)
      MPI_Isend(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, NULL);
  }
}

void gtmpi_finalize() {}

