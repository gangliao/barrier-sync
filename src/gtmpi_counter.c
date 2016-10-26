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

/**
 * Tiny changes to improve the performance of central barrier:
 *  1. Change to one master node and other slaves, let slave nodes to send msg to master node.
 *  2. Then, master node send msg to slave nodes, which reduce network communication O(N^2) to O(N). 
 *  3. Do not allocate static status array to save O(N) space.
 */

static int P;

void gtmpi_init(int num_threads) { P = num_threads; }

void gtmpi_barrier(){
  int vpid, i;

  MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
  MPI_Status stats;
  MPI_Request req;  
  if (vpid != 0) {
    MPI_Isend(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &req);
    MPI_Recv(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &stats);
  } else {
    for(i = 1; i < P; i++)
      MPI_Recv(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &stats);
    /// after receive all process msg, then singal them by process 0
    for(i = 1; i < P; i++)
      MPI_Isend(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &req);
  }
}

void gtmpi_finalize() {}

