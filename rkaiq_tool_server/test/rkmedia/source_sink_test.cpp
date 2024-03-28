#include <iostream>
#include <memory>
#include <string>

#include "assert.h"
#include "easymedia/buffer.h"
#include "easymedia/flow.h"
#include "easymedia/image.h"
#include "easymedia/key_string.h"
#include "easymedia/media_config.h"
#include "easymedia/media_type.h"
#include "easymedia/reflector.h"
#include "easymedia/stream.h"
#include "easymedia/utils.h"
#include "signal.h"
#include "stdint.h"
#include "stdio.h"
#include "unistd.h"

static std::shared_ptr<easymedia::Flow> create_flow(const std::string& flow_name, const std::string& flow_param, const std::string& elem_param)
{
    auto&& param = easymedia::JoinFlowParam(flow_param, 1, elem_param);
    // printf("create_flow :\n");
    // printf("flow_name : %s\n", flow_name.c_str());
    // printf("param : \n%s\n", param.c_str());
    auto ret = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(flow_name.c_str(), param.c_str());
    printf(" ####### create_flow flow use_count %ld\n", ret.use_count());
    if (!ret)
    {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
    }
    return ret;
}

static std::string get_video_cap_flow_param(std::string input_path, std::string pixel_format, int video_width, int video_height)
{
    std::string flow_param;
    // Reading yuv from camera
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_NAME, "v4l2_capture_stream");
    // PARAM_STRING_APPEND_TO(flow_param, KEY_FPS, video_fps);
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_SYNC);
    PARAM_STRING_APPEND(flow_param, KEK_INPUT_MODEL, KEY_DROPFRONT);
    PARAM_STRING_APPEND_TO(flow_param, KEY_INPUT_CACHE_NUM, 5);
    return flow_param;
}

static std::string get_video_cap_stream_param(std::string input_path, std::string pixel_format, int video_width, int video_height)
{
    std::string stream_param;
    stream_param = "";
    PARAM_STRING_APPEND_TO(stream_param, KEY_USE_LIBV4L2, 1);
    PARAM_STRING_APPEND(stream_param, KEY_DEVICE, input_path);
    // PARAM_STRING_APPEND(param, KEY_SUB_DEVICE, sub_input_path);
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_CAP_TYPE, KEY_V4L2_C_TYPE(VIDEO_CAPTURE));
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_MEM_TYPE, KEY_V4L2_M_TYPE(MEMORY_DMABUF));
    PARAM_STRING_APPEND_TO(stream_param, KEY_FRAMES, 4);
    PARAM_STRING_APPEND(stream_param, KEY_OUTPUTDATATYPE, pixel_format);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_WIDTH, video_width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_HEIGHT, video_height);
    return stream_param;
}

static std::shared_ptr<easymedia::Flow> create_video_capture_flow(std::string input_path, std::string pixel_format, int video_width, int video_height)
{
    std::string flow_name;
    std::string flow_param;
    std::string stream_param;
    std::shared_ptr<easymedia::Flow> video_read_flow;

    flow_name = "source_flow";
    flow_param = get_video_cap_flow_param(input_path, pixel_format, video_width, video_height);
    stream_param = get_video_cap_stream_param(input_path, pixel_format, video_width, video_height);
    video_read_flow = create_flow(flow_name, flow_param, stream_param);
    if (!video_read_flow)
    {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }
    return video_read_flow;
}

std::shared_ptr<easymedia::Flow> create_sink_flow()
{
    std::string flow_name;
    std::string flow_param;
    std::string stream_param;
    std::shared_ptr<easymedia::Flow> sink_flow;

    flow_name = "sink_flow";

    sink_flow = create_flow(flow_name, flow_param, stream_param);
    if (!sink_flow)
    {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }
    return sink_flow;
}

static bool quit = false;

static void sigterm_handler(int sig)
{
    LOG("signal %d\n", sig);
    quit = true;
}

void deinit(std::shared_ptr<easymedia::Flow>& video_cap_flow, std::shared_ptr<easymedia::Flow>& sink_flow)
{
    video_cap_flow->RemoveDownFlow(sink_flow);
    sink_flow.reset();
    video_cap_flow.reset();
    sink_flow = nullptr;
    video_cap_flow = nullptr;
}

std::string video_path;

void init_2688p(std::shared_ptr<easymedia::Flow>& video_cap_flow, std::shared_ptr<easymedia::Flow>& sink_flow)
{
    int width = 2688;
    int height = 1520;
    int fps = 30;
    std::string yuv_format = IMAGE_NV12;
    std::string enc_type = VIDEO_H264;

    const char* video_dev = "/dev/video0";
    if (!video_path.empty())
    {
        video_dev = video_path.c_str();
    }

    printf("init_2688p video_dev %s\n", video_dev);

    video_cap_flow = create_video_capture_flow(video_dev, yuv_format, width, height);
    sink_flow = create_sink_flow();
    video_cap_flow->AddDownFlow(sink_flow, 0, 0);
}

void ImageProcess(unsigned char* buffer, unsigned int buffer_size, long present_time, int nat_type)
{
    LOG("SinkFlow process_buffer size %d\n", buffer_size);
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        video_path = argv[1];
    }
    easymedia::REFLECTOR(Stream)::DumpFactories();
    easymedia::REFLECTOR(Flow)::DumpFactories();
    signal(SIGINT, sigterm_handler);
    std::shared_ptr<easymedia::Flow> video_cap_flow;
    std::shared_ptr<easymedia::Flow> sink_flow;
    init_2688p(video_cap_flow, sink_flow);
    sink_flow->SetVideoHandler(ImageProcess);
    while (!quit)
    {
        easymedia::msleep(1000);
    }
    deinit(video_cap_flow, sink_flow);
}
