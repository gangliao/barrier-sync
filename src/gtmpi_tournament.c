#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A scalable, distributed tournament barrier with only local spinning

    type round_t = record
        role : (winner, loser, bye, champion, dropout)
	opponent : ^Boolean
	flag : Boolean
    shared rounds : array [0..P-1][0..LogP] of round_t
        // row vpid of rounds is allocated in shared memory
	// locally accessible to processor vpid

    processor private sense : Boolean := true
    processor private vpid : integer // a unique virtual processor index

    //initially
    //    rounds[i][k].flag = false for all i,k
    //rounds[i][k].role = 
    //    winner if k > 0, i mod 2^k = 0, i + 2^(k-1) < P , and 2^k < P
    //    bye if k > 0, i mode 2^k = 0, and i + 2^(k-1) >= P
    //    loser if k > 0 and i mode 2^k = 2^(k-1)
    //    champion if k > 0, i = 0, and 2^k >= P
    //    dropout if k = 0
    //    unused otherwise; value immaterial
    //rounds[i][k].opponent points to 
    //    round[i-2^(k-1)][k].flag if rounds[i][k].role = loser
    //    round[i+2^(k-1)][k].flag if rounds[i][k].role = winner or champion
    //    unused otherwise; value immaterial
    procedure tournament_barrier
        round : integer := 1
	loop   //arrival
	    case rounds[vpid][round].role of
	        loser:
	            rounds[vpid][round].opponent^ :=  sense
		    repeat until rounds[vpid][round].flag = sense
		    exit loop
   	        winner:
	            repeat until rounds[vpid][round].flag = sense
		bye:  //do nothing
		champion:
	            repeat until rounds[vpid][round].flag = sense
		    rounds[vpid][round].opponent^ := sense
		    exit loop
		dropout: // impossible
	    round := round + 1
	loop  // wakeup
	    round := round - 1
	    case rounds[vpid][round].role of
	        loser: // impossible
		winner:
		    rounds[vpid[round].opponent^ := sense
		bye: // do nothing
		champion: // impossible
		dropout:
		    exit loop
	sense := not sense
*/

static int P;
void gtmpi_init(int num_threads){
  P = num_threads;
}

void gtmpi_barrier() {
  int i, pe, stride;
  int tag = 0;
  
  if (1 == P) return;

  MPI_Comm_rank(MPI_COMM_WORLD, &pe);

  MPI_Status stats;
  MPI_Request req;
  /// here, we set master process is final winner, slaves are losers.
  if (pe == 0) {
    /// waiting for wakeup when all losers reach the barrier 
    for (i = 1; i < P; i <<= 1) {
      MPI_Recv(NULL, 0, MPI_INT, i, tag, MPI_COMM_WORLD, &stats);
    }
    /// wakeup direct losers 
    for (i >>= 1; i > 0; i >>= 1) {
      MPI_Isend(NULL, 0, MPI_INT, i, tag, MPI_COMM_WORLD, &req);
    }
  } else {
    /// set stride for each round
    stride = 1;

    while (pe % (stride << 1) == 0) {
      /// waiting for its local loser
      if (pe + stride < P) {
        MPI_Recv(NULL, 0, MPI_INT, pe + stride, tag, MPI_COMM_WORLD, &stats);
      }
      stride <<= 1;
    }

    /// notify its winner for its done
    MPI_Isend(NULL, 0, MPI_INT, pe - stride, tag, MPI_COMM_WORLD, &req);
    /// waiting for wakeup from its winner
    MPI_Recv(NULL, 0, MPI_INT, pe - stride, tag, MPI_COMM_WORLD, &stats);

    /// notify its local losers
    while (stride > 1) {
      stride >>= 1;
      if ((pe + stride) < P) {
        MPI_Isend(NULL, 0, MPI_INT, pe - stride, tag, MPI_COMM_WORLD, &req);
      }  /* if */
    }  /* while */
  }  /* if-else */
}

void gtmpi_finalize() {}
