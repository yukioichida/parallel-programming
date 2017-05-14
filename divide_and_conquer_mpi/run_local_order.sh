rm -f main_local_order
mpicc -o main_local_order main_local_order.c -lm
default_workers=3
workers=${1-$default_workers}
chmod 777 main_local_order
echo "[Run with $workers nodes]"
mpirun -n $workers ./main_local_order