#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#include "rkisp_control_loop.h"
#include "mediactl.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define DBG(...) do { if(!silent) printf("DBG: " __VA_ARGS__);} while(0)
#define ERR(...) do { fprintf(stderr, "ERR: " __VA_ARGS__); } while (0)


/* Private v4l2 event */
#define CIFISP_V4L2_EVENT_STREAM_START  \
                    (V4L2_EVENT_PRIVATE_START + 1)
#define CIFISP_V4L2_EVENT_STREAM_STOP   \
                    (V4L2_EVENT_PRIVATE_START + 2)

#define RKISP_FILE_PATH_LEN                       64
#define RKISP_CAMS_NUM_MAX                        2
#define RKISP_FLASH_NUM_MAX                       2

static struct rkisp_media_info media_info;
static void* rkisp_engine;
static int silent = 0;
static const char *mdev_path;

static void errno_exit(const char *s)
{
    ERR("%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static void init_engine(void)
{
    struct rkisp_cl_prepare_params_s params = {0};
    int ret = 0;

    ret = rkisp_cl_init(&rkisp_engine, NULL, NULL);
    if (ret) {
        ERR("rkisp engine init failed !\n");
        exit(-1);
    }

    params.isp_sd_node_path = media_info.sd_isp_path;
    params.isp_vd_params_path = media_info.vd_params_path;
    params.isp_vd_stats_path = media_info.vd_stats_path;
    params.staticMeta = NULL;

    for (int i = 0; i < RKISP_CAMS_NUM_MAX; i++) {
        if (!media_info.cams[i].link_enabled) {
            DBG("Link disabled, skipped\n");
            continue;
        }

        DBG("%s: link enabled %d\n", media_info.cams[i].sd_sensor_path,
            media_info.cams[i].link_enabled);

        params.sensor_sd_node_path = media_info.cams[i].sd_sensor_path;

        if (strlen(media_info.cams[i].sd_lens_path))
            params.lens_sd_node_path = media_info.cams[i].sd_lens_path;
        if (strlen(media_info.cams[i].sd_flash_path[0]))
            params.flashlight_sd_node_path[0] = media_info.cams[i].sd_flash_path[0];

        break;
    }

    if (rkisp_cl_prepare(rkisp_engine, &params)) {
        ERR("rkisp engine prepare failed !\n");
        exit(-1);
    }
}

static void start_engine(void)
{
    DBG("device manager start\n");
    rkisp_cl_start(rkisp_engine);
    if (rkisp_engine == NULL) {
        ERR("rkisp_init engine failed\n");
        exit(-1);
    } else {
        DBG("rkisp_init engine succeed\n");
    }
}

static void stop_engine(void)
{
    rkisp_cl_stop(rkisp_engine);
}

static void deinit_engine(void)
{
    rkisp_cl_deinit(rkisp_engine);
}

// blocked func
static int wait_stream_event(int fd, unsigned int event_type, int time_out_ms)
{
    int ret;
    struct v4l2_event event;

    CLEAR(event);

    do {
	/*
	 * xioctl instead of poll.
	 * Since poll() cannot wait for input before stream on,
	 * it will return an error directly. So, use ioctl to
	 * dequeue event and block until sucess.
	 */
	ret = xioctl(fd, VIDIOC_DQEVENT, &event);
	if (ret == 0 && event.type == event_type) {
		return 0;
	}
    } while (true);

    return -1;

}

static int subscrible_stream_event(int fd, bool subs)
{
    struct v4l2_event_subscription sub;
    int ret = 0;

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_START;
    ret = xioctl(fd,
                 subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
                 &sub);
    if (ret) {
        ERR("can't subscribe %s start event!\n", media_info.vd_params_path);
        exit(EXIT_FAILURE);
    }

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
    ret = xioctl(fd,
                 subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
                 &sub);
    if (ret) {
        ERR("can't subscribe %s stop event!\n", media_info.vd_params_path);
        exit(EXIT_FAILURE);
    }

    DBG("subscribe events from %s success !\n", media_info.vd_params_path);

    return 0;
}

void parse_args(int argc, char **argv)
{
   int c;
   int digit_optind = 0;

   while (1) {
       int this_option_optind = optind ? optind : 1;
       int option_index = 0;
       static struct option long_options[] = {
           {"mmedia",    required_argument, 0, 'm' },
           {"silent",    no_argument,       0, 's' },
           {"help",      no_argument,       0, 'h' },
           {0,           0,                 0,  0  }
       };

       c = getopt_long(argc, argv, "m:sh", long_options, &option_index);
       if (c == -1)
           break;

       switch (c) {
       case 'm':
           mdev_path = optarg;
           break;
       case 's':
           silent = 1;
           break;
       case '?':
       case 'h':
           ERR("Usage: %s to start 3A engine\n"
               "         --mmedia,  required, mapped media device node\n"
               "         --silent,  optional, subpress debug log\n",
               argv[0]);
           exit(-1);

       default:
           ERR("?? getopt returned character code %c ??\n", c);
       }
   }

   if (!mdev_path) {
        ERR("argument --mmedia is required\n");
        exit(-1);
   }
}

int main(int argc, char **argv)
{
    int ret = 0;
    int isp_fd;
    unsigned int stream_event = -1;

    /* Line buffered so that printf can flash every line if redirected to
     * no-interactive device.
     */
    setlinebuf(stdout);

    parse_args(argc, argv);

    for (;;) {
        /* Refresh media info so that sensor link status updated */
        if (rkisp_get_media_info(&media_info, mdev_path))
            errno_exit("Bad media topology\n");

        isp_fd = open(media_info.vd_params_path, O_RDWR);
        if (isp_fd < 0) {
            ERR("open %s failed %s\n", media_info.vd_params_path,
                strerror(errno));
            exit(-1);
        }
        subscrible_stream_event(isp_fd, true);
        init_engine();

        {
            DBG("wait stream start event...\n");
            wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
            DBG("wait stream start event success ...\n");

            start_engine();

            DBG("wait stream stop event...\n");
            wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
            DBG("wait stream stop event success ...\n");

            stop_engine();
        }

        deinit_engine();
        subscrible_stream_event(isp_fd, false);
        close(isp_fd);

        DBG("----------------------------------------------\n\n");
    }

    return 0;
}
