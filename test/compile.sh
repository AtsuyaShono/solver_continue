#!/bin/sh
echo g++ -o cada0122 -std=c++11 -Ofast -march=native ../solver-project-final/solver.cpp -Xpreprocessor -fopenmp -lomp
g++ -o cada0122 -std=c++11 -Ofast -march=native ../solver-project-final/solver.cpp -Xpreprocessor -fopenmp -lomp
