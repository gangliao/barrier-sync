#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: The scalable, distributed dissemination barrier with only local spinning.

    type flags = record
        myflags : array [0..1] of array [0..LogP - 1] of Boolean
	partnerflags : array [0..1] of array [0..LogP - 1] of ^Boolean
	
    processor private parity : integer := 0
    processor private sense : Boolean := true
    processor private localflags : ^flags

    shared allnodes : array [0..P-1] of flags
        //allnodes[i] is allocated in shared memory
	//locally accessible to processor i

    //on processor i, localflags points to allnodes[i]
    //initially allnodes[i].myflags[r][k] is false for all i, r, k
    //if j = (i+2^k) mod P, then for r = 0 , 1:
    //    allnodes[i].partnerflags[r][k] points to allnodes[j].myflags[r][k]

    procedure dissemination_barrier
        for instance : integer :0 to LogP-1
	    localflags^.partnerflags[parity][instance]^ := sense
	    repeat until localflags^.myflags[parity][instance] = sense
	if parity = 1
	    sense := not sense
	parity := 1 - parity
*/


static int P;

static inline int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

void gtmpi_init(int num_threads) { P = num_threads; }

void gtmpi_barrier(){
  int pe, i;

  MPI_Comm_rank(MPI_COMM_WORLD, &pe);

  for (i = 1; i < P; i <<= 1) {
    MPI_Isend(NULL, 0, MPI_INT, mod(pe + i, P), 0, MPI_COMM_WORLD, NULL);
    MPI_Recv( NULL, 0, MPI_INT, mod(pe - i, P), 0, MPI_COMM_WORLD, NULL);
  }
}

void gtmpi_finalize() {}
