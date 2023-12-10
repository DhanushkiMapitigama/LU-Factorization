#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include <sys/resource.h>

// Program Modes
 // Naive - 0  
 // Naive/OMP - 1 
 // With Parallelized Improvements - 2

#define program_mode 2

void create_matrix_file(int n);
void create_random_matrix_file(int n);
double** read_mat(int n, char *filename);
void showMatrix(int size, double** matrix);
void save_LU_matrix_file(int n, double**matrix);
void save_LU_transpose_matrix_file(int n, double**matrix); // In mode-2 we have the matrix as a transpose and we need to change rows and cols
double get_wall_seconds();

int main(int argc, char *argv[]) {

    int size, nthr = 4;

    // Argument Handling
    // Size and Matrix file name - both arguments are mandatory
    // You can customize the thread count by giving a third argument -
        // If thread count is not given program will by default will run with 4 threads in parallel mode
        // If the argument is given and program is running in non-parallel mode, the argument will be discarded

    printf("\n"); // Print new line for clarity of messages

    if (argc < 3)
    {
        printf("Incorrect number of arguments!\n");
        printf("Usage: %s Size_of_the_square_matrix Matrix_file_name Thread_count(default=4)\n\n", argv[0]);
        return 0;
    }

    if (atoi(argv[1]) > 0) {
        size = atoi(argv[1]);
    } else {
        printf("Invalid value %s for array length. Please enter a positive integer\n\n", argv[1]);
        return 0;
    } 

    if (argv[3] && (atoi(argv[3]) > 0)) {
        nthr = atoi(argv[3]);
    } else if (argv[3]) {
        printf("Invalid value %s for thread count. Please enter a positive integer\n\n", argv[3]);
        return 0;
    } else {
        if (program_mode ==1 || program_mode == 2) {
            printf("Program is running in parallel mode and no argument is given for thread count. Program will run with 4 threads.\n\n");        
        }
    } 


    // create_random_matrix_file(20000); // For test case generation - answers were generated with naive LU code without openmp
    // create_matrix_file(5); // For test case generation

    double** matrix = read_mat(size, argv[2]);

    double LUstartTime, LUtotalTime;  

    # if program_mode == 0  // Naive

        printf("Program is running in MODE 0 - Naive mode without any parallelization!\n\n");
        LUstartTime = omp_get_wtime();

        double** result_matrix = (double**)malloc(size * sizeof(double*)); // Allocate memory for the matrix
        for (int i = 0; i < size; i++) {
            result_matrix[i] = (double*)malloc(size * sizeof(double));
        }

        for (int k = 0; k < size; k++) {
            for (int i = 0; i < size; i++) {
                result_matrix[i][k] = matrix[i][k];
            }
        }

        double div_to_mul_kk_element;
        for (int k = 0; k < size; k++) {
            div_to_mul_kk_element = 1 / result_matrix[k][k];
            for (int i = k + 1; i < size; i++) {
                result_matrix[i][k] = result_matrix[i][k] * div_to_mul_kk_element;
            }
            for (int i = k + 1; i < size; i++) {
                for (int j = k + 1; j < size; j++) {
                    result_matrix[i][j] -= result_matrix[i][k] * result_matrix[k][j];
                }
            }       
        }

        
        LUtotalTime = omp_get_wtime() - LUstartTime;
        printf("Time taken for the LU factorization of %d by %d Matrix = %lf seconds.\n\n", size, size, LUtotalTime);
        save_LU_matrix_file(size, result_matrix); // To compare with given outputs for correctness

    
    #elif program_mode == 1  // Naive with OMP parallelization on 2nd loop

        printf("Program is running in MODE 1 - Naive mode with openmp parallelization on the loop 2!\n\n");
        LUstartTime = omp_get_wtime();

        double** result_matrix = (double**)malloc(size * sizeof(double*)); // Allocate memory for the matrix
        for (int i = 0; i < size; i++) {
            result_matrix[i] = (double*)malloc(size * sizeof(double));
        }

        for (int k = 0; k < size; k++) {
            for (int i = 0; i < size; i++) {
                result_matrix[i][k] = matrix[i][k];
            }
        }

        double div_to_mul_kk_element;
        for (int k = 0; k < size; k++) {
            div_to_mul_kk_element = 1 / result_matrix[k][k];
            for (int i = k + 1; i < size; i++) {
                result_matrix[i][k] = result_matrix[i][k] * div_to_mul_kk_element;
            }
            #pragma omp parallel for num_threads(nthr)
            for (int i = k + 1; i < size; i++) {
                for (int j = k + 1; j < size; j++) {
                    result_matrix[i][j] -= result_matrix[i][k] * result_matrix[k][j];
                }
            }       
        }

        LUtotalTime = omp_get_wtime() - LUstartTime;
        printf("Time taken for the LU factorization of %d by %d Matrix = %lf seconds.\n\n", size, size, LUtotalTime);
        save_LU_matrix_file(size, result_matrix); // To compare with given outputs for correctness

    
    #elif program_mode == 2  // Improvements

        printf("Program is running in MODE 2 - Improved mode with one large parallel region and locks where necessary to avoid false updates!\n\n");

        double** result_matrix = (double**)malloc(size * sizeof(double*)); // Allocate memory addresses for the columns of result matrix 

        int k, col, nlim;
        int start = 0;

        // If the thread count bigger than the size of the matrix, thread count is reduced to the size of the matrix
        if (nthr > size){
            nthr = size;
        }

        omp_lock_t columnLock[size]; // define and init locks for columns
        for (int i =0; i<size; i++){
            omp_init_lock(&columnLock[i]);
        }
        
        nlim = size-nthr+1; // Should be greater than 0 otherwise the parallelization will not work and initialization will not happen

        LUstartTime = omp_get_wtime();
        
        // One large parallel region including k-loop
        #pragma omp parallel private(k, col, start) shared(nlim, columnLock) num_threads(nthr) 
        {
            int thr_id = (int)omp_get_thread_num();
            
            // First touch - parallel initialization
            for (int col = thr_id; col < size; col+=nthr) {
                result_matrix[col] = (double*)malloc(size * sizeof(double));
                for (int row = 0; row < size; row++) {
                    result_matrix[col][row] = matrix[row][col];
                }
                omp_set_lock(&columnLock[col]);
            }

            #pragma omp barrier

            // Using locks to avoid serial section interleave with loop 2
            if (thr_id == 0){
                double div_to_mul = 1/result_matrix[0][0];
                for (int i = 1; i < size; i++) {
                    result_matrix[0][i] = result_matrix[0][i]  * div_to_mul ;
                }
                omp_unset_lock(&columnLock[0]);  
            }

            for (k = 0; k < nlim; k++) {            
                
                omp_set_lock(&columnLock[k]);
                omp_unset_lock(&columnLock[k]);

                start = (k/nthr)*nthr;
                if ((start + thr_id) <= k){
                    start+=nthr;
                }
             
                for (col = start + thr_id ; col < size; col+=nthr) {
                    for (int j = k + 1; j < size; j++) {
                        result_matrix[col][j] -= result_matrix[k][j] * result_matrix[col][k];
                    }
                    if (col == (k+1) && col<nlim){
                        double div_to_mul_2 = 1 / result_matrix[col][col];
                        for (int i = col+1; i < size; i++) {
                            result_matrix[col][i] = result_matrix[col][i] * div_to_mul_2;
                        }
                        omp_unset_lock(&columnLock[k+1]); // Release lock for the column
                    }
                }

            }
        }

        // Do the computations serially for nlim to n
        for (k = nlim; k < size; k++) {   
            double div_to_mul_kk_element = 1 / result_matrix[k][k];
            for (int i = k + 1; i < size; i++) {
                result_matrix[k][i] = result_matrix[k][i] * div_to_mul_kk_element;
            }
            for (int i = k + 1; i < size; i++) {
                for (int j = k + 1; j < size; j++) {
                    result_matrix[j][i] -= result_matrix[k][i] * result_matrix[j][k];
                }
            }    
        }

        LUtotalTime = omp_get_wtime() - LUstartTime;
        
        printf("Time taken for the LU factorization of %d by %d Matrix = %lf seconds.\n\n", size, size, LUtotalTime);

        save_LU_transpose_matrix_file(size, result_matrix); // To compare with given outputs for correctness

    #else

    free(matrix);
    printf("Please select the preferred mode to run the program!\n");
    return 0;

    #endif

    free(result_matrix);

    free(matrix); // free main matrix

    return 0;

}

