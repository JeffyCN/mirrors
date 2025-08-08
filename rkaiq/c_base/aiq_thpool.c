/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Johan Hanssen Seferidis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author:       Johan Hanssen Seferidis
 * License:      MIT
 * Description:  Library providing a threading pool where you can add
 *               work. For usage, check the thpool.h file or README.md
 *
 * @file thpool.h
 *
 */

#define _GNU_SOURCE
#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#else
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#endif
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#if defined(__ANDROID__)
#include <sys/syscall.h>
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <pthread_np.h>
#endif

#include "aiq_thpool.h"

#ifdef THPOOL_DEBUG
#define THPOOL_DEBUG 1
#else
#define THPOOL_DEBUG 0
#endif

#if !defined(DISABLE_PRINT) || defined(THPOOL_DEBUG)
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif

#ifndef THPOOL_THREAD_NAME
#define THPOOL_THREAD_NAME aiq_thpool
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

static volatile int threads_keepalive;
static volatile int threads_on_hold;

/* ========================== STRUCTURES ============================ */

/* Binary semaphore */
typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int v;
} bsem;

/* Job */
typedef struct job {
    struct job* prev;            /* pointer to previous job   */
    void (*function)(void* arg); /* function pointer          */
    void* arg;                   /* function's argument       */
    size_t arg_size;             /* size of function's argument */
} job;

/* Job queue */
typedef struct jobqueue {
    pthread_mutex_t rwmutex; /* used for queue r/w access */
    job* front;              /* pointer to front of queue */
    job* rear;               /* pointer to rear  of queue */
    bsem* has_jobs;          /* flag as binary semaphore  */
    int len;                 /* number of jobs in queue   */
} jobqueue;

/* Thread */
typedef struct thread {
    int id;                   /* friendly id               */
    pthread_t pthread;        /* pointer to actual thread  */
    struct thpool_* thpool_p; /* access to thpool          */
} thread;

/* Threadpool */
typedef struct thpool_ {
    thread** threads;                 /* pointer to threads        */
    volatile int num_threads_alive;   /* threads currently alive   */
    volatile int num_threads_working; /* threads currently working */
    pthread_mutex_t thcount_lock;     /* used for thread count etc */
    pthread_cond_t threads_all_idle;  /* signal to thpool_wait     */
    jobqueue jobqueue;                /* job queue                 */
    int sched_policy;                 /* Scheduling policy (e.g. SCHED_FIFO) */
    int sched_priority;               /* Scheduling priority (0-99) */
    int cpu_cores[8];                 /* Fixed-size array for CPU core affinity (max 8 cores) */
    int num_cores;                    /* Number of actually used cores */
} thpool_;

/* ========================== PROTOTYPES ============================ */

static int thread_init(thpool_* thpool_p, struct thread** thread_p, int id);
struct thpool_* thpool_init_ex(int num_threads, int sched_policy, int sched_priority,
                               int* cpu_cores, int num_cores);
static void* thread_do(struct thread* thread_p);
static void thread_hold(int sig_id);
static void thread_destroy(struct thread* thread_p);

static int jobqueue_init(jobqueue* jobqueue_p);
static void jobqueue_clear(jobqueue* jobqueue_p);
static void jobqueue_push(jobqueue* jobqueue_p, struct job* newjob_p);
static struct job* jobqueue_pull(jobqueue* jobqueue_p);
static void jobqueue_destroy(jobqueue* jobqueue_p);

static void bsem_init(struct bsem* bsem_p, int value);
static void bsem_reset(struct bsem* bsem_p);
static void bsem_post(struct bsem* bsem_p);
static void bsem_post_all(struct bsem* bsem_p);
static void bsem_wait(struct bsem* bsem_p);

/* ========================== THREADPOOL ============================ */

