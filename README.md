# Introduction 
LU decomposition also known as Lower-Upper matrix decomposition is used to factorize a given square matrix A; as a multiplication of 2 matrices of the same dimensions L and U where L is a Lower triangular matrix and U is an Upper triangular matrix. The LU decomposition approach is vastly used in the fields of Numerical Analysis, Optimization and control theory, matrix computations etc. One of the major application of LU decomposition is solving systems of linear equations.

Even though the naive algorithm is simple and straight forward, when dealing with larger matrices (eg: a big system of linear equations) the algorithm is not efficient and will take a considerable amount of time to process. In the fields of Image processing, Data analysis as well as in complex circuit analysis and Finite element analysis where large vectorized data (matrices) are processed we cannot spare a longer time to process one single data point, thus proper parallelization of the algorithm is required in-order get the maximum throughput.

There are many ways of achieving parallelism in LU decomposition.
- Using proper loop parallelization technique
- Using Task parallelism technique
- Using Block LU factorization

Focus of this project was on improving the performance of LU decomposition by implementing the improved version of the algorithm using Open-MP loop parallelism. 

# Implementation 

Naive parellilization over inner loop is straight forward but the improved implementation needed careful consideration of below aspects.

- Usage of locks to avoid serial and parallel computations interleaving with each-other and to maintain the synchronization and to avoid lost/incorrect update problems
- Usage of private and shared variables within the threads
- Usage of Thread synchronization where necessary

# Program Inputs
The program needs multiple inputs which should be given as command line arguments.
- Size of the matrix - A positive integer
- Matrix file - A binary file which contains the values of the matrix
- Number of threads (Not mandatory) - A positive integer. In case the program is running in non-parallel mode the value will be discarded. If the program is running in parallel mode and value is not provided, the program will run with 4 threads which has been defined as the default value.

In addition to this the program mode must be set within the code by changing
the program mode constantv

# Program Outputs
The program has 2 outputs 
- The resultant LU matrix - The result matrix is saved to a binary file "result.mat" which can be used to compare with expected outputs using the compare results program
- The time taken (wall clock timings) for the LU factorization printed on terminal.

# Result Comparison
The file comparison program does not have any error handling. (As it is out of the project scope)

Please make sure to give 2 binary files as command line arguments to the program.

Eg:
./compare_results result.mat sample_outputs/n100_result.mat

