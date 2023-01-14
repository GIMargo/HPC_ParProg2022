#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -N 1
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

if [[ $# -ne 2 ]]
then
	echo Please enter 2 integers
	exit 1
fi
module purge
module load compiler/gcc/9.4.0
module load mpi/impi/2017.4.239
mpicc -D interactive_mode mpi_primes.c -lm
T=$(echo $1 | time -f "%e" mpirun -np $2 ~/mpiC/a.out 2>&1)
echo N = $1 k = $2 time = $T
