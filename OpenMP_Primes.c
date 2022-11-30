#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

//#define testing_mode
//#define experiment_mode
//#define interactive_mode

unsigned int * primes; // массив простых чисел (в нЄм будет результат)

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
	for (int i = 0; i < size; i++) {
		printf("%u ", arr[i]);
	}
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

int main(int argc, char *argv[])
{
	//int N = 1000; // верхн€€ граница, до которой ищем простые числа
	//int k = 8; // количество потоков
	const int ks_size = 8;
#ifdef testing_mode
	const int Ns_size = 8;
	unsigned int Ns[] = { 2, 105, 239, 543, 1000, 5468, 7777, 10000 };
#else
	const int Ns_size = 5;
	unsigned int Ns[] = { 100000, 1000000, 10000000, 100000000, 1000000000 };
#endif
	int ks[] = { 1, 2, 12, 24, 36, 48, 56, 96 };


	struct timeval start, end;
#ifndef interactive_mode
	int test_num = 1;
	double total = 0;
	gettimeofday(&start, NULL);
#endif
	for (int i = 0; i < Ns_size; i++) {
		for (int j = 0; j < ks_size; j++) {
			unsigned int N = Ns[i];
			int k = ks[j];
#ifdef interactive_mode
			printf("\n Enter Upper_bound (integer) and Number_of_threads (integer): \n");
			scanf("%u %d", &N, &k);
			gettimeofday(&start, NULL);
#else
			printf("\n Test #%d! N = %u, k = %d\n", test_num++, N, k);
#endif
#ifdef experiment_mode
			gettimeofday(&start, NULL);
#endif

			int m; // количество чисел, которое раздадим на обработку потокам
			if ((unsigned int)sqrt(N) % 2 == 0 && N % 2 != 0) { // корень из N чЄтный, а само N нет
				m = (N - (unsigned int)sqrt(N)) / 2 + 1;
			}
			else {
				m = (N - (unsigned int)sqrt(N)) / 2;
			}

			int amount; // количество €чеек, обрабатываемых одним потоком
			if (m % k == 0) {
				amount = m / k;
			}
			else {
				amount = m / k + 1;
			}

			int col = N * 1.5 / log(N); // оценка количества искомых простых чисел (√аусс+Ћежандр) [коэфф. 1.5 вз€т дл€ надЄжности]
			primes = (unsigned int*)malloc(col * sizeof(unsigned int)); // массив простых чисел (в нЄм будет результат)
			int curr = 0; // индекс наименьшей незан€той €чейки массива простых чисел
			primes[curr++] = 2; // сложили в массив простых двойку

			unsigned int a = sqrt(N);

			for (unsigned int x = 3; x <= a; x += 2) { // от 3 до корн€ из N перебираем все числа (с шагом 2)
				char isPrime = 1;
				for (int i = 0; i < curr; i++) {
					if (x % primes[i] == 0) { // число точно не простое
						isPrime = 0;
						break;
					}
				}
				if (isPrime) {
					primes[curr++] = x; // добавл€ем простое в массив
				}
			}

			//printf("Primes in P: \n");
			//printArray(primes, curr);

			/* ѕараллелизм с помощью OpenMP */
		
			a++; // минимальное рассматриваемое нами число: корень из N + 1 
			if (a % 2 == 0) {  // если оно чЄтное, то находим ближайшее нечЄтное
				a++;
			}

			unsigned int * grand_buff = (unsigned int*)malloc(amount * k * sizeof(unsigned int));

			int buff_curr = 0;
#pragma omp parallel private(buff_curr) shared(primes, grand_buff) num_threads(k)
			{
				buff_curr = omp_get_thread_num() * amount;
				//printf("\n Thread #%d, buff_curr = %d!\n", omp_get_thread_num(), buff_curr);
				for (int x = a + omp_get_thread_num() * 2; x <= N; x+= k * 2) {
					int isPrime = 1;
					for (int i = 0; i < curr; i++) {
						if (primes[i] > sqrt(x)) {
							break;
						}
						if (x % primes[i] == 0) {
							isPrime = 0;
							break;
						}
					}
					if (isPrime) { 
						grand_buff[buff_curr++] = x;
					}
				}
				if (buff_curr < amount * (omp_get_thread_num() + 1)) { // занул€ем €чейку после последнего найденного простого числа
					grand_buff[buff_curr++] = 0;
				}
			}

			//printf("\n amount = %d\n Resulting buff: \n", amount);
			//printArray(grand_buff, k*amount);

			// Result forming
			int * index = (int *)malloc(k * sizeof(int));
			for (int i = 0; i < k; i++) {
				index[i] = 0;
			}

			while (1) {
				unsigned int min = 0;
				int thread_num = -1;
				for (int i = 0; i < k; i++) {
					if (!min && index[i] < amount && grand_buff[i*amount+index[i]]) {
						min = grand_buff[i*amount + index[i]];
						thread_num = i;
					}
					else if (index[i] < amount && grand_buff[i*amount + index[i]] && grand_buff[i*amount + index[i]] < min) {
						min = grand_buff[i*amount + index[i]];
						thread_num = i;
					}
				}
				if (thread_num != -1) {
					primes[curr++] = min; // minimum from all buffs added to primes
					index[thread_num]++;
				}
				else {
					break;
				}
			}

			//printf("\n Resulting primes in P: \n");
			//printArray(primes, curr);
			//printf("\n Actual number of threads: %d\n", k);
#ifdef interactive_mode
				saveArrayToFile(N, primes, curr, "result.txt");
#endif

#ifdef testing_mode
			if (test(curr, "etalon.txt")) {
				printf("\n Everything is OK! \n");
			}
			else {
				printf("\n The result is wrong! \n");
			}
#endif
			free(grand_buff);
			free(primes);
#ifdef experiment_mode
			gettimeofday(&end, NULL);
			long time_spent = ((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec;
			printf("\n Time is: %.4lf s\n", (double)time_spent / 1000000.);
			total += (double)time_spent / 1000000.;
#endif
#ifdef interactive_mode
			break;
#endif
		}
#ifdef interactive_mode
		break;
#endif
	}
#ifndef experiment_mode
	gettimeofday(&end, NULL);
	long time_spent = ((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec;
	printf("\n Time is: %.4lf s\n", (double)time_spent / 1000000.);
#else
	printf("\n Average time is: %.4lf s\n", total / (test_num - 1));
#endif
}