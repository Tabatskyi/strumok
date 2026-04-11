#include "strumok_benchmark.h"

static double now_seconds(void) {
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static void bench_mode_256(uint8_t *buffer, size_t bytes, int rounds) {
    const uint64_t key[4] = {
        0x8000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
    };
    const uint64_t iv[4] = {
        0x0000000000000004ULL,
        0x0000000000000003ULL,
        0x0000000000000002ULL,
        0x0000000000000001ULL,
    };

    double best_mib_s = 0.0;
    double avg_mib_s = 0.0;
    uint8_t checksum = 0;

    for (int round = 0; round < rounds; ++round) {
        strumok_state state;
        memset(buffer, (unsigned char)(0xA5 ^ round), bytes);
        strumok256_init(&state, key, iv);

        const double start = now_seconds();
        strumok_xor_keystream(&state, buffer, bytes);
        const double elapsed = now_seconds() - start;

        const double mib_per_s = ((double)bytes / (1024.0 * 1024.0)) / elapsed;
        if (mib_per_s > best_mib_s) {
            best_mib_s = mib_per_s;
        }
        avg_mib_s += mib_per_s;

        checksum ^= buffer[(size_t)(round * 7919u) % bytes]; // so compiler doesn't optimize away the loop
    }

    avg_mib_s /= (double)rounds;
    printf("STRUMOK-256: best %.2f MiB/s, avg %.2f MiB/s, checksum %02x\n", best_mib_s, avg_mib_s, checksum);
}

static void bench_mode_512(uint8_t *buffer, size_t bytes, int rounds) {
    const uint64_t key[8] = {
        0x8000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
        0x0000000000000000ULL,
    };
    const uint64_t iv[4] = {
        0x0000000000000004ULL,
        0x0000000000000003ULL,
        0x0000000000000002ULL,
        0x0000000000000001ULL,
    };

    double best_mib_s = 0.0;
    double avg_mib_s = 0.0;
    uint8_t checksum = 0;

    for (int round = 0; round < rounds; ++round) {
        strumok_state state;
        memset(buffer, (unsigned char)(0x3C ^ round), bytes);
        strumok512_init(&state, key, iv);

        const double start = now_seconds();
        strumok_xor_keystream(&state, buffer, bytes);
        const double elapsed = now_seconds() - start;

        const double mib_per_s = ((double)bytes / (1024.0 * 1024.0)) / elapsed;
        if (mib_per_s > best_mib_s) {
            best_mib_s = mib_per_s;
        }
        avg_mib_s += mib_per_s;

        checksum ^= buffer[(size_t)(round * 7919u) % bytes];
    }

    avg_mib_s /= (double)rounds;
    printf("STRUMOK-512: best %.2f MiB/s, avg %.2f MiB/s, checksum %02x\n", best_mib_s, avg_mib_s, checksum);
}

int strumok_run_benchmark(size_t bytes_per_round, int rounds) {
    if (bytes_per_round < 64u) {
        bytes_per_round = 64u;
    }
    if (rounds < 1) {
        rounds = 1;
    }

    printf("Benchmark AVX2 mode: enabled\n");

    uint8_t *buffer = (uint8_t *)malloc(bytes_per_round);
    if (buffer == NULL) {
        fprintf(stderr, "Benchmark allocation failed for %zu bytes.\n", bytes_per_round);
        return 1;
    }

    printf("Benchmark: %zu bytes/round (%.2f MiB), %d rounds\n",
           bytes_per_round,
           (double)bytes_per_round / (double)(1024.0 * 1024.0),
           rounds);

    bench_mode_256(buffer, bytes_per_round, rounds);
    bench_mode_512(buffer, bytes_per_round, rounds);

    free(buffer);

    return 0;
}
