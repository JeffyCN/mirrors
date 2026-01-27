/*
 *  Copyright (c) 2025 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#define _GNU_SOURCE

#include <ctype.h>
#include <getopt.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "aiq_thpool.h"

/**
 * Example Command:
 *
 * sample_thpool_wait --set-fmt-video=width=2688,height=1520 \
 *            --stream-from "/data/test_thpool/input_2688_1520.yuv" \
 *            --stream-to "/data/test_thpool/output_2688_1520.yuv" \
 *            --thread-num 4 \
 *            --job-num 4 \
 *            --set-cpu-affinity 0,1,2,3 \
 *            --set-sched-policy --set-sched-priority 99
 *
 **/

#define info(fmt, args...)                                                                \
    do {                                                                                  \
        if (!options[OptSilent]) printf("[%s:%d] " fmt "\n", __func__, __LINE__, ##args); \
    } while (0)

#define stderr_info(fmt, args...)                                                                  \
    do {                                                                                           \
        if (!options[OptSilent]) fprintf(stderr, "[%s:%d] " fmt "\n", __func__, __LINE__, ##args); \
    } while (0)

enum Option {
    // clang-format off
    OptSetVideoFormat = 'v',
    OptSilent = 's',
    OptHelp = 'z',

    OptStreamCount = 128,
    OptStreamTo,
    OptStreamFrom,
    OptNumOfThread,
    OptNumOfJobs,
    OptWaitEachJob,
    OptSetSchedPolicy,
    OptSetSchedPriority,
    OptSetCpuAffinity,
    OptLast = 512
    // clang-format on
};

static char options[OptLast];

static struct option long_options[] = {
    // clang-format off
    {"help", no_argument, 0, OptHelp},
    {"silent", no_argument, 0, OptSilent},
    {"set-fmt-video", required_argument, NULL, OptSetVideoFormat},
    {"stream-to", required_argument, NULL, OptStreamTo},
    {"stream-from", required_argument, NULL, OptStreamFrom},
    {"thread-num", required_argument, NULL, OptNumOfThread},
    {"job-num", required_argument, NULL, OptNumOfJobs},
    {"wait-each-job", no_argument, NULL, OptWaitEachJob},
    {"set-sched-policy", no_argument, NULL, OptSetSchedPolicy},
    {"set-sched-priority", required_argument, NULL, OptSetSchedPriority},
    {"set-cpu-affinity", required_argument, NULL, OptSetCpuAffinity},
    {0, 0, 0, 0}
    // clang-format on
};

void common_usage(void) {
    printf(
        "\nGeneral/Common options:\n"
        "  -v, --set-fmt-video\n"
        "width=<w>,height=<h>,pixelformat=<pf>,field=<f>,colorspace=<c>,\n"
        "  --stream-from <file> stream from this file.\n"
        "  --stream-to <file>\n"
        "                     stream to this file. The default is to discard the data.\n"
        "  -s, --silent       subpress debug log.\n"
        "  --thread-num <num> set the number of threads to thread pool.\n"
        "                     The default is 2.\n"
        "  --job-num <num>    set the number of jobs to thread pool.\n"
        "                     The default is 4.\n"
        "  --wait-each-job    wait each job to finish.\n"
        "  --set-sched-policy set the sched policy to thread pool.\n"
        "  --set-sched-priority <priority>\n"
        "                     set the sched priority to thread pool.\n"
        "  --set-cpu-affinity <cpu_mask>\n"
        "                     set the cpu affinity to thread pool.\n"
        "  -h, --help         display this help message.\n");
}

typedef struct {
    const uint8_t* src_buffer;
    uint8_t* dst_buffer;
    int width;
    int height;
    int start_line;
    int copy_line_cnt;
} NV12CopyParams;

static int width, height, pixfmt;
static const char* file_from;
static const char* file_to;
static int num_threads = 2;
static int num_jobs    = 4;
static int wait_each_job;
static int cpu_cores[8];
static int num_cores;
static int sched_policy = SCHED_OTHER;
static int sched_priority;

static int parse_subopt(char** subs, const char* const* subopts, char** value) {
    int opt = getsubopt(subs, (char* const*)(subopts), value);

    if (opt == -1) {
        fprintf(stderr, "Invalid suboptions specified\n");
        return -1;
    }
    if (*value == NULL) {
        fprintf(stderr, "No value given to suboption <%s>\n", subopts[opt]);
        return -1;
    }
    return opt;
}

static int parse_fmt(char* optarg, int* width, int* height, int* pixelformat) {
    char *value, *subs;
    bool be_pixfmt;

    subs = optarg;
    while (*subs != '\0') {
        static const char* subopts[] = {"width", "height", "pixelformat", NULL};

        switch (parse_subopt(&subs, subopts, &value)) {
            case 0:
                *width = strtoul(value, NULL, 0);
                break;
            case 1:
                *height = strtoul(value, NULL, 0);
                break;
            case 2:
                be_pixfmt = strlen(value) == 7 && !memcmp(value + 4, "-BE", 3);
                if (be_pixfmt || strlen(value) == 4) {
                    *pixelformat = v4l2_fourcc(value[0], value[1], value[2], value[3]);
                    if (be_pixfmt) *pixelformat |= 1U << 31;
                } else if (isdigit(value[0])) {
                    *pixelformat = strtol(value, NULL, 0);
                } else {
                    fprintf(stderr, "The pixelformat '%s' is invalid\n", value);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                return -1;
        }
    }
    return 0;
}

static void copy_nv12_lines(const NV12CopyParams* params) {
    if (!params || !params->src_buffer || !params->dst_buffer || params->width <= 0 ||
        params->height <= 0 || params->start_line < 0 || params->copy_line_cnt <= 0) {
        return;
    }

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    int cpu                 = sched_getcpu();
    unsigned long thread_id = (unsigned long)pthread_self();
    char th_name[16]        = {0};

    if (pthread_getname_np(pthread_self(), th_name, sizeof(th_name)) != 0)
        stderr_info("Failed to pthread_getname_np");

    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);

    const char* policy_str = "UNKNOWN";
    switch (policy) {
        case SCHED_FIFO:
            policy_str = "FIFO";
            break;
        case SCHED_RR:
            policy_str = "RR";
            break;
        case SCHED_OTHER:
            policy_str = "OTHER";
            break;
    }

    info(
        "[Perf] Thr: %-12s | TID: %lu | CPU: %d | Policy: %-5s | Prio: %-3d | lines: [%d -> %d] | "
        "Copy Start",
        th_name, thread_id, cpu, policy_str, param.sched_priority, params->start_line,
        params->start_line + params->copy_line_cnt - 1);

    const uint8_t* src   = params->src_buffer;
    uint8_t* dst         = params->dst_buffer;
    const int width      = params->width;
    const int height     = params->height;
    const int start_line = params->start_line;
    const int line_count = params->copy_line_cnt;

    const int y_plane_height = height;
    const int actual_y_lines =
        (start_line + line_count > y_plane_height) ? (y_plane_height - start_line) : line_count;

    const uint8_t* y_src = src + start_line * width;
    uint8_t* y_dst       = dst + start_line * width;
    for (int i = 0; i < actual_y_lines; ++i) {
        for (int j = 0; j < width; ++j) {
            y_dst[i * width + j] = y_src[i * width + j];
        }
    }

    const uint8_t* uv_src   = src + y_plane_height * width;
    uint8_t* uv_dst         = dst + y_plane_height * width;
    const int uv_start_line = start_line / 2;
    const int uv_line_count = (actual_y_lines + 1) / 2;

    if (uv_start_line + uv_line_count > (height / 2)) {
        return;
    }

    for (int i = 0; i < uv_line_count; ++i) {
        const size_t offset = (uv_start_line + i) * width;
        for (int j = 0; j < width; ++j) {
            uv_dst[offset + j] = uv_src[offset + j];
        }
    }

    gettimeofday(&end_time, NULL);

    long seconds      = end_time.tv_sec - start_time.tv_sec;
    long micros       = ((seconds * 1000000) + end_time.tv_usec) - start_time.tv_usec;
    double elapsed_ms = micros / 1000.0;

    info(
        "[Perf] Thr: %-12s | TID: %lu | "
        "CPU: %d | Policy: %-5s | Prio: %-3d | lines: [%d -> %d] | Copy Time: %.2f ms",
        th_name, thread_id, cpu, policy_str, param.sched_priority, params->start_line,
        params->start_line + params->copy_line_cnt - 1, elapsed_ms);

    return;
}

static size_t read_image_from_file(const char* file_path, char* buffer, size_t buffer_size) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        stderr_info("Could not open %s for reading\n", file_path);
        return 0;
    }

    size_t bytes_read = fread(buffer, 1, buffer_size, file);

    fclose(file);
    return bytes_read;
}

