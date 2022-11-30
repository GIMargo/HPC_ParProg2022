
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

//#define testing_mode
//#define experiment_mode
//#define interactive_mode

unsigned int * primes; // ������ ������� ����� (� �� ����� ���������)

int test(int curr, const char * filename) {
	FILE *f;
	f = fopen(filename, "r");
	unsigned int * etalon = (unsigned int *) malloc(10000*sizeof(unsigned int));
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

typedef struct {
	int t_id; // ����� ������
	int t_num; // ������������ ���������� �����, �������������� �������
	int h; // ��� ��������� ����� �������
	int primes_size; // ������ ������� ������� ����� (���������)
	unsigned int N; // ������� �������, �� ������� ���� ������� �����
	unsigned int start; // ��������� �����
	unsigned int * buff; // ����� ��� �����������
}threadPack;

void * flowFunc(void *param) { // ��������� �������
  threadPack * myPack = (threadPack*) param; 
	myPack->buff = (unsigned int*) malloc(myPack->t_num * sizeof(unsigned int));

	int curr = 0; // ������ ���������� ��������� ������ ������
	//printf("\n Thread #%d, start=%ld\n", myPack->t_id, myPack->start);
	unsigned int a = myPack->start; // �������� �� ��������� �����
	while (a <= myPack->N) {
		int isPrime = 1;
		for (int i = 0; i < myPack->primes_size; i++) {
			if (primes[i] > sqrt(a)) {
				break;
			}
			if (a % primes[i] == 0) {
				isPrime = 0;
				break;
			}
		}
		if (isPrime) {
			myPack->buff[curr++] = a;
		}
		a += myPack->h * 2; // ���������� �� ��������� ���, ��� ��� ��� �� ��������
	}

	if (curr < myPack->t_num) { // �������� ������ ����� ���������� ���������� �������� �����
		myPack->buff[curr++] = 0;
	} 

	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	  //int N = 1000; // ������� �������, �� ������� ���� ������� �����
		//int k = 8; // ���������� �������
		const int ks_size = 8;
#ifdef testing_mode
		const int Ns_size = 8;
		unsigned int Ns[] = {2, 105, 239, 543, 1000, 5468, 7777, 10000};
#else
		const int Ns_size = 5;
		unsigned int Ns[] = { 100000, 1000000, 10000000, 100000000, 1000000000 };
#endif
		int ks[] = {1, 2, 12, 24, 36, 48, 56, 96};
		

		struct timeval start, end;
		//struct timeval start_parallel, end_parallel;
#ifndef interactive_mode
		int test_num = 1;
		double total = 0;
		//double total_parallel = 0;
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

				int m; // ���������� �����, ������� �������� �� ��������� �������
				if ((unsigned int)sqrt(N) % 2 == 0 && N % 2 != 0) { // ������ �� N ������, � ���� ����� ���
					m = (N - (unsigned int)sqrt(N)) / 2 + 1;
				}
				else {
					m = (N - (unsigned int)sqrt(N)) / 2;
				}

				int amount; // ���������� �����, �������������� ����� �������
				if (m % k == 0) {
					amount = m / k;
				}
				else {
					amount = m / k + 1;
				}

				int col = N * 1.5 / log(N); // ������ ���������� ������� ������� ����� (�����+�������) [�����. 1.5 ���� ��� ���������]
				primes = (unsigned int*)malloc(col * sizeof(unsigned int)); // ������ ������� ����� (� �� ����� ���������)
				int curr = 0; // ������ ���������� ��������� ������ ������� ������� �����
				primes[curr++] = 2; // ������� � ������ ������� ������

				unsigned int a = sqrt(N);

				for (unsigned int x = 3; x <= a; x += 2) { // �� 3 �� ����� �� N ���������� ��� ����� (� ����� 2)
					char isPrime = 1;
					for (int i = 0; i < curr; i++) {
						if (x % primes[i] == 0) { // ����� ����� �� �������
							isPrime = 0;
						}
					}
					if (isPrime) {
						primes[curr++] = x; // ��������� ������� � ������
					}
				}

				//printf("Primes in P: \n");
				//printArray(primes, curr);
#ifdef experiment_mode
				//gettimeofday(&start_parallel, NULL);
#endif
				/* ������ � �������� */
				threadPack * packs = (threadPack *)malloc(k * sizeof(threadPack)); // ��������� ����������� ������� ������

				pthread_t * tid = (pthread_t *)malloc(k * sizeof(pthread_t)); // �������������� ������� 

				a++; // ����������� ��������������� ���� �����: ������ �� N + 1 ��� ���������� ������� �������� + 1

				if (a % 2 == 0) {  // ���� ��� ������, �� ������� ��������� ��������
					a++;
				}
				for (int i = 0; i < k; i++) {
					packs[i].t_id = i;
					packs[i].t_num = amount;
					packs[i].primes_size = curr;
					packs[i].N = N;
					packs[i].buff = NULL;
					packs[i].start = a;
					if (pthread_create(&tid[i], NULL, flowFunc, packs + i)) { // ������� ����� �����
						printf("\n Thread creation error! Thread number %d\n", i);
						goto exit;
					}
					packs[i].h = k;
					a+=2;// ������ ������� ��������� �������� �����
				}

				for (int i = 0; i < k; i++) {
					if (pthread_join(tid[i], NULL)) { // ���� ���������� ���������� ������� ������
						printf("\n Thread join error! Thread number %d\n", i);
						goto exit;
					}

						//int j = 0;
						//printf("\n Thread #%d buff:\n", i);
						//while (j < packs[i].t_num) {
						//	if (packs[i].buff[j] > 0) {
						//		printf(" %d", packs[i].buff[j]);
						//	}
						//	else {
						//		break;
						//	}
						//	j++;
						//}
					
				}
#ifdef experiment_mode
				//gettimeofday(&end_parallel, NULL);
#endif

				// result forming
				int * index = (int *)malloc(k * sizeof(int)); 
				for (int i = 0; i < k; i++) {
					index[i] = 0;
				}

				while (1) {
					unsigned int min = 0;
					int thread_num = -1;
					for (int i = 0; i < k; i++) {
						if (!min && index[i] < amount && packs[i].buff[index[i]]) {
							min = packs[i].buff[index[i]];
							thread_num = i;
						}
						else if (index[i] < amount && packs[i].buff[index[i]] && packs[i].buff[index[i]] < min) {
							min = packs[i].buff[index[i]];
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
				free(index);
			exit:
				free(tid);
				for (int i = 0; i < k; i++) {
					free(packs[i].buff);
				}
				free(packs);
				free(primes);
#ifdef experiment_mode
				gettimeofday(&end, NULL);
				long time_spent = ((end.tv_sec - start.tv_sec) * 1000000) + end.tv_usec - start.tv_usec;
				printf("\n Time is: %.4lf s\n", (double)time_spent / 1000000.);
				//long parallel_time_spent = ((end_parallel.tv_sec - start_parallel.tv_sec) * 1000000) + end_parallel.tv_usec - start_parallel.tv_usec;
				//printf("\n Parallel time is: %.4lf s\n", (double)parallel_time_spent / 1000000.);
				total += (double)time_spent / 1000000.;
				//total_parallel+= (double)parallel_time_spent / 1000000.;
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
		printf("\n Time is: %.4lf s\n", (double)time_spent/ 1000000.);
#else
		printf("\n Average time is: %.4lf s\n", total / (test_num-1));
		//printf("\n Average parallel time is: %.4lf s\n", total_parallel / (test_num - 1));
#endif
}
