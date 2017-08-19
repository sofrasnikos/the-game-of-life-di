/*
 * execute.h
 *
 *  Created on: Aug 8, 2017
 *      Author: vangelis
 */

#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "functions.h"

int Execute(int rank, int num_of_proc, int dimension,
		SplitAttributes attributes);

void CalculateInnerCells(int block_dimension, int **local_grid,
		int **next_local_grid);

void CalculateEdgeCells(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value);

void CalculateEdgeCellsOpt(int block_dimension, int **local_grid,
		int **next_local_grid, int *top_buff, int *right_buff, int *bot_buff,
		int *left_buff, int top_left_value, int top_right_value,
		int bot_left_value, int bot_right_value);

#endif /* EXECUTE_H_ */