double get_wall_seconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
    return seconds;
}


double** read_mat(int n, char *filename){

    printf("Reading matrix\n\n");

    FILE *input_file = fopen(filename, "rb"); // Open input file

    if (!input_file)
    {
        printf("Error: failed to open input file '%s'.\n", filename);
        exit(0);
    }

    fseek(input_file, 0L, SEEK_END); // filesize using fseek() and ftell()
    size_t fileSize = ftell(input_file);

    // Use fseek() again to set file position back to beginning of the file.
    fseek(input_file, 0L, SEEK_SET);

    if (fileSize != n * n * sizeof(double))
    {
        printf("Error: size of input file '%s' does not match with the given n.\n", filename);
        exit(0);
    }

    double* buffer = (double *)malloc(n*n*sizeof(double)); // Allocate buffer to read inputs

    if (!fread(buffer, sizeof(char), fileSize, input_file))
    {
        printf("Failed to read.\n");
    }

    double** matrix = (double**)malloc(n * sizeof(double*)); // Allocate memory for the matrix
    for (int i = 0; i < n; i++) {
        matrix[i] = (double*)malloc(n * sizeof(double));
    }

    double temp;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++){
            temp = buffer[(n * i) + j];
            matrix[i][j] = temp;
        }
    }

    free(buffer);
    fclose(input_file);
    return matrix;
}

