#include "keccak.h"


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

// Main application
int main(int argc, char *argv[]) {
     // Allocate data buffers
     int h,v;
       int l=2;
       char M[3]="11";

       uint8_t output[1024];

       output[0]=0x54;

       keccak_256(M,l,output);



       for (int ijj = 0; ijj < 32; ijj++) {
     	  printf("O_8bits[%d] = %"PRIx8"; ",ijj,output[ijj]);
       }

    //Execute HW version
    printf("\nFinal\n");
    return 0;
}
