#define _GNU_SOURCE
#include <inttypes.h>
#include <sched.h>
#include <string.h>
#include "ff_common.h"

/* Simple single-process calibration, based on the supplied calibration code.
   Run pinned to an authorized CPU, e.g. taskset -c 0 ./ff_bench.
   The experiment recalibrates with its own victim placement. */
static unsigned char array[5 * 4096] __attribute__((aligned(4096))); /* 5 pages of memory, aligned for cache-line timing. */

/* Load the target into cache, then time a flush. */
static uint64_t flush_hit_time(void *target) { /* Build a known cached sample. */
    flush(target); /* Flush the chosen line so it is removed from cache. */
    maccess(target);                 /* Touch the line again so it is cached. */
    return flushandreload(target); /* Measure the time to flush/reload the cached line. */
}
/* Remove the target from cache, then time a flush. */
static uint64_t flush_miss_time(void *target) { /* Build a known uncached sample. */
    flush(target);                   /* Leave the target line uncached. */
    return flushandreload(target); /* Measure the time to flush/reload the uncached line. */
}

int main(void) { /* Start the calibration program. */
    /* Check that this program stays on one CPU. */
    cpu_set_t mask; /* CPU affinity mask for the current process. */
    if (sched_getaffinity(0, sizeof(mask), &mask)) { /* Get the allowed CPUs. */
        perror("sched_getaffinity"); return 1; /* Report the affinity error and stop. */
    }
    if (CPU_COUNT(&mask) != 1) { /* Require exactly one pinned CPU. */
        fputs("Pin calibration to one authorized CPU with taskset -c CPU\n", stderr); /* Explain the CPU pin requirement. */
        return 2; /* Exit with a usage error. */
    }
    /* Check CPU support and choose the memory location to measure. */
    unsigned line = check_cpu(); /* Store the cache line size reported by the CPU. */
    memset(array, 1, sizeof(array)); /* Fill the array with a known value to avoid zero-page behavior. */
    void *target = array + 2 * 4096; /* Point to the middle page, which is the measured target. */
    int bits[2 * CALIBRATION_PER_CLASS]; /* Known labels for each calibration sample: 0 or 1. */
    struct sample samples[2 * CALIBRATION_PER_CLASS]; /* Store measured ticks and their labels. */
    /* Get a random seed to shuffle the known bit labels. */
    uint64_t seed; /* Random seed used to shuffle samples. */
    FILE *random = fopen("/dev/urandom", "rb"); /* Read a seed from the OS random source. */
    if (!random || fread(&seed, sizeof(seed), 1, random) != 1) { /* Fail if the seed cannot be read. */
        fputs("Cannot obtain shuffle seed\n", stderr); return 1; /* Report the failure and exit. */
    }
    fclose(random); /* Close the random device. */
    if (!seed) seed = 1; /* Prevent a zero seed from causing a degenerate shuffle. */
    printf("CPU: %d; CLFLUSH line bytes: %u; shuffle seed: %" PRIu64 "\n",
           sched_getcpu(), line, seed); /* Print the measured CPU and seed. */
    /* Warm up before saving measurements. */
    for (int i = 0; i < 1000; i++) { /* Run a few timing steps to settle the cache state. */
        if (i & 1) (void)flush_hit_time(target); /* Alternate with a known cached case. */
        else (void)flush_miss_time(target); /* Alternate with a known uncached case. */
    }
    /* Mix equal numbers of cached and uncached measurements. */
    balanced_labels(bits, 2 * CALIBRATION_PER_CLASS, &seed); /* Generate a balanced random label order. */
    FILE *raw = fopen("bench_calibration.csv", "wx"); /* Open the output file for raw timing data. */
    if (!raw) { perror("bench_calibration.csv (use a fresh output directory)"); return 1; } /* Stop if the CSV cannot be created. */
    fputs("trial,bit,tsc_ticks\n", raw); /* Write the CSV header. */
    for (int i = 0; i < 2 * CALIBRATION_PER_CLASS; i++) { /* Collect the calibration timings. */
        uint64_t ticks = bits[i] ? flush_hit_time(target) : flush_miss_time(target); /* Measure the chosen class for this trial. */
        samples[i] = (struct sample){ticks, bits[i]}; /* Store the tick count with its known bit label. */
    }
    /* Save each timing and its known label. */
    for (int i = 0; i < 2 * CALIBRATION_PER_CLASS; i++) /* Write every recorded sample to disk. */
        fprintf(raw, "%d,%d,%" PRIu64 "\n", i, bits[i], samples[i].ticks); /* Save trial number, label, and TSC timing. */
    if (fclose(raw)) { perror("write calibration"); return 1; } /* Ensure the raw timing file is flushed. */
    /* Find and print the threshold and direction with the fewest errors. */
    struct classifier rule = calibrate(samples, 2 * CALIBRATION_PER_CLASS); /* Derive the best threshold from the samples. */
    printf("Suggested threshold: %" PRIu64 " TSC ticks\n", rule.threshold); /* Print the chosen threshold value. */
    printf("Predict cached/bit 1 when ticks %s threshold\n", rule.low_is_one ? "<=" : ">"); /* Explain whether low or high values mean bit 1. */
    printf("Calibration samples per class: %d\n", CALIBRATION_PER_CLASS); /* Print how many samples were used per class. */
    printf("Calibration accuracy: %.2f%% (not held-out accuracy)\n",
           100.0 * (1.0 - rule.errors / (2.0 * CALIBRATION_PER_CLASS))); /* Report the in-sample accuracy estimate. */
    puts("Timing matches ff_experiment.c; use its victim-matched calibration for results."); /* Tell the user which experiment file matches this calibration. */
    return 0; /* Exit successfully. */
}
