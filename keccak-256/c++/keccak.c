#include "keccak.h"

/* Round constants */

const uint64_t RC[24] =
{
  0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
  0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
  0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
  0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
  0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
  0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
  0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
  0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

/* Rotation offsets, y vertically, x horizontally: r[y * 5 + x] */
const int rx[25] = {
  0, 1, 62, 28, 27,
  36, 44, 6, 55, 20,
  3, 10, 43, 25, 39,
  41, 45, 15, 21, 8,
  18, 2, 61, 56, 14
};

void theta(uint64_t* state)
{
  /* Theta */

  uint64_t C[5] = {0, 0, 0, 0, 0};
  uint64_t D[5] = {0, 0, 0, 0, 0};

  int x, y;
  for (x = 0; x < 5; ++x) {
    C[x] = state[x] ^ state[5 + x] ^ state[10 + x] ^ state[15 + x] ^ state[20 + x];
  }

  for (x = 0; x < 5; ++x) {
    /* in order to avoid negative mod values,
      we've replaced "(x - 1) % 5" with "(x + 4) % 5" */
    D[x] = C[(x + 4) % 5] ^ ROTL64(C[(x + 1) % 5], 1);

    for (y = 0; y < 5; ++y) {
      state[y * 5 + x] = state[y * 5 + x] ^ D[x];
    }
  }
}

void rho(uint64_t* state)
{
  /* Rho */
  int x, y;
  for (y = 0; y < 5; ++y) {
    for (x = 0; x < 5; ++x) {
      state[y * 5 + x] = ROTL64(state[y * 5 + x], rx[y * 5 + x]);
    }
  }
}

void pi(uint64_t* state)
{
  /* Pi */
  uint64_t B[25];

  int x, y;
  for (y = 0; y < 5; ++y) {
    for (x = 0; x < 5; ++x) {
      B[y * 5 + x] = state[5 * y + x];
    }
  }
  int u, v;
  for (y = 0; y < 5; ++y) {
    for (x = 0; x < 5; ++x) {
      u = (0 * x + 1 * y) % 5;
      v = (2 * x + 3 * y) % 5;

      state[v * 5 + u] = B[5 * y + x];
    }
  }
}

void chi(uint64_t* state)
{
  /* Chi */
  uint64_t C[5];

  int x, y;
  for (y = 0; y < 5; ++y) {
    for (x = 0; x < 5; ++x) {
      C[x] = state[y * 5 + x] ^ ((~state[y * 5 + ((x + 1) % 5)]) & state[y * 5 + ((x + 2) % 5)]);
    }

    for (x = 0; x < 5; ++x) {
      state[y * 5 + x] = C[x];
    }
  }
}

void iota(uint64_t* state, int i)
{
  /* Iota */
  /* XXX: truncate RC[i] if w < 64 */
  state[0] = state[0] ^ RC[i];
}

////////////////////////////
int keccakf(int rounds, uint64_t* state)
{
  int i;
  for (i = 0; i < rounds; ++i) {
    theta(state);
    rho(state);
    pi(state);
    chi(state);
    iota(state, i);
  }

  return 0;
}
////////////////////////////

void sponge_absorb(int nr, int r, int w, int l, uint64_t* A, uint8_t* P)
{
	 /* absorbing phase */
	  int x, y;
	  int blocks = l / (r / 8);

	  /* for every block Pi in P */
	  for (y = 0; y < blocks; ++y) {
	    uint64_t* block = (uint64_t*)P + y * r/w;

	  for (x = 0; x < (r/w); ++x) {
	      A[x] = A[x] ^ block[x];
	    }

	    /* S = Keccak-f[r + c](S) */
	    keccakf(nr, A);
	  }
}
///////////////////////////////////////hasta aqui bn
void sponge_squeeze(int nr, int r, int n, uint64_t* A, uint8_t* O)
{  /*
    For SHA-3 we have r > n in any case, i.e., the squeezing phase
      consists of one round.
   */
  int i = 0;
  while (n) {
    size_t size = r;

    if (r > n) {
        size = n;
    }
    /* Copies A[0:size/8] to O[i:i + size/8 - 1] */
    memcpy(&O[i], A, size/8);
    i = i + size/8;

    n = n - size;

    if (n > 0) {
      keccakf(nr, A);
    }
  }
}

int pad101(int r, int blocks, int l, uint8_t* M, uint8_t* P)
{
  int block_size = 136;//int block_size = r/8;

  /* length of the padded block */
  size_t block_len = (blocks + 1) * block_size;//

  /* zero out data and copy M into P */
  memset(P, 0, block_len * sizeof(uint8_t));

  int i;
  for (i = 0; i < l; ++i) {
      P[i] = M[i];
  }

  /* add padding bytes */
  P[l] = 0x01;
  P[block_len - 1] = 0x80;

  /* padding */
  if (l % block_size == 0) {
    return l;
  }

  return block_len;
}
void keccak_256(uint8_t M[32], int l, uint8_t O[32]){

	int r=1088;
	int c=512;
	int n=256;
	/* check permutation width */
	  int b =1600; //b=r + c;
	  int i, j = 0;
	uint64_t A[25];
	  /* zero out the state */
	  memset(A, 0, 25 * sizeof(uint64_t));

	  /* lane width */
	  int w = 64;
	  /* number of rounds */
	  int nr =24;
	  /* block size in bytes */
	  int block_size = 136; //int block_size = r/8;//136

	  /* calculate how many blocks M consist of */
	  int blocks = l / block_size;
	  /* make room for padding, if necessary */
	  uint8_t P[136];//P[block_size * (blocks + 1)];

	l = pad101(r, 0, l, M, P);
	sponge_absorb(24,1088,64,l,A,P);
	sponge_squeeze(24, 1088, 256, A, O);
 }


