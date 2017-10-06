#!/bin/sh

clear 
echo "Execution 1"
echo "Processors: 1, Dimension: 30x30, Loops: 100"
../../CUDA_implementation/game_of_life -d 30 -l 100;
echo
echo "Execution 2"
echo "Processors: 1, Dimension: 300x300, Loops: 100"
../../CUDA_implementation/game_of_life -d 300 -l 100;
echo
echo "Execution 3"
echo "Processors: 1, Dimension: 3000x3000, Loops: 100"
../../CUDA_implementation/game_of_life -d 3000 -l 100;
echo
######################################################
echo "Execution 4"
echo "Processors: 4, Dimension: 30x30, Loops: 100"
../../CUDA_implementation/game_of_life -d 30 -l 100;
echo
echo "Execution 5"
echo "Processors: 4, Dimension: 300x300, Loops: 100"
../../CUDA_implementation/game_of_life -d 300 -l 100;
echo
echo "Execution 6"
echo "Processors: 4, Dimension: 3000x3000, Loops: 100"
../../CUDA_implementation/game_of_life -d 3000 -l 100;
echo
######################################################
echo "Execution 7"
echo "Processors: 9, Dimension: 30x30, Loops: 100"
../../CUDA_implementation/game_of_life -d 30 -l 100;
echo
echo "Execution 8"
echo "Processors: 9, Dimension: 300x300, Loops: 100"
../../CUDA_implementation/game_of_life -d 300 -l 100;
echo
echo "Execution 9"
echo "Processors: 9, Dimension: 3000x3000, Loops: 100"
../../CUDA_implementation/game_of_life -d 3000 -l 100;
echo
