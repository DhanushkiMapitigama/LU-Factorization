all: LU compare_results

LU:
	gcc -O3 -o LU LU.c -fopenmp

compare_results:
	gcc -o compare_results compare_results.c

clean:
	rm -f LU compare_results
