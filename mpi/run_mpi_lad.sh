ladcomp -env mpiompCC sort_vector_mpi.c -o sort_vector_mpi
ladalloc -c gates -n 2 -t 15 -e 
ladrun -np 16 sort_vector_mpi