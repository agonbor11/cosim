#include <string.h>
#include "matmul.h"

void matmul(data_t a[SIZE][SIZE], data_t b[SIZE][SIZE],data_t c[SIZE][SIZE]) {
 unsigned int i, j, k;
// Perform computation
 	 	 Loop_Rows: for (i = 0; i < SIZE; i++) {
 	 		 	 	 Loop_Columns: for (j = 0; j < SIZE; j++) {
 	 		 	 		 	 	 	 data_t aux = 0;
 	 		 	 		 	 	 	 	 Loop_Product: for (k = 0; k < SIZE; k++) {
 	 		 	 		 	 	 	 		 	 	 aux += a[i][k] * b[k][j];
 	 		 	 		 	 	 	 	 }
 	 		 	 		 	 	 	  c[i][j] = aux;
 	 		 	 	 }
 	 	 }
 }

