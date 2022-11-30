#!/bin/bash
#SBATCH -J Rita_Primes
#SBATCH -N 1
#SBATCH -p tornado-k40
#SBATCH -o ./out
#SBATCH -e ./err
#SBATCH -t 01:00:00

if [[ $# -ne 2 ]]
then
	echo Please enter 2 integers
	exit 1
fi
gcc primes.c -std=c99 -D interactive_mode -lm -lpthread
echo $@ | ./a.out
