#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strumok_benchmark.h"
#include "strumok.h"
#include "strumok_selftest.h"

static int parse_size_arg(const char *value, size_t *out) {
    char *end = NULL;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (value[0] == '\0' || (end != NULL && *end != '\0')) {
        return 1;
    }
    *out = (size_t)parsed;
    return 0;
}

static int parse_int_arg(const char *value, int *out) {
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (value[0] == '\0' || (end != NULL && *end != '\0')) {
        return 1;
    }
    *out = (int)parsed;
    return 0;
}

int main(int argc, char **argv) {
    uint8_t sample[64];
    strumok_state demo_state;
    int run_benchmark = 0;
    size_t bench_bytes = STRUMOK_BENCH_DEFAULT_BYTES;
    int bench_rounds = STRUMOK_BENCH_DEFAULT_ROUNDS;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--bench") == 0) {
            run_benchmark = 1;
        } else if (strcmp(argv[i], "--bytes") == 0 && i + 1 < argc) {
            if (parse_size_arg(argv[++i], &bench_bytes) != 0) {
                fprintf(stderr, "Invalid --bytes value.\n");
                return 2;
            }
        } else if (strcmp(argv[i], "--rounds") == 0 && i + 1 < argc) {
            if (parse_int_arg(argv[++i], &bench_rounds) != 0) {
                fprintf(stderr, "Invalid --rounds value.\n");
                return 2;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: STRUMOK_AVX2 [--bench] [--bytes N] [--rounds N]\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Try --help for usage.\n");
            return 2;
        }
    }

    memset(sample, 0x5a, sizeof(sample));
    strumok256_init(
        &demo_state,
        (const uint64_t[]){0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
        (const uint64_t[]){0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL});
    strumok_xor_keystream(&demo_state, sample, sizeof(sample));

    const int failed = strumok_run_self_tests();
    if (failed != 0) {
        printf("Self-test failed: %d vector(sa) mismatched.\n", failed);
        return 1;
    }

    printf("All STRUMOK-256/512 test vectors passed.\n");

    if (run_benchmark) {
        if (strumok_run_benchmark(bench_bytes, bench_rounds) != 0) {
            return 3;
        }
    }

    return 0;
}