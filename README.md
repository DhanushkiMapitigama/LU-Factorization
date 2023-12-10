LU decomposition also known as Lower-Upper matrix decomposition is used to factorize a given square matrix A; as a multiplication of 2 matrices of the same dimensions L and U where L is a Lower triangular matrix and U is an Upper triangular matrix.

A = LU

The LU decomposition approach is vastly used in the fields of Numerical Analysis, Optimization and control theory, matrix computations etc. One of the major application of LU decomposition is solving systems of linear equations.

# Program Inputs
The program needs multiple inputs which should be given as command line arguments.
• Size of the matrix - A positive integer
• Matrix file - A binary file which contains the values of the matrix
• Number of threads (Not mandatory) - A positive integer. In case the program is running in non-parallel mode the value will be discarded. If the program is running in parallel mode and value is not provided, the program will run with 4 threads which has been defined as the default value.

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

