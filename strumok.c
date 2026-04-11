#include "strumok.h"

static inline uint64_t a_mul(uint64_t x) {
    return (x << 8) ^ strumok_alpha_mul[x >> 56];
}

static inline uint64_t ainv_mul(uint64_t x) {
    return (x >> 8) ^ strumok_alphainv_mul[x & 0xffu];
}

static inline uint64_t transform_T(uint64_t x) {
    return (strumok_T0[x & 0xffu] ^
            strumok_T1[(x >> 8) & 0xffu] ^
            strumok_T2[(x >> 16) & 0xffu] ^
            strumok_T3[(x >> 24) & 0xffu] ^
            strumok_T4[(x >> 32) & 0xffu] ^
            strumok_T5[(x >> 40) & 0xffu] ^
            strumok_T6[(x >> 48) & 0xffu] ^
            strumok_T7[(x >> 56) & 0xffu]);
}

static inline uint64_t fsm(uint64_t x, uint64_t y, uint64_t z) {
    return (x + y) ^ z;
}

static inline void shift_lfsr(uint64_t sa[16]) {
    const __m256i v0 = _mm256_loadu_si256((const __m256i *)&sa[1]);
    const __m256i v1 = _mm256_loadu_si256((const __m256i *)&sa[5]);
    const __m256i v2 = _mm256_loadu_si256((const __m256i *)&sa[9]);

    _mm256_storeu_si256((__m256i *)&sa[0], v0);
    _mm256_storeu_si256((__m256i *)&sa[4], v1);
    _mm256_storeu_si256((__m256i *)&sa[8], v2);
    sa[12] = sa[13];
    sa[13] = sa[14];
    sa[14] = sa[15];
}

static inline uint64_t strumok_output_word(const strumok_state *state) {
    return fsm(state->sa[15], state->reg1, state->reg2) ^ state->sa[0];
}

static void strumok_next(strumok_state *state, int init_mode) {
    const uint64_t s0 = state->sa[0];
    const uint64_t s11 = state->sa[11];
    const uint64_t s13 = state->sa[13];
    const uint64_t s15 = state->sa[15];

    const uint64_t f = fsm(s15, state->reg1, state->reg2);
    const uint64_t next_r2 = transform_T(state->reg1);
    const uint64_t next_r1 = state->reg2 + s13;

    uint64_t next_s15 = a_mul(s0) ^ ainv_mul(s11) ^ s13;
    if (init_mode) {
        next_s15 ^= f;
    }

    shift_lfsr(state->sa);
    state->sa[15] = next_s15;
    state->reg1 = next_r1;
    state->reg2 = next_r2;
}

uint64_t strumok_next_word(strumok_state *state) {
    const uint64_t z = strumok_output_word(state);
    strumok_next(state, 0);
    return z;
}

void strumok256_init(strumok_state *state, const uint64_t key[4], const uint64_t iv[4]) {
    const uint64_t K3 = key[0];
    const uint64_t K2 = key[1];
    const uint64_t K1 = key[2];
    const uint64_t K0 = key[3];

    const uint64_t IV3 = iv[0];
    const uint64_t IV2 = iv[1];
    const uint64_t IV1 = iv[2];
    const uint64_t IV0 = iv[3];

    state->sa[15] = ~K0;
    state->sa[14] = K1;
    state->sa[13] = ~K2;
    state->sa[12] = K3;
    state->sa[11] = K0;
    state->sa[10] = ~K1;
    state->sa[9] = K2;
    state->sa[8] = K3;
    state->sa[7] = ~K0;
    state->sa[6] = ~K1;
    state->sa[5] = K2 ^ IV3;
    state->sa[4] = K3;
    state->sa[3] = K0 ^ IV2;
    state->sa[2] = K1 ^ IV1;
    state->sa[1] = K2;
    state->sa[0] = K3 ^ IV0;

    state->reg1 = 0;
    state->reg2 = 0;

    for (int i = 0; i < 32; ++i) {
        strumok_next(state, 1);
    }

    strumok_next(state, 0);
}

void strumok512_init(strumok_state *state, const uint64_t key[8], const uint64_t iv[4]) {
    const uint64_t K7 = key[0];
    const uint64_t K6 = key[1];
    const uint64_t K5 = key[2];
    const uint64_t K4 = key[3];
    const uint64_t K3 = key[4];
    const uint64_t K2 = key[5];
    const uint64_t K1 = key[6];
    const uint64_t K0 = key[7];

    const uint64_t IV3 = iv[0];
    const uint64_t IV2 = iv[1];
    const uint64_t IV1 = iv[2];
    const uint64_t IV0 = iv[3];

    state->sa[15] = K0;
    state->sa[14] = ~K1;
    state->sa[13] = K2;
    state->sa[12] = K3;
    state->sa[11] = ~K7;
    state->sa[10] = K5;
    state->sa[9] = ~K6;
    state->sa[8] = K4 ^ IV3;
    state->sa[7] = ~K0;
    state->sa[6] = K1;
    state->sa[5] = K2 ^ IV2;
    state->sa[4] = K3;
    state->sa[3] = K4 ^ IV1;
    state->sa[2] = K5;
    state->sa[1] = K6;
    state->sa[0] = K7 ^ IV0;

    state->reg1 = 0;
    state->reg2 = 0;

    for (int i = 0; i < 32; ++i) {
        strumok_next(state, 1);
    }

    strumok_next(state, 0);
}

void strumok_xor_keystream(strumok_state *state, uint8_t *data, size_t length) {
    size_t i = 0;

    for (; i + 31 < length; i += 32) {
        uint64_t stream_words[4] = {
            strumok_next_word(state),
            strumok_next_word(state),
            strumok_next_word(state),
            strumok_next_word(state),
        };

        const __m256i d = _mm256_loadu_si256((const __m256i *)(data + i));
        const __m256i k = _mm256_loadu_si256((const __m256i *)stream_words);
        const __m256i r = _mm256_xor_si256(d, k);
        _mm256_storeu_si256((__m256i *)(data + i), r);
    }

    if (i < length) {
        uint8_t tail[8];
        size_t tail_index = 8;

        while (i < length) {
            if (tail_index == 8) {
                const uint64_t word = strumok_next_word(state);
                memcpy(tail, &word, sizeof(word));
                tail_index = 0;
            }
            data[i++] ^= tail[tail_index++];
        }
    }
}
