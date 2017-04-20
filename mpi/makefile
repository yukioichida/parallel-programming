EXECS=sort_vector_mpi
MPICC?=mpicc

all: ${EXECS}

sort_vector_mpi: sort_vector_mpi.c
	${MPICC} -o sort_vector_mpi sort_vector_mpi.c

clean:
	rm -f ${EXECS}