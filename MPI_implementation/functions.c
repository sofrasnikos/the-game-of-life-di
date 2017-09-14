#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>

#include "functions.h"

int createGrid(int ***grid, int dimension) {
	/* Contiguous memory allocation
	 * Allocate rows * columns*/
	int *p = (int *) malloc(dimension * dimension * sizeof(int));
	if (!p)
		return -1;

	/* Allocate rows*/
	(*grid) = (int **) malloc(dimension * sizeof(int*));
	if (grid == NULL) {
		free(grid);
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Set up the pointers*/
	for (int i = 0; i < dimension; i++)
		(*grid)[i] = &(p[i * dimension]);

	return 0;
}

void freeGrid(int ***grid) {
	free(&((*grid)[0][0]));
	free(*grid);
}

void initGrid(int **grid, int dimension) {
	int i, j, r;
	/* Initialize the grid*/
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			/* Range from 1 to 100*/
			r = rand() % 100 + 1;
			/* 30% possibility to create a cell
			 * 70% possibility to create empty space*/
			if (r < 30) {
				grid[i][j] = 1;
			} else {
				grid[i][j] = 0;
			}
		}
	}
}

void readGrid(int **grid, char* filename, int dimension) {
	FILE *f;
	char line[MAXROW + 1];
	printf("Opening %s...\n", filename);
	f = fopen(filename, "r");
	if (!f) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	int j;
	while (1) {
		if (fgets(line, MAXROW + 1, f) == NULL) {
			break;
		}
		int len = strlen(line) - 1;
		if (len != dimension) {
			printf("Found line size different than dimension given\n");
			exit(EXIT_FAILURE);
		}
		for (j = 0; j < len; j++) {
			if (line[j] == '*' || line[j] == '1') {
				grid[i][j] = 1;
			} else if (line[j] == '.' || line[j] == '0') {
				grid[i][j] = 0;
			} else {
				printf("Wrong characters found in %s. Accepted characters are: 01.*\n", filename);
				exit(EXIT_FAILURE);
			}
		}
		i++;
	}
}

void printGrid(int **grid, int dimension, int rank, int glob_grid) {
	int i, j;
	char *filename = malloc(sizeof(char) * 256);
	struct stat buffer;
	int exist;
	i = 0;
	do {
		if (glob_grid == 1) {
			snprintf(filename, 256, "outputs/grid_(%d)", i);
		} else {
			snprintf(filename, 256, "outputs/process_%d_(%d)", rank, i);
		}
		exist = stat(filename, &buffer);
		if (exist == 0) {
			i++;
		}
	} while (exist == 0);

	FILE *fd = fopen(filename, "w+");
	if (!fd) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			if (grid[i][j] == 1) {
				fprintf(fd, "*");
			} else {
				fprintf(fd, ".");
			}
		}
		fprintf(fd, "\n");
	}
	fclose(fd);
	free(filename);
}

int calculateSubgridSize(int dimension, int number_of_processes) {
	int int_root;
	float float_root;
	int subGridSize;
	int remainder;
	float_root = sqrt((double) number_of_processes);
	int_root = float_root;
	// Check if number_of_processes is a perfect square
	// if it's not
	if (int_root != float_root) {
		return -1;
	}
	subGridSize = dimension / int_root;
	remainder = dimension % int_root;
	if (remainder != 0) {
		return -2;
	}
	return subGridSize;
}

