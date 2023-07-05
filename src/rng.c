#include <efi.h> 
#include "rng.h"

// implementation of a xorshiro256** PRNG based on:
// https://en.wikipedia.org/wiki/Xorshift#xoshiro256**
// (i would explain how it works if i knew)

static UINT64 s[4];

VOID srand(UINT64 seed) {
    // modulo prevents seed from always seeding first byte
    s[seed % 4] = seed;
    // first value is frequently (always?) 0, so we flush that out
    rand(); 
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
