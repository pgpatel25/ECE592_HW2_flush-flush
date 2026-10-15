#define _GNU_SOURCE
#include <inttypes.h>
#include <sched.h>
#include <string.h>
#include "ff_common.h"

/* Simple single-process calibration, based on the supplied calibration code.
   Run pinned to an authorized CPU, e.g. taskset -c 0 ./ff_bench.
   The experiment recalibrates with its own victim placement. */
static unsigned char array[5 * 4096] __attribute__((aligned(4096)));

static uint64_t flush_hit_time(void *target) {
    flush(target);
    maccess(target);                 /* known cached class */
    return flushandreload(target);
}
static uint64_t flush_miss_time(void *target) {
    flush(target);                   /* known uncached class */
    return flushandreload(target);
}

int main(void) {
    cpu_set_t mask;
    if (sched_getaffinity(0, sizeof(mask), &mask)) {
        perror("sched_getaffinity"); return 1;
    }
    if (CPU_COUNT(&mask) != 1) {
        fputs("Pin calibration to one authorized CPU with taskset -c CPU\n", stderr);
        return 2;
    }
    unsigned line = check_cpu();
    memset(array, 1, sizeof(array));
    void *target = array + 2 * 4096;
    int bits[2 * CALIBRATION_PER_CLASS];
    struct sample samples[2 * CALIBRATION_PER_CLASS];
    uint64_t seed;
    FILE *random = fopen("/dev/urandom", "rb");
    if (!random || fread(&seed, sizeof(seed), 1, random) != 1) {
        fputs("Cannot obtain shuffle seed\n", stderr); return 1;
    }
    fclose(random);
    if (!seed) seed = 1;
    printf("CPU: %d; CLFLUSH line bytes: %u; shuffle seed: %" PRIu64 "\n",
           sched_getcpu(), line, seed);
    for (int i = 0; i < 1000; i++) {
        if (i & 1) (void)flush_hit_time(target);
        else (void)flush_miss_time(target);
    }
    balanced_labels(bits, 2 * CALIBRATION_PER_CLASS, &seed);
    FILE *raw = fopen("bench_calibration.csv", "wx");
    if (!raw) { perror("bench_calibration.csv (use a fresh output directory)"); return 1; }
    fputs("trial,bit,tsc_ticks\n", raw);
    for (int i = 0; i < 2 * CALIBRATION_PER_CLASS; i++) {
        uint64_t ticks = bits[i] ? flush_hit_time(target) : flush_miss_time(target);
        samples[i] = (struct sample){ticks, bits[i]};
    }
    for (int i = 0; i < 2 * CALIBRATION_PER_CLASS; i++)
        fprintf(raw, "%d,%d,%" PRIu64 "\n", i, bits[i], samples[i].ticks);
    if (fclose(raw)) { perror("write calibration"); return 1; }
    struct classifier rule = calibrate(samples, 2 * CALIBRATION_PER_CLASS);
    printf("Suggested threshold: %" PRIu64 " TSC ticks\n", rule.threshold);
    printf("Predict cached/bit 1 when ticks %s threshold\n", rule.low_is_one ? "<=" : ">");
    printf("Calibration samples per class: %d\n", CALIBRATION_PER_CLASS);
    printf("Calibration accuracy: %.2f%% (not held-out accuracy)\n",
           100.0 * (1.0 - rule.errors / (2.0 * CALIBRATION_PER_CLASS)));
    puts("Timing matches ff_experiment.c; use its victim-matched calibration for results.");
    return 0;
}
