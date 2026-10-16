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
#define N CALIBRATION_PER_CLASS
/* Keep messages and measured memory on separate pages. */
struct shared {
    _Alignas(4096) atomic_uint request;
    _Alignas(4096) atomic_uint done;
    _Alignas(4096) volatile unsigned char target[4096];
    _Alignas(4096) volatile unsigned char unrelated[4096];
};
static struct shared *s;
static pid_t child = -1;
static long victim_peak_rss_kib;
static unsigned seq;
static uint64_t rng;
/* Record victim memory use and stop the victim process. */
static void cleanup(void) {
    if (child > 0) {
        /* Some kernels report zero child ru_maxrss. Preserve the live VmHWM
           too; both interfaces report the process high-water RSS in KiB. */
        char status_path[64], status_line[256];
        snprintf(status_path, sizeof(status_path), "/proc/%ld/status", (long)child);
        FILE *status_file = fopen(status_path, "r");
        if (status_file) {
            while (fgets(status_line, sizeof(status_line), status_file)) {
                long hwm;
                if (sscanf(status_line, "VmHWM: %ld kB", &hwm) == 1)
                    victim_peak_rss_kib = hwm;
            }
            fclose(status_file);
        }
        struct rusage usage;
        kill(child, SIGTERM);
        if (wait4(child, NULL, 0, &usage) == child && usage.ru_maxrss > victim_peak_rss_kib)
            victim_peak_rss_kib = usage.ru_maxrss;
        child = -1;
    }
}
static void fail(const char *msg) { perror(msg); exit(1); }
/* Keep this process on the selected CPU. */
static void pin(int cpu) {
    cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(cpu, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask)) fail("sched_setaffinity");
    if (sched_getcpu() != cpu) { fprintf(stderr,"CPU pin verification failed\n"); exit(1); }
}
/* Read elapsed time in seconds. */
static double now(void) {
    struct timespec t; if (clock_gettime(CLOCK_MONOTONIC_RAW, &t)) fail("clock_gettime");
    return t.tv_sec + t.tv_nsec * 1e-9;
}
/* Reset the target, wait for the victim if needed, and time a flush. */
static uint64_t trial(int bit, int mode) {
    flush(s->target);
    if (mode != 2) {
        /* Sequence, bit and mode travel in a separate page, never the target. */
        unsigned cmd = (++seq << 2) | (unsigned)bit | ((unsigned)mode << 1);
        atomic_store_explicit(&s->request, cmd, memory_order_release);
        unsigned spins=0;
        while (atomic_load_explicit(&s->done, memory_order_acquire) != cmd) {
            _mm_pause();
            if (++spins == 1000000) {
                int status;
                if (waitpid(child,&status,WNOHANG)==child) {
                    child=-1;
                    fprintf(stderr,"Victim failed preflight or execution; dataset incomplete\n");
                    exit(3);
                }
                spins=0;
            }
        }
    }
    return flushandreload(s->target);
}
/* Wait for requests and read the requested line for bit 1. */
static void victim(int cpu) {
    pin(cpu);
    (void)check_cpu();
    (void)flushandreload(s->unrelated); /* execute the timer and flush on this CPU */
    unsigned prev = 0;
    for (;;) {
        unsigned cmd = atomic_load_explicit(&s->request, memory_order_acquire);
        if (cmd == prev) { _mm_pause(); continue; }
        if (cmd & 1) {
            volatile unsigned char *p = (cmd & 2) ? s->unrelated : s->target;
            maccess(p);
        }
        _mm_mfence();
        atomic_store_explicit(&s->done, cmd, memory_order_release);
        prev = cmd;
    }
}
/* Read and check a CPU number from the command line. */
static int cpu_arg(const char *v) {
    char *end; errno=0; long n=strtol(v,&end,10);
    if (errno || *end || n<0 || n>=CPU_SETSIZE) { fprintf(stderr,"Invalid CPU ID\n"); exit(2); }
    return (int)n;
}
int main(int argc,char **argv) {
    double setup_start = now();
    if (argc!=4) { fprintf(stderr,"Usage: %s ATTACKER_CPU VICTIM_CPU OUTPUT_DIR\nDirectory must already exist. Uses only an owned synthetic victim.\n",argv[0]); return 2; }
    /* Check that two different, allowed CPUs were selected. */
    int ac=cpu_arg(argv[1]),vc=cpu_arg(argv[2]);
    if(ac==vc) { fprintf(stderr,"Use two different logical CPUs to avoid spin-wait starvation\n"); return 2; }
    cpu_set_t allowed;
    if(sched_getaffinity(0,sizeof(allowed),&allowed)) fail("sched_getaffinity");
    if(!CPU_ISSET(ac,&allowed)||!CPU_ISSET(vc,&allowed)) { fprintf(stderr,"CPU unavailable in inherited affinity; do not wrap this program in taskset -c 0\n"); return 2; }
    pin(ac);
    unsigned line = check_cpu();
    unsigned a,b,c,d;
    int invariant=0;
    if(__get_cpuid_max(0x80000000,NULL)>=0x80000007) { __cpuid(0x80000007,a,b,c,d); invariant=!!(d&(1u<<8)); }
    char path[4096];
#define OPEN_FILE(var,name) \
    if(snprintf(path,sizeof(path),"%s/%s",argv[3],name)>=(int)sizeof(path)) return 2; \
    FILE *var=fopen(path,"wx"); if(!var) fail(path)
    /* Save the machine settings and random seed for this run. */
    OPEN_FILE(meta,"metadata.txt");
    fprintf(meta,"technique=Flush+Flush (CLFLUSH+MFENCE timed with RDTSCP/LFENCE)\nattacker_cpu=%d\nvictim_cpu=%d\ncpuid_line_bytes=%u\ninvariant_tsc=%d\nsharing=MAP_SHARED|MAP_ANONYMOUS inherited by fork; identical physical target\neviction_sets=not applicable (explicit CLFLUSH)\nother_cpus=not measured\nother_primitives=not implemented; no substitution\nunrelated_control=different cache line, set not verified\nthroughput=wall clock includes resets and synchronization, excludes CSV writes\n",ac,vc,line,invariant);
    FILE *seed=fopen("/dev/urandom","rb"); if(!seed||fread(&rng,sizeof(rng),1,seed)!=1) fail("random seed"); fclose(seed);
    if(!rng) rng=1;
    fprintf(meta,"seed=%" PRIu64 "\n",rng); fclose(meta);
    /* Create memory shared by the attacker and victim. */
    s=mmap(NULL,sizeof(*s),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(s==MAP_FAILED) fail("mmap");
    memset(s,0,sizeof(*s)); atomic_init(&s->request,0); atomic_init(&s->done,0);
    if(!atomic_is_lock_free(&s->request)) { fprintf(stderr,"UNSUPPORTED: shared lock-free atomics\n"); return 3; }
    atexit(cleanup);
    pid_t parent=getpid();
    /* Start the victim as a separate process. */
    child=fork(); if(child<0) fail("fork");
    if(child==0) {
        if(prctl(PR_SET_PDEATHSIG,SIGTERM)) fail("prctl");
        if(getppid()!=parent) _exit(1);
        victim(vc);
    }
    pin(ac);
    /* Watchdog bounds a failed child/handshake; external timeout also recommended. */
    alarm(60);
    int bits[2*N]; struct sample cal[2*N];
    /* Discard warmup measurements before calibration. */
    for(int i=0;i<1000;i++) (void)trial(i&1,0);
    /* Collect timings for equal numbers of known zeros and ones. */
    balanced_labels(bits,2*N,&rng);
    OPEN_FILE(raw,"calibration.csv"); fprintf(raw,"trial,bit,tsc_ticks\n");
    double calibration_start = now();
    for(int i=0;i<2*N;i++) { cal[i]=(struct sample){trial(bits[i],0),bits[i]}; }
    double calibration_seconds = now() - calibration_start;
    for(int i=0;i<2*N;i++) fprintf(raw,"%d,%d,%" PRIu64 "\n",i,cal[i].bit,cal[i].ticks);
    fclose(raw);
    /* Choose the rule using only calibration data and save it before tests. */
    struct classifier rule = calibrate(cal, 2*N);
    OPEN_FILE(frozen,"threshold.txt");
    fprintf(frozen,"threshold_tsc_ticks=%" PRIu64 "\nlow_is_one=%d\ncalibration_per_class=%d\ncalibration_accuracy=%.6f\n",rule.threshold,rule.low_is_one,N,1.-rule.errors/(2.*N));
    if(fflush(frozen)||fsync(fileno(frozen))) fail("freeze threshold");
    fclose(frozen);
    double setup_seconds = now() - setup_start;
    OPEN_FILE(trials,"trials.csv"); fprintf(trials,"mode,run,trial,bit,tsc_ticks,prediction\n");
    OPEN_FILE(summary,"summary.csv"); fprintf(summary,"mode,run,trials,correct,tp,tn,fp,fn,seconds,trials_per_s,correct_bits_per_s,accuracy\n");
    /* Test target reads, different-line reads, and no victim. */
    const char *names[]={"signal","unrelated_line","no_victim"};
    for(int mode=0;mode<3;mode++) {
        /* Stop the victim before the no-victim condition. */
        if(mode==2) cleanup();
        for(int run=1;run<=RUNS;run++) {
            /* Shuffle fresh labels and measure this run. */
            balanced_labels(bits,TRIALS_PER_RUN,&rng); struct sample samples[TRIALS_PER_RUN];
            double start=now();
            for(int i=0;i<TRIALS_PER_RUN;i++) samples[i]=(struct sample){trial(bits[i],mode),bits[i]};
            double elapsed=now()-start;
            /* Apply the saved rule and count correct and incorrect predictions. */
            int tp=0,tn=0,fp=0,fn=0;
            for(int i=0;i<TRIALS_PER_RUN;i++) {
                int pred=predict(samples[i].ticks,rule);
                if(bits[i]) { if(pred) tp++; else fn++; } else { if(pred) fp++; else tn++; }
                fprintf(trials,"%s,%d,%d,%d,%" PRIu64 ",%d\n",names[mode],run,i,bits[i],samples[i].ticks,pred);
            }
            /* Save accuracy and measurement rates for this run. */
            int correct=tp+tn;
            fprintf(summary,"%s,%d,%d,%d,%d,%d,%d,%d,%.9f,%.3f,%.3f,%.6f\n",names[mode],run,TRIALS_PER_RUN,correct,tp,tn,fp,fn,elapsed,TRIALS_PER_RUN/elapsed,correct/elapsed,correct/(double)TRIALS_PER_RUN);
            fflush(summary);
        }
    }
    if (fclose(summary) || fclose(trials)) fail("write results");
    /* Save setup time, shared memory size, and peak memory use. */
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage)) fail("getrusage");
    OPEN_FILE(resources,"resources.csv");
    fputs("setup_seconds,calibration_collection_seconds,shared_mapping_bytes,parent_peak_rss_kib,victim_peak_rss_kib\n", resources);
    fprintf(resources,"%.9f,%.9f,%zu,%ld,%ld\n",setup_seconds,calibration_seconds,sizeof(*s),usage.ru_maxrss,victim_peak_rss_kib);
    if (fclose(resources)) fail("write resources");
    alarm(0);
    puts("Complete: calibration.csv, threshold.txt, trials.csv, summary.csv, metadata.txt");
    return 0;
}
#else
int main(void) { fputs("UNSUPPORTED: this harness requires x86-64 CLFLUSH/RDTSCP; no substitute used\n",stderr); return 3; }
#endif
