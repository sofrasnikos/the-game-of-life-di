/*
 * functions.h
 *
 *  Created on: Jul 23, 2017
 *      Author: vangelis
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

int** CreateGrid(int rows, int columns);
void FreeGrid(int **grid, int rows, int columns);
void InitGrid(int **grid, int rows, int columns);
void PrintGrid(int **grid, int rows, int columns, int rank);

#endif /* FUNCTIONS_H_ */
