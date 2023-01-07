#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

module purge
module load compiler/gcc/9.4.0
module load mpi/impi/2017.4.239
mpicc -D testing_mode mpi_primes.c -lm
for k in 1 2 12 24 36 48 56 96
do 
 mpirun -np $k ~/mpiC/a.out
done 
