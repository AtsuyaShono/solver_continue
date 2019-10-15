#!/bin/sh

echo "synopsys01.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys01.txt ../outputs/01_out.txt
echo "synopsys02.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys02.txt ../outputs/02_out.txt
echo "synopsys03.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys03.txt ../outputs/03_out.txt
echo "synopsys04.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys04.txt ../outputs/04_out.txt
echo "synopsys05.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys05.txt ../outputs/05_out.txt
echo "synopsys06.txt"
time env OMP_NUM_THREADS=16 ./cada0122 ../testcases/synopsys06.txt ../outputs/06_out.txt
