/*
 * V4L2 video capture example
 * AUTHOT : Jacob Chen
 * DATA : 2018-02-25
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>

#include <linux/videodev2.h>
#include <rkisp_control_loop.h>
#include <rkisp_dev_manager.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1

const char* _iq_file="/etc/cam_iq.xml";
void* _rkisp_engine;

typedef int (*rkisp_init_func)(void** cl_ctx, const char* tuning_file_path);
typedef int (*rkisp_prepare_func)(void* cl_ctx,
                     const struct rkisp_cl_prepare_params_s* prepare_params);
typedef int (*rkisp_start_func)(void* cl_ctx);
typedef int (*rkisp_stop_func)(void* cl_ctx);
typedef int (*rkisp_deinit_func)(void* cl_ctx);

struct RKIspFunc {
    void* rkisp_handle;
	rkisp_init_func init_func;
	rkisp_prepare_func prepare_func;
    rkisp_start_func start_func;
    rkisp_stop_func stop_func;
	rkisp_deinit_func deinit_func;
};
struct RKIspFunc _RKIspFunc;

struct buffer {
        void *start;
        size_t length;
};

static char *dev_name;
static int fd = -1;
static enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
struct buffer *buffers;
static unsigned int n_buffers;
static int frame_count = 3;
FILE *fp;

static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
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

static void process_image(const void *p, int size)
{
		printf("process_image size: %d\n",size);
        fwrite(p,size, 1, fp); 
}

static int read_frame(FILE *fp)
{
        struct v4l2_buffer buf;
        CLEAR(buf);

        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_MMAP;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            struct v4l2_plane planes[FMT_NUM_PLANES];
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) 
                errno_exit("VIDIOC_DQBUF");

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            printf("multi-planes bytesused %d\n", buf.m.planes[0].bytesused);
            process_image(buffers[buf.index].start, buf.m.planes[0].bytesused);
        } else {
            printf("bytesused %d\n", buf.m.planes[0].bytesused);
            process_image(buffers[buf.index].start, buf.bytesused);
        }

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF"); 

        return 1;
}

static void mainloop(void)
{
        unsigned int count;
        count = frame_count;
        while (count-- > 0) {
            printf("No.%d\n",frame_count - count);        //显示当前帧数目
            read_frame(fp);
        }
        printf("\nREAD AND SAVE DONE!\n");
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

    	if (_RKIspFunc.stop_func != NULL) {
    	    printf ("stop rkisp engine\n");
    	    _RKIspFunc.stop_func(_rkisp_engine);
    	}

        type = buf_type;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

    	if (_RKIspFunc.init_func != NULL) {
			_RKIspFunc.init_func(&_rkisp_engine, _iq_file);
		}

    	if (_RKIspFunc.prepare_func != NULL) {
			struct rkisp_cl_prepare_params_s params={0};

            // isp subdev node path
            params.isp_sd_node_path="/dev/v4l-subdev0";
            // isp params video node path
            params.isp_vd_params_path="/dev/video3";
            // isp statistics video node path
            params.isp_vd_stats_path="/dev/video2";
            // camera sensor subdev node path
            params.sensor_sd_node_path="/dev/v4l-subdev2";

			_RKIspFunc.prepare_func(_rkisp_engine, &params);
		}

    	if (_RKIspFunc.start_func != NULL) {
    	    printf ("device manager start, capture dev fd: %d\n", fd);
    	    _RKIspFunc.start_func(_rkisp_engine);
    	    printf ("device manager isp_init\n");

    	    if (_rkisp_engine == NULL) {
    	        printf ("rkisp_init engine failed\n");
    	    } else {
    	        printf ("rkisp_init engine succeed\n");
    	    }
    	}

        for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    struct v4l2_plane planes[FMT_NUM_PLANES];
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
        }
        type = buf_type;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(void)
{
        unsigned int i;

        for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length))
                        errno_exit("munmap");

        free(buffers);

        if (_RKIspFunc.deinit_func != NULL) {
            _RKIspFunc.deinit_func(_rkisp_engine);
        }

        dlclose(_RKIspFunc.rkisp_handle);
}



static void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = buf_type;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    struct v4l2_plane planes[FMT_NUM_PLANES];
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }

                if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.m.planes[0].length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.planes[0].m.mem_offset);
                } else {
                    buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);
                }

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}


static void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_format fmt;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\n",
                                 dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
                !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
            fprintf(stderr, "%s is not a video capture device, capabilities: %x\n",
                         dev_name, cap.capabilities);
                exit(EXIT_FAILURE);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                fprintf(stderr, "%s does not support streaming i/o\n",
                    dev_name);
                exit(EXIT_FAILURE);
        }



        CLEAR(fmt);
        if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
            buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

        fmt.type = buf_type;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");

        init_mmap();

		//INIT RKISP
		_RKIspFunc.rkisp_handle = dlopen("/usr/lib/librkisp.so", RTLD_NOW);
    	if (_RKIspFunc.rkisp_handle == NULL) {
    	    printf ("open /usr/lib/librkisp.so failed");
    	} else {
    	    printf ("open /usr/lib/librkisp.so successed");
    	    _RKIspFunc.init_func=(rkisp_init_func)dlsym(_RKIspFunc.rkisp_handle, "rkisp_cl_init");
    	    _RKIspFunc.prepare_func=(rkisp_prepare_func)dlsym(_RKIspFunc.rkisp_handle, "rkisp_cl_prepare");
    	    _RKIspFunc.start_func=(rkisp_start_func)dlsym(_RKIspFunc.rkisp_handle, "rkisp_cl_start");
    	    _RKIspFunc.stop_func=(rkisp_stop_func)dlsym(_RKIspFunc.rkisp_handle, "rkisp_cl_stop");
    	    _RKIspFunc.deinit_func=(rkisp_deinit_func)dlsym(_RKIspFunc.rkisp_handle, "rkisp_cl_deinit");
    	    if (_RKIspFunc.start_func == NULL) {
    	        printf ("func rkisp_start not found.");
    	        const char *errmsg;
    	        if ((errmsg = dlerror()) != NULL) {
    	            printf("dlsym rkisp_start fail errmsg: %s", errmsg);
    	        }
    	    } else {
    	        printf("dlsym rkisp_start success");
    	    }
    	}

}

static void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

static void open_device(void)
{
        fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}


int main(int argc, char **argv)
{
        dev_name = "/dev/video0";
        
        if(argc != 2)
        {
            printf("usage :%s filename\n", argv[0]);
            exit(0);
        }
        if ((fp = fopen(argv[1], "w")) == NULL) { 
            perror("Creat file failed"); 
            exit(0); 
        } 
        open_device();
        init_device();
        start_capturing();
        mainloop();
        fclose(fp);
        stop_capturing();
        uninit_device();
        close_device();
        return 0;
}
