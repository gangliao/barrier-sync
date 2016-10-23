#include <omp.h>
#include <stdint.h>
#include "gtmp.h"

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


#define true   1
#define false  0

typedef struct { 
  uint8_t  sense_;
  uint32_t count_;
  uint32_t num_threads_;
} central_barrier_t;

central_barrier_t barrier_;

void gtmp_init(int num_threads) {
  barrier_.count_ = num_threads;
  barrier_.num_threads_ = num_threads;
  barrier_.sense_ = true;
}

void gtmp_barrier() {
  uint8_t l_sense = !barrier_.sense_;

  // in c++11, __sync_* are replaced by __atomic_*
  if (__sync_fetch_and_sub(&barrier_.count_, 1) == 1) {
    barrier_.count_ = barrier_.num_threads_;
    barrier_.sense_ = l_sense; 
  } else {
    while (barrier_.sense_ != l_sense) {}
  }
}

void gtmp_finalize() {}
