# barrier-sync
Scalable Barriers for Shared Memory and Distributed Systems

### Results

Test these barrier algorithms on both shared memory machine and distributed cluster:

Shared Memory CPUs:

<img src=/results/omp_barrier.png width=60%/>

After checking the performance figure generated by gnuplot, the results
do not match with the original paper. Counter Barrier reaches the best
performance out of these implementation. Why?

If we read MCS paper again, go back to 90's, it states that most of
machine in that days does not have cache and its coherence mechanism,
which means that all of threads have to access the same global memory
address to atomic decrement the counter. Heavy traffic communication
cause hot spot, thus lose the performance.

However, things are quite different in nowadays modern processors,
in here, that is Intel(R) Xeon(R) CPU E5-2630 0 @ 2.30GHz including
cache size: 15360 KB on each processor (6 cores per processor). Each
thread can spin on it local variable in cache line. That's the first
reason why Counter Barrier works best.
 
 The second reason is that do not have enough scale test. From the
 result, only 24 threads be used which is not convincible since counter
 barrier has not really hit the point where the scale at which you are
 running has overtaken the complexity of your implementation of the
 other algorithms. cpuinfo text shows that each processor has 6 cores,
 so at least we can scale to 144 threads (24 processor * 6 cores).

Distributed CPUs:

<img src=/results/mpi_barrier.png width=60%/>

You mentioned that Infiniband and 1 Gbps ethernet connection be used for
communication. Are both of those used for the most parallelism?

Similar to openmp version's barriers, I am wondering if the small scale in the
simple algorithms winning out merely since the complexity of the other algorithms
is overshadowing their scalability.

I also expect dissemination barrier and tournament barrier to perform better at scale.
Since both of them can be scaled very well in distributed systems from theoretic analysis
in the paper. For tournament barrier, it has logN rounds and each round of the tournament
should be able to use the network independently of other nodes. For the dissemination barrier,
each message will be sent independently, and it also has ~log N rounds.

### Reference
John M. Crummey and Michael L. Scott, [Algorithms for Scalable Synchronization on Shared Memory Multiprocessors](https://www.cs.rice.edu/~johnmc/papers/tocs91.pdf)