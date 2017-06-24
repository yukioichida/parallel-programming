default_threads=2
threads=${1-$default_threads}
rm -f sort_vector_omp
gcc sort_vector_omp.c -o sort_vector_omp -fopenmp
./sort_vector_omp $threads