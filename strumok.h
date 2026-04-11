#ifndef STRUMOK_H
#define STRUMOK_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <immintrin.h>

#include "strumok_tables.h"

typedef struct {
    uint64_t sa[16];
    uint64_t reg1;
    uint64_t reg2;
} strumok_state;

void strumok256_init(strumok_state *state, const uint64_t key[4], const uint64_t iv[4]);
void strumok512_init(strumok_state *state, const uint64_t key[8], const uint64_t iv[4]);
uint64_t strumok_next_word(strumok_state *state);
void strumok_xor_keystream(strumok_state *state, uint8_t *data, size_t length);

#endif
