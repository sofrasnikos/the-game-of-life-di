/*
 * functions.c
 *
 *  Created on: Jul 23, 2017
 *      Author: vangelis
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "functions.h"

#define MAX_SIDE_LENGTH_OF_SQUARES 40

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
	FILE* f;
	char line[MAXROW + 1];
	printf("%s\n", filename);
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
		printf("%3d: %s", i, line);
		int len = strlen(line) - 1;
		if (len != dimension) {
			printf("Found line greater than dimension given\n");
			exit(EXIT_FAILURE); //todo den 3erw kata poso einai swsto na termatizoume etsi
		}
		for (j = 0; j < len; ++j) {
			if (line[j] == '*' || line[j] == '1') {
				grid[i][j] = 1;
			} else if (line[j] == '.' || line[j] == '0') {
				grid[i][j] = 0;
			} else {
				printf(
						"Wrong characters found in %s. Accepted characters are: 01.*\n",
						filename);
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
			snprintf(filename, 256, "../outputs/grid_(%d)", i);
		} else {
			snprintf(filename, 256, "../outputs/process_%d_(%d)", rank, i);
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
				//printf("*");
				fprintf(fd, "*");
			} else {
				//printf(".");
				fprintf(fd, ".");
			}
		}
		fprintf(fd, "\n");
	}
	fclose(fd);
	free(filename);
}

/* This works only for squares*/
SplitAttributes processNumber(int dimension) {
	SplitAttributes attributes;
	int x, x_square, inner_number_of_squares, square_side_length;
	int n_square = dimension * dimension;

	for (x = 1; x <= MAX_SIDE_LENGTH_OF_SQUARES; x++) {
		x_square = x * x;
		//printf("x == %d, x_square == %d\n", x, x_square);
		if (n_square % x_square == 0) {
			inner_number_of_squares = n_square / x_square;
//			printf("number of proc %d, length of side %d\n", inner_number_of_squares, x);
			square_side_length = x;
		}
	}

	attributes.length_of_sides = square_side_length;
	attributes.number_of_processes = inner_number_of_squares;
	return attributes;
}
