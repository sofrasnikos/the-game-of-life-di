/*
 * execute.h
 *
 *  Created on: Aug 8, 2017
 *      Author: vangelis
 */

#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "functions.h"

int execute(int rank, int num_of_proc, int dimension,
		SplitAttributes attributes, int number_of_loops, char *file);

void calculateInnerCells(int block_dimension, int **local_grid,
		int **next_local_grid);

void calculateEdgeCells(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value);

int deadOrAlive(int alive_neighbors, int status);

#endif /* EXECUTE_H_ */
