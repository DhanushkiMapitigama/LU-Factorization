#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Open files for reading
    FILE* file1 = fopen(argv[1], "rb");
    FILE* file2 = fopen(argv[2], "rb");

    if (file1 == NULL) {
        printf("Failed to open %s.\n", argv[1]);
        return 1;
    }
    if (file2 == NULL) {
        printf("Failed to open %s.\n", argv[2]);
        fclose(file1);
        return 1;
    }

    // Read doubles from both files and compare them
    double num1, num2;
    int count = 0;
    int mismatchCount = 0;

    while (fread(&num1, sizeof(double), 1, file1) == 1 && fread(&num2, sizeof(double), 1, file2) == 1) {
        count++;

        if (num1 != num2) {
            printf("Incorrect results - mismatch at line %d: %lf != %lf\n", count, num1, num2);
            mismatchCount = 1;
            break;
        }
    }

    // Print the comparison result
    if (mismatchCount == 0) {
        printf("Results seems correct - %d values were matching.\n", count);
    } else {
        printf("Results seems wrong. \n");
    }

    // Close the files
    fclose(file1);
    fclose(file2);

    return 0;
}
