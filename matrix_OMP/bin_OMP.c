#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define ROUND 3

void print_matrix(int *m, int rows, int cols)
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			printf("[%d]\t", m[i * cols + j]);
		printf("\n");
	}
}

int mirror_index(int idx, int N) {
    if (idx < 0) return -idx;   // oob right      
    if (idx >= N) return 2*N - idx - 2; //oob left
    return idx;
}

int main(int argc, char *argv[])
{
	double start, end;

	if (argc < 3)
	{
		printf("Uso: %s n_thread dim\n", argv[0]);
		exit(1);
	}

	// Get number of processes and check that 4 processes are used
	int p, my_rank, N, i, j, k;
	p = atoi(argv[1]); // number of threads
	N = atoi(argv[2]); // number of rows and columns matrix
	int radius = ROUND / 2;

	if (p > N)
	{
		printf("il numero di processi %d è maggiore della dimensione %d.\n", p, N);
		exit(1);
	}

	// matrix def and init in shared memory
	int *A = (int *)malloc(N * N * sizeof(int));
	int *R = (int *)malloc(N * N * sizeof(int));
	srand((unsigned int)time(NULL));

	// init A matrix
	for (int i = 0; i < N * N; i++)
	{
		A[i] = rand() % 255;
	}

	// tests
	//printf("[Master] matrix A:\n");
	//print_matrix(A, N, N);

	// Master: get start time
	start = omp_get_wtime();

	#pragma omp parallel num_threads(p) shared(A, R) private(i, j, my_rank) firstprivate(p, N, radius)
	{
		my_rank = omp_get_thread_num();
		//printf("[Thread %d di %d]: calcolo porzione del risultato \n", my_rank, p);

		int ix, iy, tempX, tempY;
		double sum, med;
		
		//workload distribution, round-robin policy on rows cicle
		#pragma omp for schedule(static)
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {

				sum = 0.0;

				for (ix = -radius; ix <= radius; ix++) {
					for (iy = -radius; iy <= radius; iy++) {

						int row = i + ix;
						int col = j + iy;

						row = mirror_index(row, N);
						col = mirror_index(col, N);

						sum += A[row * N + col];
					}
				}

				med = sum / (ROUND * ROUND);
				R[i * N + j] = A[i * N + j] > med;
			}
		}
				
	}

	// Master: get end time
	end = omp_get_wtime();
	printf("[Master] Matrix R:\n");
	print_matrix(R, N, N);

	printf("[Master]:processi %d matrice %d tempo di esecuzione: %lf\n", p, N, end - start);

	return EXIT_SUCCESS;
}
