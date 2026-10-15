#ifndef FF_COMMON_H
#define FF_COMMON_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <cpuid.h>

#define CALIBRATION_PER_CLASS 10000
#define TRIALS_PER_RUN 10000
#define RUNS 10

struct sample { uint64_t ticks; int bit; };
struct classifier { uint64_t threshold; int low_is_one; int errors; };

/* x86intrin.h provides the intrinsics; these are the missing base-code helpers. */
static inline void maccess(const volatile void *addr) {
    (void)*(const volatile unsigned char *)addr;
}
static inline void flush(const volatile void *addr) {
    _mm_clflush((const void *)addr);
    _mm_mfence();
}
static inline uint64_t rdtsc(unsigned *aux) {
    _mm_lfence();
    uint64_t ticks = __rdtscp(aux);
    _mm_lfence();
    return ticks;
}
/* As in ff.c, the observable is flush time. No reload occurs here. */
static inline uint64_t flushandreload(const volatile void *addr) {
    unsigned before, after;
    _mm_mfence();
    uint64_t start = rdtsc(&before);
    flush(addr);
    uint64_t end = rdtsc(&after);
    if (before != after || end <= start) {
        fputs("Invalid timer or CPU migration; discard incomplete dataset\n", stderr);
        exit(1);
    }
    return end - start;
}
static inline unsigned check_cpu(void) {
    unsigned a, b, c, d;
    __cpuid(1, a, b, c, d);
    if (!(d & (1u<<4)) || !(d & (1u<<19)) || !(d & (1u<<26))) {
        fputs("UNSUPPORTED: TSC/CLFLUSH/SSE2\n", stderr); exit(3);
    }
    unsigned line = ((b >> 8) & 255) * 8;
    if (!line || line > 4096 || __get_cpuid_max(0x80000000, NULL) < 0x80000001) {
        fputs("UNSUPPORTED: line size or extended CPUID\n", stderr); exit(3);
    }
    __cpuid(0x80000001, a, b, c, d);
    if (!(d & (1u<<27))) {
        fputs("UNSUPPORTED: RDTSCP\n", stderr); exit(3);
    }
    return line;
}
static inline uint64_t random64(uint64_t *state) {
    *state ^= *state >> 12; *state ^= *state << 25; *state ^= *state >> 27;
    return *state * UINT64_C(2685821657736338717);
}
/* Equal numbers of both known bits, shuffled afresh for each collection. */
static inline void balanced_labels(int *bits, int count, uint64_t *state) {
    for (int i = 0; i < count; i++) bits[i] = i & 1;
    for (int i = count - 1; i > 0; i--) {
        int j = (int)(random64(state) % (unsigned)(i + 1));
        int tmp = bits[i]; bits[i] = bits[j]; bits[j] = tmp;
    }
}
static inline int compare_samples(const void *a, const void *b) {
    uint64_t x = ((const struct sample *)a)->ticks;
    uint64_t y = ((const struct sample *)b)->ticks;
    return (x > y) - (x < y);
}
/* Choose the lowest calibration error, rather than an empty histogram bin.
   Both directions are tested: cached flushes need not be faster on every CPU.
   Requires equal numbers of 0 and 1 samples; sorts samples in place. */
static inline struct classifier calibrate(struct sample *samples, int count) {
    qsort(samples, count, sizeof(*samples), compare_samples);
    struct classifier best = {0, 1, count / 2};
    int ones = 0, zeros = 0;
    for (int i = 0; i < count;) {
        uint64_t ticks = samples[i].ticks;
        do {
            if (samples[i].bit) ones++; else zeros++;
            i++;
        } while (i < count && samples[i].ticks == ticks);
        int low_error = zeros + count / 2 - ones;
        int high_error = ones + count / 2 - zeros;
        if (low_error < best.errors) best = (struct classifier){ticks, 1, low_error};
        if (high_error < best.errors) best = (struct classifier){ticks, 0, high_error};
    }
    return best;
}
static inline int predict(uint64_t ticks, struct classifier rule) {
    return ticks <= rule.threshold ? rule.low_is_one : !rule.low_is_one;
}
#endif
