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

#define DEFAULT_DIMENSION_SIZE 200

int main(int argc, char *argv[]) {

	srand(time(NULL));

	int rank, num_of_proc;
	int dimension;

	int terminate = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc);

	/* Arguments sanitation */
	if (argc == 1) {
		if (rank == 0) {
			printf(
					"usage: ./mpiexec -n <number_of_processes> MPI_implementation -d <dimension_of_grid>\n");
			printf("NOTE: dimension_of_grid must be at least '50'\n");
			printf(
					"the program will be executed with default grid size (200x200)\n");
		}
		dimension = DEFAULT_DIMENSION_SIZE;
	} else if (argc == 3) {
		if (strncmp(argv[1], "-d", 2) != 0) {
			if (rank == 0) {
				printf("unknown flag '%s'\n", argv[1]);
				printf(
						"usage: ./mpiexec -n <number_of_processes> MPI_implementation -d <dimension_of_grid>\n");
				printf("NOTE: dimension_of_grid must be at least '50'\n");
				printf("program exiting...\n");
			}
			terminate = 1;
		} else {
			int arg_2 = atoi(argv[2]);
			if (arg_2 < 50) {
				printf("dimension_of_grid must be at least '50'\n");
				printf("program exiting...\n");
				terminate = 1;
			} else {
				dimension = arg_2;
			}
		}
	} else {
		if (rank == 0) {
			printf(
					"usage: ./mpiexec -n <number_of_processes> MPI_implementation -d <dimension_of_grid>\n");
			printf("NOTE: dimension_of_grid must be at least '50'\n");
			printf("program exiting...\n");
			terminate = 1;
		}
	}
	if (terminate == 1) {
		MPI_Finalize();
		exit(0);
	}

	/* Calculate the optimal number of processes that are required */
	SplitAttributes attributes = ProcessNumber(dimension);
	if (num_of_proc != attributes.number_of_processes) {
		if (rank == 0) {
			printf(
					"the given grid is %d x %d\nthe optimal number of processes is %d\n",
					dimension, dimension, attributes.number_of_processes);
			printf("program exiting...\n");
		}
		MPI_Finalize();
		exit(0);
	}

	Execute(rank, num_of_proc, dimension, attributes);

	MPI_Finalize();
	printf("Terminated successfully\n");
	return 0;
}
