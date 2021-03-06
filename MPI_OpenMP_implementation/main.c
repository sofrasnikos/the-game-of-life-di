#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "execute.h"

int main(int argc, char *argv[]) {

	srand(time(NULL));

	int i;
	int rank, num_of_proc, num_of_threads = -1;
	int dimension = -1, sub_grid_size = -1, loops = -1;
	int prints_enabled = 0;
	char *input_file = NULL;
	int error = 0;
	int provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
	if (provided < MPI_THREAD_FUNNELED) {
		printf("Error: MPI does not provide the required thread support\n");
    	MPI_Abort(MPI_COMM_WORLD, 1);
    	exit(1);
	}
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc);

	double start_time = omp_get_wtime();

	// Parse arguments
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			i++;
			if (argv[i] != NULL) {
				dimension = atoi(argv[i]);
				if (dimension < 16) {
					error = 1;
					break;
				}
			} else {
				error = 2;
				break;
			}
			continue;
		}
		else if (!strcmp(argv[i], "-f")) {
			i++;
			if (argv[i] != NULL) {
				input_file = argv[i];
			} else {
				error = 3;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-l")) {
			i++;
			if (argv[i] != NULL) {
				loops = atoi(argv[i]);
				if (loops <= 0) {
					error = 4;
					break;
				}
			} else {
				error = 5;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-t")) {
			i++;
			if (argv[i] != NULL) {
				num_of_threads = atoi(argv[i]);
				if (num_of_threads <= 0) {
					error = 6;
					break;
				}
			} else {
				error = 7;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-p")) {
			prints_enabled = 1;
			continue;
		} else {
			error = 8;
			break;
		}
	}

	// Default values if user does not specify
	if (dimension == -1) {
		dimension = DEFAULT_DIMENSION_SIZE;
	}
	if (loops == -1) {
		loops = DEFAULT_NUMBER_OF_LOOPS;
	}
	if(num_of_threads == -1){
		num_of_threads = NUM_OF_THREADS;
	}

	// Print error messages if you are root process
	if (rank == 0) {
		if (error != 0) {
			switch (error) {
			case 1:
				printf("Error: wrong dimension size\n");
				break;
			case 2:
				printf("Error: you did not specify dimension size\n");
				break;
			case 3:
				printf("Error: you did not specify input file\n");
				break;
			case 4:
				printf("Error: wrong number of loops\n");
				break;
			case 5:
				printf("Error: you did not specify number of loops\n");
				break;
			case 6:
				printf("Error: wrong number of threads\n");
				break;
			case 7:
				printf("Error: you did not specify number of threads\n");
				break;
			case 8:
				printf("Error: unknown argument: %s\n", argv[i]);
				break;
			}
			printf(
					"\nUSAGE: mpiexec -f <machine_file> -p <number_of_processes> -bind-to core ./game_of_life -t <number_of_threads> -d <dimension_of_grid> -f <input_file> -l <number_of_loops> -p\n");
			printf("\nFLAGS\n");
			printf("-t <number_of_threads> : This flag defines the number of threads that the program will use. If it is not set up the program will use %d threads\n", NUM_OF_THREADS);
			printf("-d <dimension_of_grid> : This flag sets up the size of the grid. If it is not set up, the program will use %d as deafault value.\n", DEFAULT_DIMENSION_SIZE);
			printf("-f <input_file> : This flag forces the program to read an initial state of grid from file, and use it as the first generation.\n");
			printf("-l <number_of_loops> : This flag determines the generations that will be completed.\n");
			printf("-p : (OPTIONAL FLAG) This flag forces the program to print the generations to output files. ATTENTION! This flag causes major slowdown to the execution!\n");
		} 
	}
	if (error != 0) {
		MPI_Finalize();
		exit(0);
	}

	sub_grid_size = calculateSubgridSize(dimension, num_of_proc);

	if (sub_grid_size == -1) {
		if (rank == 0) {
			printf("Number of processes must be a perfect square. %d is not. \n", num_of_proc);

		}
		MPI_Finalize();
		exit(0);
	} else if (sub_grid_size == -2) {
		if (rank == 0) {
			printf("the grid cannot be divided with %d processes\n", num_of_proc);
		}
		MPI_Finalize();
		exit(0);
	}

	if (prints_enabled == 1 && rank == 0) {
		printf("dimension size: %d\n", dimension);
		printf("number of loops: %d\n", loops);
	}

	execute(rank, num_of_proc, num_of_threads, dimension, sub_grid_size, loops, input_file, prints_enabled);

	if (rank == 0) {
		double time = omp_get_wtime() - start_time;
		printf("time elapsed: %lf seconds\n", time);
		printf("Terminated successfully\n");
	}
	
	MPI_Finalize();
	exit(0);
}
