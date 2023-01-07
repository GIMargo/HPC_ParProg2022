#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

//#define testing_mode
//#define experiment_mode
//#define interactive_mode

unsigned int * primes; // массив простых чисел (в нём будет результат)

int test(int curr, const char * filename) {
	FILE *f;
	f = fopen(filename, "r");
	unsigned int * etalon = (unsigned int *)malloc(10000 * sizeof(unsigned int));
	int i = 0;
	while (fscanf(f, "%u", &etalon[i]) != EOF) {
		i++;
	}
	//printf("\n Etalon: \n");
	//for (int i = 0; i < curr; i++) {
	//	printf("%d ", etalon[i]);
	//}
	for (int i = 0; i < curr; i++) {
		if (primes[i] != etalon[i]) {
			printf("\n Err, result is %u, but etalon is %u!\n", primes[i], etalon[i]);
			fclose(f);
			free(etalon);
			return 0;
		}
	}
	fclose(f);
	free(etalon);
	return 1;
}

void printArray(unsigned int * arr, int size) {
	printf("\n");
	for (int i = 0; i < size; i++) {
		printf("%u ", arr[i]);
	}
	printf("\n");
}

void saveArrayToFile(int N, unsigned int * arr, int size, const char * filename) {
	FILE *results;
	results = fopen(filename, "w");
	fprintf(results, "%u \n", N);
	for (int i = 0; i < size; i++) {
		fprintf(results, "%u ", arr[i]);
	}
	fclose(results);
}

int findD(int number_of_processes) {
	int d = 1;
	while (d * 2 < number_of_processes - 1) {
		d *= 2;
	}
	return d;
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv); // MPI initialization
#ifdef testing_mode
	const int Ns_size = 8;
	unsigned int Ns[] = { 2, 105, 239, 543, 1000, 5468, 7777, 10000 };
#else
	const int Ns_size = 5;
	unsigned int Ns[] = { 100000, 1000000, 10000000, 100000000, 1000000000 };
#endif

#ifndef interactive_mode
	int test_num = 1;
	int wrong_num = 0;
	double total = 0;
