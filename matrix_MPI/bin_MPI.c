#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>
#include <time.h>
#define ROUND 3


void print_matrix(int *m, int rows, int cols) {
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++)
            printf("[%d]\t", m[i*cols+j]);
        printf("\n");
    }
}

int mirror_index(int idx, int N) {
    if (idx < 0) return -idx;   // oob right      
    if (idx >= N) return 2*N - idx - 2; //oob left
    return idx;
}

int main(int argc, char *argv[]) {   
    double begin, end, local_elaps,global_elaps;
    int N, dim; 

    if(argc < 2){
        printf("<usage>: dim_array\n");
        return 1;
    }

    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    N = atoi(argv[1]);
    dim = N * N;
    if(size > N && rank == 0){
        printf("il numero di processi %d è maggiore della dimensione della matrice %d.\n",size, N);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Define values
    int i, j;
    //define dinamically matrix
    int* A = (int*) malloc(dim * sizeof(int));
    int* T = (int*) malloc(dim * sizeof(int));

    

     
    if (rank == 0){
        //printf("[Processo %d]: sono stati lanciati %d processi, inizializzo la matrice", rank, size);
        srand((unsigned int)time(NULL)); 

        for(int i=0 ; i < dim ; i++){
			A[i] = rand()%255;
		}

        // test
        //printf("[processo %d] matrice A:\n", rank);
        //print_matrix(A,N,N);
    }

    // istante inizio:
    MPI_Barrier(MPI_COMM_WORLD);
    begin=MPI_Wtime();

    //calcolo numero di righe calcolate per thread
    int num_rows_A = N / size;
    int resto = N % size;
    if(rank < resto){
        num_rows_A = num_rows_A + 1;
    }

    //i processi != dal primo e l'ultimo necessitano di una riga in più di A
    int num_rows_T = num_rows_A; 
    if (rank != 0 && rank != size - 1)
        num_rows_A++; //aggiungo 1 riga di A

    //test
    //printf("[Processo %d]: utilizzo %d righe di A\n", rank, num_rows_A);

    //init strutture per scatterv
    int * sendcounts;
    int * displs;

    if (rank == 0){
        sendcounts = (int *) malloc(size * sizeof(int));
        displs = (int *) malloc(size * sizeof(int));

        int base = N / size;
        int offset = 0;

        for(i = 0; i < size; i++){
            int rows = (i < resto) ? base + 1 : base;
            if(i != 0 && i != size-1) rows++; // aggiungo sopra/sotto se serve
            sendcounts[i] = rows * N; //numero di elementi per ogni thread
            displs[i] = offset; //offset relativo del sendbuf (A)
            offset += sendcounts[i];
        }
    }

    //definisco gli array locali ad ogni processo, in cui divido la matrice
    //anche se uso num_rows righe di A, produco solo num_rows - 1 di T
    int* my_A = (int*) malloc(num_rows_A * N * sizeof(int));
    int* my_T = (int*) malloc(num_rows_T * N * sizeof(int));

    //scatterv, ad ogni processo verrà assegnato num_rows * N elementi      
    MPI_Scatterv(A, sendcounts, displs, MPI_INT, my_A, num_rows_A * N, MPI_INT,0,MPI_COMM_WORLD);
    
    //testing purpose only
    /* if (rank == 1){
        printf("\n\n VERIFICA: [processo %d] matrice my_A, %d elementi:\n", rank, num_rows_A * N);
        print_matrix(my_A,num_rows_A,N);
    } */

    //processo Pi
    //calcolo della porzione di matrice locale my_T
    int ix, iy, tempX, tempY;
    int radius = ROUND / 2; 
    double sum, med;

    for(i = 0; i<num_rows_T; i++){
        for(j = 0; j < N; j++){
            sum = 0.0;

            //per ogni intorno 3x3 di a[i][j]
            for(ix = 0; ix < ROUND; ix++){
                for(iy = 0; iy< ROUND; iy++){
                    tempX = i + ix - radius;
                    tempY = j + iy - radius; 
                    
                    //gestione degli indici che eccedono in modo specchiato
                    tempX = mirror_index(tempX, N);
                    tempY = mirror_index(tempY, N);
                    
                    sum += my_A[tempY * N + tempX];

                }
            }
            med = sum / (ROUND*ROUND);
            //update result matrix with binary values
            my_T[i * N + j] = my_A[i * N + j] > med;

        }
    }
    
	//gather delle sottomatrici risultato (blocchi di DIM/size righe) nella matrice C del rank 0
    MPI_Gatherv(my_T, num_rows_T*N, MPI_INT, T, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    //misura tempo:
    MPI_Barrier(MPI_COMM_WORLD);
    end=MPI_Wtime();
    local_elaps= end-begin;
    MPI_Reduce(&local_elaps, &global_elaps,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    //testing purpose only
    if (rank == 0)
    {       printf("Con n = %d processi e dimensione %d il tempo impiegato è: %f secondi\n",size, N, global_elaps);
            //stampa risultato per test
            //printf("\n\n[processo %d] matrice binaria T:\n", rank);
            //print_matrix(T,N,N);
    }

    // libera memoria
    free(A);
    free(T);
    free(my_A);
    free(my_T);
    if (rank == 0) {
        free(sendcounts);
        free(displs);
    }
    
    MPI_Finalize();
    
}