/* Initialise thread pool */
struct thpool_* thpool_init_ex(int num_threads, int sched_policy, int sched_priority,
                               int* cpu_cores, int num_cores) {
    threads_on_hold   = 0;
    threads_keepalive = 1;

    if (num_threads < 0) {
        num_threads = 0;
    }

    if (sched_policy != SCHED_FIFO && sched_policy != SCHED_RR && sched_policy != SCHED_OTHER) {
        fprintf(stderr, "Invalid scheduling policy\n");
        return NULL;
    }

    /* Make new thread pool */
    thpool_* thpool_p;
    thpool_p = (struct thpool_*)malloc(sizeof(struct thpool_));
    if (thpool_p == NULL) {
        err("thpool_init(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    thpool_p->num_threads_alive   = 0;
    thpool_p->num_threads_working = 0;

    /* Initialize scheduling parameters */
    thpool_p->sched_policy   = sched_policy;
    thpool_p->sched_priority = sched_priority;
    thpool_p->num_cores      = 0;
    memset(thpool_p->cpu_cores, -1, sizeof(thpool_p->cpu_cores));

    if (cpu_cores && num_cores > 0) {
        int valid_cores = num_cores > 8 ? 8 : num_cores;
        for (int i = 0; i < valid_cores; i++) {
            if (cpu_cores[i] >= 0) {
                thpool_p->cpu_cores[i] = cpu_cores[i];
                thpool_p->num_cores++;
            }
        }
    }

    /* Initialise the job queue */
    if (jobqueue_init(&thpool_p->jobqueue) == -1) {
        err("thpool_init(): Could not allocate memory for job queue\n");
        free(thpool_p);
        return NULL;
    }

    /* Make threads in pool */
    thpool_p->threads = (struct thread**)malloc(num_threads * sizeof(struct thread*));
    if (thpool_p->threads == NULL) {
        err("thpool_init(): Could not allocate memory for threads\n");
        jobqueue_destroy(&thpool_p->jobqueue);
        free(thpool_p);
        return NULL;
    }

    pthread_mutex_init(&(thpool_p->thcount_lock), NULL);
    pthread_cond_init(&thpool_p->threads_all_idle, NULL);

    /* Thread init */
    int n;
    for (n = 0; n < num_threads; n++) {
        thread_init(thpool_p, &thpool_p->threads[n], n);
#if THPOOL_DEBUG
        printf("THPOOL_DEBUG: Created thread %d in pool \n", n);
#endif
    }

    /* Wait for threads to initialize */
    while (thpool_p->num_threads_alive != num_threads) {
    }

    return thpool_p;
}

struct thpool_* thpool_init(int num_threads) {
    return thpool_init_ex(num_threads, SCHED_OTHER, 0, NULL, 0);
}

/* Add work to the thread pool */
int thpool_add_work(thpool_* thpool_p, void (*function_p)(void*), void* arg_p) {
    return thpool_add_work_ex(thpool_p, function_p, arg_p, 0);
}

int thpool_add_work_ex(thpool_* thpool_p, void (*function)(void*), void* arg, size_t arg_size) {
    job* newjob = malloc(sizeof(job));
    if (!newjob) return -1;

    if (arg_size > 0) {
        newjob->arg = malloc(arg_size);
        memcpy(newjob->arg, arg, arg_size);
        newjob->arg_size = arg_size;
    } else {
        newjob->arg      = arg;
        newjob->arg_size = 0;
    }

    newjob->function = function;

    /* add job to queue */
    jobqueue_push(&thpool_p->jobqueue, newjob);

    return 0;
}

/* Wait until all jobs have finished */
void thpool_wait(thpool_* thpool_p) {
    pthread_mutex_lock(&thpool_p->thcount_lock);
    while (thpool_p->jobqueue.len || thpool_p->num_threads_working) {
        pthread_cond_wait(&thpool_p->threads_all_idle, &thpool_p->thcount_lock);
    }
    pthread_mutex_unlock(&thpool_p->thcount_lock);
}

/* Destroy the threadpool */
void thpool_destroy(thpool_* thpool_p) {
    /* No need to destroy if it's NULL */
    if (thpool_p == NULL) return;

    volatile int threads_total = thpool_p->num_threads_alive;

    /* End each thread 's infinite loop */
    threads_keepalive = 0;

    /* Give one second to kill idle threads */
    double TIMEOUT = 1.0;
    time_t start, end;
    double tpassed = 0.0;
    time(&start);
    while (tpassed < TIMEOUT && thpool_p->num_threads_alive) {
        bsem_post_all(thpool_p->jobqueue.has_jobs);
        time(&end);
        tpassed = difftime(end, start);
    }

    /* Poll remaining threads */
    while (thpool_p->num_threads_alive) {
        bsem_post_all(thpool_p->jobqueue.has_jobs);
        sleep(1);
    }

    /* Job queue cleanup */
    jobqueue_destroy(&thpool_p->jobqueue);
    /* Deallocs */
    int n;
    for (n = 0; n < threads_total; n++) {
        thread_destroy(thpool_p->threads[n]);
    }
    free(thpool_p->threads);
    free(thpool_p);
}

/* Pause all threads in threadpool */
void thpool_pause(thpool_* thpool_p) {
    int n;
    for (n = 0; n < thpool_p->num_threads_alive; n++) {
        pthread_kill(thpool_p->threads[n]->pthread, SIGUSR1);
    }
}

/* Resume all threads in threadpool */
void thpool_resume(thpool_* thpool_p) {
    // resuming a single threadpool hasn't been
    // implemented yet, meanwhile this suppresses
    // the warnings
    (void)thpool_p;

    threads_on_hold = 0;
}

int thpool_num_threads_working(thpool_* thpool_p) { return thpool_p->num_threads_working; }

/* ============================ THREAD ============================== */

/* Initialize a thread in the thread pool
 *
 * @param thread        address to the pointer of the thread to be created
 * @param id            id to be given to the thread
 * @return 0 on success, -1 otherwise.
 */
static int thread_init(thpool_* thpool_p, struct thread** thread_p, int id) {
    *thread_p = (struct thread*)malloc(sizeof(struct thread));
    if (*thread_p == NULL) {
        err("thread_init(): Could not allocate memory for thread\n");
        return -1;
    }

    (*thread_p)->thpool_p = thpool_p;
    (*thread_p)->id       = id;

    pthread_attr_t attr;

    int ret = pthread_attr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "Failed to init attr for TID (%lu): %s\n", (*thread_p)->pthread,
                strerror(ret));
    }

    /* Set thread scheduling policy */
    ret = pthread_attr_setschedpolicy(&attr, thpool_p->sched_policy);
    if (ret != 0) {
        fprintf(stderr, "Failed to set sched policy for TID (%lu): %s\n", (*thread_p)->pthread,
                strerror(ret));
    }

    if (thpool_p->sched_policy != SCHED_OTHER) {
        int min = sched_get_priority_min(thpool_p->sched_policy);
        int max = sched_get_priority_max(thpool_p->sched_policy);
        if (thpool_p->sched_priority < min || thpool_p->sched_priority > max) {
            fprintf(stderr,
                    "Invalid scheduling priority %d, Using minimum priority %d for policy %d",
                    thpool_p->sched_priority, min, thpool_p->sched_policy);
            thpool_p->sched_priority = min;
        }

        struct sched_param param;
        param.sched_priority = thpool_p->sched_priority;
        ret                  = pthread_attr_setschedparam(&attr, &param);
        if (ret) {
            fprintf(stderr, "Failed to set sched param for TID (%lu): %s\n", (*thread_p)->pthread,
                    strerror(ret));
        }
    }

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
        fprintf(stderr, "Failed to set inherit sched for TID (%lu): %s\n", (*thread_p)->pthread,
                strerror(ret));
    }

    pthread_create(&(*thread_p)->pthread, &attr, (void* (*)(void*))thread_do, (*thread_p));
    pthread_detach((*thread_p)->pthread);
    pthread_attr_destroy(&attr);

    return 0;
}

