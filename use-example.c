#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

#include "strumok_tables.h"


static inline uint64_t
a_mul(const uint64_t x) {
  return ( ( (x) << 8 ) ^ ( strumok_alpha_mul[x >> 56] ) );
}


static inline uint64_t
ainv_mul(const uint64_t x) {
  return ( ( (x) >> 8 ) ^ ( strumok_alphainv_mul[x & 0xff] ) );
}


static inline uint64_t
transform_T(const uint64_t x) {
  return ((strumok_T0[  x        & 0xff ]) ^
          (strumok_T1[ (x >>  8) & 0xff ]) ^
          (strumok_T2[ (x >> 16) & 0xff ]) ^
          (strumok_T3[ (x >> 24) & 0xff ]) ^
          (strumok_T4[ (x >> 32) & 0xff ]) ^
          (strumok_T5[ (x >> 40) & 0xff ]) ^
          (strumok_T6[ (x >> 48) & 0xff ]) ^
          (strumok_T7[ (x >> 56) & 0xff ]));
}

void encrypt_stream_avx2(uint8_t *data, const uint8_t *keystream, size_t length) {
    size_t i = 0;
    
    for (; i + 31 < length; i += 32) {
        __m256i d = _mm256_loadu_si256((__m256i*)&data[i]);
        __m256i k = _mm256_loadu_si256((__m256i*)&keystream[i]);
        
        __m256i r = _mm256_xor_si256(d, k);
        
        _mm256_storeu_si256((__m256i*)&data[i], r);
    }
    
    for (; i < length; i++) {
        data[i] ^= keystream[i];
    }
}


void shift_lfsr_avx2(uint64_t *state) {
    // load state blocks
    __m256i block1 = _mm256_loadu_si256((__m256i*)&state[1]);
    __m256i block2 = _mm256_loadu_si256((__m256i*)&state[5]);
    __m256i block3 = _mm256_loadu_si256((__m256i*)&state[9]);
    __m256i block4 = _mm256_loadu_si256((__m256i*)&state[13]);

    // shift by -1
    _mm256_storeu_si256((__m256i*)&state[0], block1);
    _mm256_storeu_si256((__m256i*)&state[4], block2);
    _mm256_storeu_si256((__m256i*)&state[8], block3);
    _mm256_storeu_si256((__m256i*)&state[12], block4);
}