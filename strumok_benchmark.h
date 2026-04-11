#ifndef STRUMOK_BENCHMARK_H
#define STRUMOK_BENCHMARK_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "strumok.h"

#define STRUMOK_BENCH_DEFAULT_BYTES ((size_t)(64u * 1024u * 1024u))
#define STRUMOK_BENCH_DEFAULT_ROUNDS 5

int strumok_run_benchmark(size_t bytes_per_round, int rounds);

#endif