/* Sets the calling thread on hold */
static void thread_hold(int sig_id) {
    (void)sig_id;
    threads_on_hold = 1;
    while (threads_on_hold) {
        sleep(1);
    }
}

/* What each thread is doing
 *
 * In principle this is an endless loop. The only time this loop gets interrupted is once
 * thpool_destroy() is invoked or the program exits.
 *
 * @param  thread        thread that will run this function
 * @return nothing
 */
static void* thread_do(struct thread* thread_p) {
    /* Set thread name for profiling and debugging */
    char thread_name[16] = {0};

    snprintf(thread_name, 16, TOSTRING(THPOOL_THREAD_NAME) "-%d", thread_p->id);

#if defined(__linux__)
    /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
    prctl(PR_SET_NAME, thread_name);
#elif defined(__APPLE__) && defined(__MACH__)
    pthread_setname_np(thread_name);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    pthread_set_name_np(thread_p->pthread, thread_name);
#else
    err("thread_do(): pthread_setname_np is not supported on this system");
#endif

    /* Assure all threads have been created before starting serving */
    thpool_* thpool_p = thread_p->thpool_p;

    /* Set CPU affinity */
    if (thpool_p->num_cores > 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        int core_idx = thread_p->id % thpool_p->num_cores;
        if (thpool_p->cpu_cores[core_idx] >= 0) {
            CPU_SET(thpool_p->cpu_cores[core_idx], &cpuset);

            int ret = 0;
#if defined(__ANDROID__)
            pid_t tid = (pid_t)syscall(__NR_gettid);
            ret       = sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
            if (ret != 0) {
                fprintf(stderr, "Failed to set CPU affinity for Thread (%s) to core %d: %s\n",
                        thread_name, thpool_p->cpu_cores[core_idx], strerror(ret));
            }
#elif defined(__linux__) && defined(_GNU_SOURCE)
            ret = pthread_setaffinity_np(thread_p->pthread, sizeof(cpu_set_t), &cpuset);
            if (ret != 0) {
                fprintf(stderr, "Failed to set CPU affinity for Thread (%s) to core %d: %s\n",
                        thread_name, thpool_p->cpu_cores[core_idx], strerror(ret));
            }
#else
            fprintf(stderr, "CPU affinity not supported on this platform.\n");
#endif
        }
    }

    /* Register signal handler */
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags   = SA_ONSTACK;
    act.sa_handler = thread_hold;
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        err("thread_do(): cannot handle SIGUSR1");
    }

    /* Mark thread as alive (initialized) */
    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive += 1;
    pthread_mutex_unlock(&thpool_p->thcount_lock);

    while (threads_keepalive) {
        bsem_wait(thpool_p->jobqueue.has_jobs);

        if (threads_keepalive) {
            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working++;
            pthread_mutex_unlock(&thpool_p->thcount_lock);

            /* Read job from queue and execute it */
            void (*func_buff)(void*);
            void* arg_buff;
            job* job_p = jobqueue_pull(&thpool_p->jobqueue);
            if (job_p) {
                func_buff = job_p->function;
                arg_buff  = job_p->arg;
                func_buff(arg_buff);
                if (job_p->arg_size > 0) free(job_p->arg);
                free(job_p);
            }

            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working--;
            if (!thpool_p->num_threads_working) {
                pthread_cond_signal(&thpool_p->threads_all_idle);
            }
            pthread_mutex_unlock(&thpool_p->thcount_lock);
        }
    }
    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive--;
    pthread_mutex_unlock(&thpool_p->thcount_lock);

    return NULL;
}

