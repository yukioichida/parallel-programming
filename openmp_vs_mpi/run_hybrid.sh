rm -f sort_vector_hybrid
mpicc -o sort_vector_hybrid sort_vector_hybrid.c -fopenmp

default_workers=2
workers=${1-$default_workers}
chmod 777 sort_vector_hybrid
echo "[Run with $workers nodes]"
mpirun -n $workers ./sort_vector_hybrid 