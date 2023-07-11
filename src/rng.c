#include <efi.h>
#include "rng.h"

// implementation of a xorshiro256** PRNG based on:
// https://en.wikipedia.org/wiki/Xorshift#xoshiro256**
// (i would explain how it works if i knew)

static UINT64 s[4];

VOID srand(UINT64 seed) {
    *s = seed;
    // first few values generated are not very random
    // so we prime it a little.
    for (UINTN i = 0; i < 8; ++i) (VOID)rand();
}

inline UINT64 roll64(UINT64 x, INT32 k) {
    return (x << k) | (x >> (64 - k));
}

UINT64 rand(VOID) {
    UINT64 const result = roll64(s[1] * 5, 7) * 9;
    UINT64 const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = roll64(s[3], 45);

   return result;
}