static size_t write_buffer_to_file(const char* file_path, const char* buffer, size_t buffer_size) {
    FILE* fout = fopen(file_path, "w+");
    if (fout == NULL) {
        stderr_info("Could not open %s for reading\n", file_path);
        return 0;
    }

    size_t bytes_written = fwrite(buffer, 1, buffer_size, fout);
    if (bytes_written != buffer_size) {
        stderr_info("Error writing to file");
    }

    fclose(fout);
    return bytes_written;
}

int main(int argc, char* argv[]) {
    int32_t i;
    struct timeval start_time, end_time;

    /* command args */
    int32_t ch;
    char short_options[6 * 2 * 3 + 1];
    int32_t idx = 0;
    FILE* fout  = NULL;

    if (argc == 1) {
        common_usage();
        return 0;
    }
    for (i = 0; long_options[i].name; i++) {
        if (!isalpha(long_options[i].val)) continue;
        short_options[idx++] = long_options[i].val;
        if (long_options[i].has_arg == required_argument) {
            short_options[idx++] = ':';
        } else if (long_options[i].has_arg == optional_argument) {
            short_options[idx++] = ':';
            short_options[idx++] = ':';
        }
    }
    while (1) {
        int option_index = 0;
        int ret          = 0;

        short_options[idx] = 0;
        ch                 = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (ch == -1) break;

        options[(int)ch] = 1;
        switch (ch) {
            case OptHelp:
                common_usage();
                return 0;
            case OptSetVideoFormat:
                ret = parse_fmt(optarg, &width, &height, &pixfmt);
                if (ret < 0) {
                    common_usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case OptStreamTo:
                file_to = optarg;
                break;
            case OptStreamFrom:
                file_from = optarg;
                break;
            case OptNumOfThread:
                num_threads = strtoul(optarg, NULL, 0);
                if (num_threads < 1) {
                    fprintf(stderr, "Invalid thread number %d", num_threads);
                    exit(EXIT_FAILURE);
                }
                break;
            case OptNumOfJobs:
                num_jobs = strtoul(optarg, NULL, 0);
                if (num_jobs < 1) {
                    fprintf(stderr, "Invalid job number %d", num_jobs);
                    exit(EXIT_FAILURE);
                }
                break;
            case OptWaitEachJob:
                wait_each_job = 1;
                break;
            case OptSilent:
                options[OptSilent] = 1;
                break;
            case OptSetSchedPolicy:
                sched_policy = SCHED_RR;
                break;
            case OptSetSchedPriority:
                sched_priority = strtoul(optarg, NULL, 0);
                if (sched_priority < 0 || sched_priority > 99) {
                    fprintf(stderr, "Invalid sched priority %d", sched_priority);
                    exit(EXIT_FAILURE);
                }
                break;
            case OptSetCpuAffinity:
                num_cores = sscanf(optarg, "%d,%d,%d,%d,%d,%d,%d,%d", &cpu_cores[0], &cpu_cores[1],
                                   &cpu_cores[2], &cpu_cores[3], &cpu_cores[4], &cpu_cores[5],
                                   &cpu_cores[6], &cpu_cores[7]);
                if (num_cores < 1 || num_cores > 8) {
                    fprintf(stderr, "Invalid cpu affinity %s", optarg);
                    exit(EXIT_FAILURE);
                }

                info("cpu_cores: %d", num_cores);
                for (int i = 0; i < num_cores; i++) {
                    info("cpu_cores[%d]: %d", i, cpu_cores[i]);
                }
                break;
            default:
                common_usage();
                break;
        }
    }
    if (optind < argc) {
        stderr_info("unknown arguments: ");
        while (optind < argc) stderr_info("%s ", argv[optind++]);
        stderr_info("\n");
        common_usage();
        exit(EXIT_FAILURE);
    }

    info("Width: %d, Height: %d", width, height);
    info("Num_threads : %d, Num_jobs: %d", num_threads, num_jobs);

    uint8_t* src_buffer = malloc(width * height * 3 / 2);  // NV12��С
    uint8_t* dst_buffer = malloc(width * height * 3 / 2);

    if (file_from) {
        size_t bytes_read =
            read_image_from_file(file_from, (char*)src_buffer, width * height * 3 / 2);
        if (bytes_read != width * height * 3 / 2) {
            fprintf(stderr, "Error reading file %s, expected %d bytes, got %zu\n", file_from,
                    width * height * 3 / 2, bytes_read);
            goto error;
        }
    }

    gettimeofday(&start_time, NULL);

    threadpool thpool = NULL;
    if (sched_policy != 0 || num_cores > 0)
        thpool = thpool_init_ex(num_threads, sched_policy, sched_priority, cpu_cores, num_cores);
    else
        thpool = thpool_init(num_threads);

    if (thpool == NULL) {
        fprintf(stderr, "Error creating thread pool\n");
        goto error;
    }

    for (int n = 0; n < num_jobs; n++) {
        // clang-format off
        NV12CopyParams copy_params = {
            .src_buffer    = src_buffer,
            .dst_buffer    = dst_buffer,
            .width         = width,
            .height        = height,
            .start_line    = n * height / num_jobs,
            .copy_line_cnt = (n + 1 == num_jobs) ? (height - n * height / num_jobs) : height / num_jobs
        };
        // clang-format on

        info("Adding job %d to thread pool", n);

        thpool_add_work_ex(thpool, (void*)copy_nv12_lines, &copy_params, sizeof(copy_params));
        if (wait_each_job) thpool_wait(thpool);
    }

    if (!wait_each_job)
        thpool_wait(thpool);
    else
        info("[Mode] Running in 'wait_each_job' mode");

    gettimeofday(&end_time, NULL);

    long seconds      = end_time.tv_sec - start_time.tv_sec;
    long micros       = ((seconds * 1000000) + end_time.tv_usec) - start_time.tv_usec;
    double elapsed_ms = micros / 1000.0;

    info("[Perf] Total execution time: %.2f ms | Thread pool size: %d\n", elapsed_ms, num_threads);

    if (file_to) {
        size_t bytes_written =
            write_buffer_to_file(file_to, (char*)dst_buffer, width * height * 3 / 2);
        if (bytes_written != width * height * 3 / 2) {
            fprintf(stderr, "Error writing file %s\n", file_to);
            goto error;
        }
    }

    if (src_buffer) free(src_buffer);
    if (dst_buffer) free(dst_buffer);
    thpool_destroy(thpool);

    return 0;

error:
    if (src_buffer) free(src_buffer);
    if (dst_buffer) free(dst_buffer);
    if (thpool) thpool_destroy(thpool);
    return -1;
}