#endif
	int k = 0, pid = 0;
	for (int i = 0; i < Ns_size; i++) {
		unsigned int N = Ns[i];
		MPI_Comm_size(MPI_COMM_WORLD, &k); // общее количество процессов
		MPI_Comm_rank(MPI_COMM_WORLD, &pid); // номер текущего процесса
#ifdef interactive_mode
		if (!pid) {
			//printf("\n Enter Upper_bound (integer): \n");
			scanf("%u", &N);
		}
		MPI_Bcast(&N, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		//printf("\n Process #%d! N = %u, k = %d\n", pid, N, k);
#else
		if (!pid) { // only master prints this string
			printf("\n Test #%d! N = %u, k = %d\n", test_num++, N, k);
		}
#endif

		int m; // amount of numbers, considered by all processes
		if ((unsigned int)sqrt(N) % 2 == 0 && N % 2 != 0) { // корень из N чётный, а само число нет
			m = (N - (unsigned int)sqrt(N)) / 2 + 1;
		}
		else {
			m = (N - (unsigned int)sqrt(N)) / 2;
		}

		int amount; // amount of numbers, considered by one process
		if (m % k == 0) {
			amount = m / k;
		}
		else {
			amount = m / k + 1;
		}

		int col = N * 1.5 / log(N); // estimation of the number of required primes (Gauss+Legendre) [coefficient 1.5 was taken for reliability]
		primes = (unsigned int*)malloc(col * sizeof(unsigned int)); // primes array (result will be here)
		int curr = 0; // index of the min unoccupied cell of an array of prime numbers
		primes[curr++] = 2; // put 2 into primes

		unsigned int a = sqrt(N);


		for (unsigned int x = 3; x <= a; x += 2) { // from 3 to sqrt(N) considering all numbers (with doubled step)
			char isPrime = 1;
			for (int i = 0; i < curr; i++) {
				if (x % primes[i] == 0) { // the number is definitely not prime
					isPrime = 0;
				}
			}
			if (isPrime) {
				primes[curr++] = x; // add prime number to "primes"
			}
		}

		//printf("Primes in P: \n");
		//printArray(primes, curr);

		a++; // min considered number: sqrt(N) + 1 
		if (a % 2 == 0) {  // if it's not odd, finding the nearest odd
			a++;
		}

		/* Parallel part */
		unsigned int * buff = NULL;
		if (amount) { // if we have numbers to consider
			unsigned int * buff = (unsigned int*)malloc(amount * sizeof(unsigned int)); // creating a buffer for the process
			int buff_curr = 0; // index of the min unoccupied buffer cell
			a += pid * 2; // starting from the starting point
			//printf("\n Process #%d! start = %u\n", pid, a);
			while (a <= N) {
				int isPrime = 1;
				for (int i = 0; i < curr; i++) {
					if (primes[i] > sqrt(a)) {
						break;
					}
					if (a % primes[i] == 0) {
						isPrime = 0;
						break;
					}
				}
				if (isPrime) {
					buff[buff_curr++] = a; // putting prime to the process buff 
				}
				a += k * 2; // we shift by a doubled step, as we go by odd
			}

			if (buff_curr < amount) { // we put 0 to the cell after the cell with last prime number found by the process
				buff[buff_curr++] = 0;
			}
			
			//printArray(buff, buff_curr);

			/* Result forming */
			int d = findD(k);
			//printf("\n Process #%d! d = %d, k = %d, amount = %d\n", pid, d, k, amount);
			int curr_k = k;
			while (d > 0) {
				if (pid - d >= 0) { // sending data
					//printf("\n Process #%d is sending data to process #%d! Size of the data is %d.\n", pid, pid-d, buff_curr);
					MPI_Send(&buff_curr, 1, MPI_INT, pid - d, 1, MPI_COMM_WORLD); // sending the dimension of our buffer
					//printf("\n Process #%d send data size successfully!\n", pid);
					if (buff_curr) {
						MPI_Send(buff, buff_curr, MPI_UNSIGNED, pid - d, 2, MPI_COMM_WORLD); // sending the buffer	
						//printf("\n Process #%d send data successfully and now exiting!\n", pid);
					}
					goto exit; // now this process is done
				}
				else if (pid + d < curr_k) {
					MPI_Status status;
					int dim = -1;
				  //printf("\n Process #%d is ready to receiving data size!\n", pid);
					MPI_Recv(&dim, 1, MPI_INT, pid + d, 1, MPI_COMM_WORLD, &status); // receiving dimension
					//printf("\n Process #%d received data size from process #%d successfully! Size of the data is %d.\n", pid, pid + d, dim);
					if (dim > 0) {
						unsigned int * tmp = (unsigned int*)malloc(dim * sizeof(unsigned int)); // creating a tmp buffer for the message
						MPI_Recv(tmp, dim, MPI_UNSIGNED, pid + d, 2, MPI_COMM_WORLD, &status); // receiving data
						//printf("\n Process #%d received data successfully!\n", pid);
						unsigned int * bigbuff = (unsigned int*)malloc((dim + buff_curr) * sizeof(unsigned int)); // creating a big buffer for the union
						int i = 0, j = 0, curr_big = 0;
						while (i < buff_curr || j < dim) { // i moves through process buff, j moves through tmp 
							if ((i < buff_curr && !buff[i]) && (j < dim && !tmp[j])) {
								break;
							}
							if ((i < buff_curr && j < dim && ((buff[i] && tmp[j] && buff[i] < tmp[j]) || (buff[i] && !tmp[j]))) || (i < buff_curr && j >= dim)) {
								bigbuff[curr_big++] = buff[i];
								i++;
							}
							else if ((i < buff_curr && j < dim && ((buff[i] && tmp[j] && buff[i] > tmp[j]) || (!buff[i] && tmp[j]))) || (i >= buff_curr && j < dim)) {
								bigbuff[curr_big++] = tmp[j];
								j++;
							}
						}
						if (curr_big < dim + buff_curr) {
							bigbuff[curr_big++] = 0;
						}
					
						free(buff);
						buff = bigbuff;
						bigbuff = NULL;
						buff_curr = curr_big;
						free(tmp);
					}
					else if (dim == -1) {
						printf("\n Error! Process #%d isn't received dimension from process #%d!\n ", pid, pid + d);
						goto exit;
					}
				}
			  //printArray(buff, buff_curr);
				curr_k = d;
				d /= 2;
				//printf("\n Process #%d! curr_k = %d, d = %d!\n ", pid, curr_k, d);
			}
			/* Only master (0) process will reach this part of code*/
			int index = 0;
			while (buff[index]) {
				primes[curr++] = buff[index++];
			}
		}
		if (!pid) {
			//printf("\n Resulting primes in P: \n");
			//printArray(primes, curr);
#ifdef interactive_mode
			//saveArrayToFile(N, primes, curr, "result.txt");
#endif
#ifdef testing_mode
			if (test(curr, "etalon.txt")) {
				printf("\n Everything is OK! \n");
			}
			else {
				printf("\n The result is wrong! \n");
				wrong_num++;
			}
#endif
		}
	exit:
		free(buff);
		free(primes);

#ifdef interactive_mode 
		break;
#endif
		// synchronizing part
		MPI_Barrier(MPI_COMM_WORLD);
	}
#ifdef testing_mode
	if (!pid) {
		if (!wrong_num) {
			printf("\n All %d tests passed succesfully! \n", test_num-1);
    } 
		else {
			printf("\n %d of %d tests passed succesfully! \n %d tests failed! \n", test_num - wrong_num - 1, test_num - 1, wrong_num);
		}
  }  
#endif

	MPI_Finalize(); // завершение MPI программы
	return 0;
}


