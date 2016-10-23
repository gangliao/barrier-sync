#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <omp.h>
#include "gtmp.h"

/*
    From the MCS Paper: A scalable, distributed tree-based barrier with only local spinning.

    type treenode = record
        parentsense : Boolean
	parentpointer : ^Boolean
	childpointers : array [0..1] of ^Boolean
	havechild : array [0..3] of Boolean
	childnotready : array [0..3] of Boolean
	dummy : Boolean //pseudo-data

    shared nodes : array [0..P-1] of treenode
        // nodes[vpid] is allocated in shared memory
        // locally accessible to processor vpid
    processor private vpid : integer // a unique virtual processor index
    processor private sense : Boolean

    // on processor i, sense is initially true
    // in nodes[i]:
    //    havechild[j] = true if 4 * i + j + 1 < P; otherwise false
    //    parentpointer = &nodes[floor((i-1)/4].childnotready[(i-1) mod 4],
    //        or dummy if i = 0
    //    childpointers[0] = &nodes[2*i+1].parentsense, or &dummy if 2*i+1 >= P
    //    childpointers[1] = &nodes[2*i+2].parentsense, or &dummy if 2*i+2 >= P
    //    initially childnotready = havechild and parentsense = false
	
    procedure tree_barrier
        with nodes[vpid] do
	    repeat until childnotready = {false, false, false, false}
	    childnotready := havechild //prepare for next barrier
	    parentpointer^ := false //let parent know I'm ready
	    // if not root, wait until my parent signals wakeup
	    if vpid != 0
	        repeat until parentsense = sense
	    // signal children in wakeup tree
	    childpointers[0]^ := sense
	    childpointers[1]^ := sense
	    sense := not sense
*/

#define  true   1
#define  false  0

#ifndef LEVEL1_DCACHE_LINESIZE
#define LEVEL1_DCACHE_LINESIZE 64
#endif

#define MCS_BARRIER_PADDING  (LEVEL1_DCACHE_LINESIZE - 56)
typedef struct {

  volatile int32_t  parentsense;
  volatile int8_t*  parent_ptr;
  volatile int32_t* child_ptrs[2];

  int32_t dummy;
  int32_t sense;
  int32_t num_childs;

  union {
    volatile int8_t  array[4];
    volatile int32_t value;
  } childs_not_ready;

  union {
    int8_t  array[4];
    int32_t value;
  } have_childs;
 
  int8_t padding[MCS_BARRIER_PADDING];
} omp_mcs_barrier_node_t;

typedef struct {
  omp_mcs_barrier_node_t* nodes;
} omp_mcs_barrier_t;

/// define global mcs barrier data structure
omp_mcs_barrier_t* barrier_;

void gtmp_init(int num_threads) {
  /// allocate memory for mcs barrier nodes
  posix_memalign((void**)&barrier_->nodes, 64,
	sizeof(omp_mcs_barrier_node_t) * num_threads);

  if (barrier_->nodes == NULL) {
    perror("failed to allocate memory for mcs barrier nodes.");
    exit(EXIT_FAILURE);
  }

  int i = 0, j = 0;
  omp_mcs_barrier_node_t* node;
  for (i = 0; i < num_threads; ++i) {
    node = &barrier_->nodes[i];
    node->sense = true;
    node->parentsense = false;

    /// specify correct number of childs
    node->num_childs = 0;
    int child_idx = i * 4 + 1;
    if (child_idx < num_threads) {
      node->num_childs = num_threads - child_idx;
      if (node->num_childs > 4) { node->num_childs = 4; }
    }
 
    /// specify node's child states
    for (j = 0; j < 4; ++j) {
      if (j < node->num_childs) {
        node->have_childs.array[j] = true;
        node->childs_not_ready.array[j] = true;
      } else {
        node->have_childs.array[j] = false;
        node->childs_not_ready.array[j] = false;
      }
    }
    
    /// specify parent not ready pointer
    if (i == 0) {
      node->parent_ptr = (int8_t*)&node->dummy;
    } else {
      node->parent_ptr = &(barrier_->nodes[(i - 1) / 4]
			 .childs_not_ready.array[(i - 1) % 4]);
    }

    /// specify childs pointer for release
    if ((2 * i + 2) < num_threads) {
      node->child_ptrs[0] = &(barrier_->nodes[2 * i + 1].parentsense);
      node->child_ptrs[1] = &(barrier_->nodes[2 * i + 2].parentsense);
    } else if ((2 * i + 1) < num_threads) {
      node->child_ptrs[0] = &(barrier_->nodes[2 * i + 1].parentsense);
      node->child_ptrs[1] = &node->dummy;
    } else {
      node->child_ptrs[0] = &node->dummy;
      node->child_ptrs[1] = &node->dummy;
    }
  }
}

void gtmp_barrier(){
  int tid = omp_get_thread_num();
  omp_mcs_barrier_node_t* node = &barrier_->nodes[tid];

  if (node->num_childs > 0) {
    while (0 != node->childs_not_ready.value) {};
    node->childs_not_ready.value = node->have_childs.value;
  }

  if (tid != 0) {
    *node->parent_ptr = false;
    while (node->parentsense != node->sense) {};
  }

  *node->child_ptrs[0] = node->sense;
  *node->child_ptrs[1] = node->sense;
   node->sense = !node->sense;
}

void gtmp_finalize(){
  free(barrier_->nodes);
  barrier_ = NULL;
}
