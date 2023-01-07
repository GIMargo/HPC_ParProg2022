#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

module purge
module load compiler/gcc/11.2.0
module load mpi/openmpi/4.1.3/gcc/11
module load python/3.9
for k in 1 2 12 24 36 49 56
do
 echo "t" | mpiexec -n $k python3 -m mpi4py ~/mpiPython/mpi_primes.py
done 
