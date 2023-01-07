#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -p tornado-k40
#SBATCH -N 4
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 04:00:00

module purge
module load compiler/gcc/11.2.0
module load mpi/openmpi/4.1.3/gcc/11
module load python/3.9
rm time.txt
for N in 100000 1000000 10000000 100000000 1000000000
do
 for k in 36 48 56 112 168 224
 do
  echo "e $N" | time -o tmp.out -f "%e" mpirun -np $k python3 -m mpi4py ~/mpiPython/mpi_primes.py >/dev/null 2>&1
  echo N = $N k = $k time = `cat tmp.out`  >> time.txt
  echo $N $k `cat tmp.out` >> time_short.txt
 done
done 
