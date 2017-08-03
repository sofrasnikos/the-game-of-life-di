/*
 * functions.h
 *
 *  Created on: Jul 23, 2017
 *      Author: vangelis
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

/* This struct is used to determine how many processes are needed
 * and to pass the length of the inner squares sides*/
typedef struct SplitAttributes {
	int number_of_processes;
	int length_of_sides;
} SplitAttributes;

int CreateGrid(int ***grid, int dimension);
void FreeGrid(int ***grid);
void InitGrid(int **grid, int dimension);
void PrintGrid(int **grid, int dimension, int rank, int glob_grid);
SplitAttributes ProcessNumber(int rows);

#endif /* FUNCTIONS_H_ */
