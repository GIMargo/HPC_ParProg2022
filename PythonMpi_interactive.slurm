#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -N 1
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 03:00:00

if [[ $# -ne 2 ]]
then
	echo Please enter 2 integers
	exit 1
fi
module purge
module load compiler/gcc/11.2.0
module load mpi/openmpi/4.1.3/gcc/11
module load python/3.9
echo "i $1" | time -o tmp.out -f "%e" mpirun -np $2 python3 -m mpi4py ~/mpiPython/mpi_primes.py >/dev/null 2>&1
echo N = $1 k = $2 time = `cat tmp.out`  > time.txt
