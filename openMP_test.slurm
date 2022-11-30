#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -N 1
#SBATCH -p tornado-k40
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

gcc openMP_primes.c -D testing_mode -std=c99 -lm -fopenmp
./a.out