/* Frees a thread  */
static void thread_destroy(thread* thread_p) { free(thread_p); }

/* ============================ JOB QUEUE =========================== */

/* Initialize queue */
static int jobqueue_init(jobqueue* jobqueue_p) {
    jobqueue_p->len   = 0;
    jobqueue_p->front = NULL;
    jobqueue_p->rear  = NULL;

    jobqueue_p->has_jobs = (struct bsem*)malloc(sizeof(struct bsem));
    if (jobqueue_p->has_jobs == NULL) {
        return -1;
    }

    pthread_mutex_init(&(jobqueue_p->rwmutex), NULL);
    bsem_init(jobqueue_p->has_jobs, 0);

    return 0;
}

/* Clear the queue */
static void jobqueue_clear(jobqueue* jobqueue_p) {
    while (jobqueue_p->len) {
        free(jobqueue_pull(jobqueue_p));
    }

    jobqueue_p->front = NULL;
    jobqueue_p->rear  = NULL;
    bsem_reset(jobqueue_p->has_jobs);
    jobqueue_p->len = 0;
}

/* Add (allocated) job to queue
 */
static void jobqueue_push(jobqueue* jobqueue_p, struct job* newjob) {
    pthread_mutex_lock(&jobqueue_p->rwmutex);
    newjob->prev = NULL;

    switch (jobqueue_p->len) {
        case 0: /* if no jobs in queue */
            jobqueue_p->front = newjob;
            jobqueue_p->rear  = newjob;
            break;

        default: /* if jobs in queue */
            jobqueue_p->rear->prev = newjob;
            jobqueue_p->rear       = newjob;
    }
    jobqueue_p->len++;

    bsem_post(jobqueue_p->has_jobs);
    pthread_mutex_unlock(&jobqueue_p->rwmutex);
}

