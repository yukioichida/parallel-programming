make
default_workers=2
workers=${1-$default_workers}
chmod 777 main
echo "[Run with $workers nodes]"
mpirun -n $workers ./main