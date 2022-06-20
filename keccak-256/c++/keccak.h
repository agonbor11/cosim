#ifndef KECCAK_H
#define KECCAK_H

#include <inttypes.h>
#include <assert.h>
#include <string.h> /* memcpy, memset */
#include <stdio.h>
#include <stdlib.h> /* malloc */

typedef struct {
	int b, l, w, nr;
} keccak_t;

/* 64 bitwise rotation to left */
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

void keccak_256(uint8_t M[32], int l, uint8_t O[32]);

#endif
