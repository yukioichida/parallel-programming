rm -f sort_vector_mpi
mpicc -o sort_vector_mpi sort_vector_mpi.c
default_workers=2
workers=${1-$default_workers}
chmod 777 sort_vector_mpi
echo "[Run with $workers nodes]"
mpirun -n $workers ./sort_vector_mpi