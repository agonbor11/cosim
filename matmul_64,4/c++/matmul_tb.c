#include <stdio.h>
#include <stdlib.h>
#include <math.h>     // sqrt() -> needs -lm when linking
#include <float.h>    // FLT_MIN
#include "matmul.h"

#if IS_FLOAT
    static inline unsigned int ftoi(float f) {
        union { float f; unsigned int u; } un;
        un.f = f;
        return un.u;
    }
#else
    #define ftoi(...) __VA_ARGS__
#endif


// Software reference implementation
void matmul_sw(data_t a[SIZE][SIZE], data_t b[SIZE][SIZE], data_t c[SIZE][SIZE]) {
    unsigned int i, j, k;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0;
            for (k = 0; k < SIZE; k++) {
            	c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}


// Main application
int main(int argc, char *argv[]) {

    unsigned int i, j, k, i2, j2, errors;
    float max_error;

     // Allocate data buffers
     data_t a[SIZE][SIZE];
     data_t b[SIZE][SIZE];
     data_t hw[SIZE][SIZE];
     data_t sw[SIZE][SIZE];

     for (i = 0; i < SIZE; i++) {
     	for (j = 0; j < SIZE; j++) {
 #if IS_FLOAT
     		a[i][j] = i / sqrt(i + 1.0);
     		b[i][j] = i / (i + 1.0);
 #else
     		a[i][j] = rand() % 256;
     		b[i][j] = rand() % 256;
 #endif
     		hw[i][j] = 0;
     	}
     }


    //Execute HW version
    matmul(a, b, hw);

    //Execute SW version
    matmul_sw(a, b, sw);

    // Check results vs. software reference
    printf("Checking results...\n");
    errors = 0;
    max_error = 0.0;
    for (i = 0; i < SIZE; i++) {
    	for (j = 0; j < SIZE; j++) {

    		if (fabsf(hw[i][j] - sw[i][j] ) > fabsf(1e-5 * sw[i][j])) errors++;
    		if (fabsf(sw[i][j]) > FLT_MIN) {
    			float error = fabsf((hw[i][j] - sw[i][j] ) / sw[i][j] );
    			max_error = (error > max_error) ? error : max_error;
    		}
    	}
    }
    printf("Found %d errors\n", errors);
    printf("Maximum relative error is %g\n", max_error);

    // Show partial results
    printf("A:\n");
    for (i = 0; i < SIZE; i++) {
        printf("    ");
    	for (j = 0; j < SIZE; j++) {
    		printf("%08x ", ftoi(a[i][j]));
    	}
        printf("\n");
    }

    printf("B:\n");
    for (i = 0; i < SIZE; i++) {
        printf("    ");
    	for (j = 0; j < SIZE; j++) {
    		printf("%08x ", ftoi(b[i][j]));
    	}
        printf("\n");
    }

    printf("SOFTWARE:\n");
    for (i = 0; i < SIZE; i++) {
        printf("    ");
    	for (j = 0; j < SIZE; j++) {
    		printf("%08x ", ftoi(sw[i][j]));
    	}
        printf("\n");
    }

    printf("HARDWARE:\n");
    for (i = 0; i < SIZE; i++) {
        printf("    ");
    	for (j = 0; j < SIZE; j++) {
    		printf("%08x ", ftoi(hw[i][j]));
    	}
        printf("\n");
    }

    return errors ? 1 : 0;
}

