#OpenMP Flags Etc.
OMPFLAGS = -g -Wall -fopenmp 
OMPLIBS = -lgomp
CC = /usr/local/bin/gcc-6

#MPI Flags Etc (may need to customize)
#MPICH = /usr/lib64/openmpi/1.4-gcc
MPIFLAGS = -g -Wall #-I$(MPICH)/include
MPILIBS =
#MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc
MPICC = mpicc

all: gtmp_counter gtmp_mcs gtmp_tree hello_openmp gtmpi_counter gtmpi_dissemination gtmpi_tournament hello_mpi

gtmp_counter: src/gtmp_counter.c
	$(CC) $(OMPFLAGS) -o $@ $^ $(OMPLIBS)

gtmp_mcs: src/gtmp_mcs.c
	$(CC) $(OMPFLAGS) -o $@ $^ $(OMPLIBS)

gtmp_tree: src/gtmp_tree.c
	$(CC) $(OMPFLAGS) -o $@ $^ $(OMPLIBS)

hello_openmp: hello_openmp.c
	$(CC) $(OMPFLAGS) -o $@ $^ $(OMPLIBS)

gtmpi_counter: src/gtmpi_counter.c
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(MPILIBS)

gtmpi_dissemination: src/gtmpi_dissemination.c
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(MPILIBS)

gtmpi_tournament: src/gtmpi_tournament.c
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(MPILIBS)

hello_mpi: hello_mpi.c
	$(MPICC) $(MPIFLAGS) -o $@ $^ $(MPILIBS)

clean:
	rm -rf *.o *.dSYM hello_openmp hello_mpi gtmp_tree gtmp_counter gtmp_mcs gtmpi_counter gtmpi_dissemination \
	gtmpi_tournamen