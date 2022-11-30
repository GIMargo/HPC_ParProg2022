#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -N 1
#SBATCH -p tornado-k40
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

gcc primes.c -std=c99 -D testing_mode -lm -lpthread
./a.out
