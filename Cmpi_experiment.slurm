#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -N 1
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 04:00:00

module purge
module load compiler/gcc/9.4.0
module load mpi/impi/2017.4.239
mpicc -D interactive_mode mpi_primes.c -lm
for N in 100000 1000000 10000000 100000000 1000000000
do
 for k in 1 2 12 24 36 48 56
 do 
  T=$(echo $N | time -f "%e" mpirun -np $k ~/mpiC/a.out 2>&1)
  echo N = $N k = $k time = $T
 done
done
