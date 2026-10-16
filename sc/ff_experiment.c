#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <time.h>
#if defined(__x86_64__)
#include "ff_common.h"
/* Measure a shared target using a synthetic victim process. */
#define N CALIBRATION_PER_CLASS /* Number of calibration samples per class. */
/* Keep messages and measured memory on separate pages. */
struct shared {
    _Alignas(4096) atomic_uint request; /* Shared command from attacker to victim. */
    _Alignas(4096) atomic_uint done; /* Shared acknowledgment that victim handled the command. */
    _Alignas(4096) volatile unsigned char target[4096]; /* The line being probed by the attacker. */
    _Alignas(4096) volatile unsigned char unrelated[4096]; /* A different line used as a control target. */
};
static struct shared *s; /* Pointer to the shared memory block. */
static pid_t child = -1; /* PID of the synthetic victim process. */
static long victim_peak_rss_kib; /* Peak RSS of the victim process, in KiB. */
static unsigned seq; /* Sequence counter used to make each command unique. */
static uint64_t rng; /* Random generator seed used for label shuffling. */
/* Record victim memory use and stop the victim process. */
static void cleanup(void) { /* Clean up the child victim on exit. */
    if (child > 0) { /* Only do this if the victim exists. */
        /* Some kernels report zero child ru_maxrss. Preserve the live VmHWM
           too; both interfaces report the process high-water RSS in KiB. */
        char status_path[64], status_line[256]; /* Paths and line buffer for /proc status reading. */
        snprintf(status_path, sizeof(status_path), "/proc/%ld/status", (long)child); /* Build the victim status path. */
        FILE *status_file = fopen(status_path, "r"); /* Open the status file. */
        if (status_file) { /* If it opens, read the peak memory. */
            while (fgets(status_line, sizeof(status_line), status_file)) { /* Scan the status file line by line. */
                long hwm; /* High-water RSS from /proc. */
                if (sscanf(status_line, "VmHWM: %ld kB", &hwm) == 1) /* Record the current high-water mark. */
                    victim_peak_rss_kib = hwm;
            }
            fclose(status_file); /* Close the status file. */
        }
        struct rusage usage; /* Resource usage for wait4. */
        kill(child, SIGTERM); /* Stop the child so it cannot keep running. */
        if (wait4(child, NULL, 0, &usage) == child && usage.ru_maxrss > victim_peak_rss_kib) /* Keep the larger value if present. */
            victim_peak_rss_kib = usage.ru_maxrss;
        child = -1; /* Mark the victim as cleaned up. */
    }
}
static void fail(const char *msg) { perror(msg); exit(1); } /* Print a system error and exit. */
/* Keep this process on the selected CPU. */
static void pin(int cpu) { /* Pin the current process to one logical CPU. */
    cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(cpu, &mask); /* Build the affinity mask for the target CPU. */
    if (sched_setaffinity(0, sizeof(mask), &mask)) fail("sched_setaffinity"); /* Apply the pin. */
    if (sched_getcpu() != cpu) { fprintf(stderr,"CPU pin verification failed\n"); exit(1); } /* Ensure the process stayed on that CPU. */
}
/* Read elapsed time in seconds. */
static double now(void) { /* Return a monotonic wall-clock timestamp. */
    struct timespec t; if (clock_gettime(CLOCK_MONOTONIC_RAW, &t)) fail("clock_gettime"); /* Read the clock. */
    return t.tv_sec + t.tv_nsec * 1e-9; /* Convert nanoseconds to seconds. */
}
/* Reset the target, wait for the victim if needed, and time a flush. */
static uint64_t trial(int bit, int mode) { /* Measure one sample under a requested victim behavior. */
    flush(s->target); /* Flush the target line before access. */
    if (mode != 2) { /* The victim is active in modes 0 and 1. */
        /* Sequence, bit and mode travel in a separate page, never the target. */
        unsigned cmd = (++seq << 2) | (unsigned)bit | ((unsigned)mode << 1); /* Encode the desired action and bit. */
        atomic_store_explicit(&s->request, cmd, memory_order_release); /* Tell the victim what to do. */
        unsigned spins=0; /* Counter for waiting loops. */
        while (atomic_load_explicit(&s->done, memory_order_acquire) != cmd) { /* Wait until the victim handles the request. */
            _mm_pause(); /* Yield while spinning. */
            if (++spins == 1000000) { /* Watchdog after a long wait. */
                int status; /* Exit status for waitpid. */
                if (waitpid(child,&status,WNOHANG)==child) { /* Check if the child died. */
                    child=-1; /* Mark the child as dead. */
                    fprintf(stderr,"Victim failed preflight or execution; dataset incomplete\n"); /* Stop with a clear error. */
                    exit(3); /* Exit with a data-quality error. */
                }
                spins=0; /* Reset the watchdog counter. */
            }
        }
    }
    return flushandreload(s->target); /* Time how fast the target line can be accessed after the flush. */
}
/* Wait for requests and read the requested line for bit 1. */
static void victim(int cpu) { /* Synthetic victim that receives commands and touches a line. */
    pin(cpu); /* Pin the victim to its dedicated CPU. */
    (void)check_cpu(); /* Verify the CPU supports the timing mechanism. */
    (void)flushandreload(s->unrelated); /* Execute the timer and flush on this CPU. */
    unsigned prev = 0; /* Last command seen by the victim. */
    for (;;) { /* Loop forever while waiting on commands. */
        unsigned cmd = atomic_load_explicit(&s->request, memory_order_acquire); /* Read the latest command. */
        if (cmd == prev) { _mm_pause(); continue; } /* Ignore repeated commands. */
        if (cmd & 1) { /* Bit 0 means do nothing; bit 1 means touch memory. */
            volatile unsigned char *p = (cmd & 2) ? s->unrelated : s->target; /* Choose the target line based on the mode. */
            maccess(p); /* Touch the selected cache line. */
        }
        _mm_mfence(); /* Order the access before acknowledging completion. */
        atomic_store_explicit(&s->done, cmd, memory_order_release); /* Tell the attacker the command is complete. */
        prev = cmd; /* Remember the last completed command. */
    }
}
/* Read and check a CPU number from the command line. */
static int cpu_arg(const char *v) { /* Parse a CPU ID from a string. */
    char *end; errno=0; long n=strtol(v,&end,10); /* Convert the argument to an integer. */
    if (errno || *end || n<0 || n>=CPU_SETSIZE) { fprintf(stderr,"Invalid CPU ID\n"); exit(2); } /* Reject invalid CPU values. */
    return (int)n; /* Return the valid CPU ID. */
}
int main(int argc,char **argv) { /* Program entry point. */
    double setup_start = now(); /* Record when setup begins. */
    if (argc!=4) { fprintf(stderr,"Usage: %s ATTACKER_CPU VICTIM_CPU OUTPUT_DIR\nDirectory must already exist. Uses only an owned synthetic victim.\n",argv[0]); return 2; } /* Require the expected arguments. */
    /* Check that two different, allowed CPUs were selected. */
    int ac=cpu_arg(argv[1]),vc=cpu_arg(argv[2]); /* Parse attacker and victim CPU IDs. */
    if(ac==vc) { fprintf(stderr,"Use two different logical CPUs to avoid spin-wait starvation\n"); return 2; } /* Require distinct CPUs. */
    cpu_set_t allowed; /* Allowed CPUs for this process. */
    if(sched_getaffinity(0,sizeof(allowed),&allowed)) fail("sched_getaffinity"); /* Read the inherited CPU affinity. */
    if(!CPU_ISSET(ac,&allowed)||!CPU_ISSET(vc,&allowed)) { fprintf(stderr,"CPU unavailable in inherited affinity; do not wrap this program in taskset -c 0\n"); return 2; } /* Ensure both CPUs are allowed. */
    pin(ac); /* Pin the parent to the attacker CPU. */
    unsigned line = check_cpu(); /* Determine the cache line size for this CPU. */
    unsigned a,b,c,d; /* CPUID register variables for TSC feature detection. */
    int invariant=0; /* 1 when invariant TSC is available. */
    if(__get_cpuid_max(0x80000000,NULL)>=0x80000007) { __cpuid(0x80000007,a,b,c,d); invariant=!!(d&(1u<<8)); } /* Check for invariant TSC support. */
    char path[4096]; /* Buffer for output file paths. */
#define OPEN_FILE(var,name) \
    if(snprintf(path,sizeof(path),"%s/%s",argv[3],name)>=(int)sizeof(path)) return 2; \
    FILE *var=fopen(path,"wx"); if(!var) fail(path)
    /* Save the machine settings and random seed for this run. */
    OPEN_FILE(meta,"metadata.txt"); /* Open the metadata file. */
    fprintf(meta,"technique=Flush+Flush (CLFLUSH+MFENCE timed with RDTSCP/LFENCE)\nattacker_cpu=%d\nvictim_cpu=%d\ncpuid_line_bytes=%u\ninvariant_tsc=%d\nsharing=MAP_SHARED|MAP_ANONYMOUS inherited by fork; identical physical target\neviction_sets=not applicable (explicit CLFLUSH)\nother_cpus=not measured\nother_primitives=not implemented; no substitution\nunrelated_control=different cache line, set not verified\nthroughput=wall clock includes resets and synchronization, excludes CSV writes\n",ac,vc,line,invariant); /* Write the environment metadata for the run. */
    FILE *seed=fopen("/dev/urandom","rb"); if(!seed||fread(&rng,sizeof(rng),1,seed)!=1) fail("random seed"); fclose(seed); /* Read a random seed for shuffling. */
    if(!rng) rng=1; /* Prevent a zero seed. */
    fprintf(meta,"seed=%" PRIu64 "\n",rng); fclose(meta); /* Save the random seed used by the experiment. */
    /* Create memory shared by the attacker and victim. */
    s=mmap(NULL,sizeof(*s),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0); /* Map shared memory between parent and child. */
    if(s==MAP_FAILED) fail("mmap"); /* Stop if shared memory cannot be created. */
    memset(s,0,sizeof(*s)); atomic_init(&s->request,0); atomic_init(&s->done,0); /* Clear the shared state and initialize atomics. */
    if(!atomic_is_lock_free(&s->request)) { fprintf(stderr,"UNSUPPORTED: shared lock-free atomics\n"); return 3; } /* Check whether the shared atomics are lock-free. */
    atexit(cleanup); /* Register cleanup so the victim is killed on exit. */
    pid_t parent=getpid(); /* Save the parent PID for the child check. */
    /* Start the victim as a separate process. */
    child=fork(); if(child<0) fail("fork"); /* Spawn the victim process. */
    if(child==0) { /* Child process enters victim mode. */
        if(prctl(PR_SET_PDEATHSIG,SIGTERM)) fail("prctl"); /* Ensure the victim dies with the parent. */
        if(getppid()!=parent) _exit(1); /* Abort if the parent changed unexpectedly. */
        victim(vc); /* Run the victim loop on the victim CPU. */
    }
    pin(ac); /* Re-pin the parent to the attacker CPU after forking. */
    /* Watchdog bounds a failed child/handshake; external timeout also recommended. */
    alarm(60); /* Set a watchdog to time out if the program hangs. */
    int bits[2*N]; struct sample cal[2*N]; /* Array of labels and timing samples for calibration. */
    /* Discard warmup measurements before calibration. */
    for(int i=0;i<1000;i++) (void)trial(i&1,0); /* Run a warm-up loop to stabilize the cache state. */
    /* Collect timings for equal numbers of known zeros and ones. */
    balanced_labels(bits,2*N,&rng); /* Generate balanced labels for calibration samples. */
    OPEN_FILE(raw,"calibration.csv"); fprintf(raw,"trial,bit,tsc_ticks\n"); /* Open and initialize the calibration CSV. */
    double calibration_start = now(); /* Start the calibration timer. */
    for(int i=0;i<2*N;i++) { cal[i]=(struct sample){trial(bits[i],0),bits[i]}; } /* Record each calibration sample with its known label. */
    double calibration_seconds = now() - calibration_start; /* How long calibration took. */
    for(int i=0;i<2*N;i++) fprintf(raw,"%d,%d,%" PRIu64 "\n",i,cal[i].bit,cal[i].ticks); /* Write the raw calibration data. */
    fclose(raw); /* Close the calibration file. */
    /* Choose the rule using only calibration data and save it before tests. */
    struct classifier rule = calibrate(cal, 2*N); /* Derive the best threshold from calibration data. */
    OPEN_FILE(frozen,"threshold.txt"); /* Open the threshold output file. */
    fprintf(frozen,"threshold_tsc_ticks=%" PRIu64 "\nlow_is_one=%d\ncalibration_per_class=%d\ncalibration_accuracy=%.6f\n",rule.threshold,rule.low_is_one,N,1.-rule.errors/(2.*N)); /* Save the chosen classifier. */
    if(fflush(frozen)||fsync(fileno(frozen))) fail("freeze threshold"); /* Flush the threshold file to disk. */
    fclose(frozen); /* Close the threshold file. */
    double setup_seconds = now() - setup_start; /* Total setup time for the experiment. */
    OPEN_FILE(trials,"trials.csv"); fprintf(trials,"mode,run,trial,bit,tsc_ticks,prediction\n"); /* Open trial CSV for per-sample results. */
    OPEN_FILE(summary,"summary.csv"); fprintf(summary,"mode,run,trials,correct,tp,tn,fp,fn,seconds,trials_per_s,correct_bits_per_s,accuracy\n"); /* Open summary CSV for aggregate results. */
    /* Test target reads, different-line reads, and no victim. */
    const char *names[]={"signal","unrelated_line","no_victim"}; /* Labels for each experiment mode. */
    for(int mode=0;mode<3;mode++) { /* Run all three conditions. */
        /* Stop the victim before the no-victim condition. */
        if(mode==2) cleanup(); /* Kill the victim for the no-victim baseline. */
        for(int run=1;run<=RUNS;run++) { /* Repeat each mode for a fixed number of runs. */
            /* Shuffle fresh labels and measure this run. */
            balanced_labels(bits,TRIALS_PER_RUN,&rng); struct sample samples[TRIALS_PER_RUN]; /* Generate labels and sample storage for this run. */
            double start=now(); /* Start the run timer. */
            for(int i=0;i<TRIALS_PER_RUN;i++) samples[i]=(struct sample){trial(bits[i],mode),bits[i]}; /* Collect timing samples for this mode. */
            double elapsed=now()-start; /* Time the run. */
            /* Apply the saved rule and count correct and incorrect predictions. */
            int tp=0,tn=0,fp=0,fn=0; /* Confusion matrix counters. */
            for(int i=0;i<TRIALS_PER_RUN;i++) { /* Evaluate every sample against the saved classifier. */
                int pred=predict(samples[i].ticks,rule); /* Predict whether the bit was 1 or 0. */
                if(bits[i]) { if(pred) tp++; else fn++; } else { if(pred) fp++; else tn++; } /* Update the confusion matrix. */
                fprintf(trials,"%s,%d,%d,%d,%" PRIu64 ",%d\n",names[mode],run,i,bits[i],samples[i].ticks,pred); /* Save each prediction. */
            }
            /* Save accuracy and measurement rates for this run. */
            int correct=tp+tn; /* Number of correct predictions in this run. */
            fprintf(summary,"%s,%d,%d,%d,%d,%d,%d,%d,%.9f,%.3f,%.3f,%.6f\n",names[mode],run,TRIALS_PER_RUN,correct,tp,tn,fp,fn,elapsed,TRIALS_PER_RUN/elapsed,correct/elapsed,correct/(double)TRIALS_PER_RUN); /* Save summary metrics. */
            fflush(summary); /* Flush the summary file after each run. */
        }
    }
    if (fclose(summary) || fclose(trials)) fail("write results"); /* Close the output CSVs and check for errors. */
    /* Save setup time, shared memory size, and peak memory use. */
    struct rusage usage; /* Resource usage for parent process stats. */
    if (getrusage(RUSAGE_SELF, &usage)) fail("getrusage"); /* Read memory and time usage. */
    OPEN_FILE(resources,"resources.csv"); /* Open the resource summary CSV. */
    fputs("setup_seconds,calibration_collection_seconds,shared_mapping_bytes,parent_peak_rss_kib,victim_peak_rss_kib\n", resources); /* CSV column names. */
    fprintf(resources,"%.9f,%.9f,%zu,%ld,%ld\n",setup_seconds,calibration_seconds,sizeof(*s),usage.ru_maxrss,victim_peak_rss_kib); /* Save setup and memory totals. */
    if (fclose(resources)) fail("write resources"); /* Close the resource file. */
    alarm(0); /* Clear the watchdog alarm. */
    puts("Complete: calibration.csv, threshold.txt, trials.csv, summary.csv, metadata.txt"); /* Tell the user the run finished. */
    return 0; /* Exit successfully. */
}
#else
int main(void) { fputs("UNSUPPORTED: this harness requires x86-64 CLFLUSH/RDTSCP; no substitute used\n",stderr); return 3; } /* Show the unsupported platform message. */
#endif
