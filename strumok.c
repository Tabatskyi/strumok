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

static inline uint8_t lfsr_index(uint8_t head, uint8_t offset) {
    return (uint8_t)((head + offset) & 15u);
}

static inline uint64_t lfsr_get(const strumok_state *state, uint8_t offset) {
    return state->sa[lfsr_index(state->head, offset)];
}

static inline void lfsr_set(strumok_state *state, uint8_t offset, uint64_t value) {
    state->sa[lfsr_index(state->head, offset)] = value;
}

static inline void shift_lfsr(strumok_state *state) {
    state->head = (uint8_t)((state->head + 1u) & 15u);
}

static inline uint64_t strumok_output_word(const strumok_state *state) {
    return fsm(lfsr_get(state, 15), state->reg1, state->reg2) ^ lfsr_get(state, 0);
}

static void strumok_next(strumok_state *state, int init_mode) {
    const uint64_t s0 = lfsr_get(state, 0);
    const uint64_t s11 = lfsr_get(state, 11);
    const uint64_t s13 = lfsr_get(state, 13);
    const uint64_t s15 = lfsr_get(state, 15);

    const uint64_t f = fsm(s15, state->reg1, state->reg2);
    const uint64_t next_r2 = transform_T(state->reg1);
    const uint64_t next_r1 = state->reg2 + s13;

    uint64_t next_s15 = a_mul(s0) ^ ainv_mul(s11) ^ s13;
    if (init_mode) {
        next_s15 ^= f;
    }

    shift_lfsr(state);
    lfsr_set(state, 15, next_s15);
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

    state->head = 0;
    lfsr_set(state, 15, ~K0);
    lfsr_set(state, 14, K1);
    lfsr_set(state, 13, ~K2);
    lfsr_set(state, 12, K3);
    lfsr_set(state, 11, K0);
    lfsr_set(state, 10, ~K1);
    lfsr_set(state, 9, K2);
    lfsr_set(state, 8, K3);
    lfsr_set(state, 7, ~K0);
    lfsr_set(state, 6, ~K1);
    lfsr_set(state, 5, K2 ^ IV3);
    lfsr_set(state, 4, K3);
    lfsr_set(state, 3, K0 ^ IV2);
    lfsr_set(state, 2, K1 ^ IV1);
    lfsr_set(state, 1, K2);
    lfsr_set(state, 0, K3 ^ IV0);

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

    state->head = 0;
    lfsr_set(state, 15, K0);
    lfsr_set(state, 14, ~K1);
    lfsr_set(state, 13, K2);
    lfsr_set(state, 12, K3);
    lfsr_set(state, 11, ~K7);
    lfsr_set(state, 10, K5);
    lfsr_set(state, 9, ~K6);
    lfsr_set(state, 8, K4 ^ IV3);
    lfsr_set(state, 7, ~K0);
    lfsr_set(state, 6, K1);
    lfsr_set(state, 5, K2 ^ IV2);
    lfsr_set(state, 4, K3);
    lfsr_set(state, 3, K4 ^ IV1);
    lfsr_set(state, 2, K5);
    lfsr_set(state, 1, K6);
    lfsr_set(state, 0, K7 ^ IV0);

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
        uint64_t w0 = strumok_next_word(state);
        uint64_t w1 = strumok_next_word(state);
        uint64_t w2 = strumok_next_word(state);
        uint64_t w3 = strumok_next_word(state);

        const __m256i d = _mm256_loadu_si256((const __m256i *)(data + i));
        const __m256i k = _mm256_setr_epi64x(w0, w1, w2, w3);
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
