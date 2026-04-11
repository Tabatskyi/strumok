#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "strumok.h"
#include "strumok_selftest.h"

int main(void) {
    uint8_t sample[64];
    strumok_state demo_state;

    memset(sample, 0x5a, sizeof(sample));
    strumok256_init(
        &demo_state,
        (const uint64_t[]){0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
        (const uint64_t[]){0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL});
    strumok_xor_keystream(&demo_state, sample, sizeof(sample));

#if defined(STRUMOK_HAVE_AVX2)
    printf("STRUMOK AVX2 path: enabled\n");
#else
    printf("STRUMOK AVX2 path: disabled, scalar fallback\n");
#endif

    const int failed = strumok_run_self_tests();
    if (failed == 0) {
        printf("All STRUMOK-256/512 test vectors passed.\n");
        return 0;
    }

    printf("Self-test failed: %d vector(s) mismatched.\n", failed);
    return 1;
}