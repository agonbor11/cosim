#ifndef __MATMUL_H__
#define __MATMUL_H__
#include <stdint.h> // uint32_t
#define IS_FLOAT 0
#define SIZE 4
#if IS_FLOAT
    typedef float data_t;
#else
    typedef uint32_t data_t;
#endif
void matmul(data_t a[SIZE][SIZE], data_t b[SIZE][SIZE], data_t c[SIZE][SIZE]);
#endif /* __MATMUL_H__ */
