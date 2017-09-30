#!/bin/sh

clear 
echo "Execution 1"
echo "Processors: 4, Dimension: 100x100, Loops: 100"
mpiexec -n 4 ../../MPI_implementation/game_of_life -d 100 -l 100;
echo
echo "Execution 2"
echo "Processors: 4, Dimension: 100x100, Loops: 1000"
mpiexec -n 4 ../../MPI_implementation/game_of_life -d 100 -l 1000;
echo
echo "Execution 3"
echo "Processors: 4, Dimension: 1000x1000, Loops: 100"
mpiexec -n 4 ../../MPI_implementation/game_of_life -d 1000 -l 100;
echo
echo "Execution 4"
echo "Processors: 4, Dimension: 1000x1000, Loops: 1000"
mpiexec -n 4 ../../MPI_implementation/game_of_life -d 1000 -l 1000;
