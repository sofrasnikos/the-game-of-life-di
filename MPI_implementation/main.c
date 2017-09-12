/*
 * main.c
 *
 *  Created on: Jul 10, 2017
 *      Author: vangelis
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "execute.h"

int main(int argc, char *argv[]) {
	clock_t start, end;
	double cpu_time_used;
	start = clock();

	srand(time(NULL));

	int i;
	int rank, num_of_proc;
	int dimension = -1, sub_grid_size = -1, loops = -1;
	char *input_file = NULL;
	int error = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc);

	for (i = 1; i < argc; ++i) {
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
		} else if (!strcmp(argv[i], "-m")) {
			i++;
			if (argv[i] != NULL) {
				sub_grid_size = atoi(argv[i]);
				if (sub_grid_size < 16) {
					error = 3;
					break;
				}
			} else {
				error = 4;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-f")) {
			i++;
			if (argv[i] != NULL) {
				input_file = argv[i];
			} else {
				error = 5;
				break;
			}
			continue;
		} else if (!strcmp(argv[i], "-l")) {
			i++;
			if (argv[i] != NULL) {
				loops = atoi(argv[i]);
				if (loops <= 0) {
					error = 6;
					break;
				}
			} else {
				error = 7;
				break;
			}
			continue;
		} else {
			error = 8;
			break;
		}
	}
	if (dimension == -1) {
		dimension = DEFAULT_DIMENSION_SIZE;
	}
	if (sub_grid_size == -1) {
		sub_grid_size = MAX_SIDE_LENGTH_OF_SQUARES;
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
				printf("Error: wrong sub grid size\n");
				break;
			case 4:
				printf("Error: you did not specify sub grid size\n");
				break;
			case 5:
				printf("Error: you did not specify input file\n");
				break;
			case 6:
				printf("Error: wrong number of loops\n");
				break;
			case 7:
				printf("Error: you did not specify number of loops\n");
				break;
			case 8:
				printf("Error: unknown argument: %s\n", argv[i]);
				break;
			}
			printf(
					"USAGE:./mpiexec -n <number_of_processes> MPI_implementation -d <dimension_of_grid> -m <sub_grid_size> -f <input_file> -l <number_of_loops>\n");
		} else {
			printf("dimension size: %d\n", dimension);
			printf("sub grid size: %d\n", sub_grid_size);
			printf("number of loops: %d\n", loops);
		}
	}
	if (error != 0) {
		MPI_Finalize();
		//todo
		exit(0);
	}

	/* Calculate the optimal number of processes that are required */
	SplitAttributes attributes = processNumber(dimension, sub_grid_size);
	if (num_of_proc != attributes.number_of_processes) {
		if (rank == 0) {
			printf("the given grid is %d x %d\nthe optimal number of processes is %d\n", dimension, dimension, attributes.number_of_processes);
			printf("program exiting...\n");
		}
		MPI_Finalize();
		exit(0);
	}

	execute(rank, num_of_proc, dimension, attributes, loops, input_file);

	MPI_Finalize();
	if (rank == 0) {

		end = clock();
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("time elapsed: %lf\n", cpu_time_used);
		printf("Terminated successfully\n");
	}

	exit(0);
}

// TODO proper input gia command line arguments
// TODO svisimo apo print se o8onh kai arxeia
// TODO paraver gia statistika

