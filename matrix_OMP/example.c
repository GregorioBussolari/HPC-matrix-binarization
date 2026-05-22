	#include <stdio.h>
	#include <stdlib.h>
	#include <omp.h>
	#define DIM 256
	
   int main(int argc, char* argv[])
	{   double start, end;
		if (argc!=2)
		{   printf("sintassi sbagliata -- %s n_proc", argv[0]);
			exit(1);
		}
	 
		// Get number of processes and check that 4 processes are used
		int size, my_rank;
		size=atoi(argv[1]); // il numero di processi viene passato come argomento
		if(size > DIM)
		{
			printf("il numero di processi %d è maggiore della dimensione %d.\n",size, DIM);
			exit(1);
		}

		
	 
		// inizializzazione vettori:
	  
		int A[DIM],B[DIM], C[DIM], i ;
		srand((unsigned int)time(NULL)); 
		for(i=0;i<DIM;i++){
				A[i]=rand()%100;
				B[i]=rand()%100;
		}
			
		// verifica
		printf("[processo master] vettore A:\n");
		for(i=0;i<DIM;i++)
			printf("\t%d\n",A[i]);
		printf("[processo master] vettore B:\n");
			for(i=0;i<DIM;i++)
				printf("\t%d\n",B[i]);
				
    start = omp_get_wtime(); 
	
    # pragma omp parallel  num_threads(size) shared(A,B,C) private(i,my_rank) firstprivate(size)
    {   my_rank=omp_get_thread_num();   
        printf("thread %d di %d: inizio il calcolo...\n", my_rank, size);
        # pragma omp for
        for(i=0; i<DIM; i++)
            C[i]=A[i]+B[i];
    }
    end = omp_get_wtime(); 
    printf("[Master] Risultato C=A+B:\n");
	for(i=0; i<DIM; i++)
			printf("\t%d\n", C[i]);
    printf("tempo di esecuzione: %lf\n", end-start);
	 
    return EXIT_SUCCESS;
}
