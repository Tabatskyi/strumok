#include "strumok_selftest.h"

typedef struct {
    uint64_t iv[4];
    uint64_t key[4];
    uint64_t expected[8];
} strumok256_vector;

typedef struct {
    uint64_t iv[4];
    uint64_t key[8];
    uint64_t expected[8];
} strumok512_vector;

static int run_vector_256(const strumok256_vector *vector, size_t index) {
    strumok_state state;
    strumok256_init(&state, vector->key, vector->iv);

    for (size_t i = 0; i < 8; ++i) {
        const uint64_t got = strumok_next_word(&state);
        if (got != vector->expected[i]) {
            printf("[256][%zu] Z%zu mismatch: got=%016" PRIx64 " expected=%016" PRIx64 "\n", index, i, got, vector->expected[i]);
            return 1;
        }
    }

    return 0;
}

static int run_vector_512(const strumok512_vector *vector, size_t index) {
    strumok_state state;
    strumok512_init(&state, vector->key, vector->iv);

    for (size_t i = 0; i < 8; ++i) {
        const uint64_t got = strumok_next_word(&state);
        if (got != vector->expected[i]) {
            printf("[512][%zu] Z%zu mismatch: got=%016" PRIx64 " expected=%016" PRIx64 "\n", index, i, got, vector->expected[i]);
            return 1;
        }
    }

    return 0;
}

int strumok_run_self_tests(void) {
    static const strumok256_vector vectors_256[] = {
        {
            .iv = {0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .key = {0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .expected = {
                0xe442d15345dc66caULL, 0xf47d700ecc66408aULL, 0xb4cb284b5477e641ULL, 0xa2afc9092e4124b0ULL,
                0x728e5fa26b11a7d9ULL, 0xe6a7b9288c68f972ULL, 0x70eb3606de8ba44cULL, 0xaced7956bd3e3de7ULL,
            },
        },
        {
            .iv = {0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .key = {0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL},
            .expected = {
                0xa7510b38c7a95d1dULL, 0xcd5ea28a15b8654fULL, 0xc5e2e2771d0373b2ULL, 0x98ae829686d5fceeULL,
                0x45bddf65c523dbb8ULL, 0x32a93fcdd950001fULL, 0x752a7fb588af8c51ULL, 0x9de92736664212d4ULL,
            },
        },
        {
            .iv = {0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL},
            .key = {0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .expected = {
                0xfe44a2508b5a2acdULL, 0xaf355b4ed21d2742ULL, 0xdcd7fdd6a57a9e71ULL, 0x5d267bd2739fb5ebULL,
                0xb22eee96b2832072ULL, 0xc7de6a4cdaa9a847ULL, 0x72d5da93812680f2ULL, 0x4a0acb7e93da2ce0ULL,
            },
        },
        {
            .iv = {0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL},
            .key = {0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL},
            .expected = {
                0xe6d0efd9cea5abcdULL, 0x1e78ba1a9b0e401eULL, 0xbcfbea2c02ba0781ULL, 0x1bd375588ae08794ULL,
                0x5493cf21e114c209ULL, 0x66cd5d7cc7d0e69aULL, 0xa5cdb9f3380d07faULL, 0x2940d61a4d4e9ce4ULL,
            },
        },
    };

    static const strumok512_vector vectors_512[] = {
        {
            .iv = {0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .key = {
                0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
                0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
            },
            .expected = {
                0xf5b9ab51100f8317ULL, 0x898ef2086a4af395ULL, 0x59571fecb5158d0bULL, 0xb7c45b6744c71fbbULL,
                0xff2efcf05d8d8db9ULL, 0x7a585871e5c419c0ULL, 0x6b5c4691b9125e71ULL, 0xa55be7d2b358ec6eULL,
            },
        },
        {
            .iv = {0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL},
            .key = {
                0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL,
                0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL,
            },
            .expected = {
                0xd2a6103c50bd4e04ULL, 0xdc6a21af5eb13b73ULL, 0xdf4ca6cb07797265ULL, 0xf453c253d8d01876ULL,
                0x039a64dc7a01800cULL, 0x688ce327dccb7e84ULL, 0x41e0250b5e526403ULL, 0x9936e478aa200f22ULL,
            },
        },
        {
            .iv = {0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL},
            .key = {
                0x8000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
                0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL, 0x0000000000000000ULL,
            },
            .expected = {
                0xcca12eae8133aaaaULL, 0x528d85507ce8501dULL, 0xda83c7fe3e1823f1ULL, 0x21416ebf63b71a42ULL,
                0x26d76d2bf1a625ebULL, 0xeec66ee0cd0b1efcULL, 0x02dd68f338a345a8ULL, 0x47538790a5411adbULL,
            },
        },
        {
            .iv = {0x0000000000000004ULL, 0x0000000000000003ULL, 0x0000000000000002ULL, 0x0000000000000001ULL},
            .key = {
                0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL,
                0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL, 0xaaaaaaaaaaaaaaaaULL,
            },
            .expected = {
                0x965648e775c717d5ULL, 0xa63c2a7376e92df3ULL, 0x0b0eb0bbd47ca267ULL, 0xea593d979ae5bd39ULL,
                0xd773b5e5193cafe1ULL, 0xb0a26671d259422bULL, 0x85b2aa326b280156ULL, 0x511ace6451435f0cULL,
            },
        },
    };

    int failed = 0;

    for (size_t i = 0; i < (sizeof(vectors_256) / sizeof(vectors_256[0])); ++i) {
        failed += run_vector_256(&vectors_256[i], i + 1);
    }

    for (size_t i = 0; i < (sizeof(vectors_512) / sizeof(vectors_512[0])); ++i) {
        failed += run_vector_512(&vectors_512[i], i + 1);
    }

    return failed;
}