void save_LU_transpose_matrix_file(int n, double**matrix) // This is used in mode - 2
{

    printf("Saving result matrix to result.mat file!\n\n");

    FILE *output_file = fopen("result.mat", "wb");

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            fwrite(&matrix[j][i], sizeof(double), 1, output_file);
        }
    }
    fclose(output_file);
}

void save_LU_matrix_file(int n, double**matrix)
{

    printf("Saving result matrix to result.mat file!\n\n");

    FILE *output_file = fopen("result.mat", "wb");

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            fwrite(&matrix[i][j], sizeof(double), 1, output_file);
        }
    }
    fclose(output_file);
}

void create_random_matrix_file(int n) {

    FILE *output_file = fopen("n20000.mat", "wb");

    srand(time(NULL));

    double random;
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            random = rand() % 100 + 0.1;
            fwrite(&random, sizeof(double), 1, output_file);
        }
    }
    fclose(output_file);
}

void showMatrix(int n, double** matrix) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%lf\t", matrix[i][j]);
        }
        printf("\n"); //seperate row
    }
    printf("\n");
}

// For small test cases
void create_matrix_file(int n) { 

    FILE *output_file = fopen("n5.mat", "wb");

    // double arr[] = {2, 1, 4, 3, 4, -1, 1, -2, 1};
    double arr [] = {1.0, 2.0, 3.0, 4.0, 5.0,
        6.0, 7.0, 8.0, 9.0, 10.0,
        11.0, 12.0, 13.0, 14.0, 15.0,
        16.0, 17.0, 18.0, 19.0, 20.0,
        21.0, 22.0, 23.0, 24.0, 25.0};
    int k =0;

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            fwrite(&arr[k], sizeof(double), 1, output_file);
            k++;
        }
    }
    fclose(output_file);
}