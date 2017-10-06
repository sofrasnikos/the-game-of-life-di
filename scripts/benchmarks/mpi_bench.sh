#!/bin/sh

clear 
echo "Execution 1"
echo "Processors: 1, Dimension: 30x30, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 1 ../../MPI_implementation/game_of_life -d 30 -l 500;
echo
echo "Execution 2"
echo "Processors: 1, Dimension: 300x300, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 1 ../../MPI_implementation/game_of_life -d 300 -l 500;
echo
echo "Execution 3"
echo "Processors: 1, Dimension: 3000x3000, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 1 ../../MPI_implementation/game_of_life -d 3000 -l 500;
echo
########################################################################################################
echo "Execution 4"
echo "Processors: 4, Dimension: 30x30, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 30 -l 500;
echo
echo "Execution 5"
echo "Processors: 4, Dimension: 300x300, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 300 -l 500;
echo
echo "Execution 6"
echo "Processors: 4, Dimension: 3000x3000, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 3000 -l 500;
echo
########################################################################################################
echo "Execution 7"
echo "Processors: 9, Dimension: 30x30, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 30 -l 500;
echo
echo "Execution 8"
echo "Processors: 9, Dimension: 300x300, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 300 -l 500;
echo
echo "Execution 9"
echo "Processors: 9, Dimension: 3000x3000, Loops: 500"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 3000 -l 500;
echo