/* Get first job from queue(removes it from queue)
 * Notice: Caller MUST hold a mutex
 */
static struct job* jobqueue_pull(jobqueue* jobqueue_p) {
    pthread_mutex_lock(&jobqueue_p->rwmutex);
    job* job_p = jobqueue_p->front;

    switch (jobqueue_p->len) {
        case 0: /* if no jobs in queue */
            break;

        case 1: /* if one job in queue */
            jobqueue_p->front = NULL;
            jobqueue_p->rear  = NULL;
            jobqueue_p->len   = 0;
            break;

        default: /* if >1 jobs in queue */
            jobqueue_p->front = job_p->prev;
            jobqueue_p->len--;
            /* more than one job in queue -> post it */
            bsem_post(jobqueue_p->has_jobs);
    }

    pthread_mutex_unlock(&jobqueue_p->rwmutex);
    return job_p;
}

/* Free all queue resources back to the system */
static void jobqueue_destroy(jobqueue* jobqueue_p) {
    jobqueue_clear(jobqueue_p);
    free(jobqueue_p->has_jobs);
}

/* ======================== SYNCHRONISATION ========================= */

/* Init semaphore to 1 or 0 */
static void bsem_init(bsem* bsem_p, int value) {
    if (value < 0 || value > 1) {
        err("bsem_init(): Binary semaphore can take only values 1 or 0");
        exit(1);
    }
    pthread_mutex_init(&(bsem_p->mutex), NULL);
    pthread_cond_init(&(bsem_p->cond), NULL);
    bsem_p->v = value;
}

/* Reset semaphore to 0 */
static void bsem_reset(bsem* bsem_p) {
    pthread_mutex_destroy(&(bsem_p->mutex));
    pthread_cond_destroy(&(bsem_p->cond));
    bsem_init(bsem_p, 0);
}

/* Post to at least one thread */
static void bsem_post(bsem* bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_signal(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/* Post to all threads */
static void bsem_post_all(bsem* bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_broadcast(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

/* Wait on semaphore until semaphore has value 0 */
static void bsem_wait(bsem* bsem_p) {
    pthread_mutex_lock(&bsem_p->mutex);
    while (bsem_p->v != 1) {
        pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
    }
    bsem_p->v = 0;
    pthread_mutex_unlock(&bsem_p->mutex);
}
