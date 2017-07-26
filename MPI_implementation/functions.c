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

int** CreateGrid(int rows, int columns) {
	int i;
	int **grid;
	/* Allocate grid*/
	grid = malloc(sizeof(int *) * rows);
	if (grid == NULL) {
		printf("malloc error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < rows; i++) {
		grid[i] = malloc(sizeof(int) * columns);
		if (grid[i] == NULL) {
			printf("malloc error %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	return grid;
}

void FreeGrid(int **grid, int rows, int columns) {
	int i;
	for (i = 0; i < rows; i++) {
		free(grid[i]);
	}
	free(grid);
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
