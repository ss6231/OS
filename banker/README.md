Written in C

ABOUT:
Simulating resource allocation with an optimistic resource manager and Djikstra's banker algorithm. The optimistic manager grants requests in a FIFO manner, while the banker first checks for safe states.


NOTE: 
-The main programs are listed under "main_banker.c" and "main_fifo.c" with the corresponding algorithms in "fifo.c" and "banker.c" and all the structs are located in struc.c
-My fifo and banker algorithms are run with their own main program. See below for explanation: 
-All input files that were given are provided in this zipped file in the format "input-xx.txt"

To compile:

1) gcc -c main_fifo.c
2) gcc -c main_banker.c
2) gcc -c fifo.c (ignore warnings)
3) gcc -c banker.c (ignore warnings)
To run FIFO:
4) gcc -o main_fifo main_fifo.o fifo.o 
5) ./main_fifo "inputFileHere"
To run Banker:
6) gcc -o main_banker main_banker.o banker.o
7) ./main_banker "inputFileHere"
