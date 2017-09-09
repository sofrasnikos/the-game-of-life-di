/*
 * functions.h
 *
 *  Created on: Jul 23, 2017
 *      Author: vangelis
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#define MAXROW 1024

/* This struct is used to determine how many processes are needed
 * and to pass the length of the inner squares sides*/
typedef struct SplitAttributes {
	int number_of_processes;
	int length_of_sides;
} SplitAttributes;

int createGrid(int ***grid, int dimension);
void freeGrid(int ***grid);
void initGrid(int **grid, int dimension);
void readGrid(int **grid, char* filename, int dimension);
void printGrid(int **grid, int dimension, int rank, int glob_grid);
void printGrid2(int **grid, int dimension, int rank, int glob_grid);
SplitAttributes processNumber(int rows);

#endif /* FUNCTIONS_H_ */
