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

#include "functions.h"

int CreateGrid(int ***grid, int rows, int columns) {

	/* allocate the n*m contiguous items */
	int *p = (int *) malloc(rows * columns * sizeof(int));
	if (!p)
		return -1;

	/* allocate the row pointers into the memory */
	(*grid) = (int **) malloc(rows * sizeof(int*));
	if (grid == NULL) {
		free(grid);
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* set up the pointers into the contiguous memory */
	for (int i = 0; i < rows; i++)
		(*grid)[i] = &(p[i * columns]);

	return 0;
}

void FreeGrid(int ***grid) {
	/* free the memory - the first element of the array is at the start */
	free(&((*grid)[0][0]));

	/* free the pointers into the memory */
	free(*grid);
}

void InitGrid(int **grid, int rows, int columns) {
	int i, j, r;
	/* Initialize the grid*/
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
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

void PrintGrid(int **grid, int rows, int columns, int rank) {
	char *filename = malloc(sizeof(char) * 256);
	snprintf(filename, 256, "../outputs/process_%d", rank);
	FILE *fd = fopen(filename, "w+");
	if (!fd) {
		printf("fopen failed\n");
		exit(EXIT_FAILURE);
	}
	int i, j;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
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
SplitAttributes ProcessNumber(int rows) {
	SplitAttributes attributes;
	int x, x_square, inner_number_of_squares, square_side_length;
	int n_square = rows * rows;

	for (x = 1; x <= MAX_SIDE_LENGTH_OF_SQUARES; x++) {
		x_square = x * x;
		//printf("x == %d, x_square == %d\n", x, x_square);
		if (n_square % x_square == 0) {
			inner_number_of_squares = n_square / x_square;
			square_side_length = x;
		}
	}

	attributes.length_of_sides = square_side_length;
	attributes.number_of_processes = inner_number_of_squares;
	return attributes;
}
