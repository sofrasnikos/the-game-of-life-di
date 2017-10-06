#!/bin/sh

clear 
echo "Execution 1"
echo "Processors: 4, Dimension: 30x30, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 30 -l 100;
echo
echo "Execution 2"
echo "Processors: 4, Dimension: 30x30, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 30 -l 1000;
echo
echo "Execution 3"
echo "Processors: 4, Dimension: 300x300, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 300 -l 100;
echo
echo "Execution 4"
echo "Processors: 4, Dimension: 300x300, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 300 -l 1000;
echo
echo "Execution 5"
echo "Processors: 4, Dimension: 3000x3000, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 3000 -l 100;
echo
echo "Execution 6"
echo "Processors: 4, Dimension: 3000x3000, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 4 ../../MPI_implementation/game_of_life -d 3000 -l 1000;
echo
echo "Execution 7"
echo "Processors: 9, Dimension: 30x30, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 30 -l 100;
echo
echo "Execution 8"
echo "Processors: 9, Dimension: 30x30, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 30 -l 1000;
echo
echo "Execution 9"
echo "Processors: 9, Dimension: 300x300, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 300 -l 100;
echo
echo "Execution 10"
echo "Processors: 9, Dimension: 300x300, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 300 -l 1000;
echo
echo "Execution 11"
echo "Processors: 9, Dimension: 3000x3000, Loops: 100"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 3000 -l 100;
echo
echo "Execution 12"
echo "Processors: 9, Dimension: 3000x3000, Loops: 1000"
mpiexec -f ../../MPI_implementation/machines -n 9 ../../MPI_implementation/game_of_life -d 3000 -l 1000;
echo
