#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "execute.h"

int main(int argc, char *argv[]) {

	// clock_t start, end;
	// double cpu_time_used;
	// start = clock();
	srand(time(NULL));
	double start_time = MPI_Wtime();

	int i;
	int rank, num_of_proc;
	int dimension = -1, sub_grid_size = -1, loops = -1;
	int prints_enabled = 0;
	char *input_file = NULL;
	int error = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc);

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
		} else if (!strcmp(argv[i], "-p")) {
			prints_enabled = 1;
			continue;
		} else {
			error = 6;
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
				printf("Error: unknown argument: %s\n", argv[i]);
				break;
			}
			printf(
					"\nUSAGE: mpirun -np <number_of_processes> ./game_of_life -d <dimension_of_grid> -f <input_file> -l <number_of_loops> -p\n");
			printf("\nFLAGS\n");
			printf("-d <dimension_of_grid> : This flag sets up the size of the grid. If it is not set up, the program will use %d as deafault value.\n",DEFAULT_DIMENSION_SIZE);
			printf("-f <input_file> : This flag forces the program to read an initial state of grid from file, and use it as the first generation.\n");
			printf("-l <number_of_loops> : This flag determines the generations that will be completed.\n");
			printf("-p : (OPTIONAL FLAG) This flag forces the program to print the generations to output files. ATTENTION! This flag causes major slowdown to the execution!\n");
		} else {
			printf("dimension size: %d\n", dimension);
			printf("number of loops: %d\n", loops);
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

	execute(rank, num_of_proc, dimension, sub_grid_size, loops, input_file, prints_enabled);

	MPI_Finalize();
	if (rank == 0) {
		// end = clock();
		// cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		double time = MPI_Wtime() - start_time;
		printf("time elapsed: %lf\n", time);
		printf("Terminated successfully\n");
	}

	exit(0);
}
