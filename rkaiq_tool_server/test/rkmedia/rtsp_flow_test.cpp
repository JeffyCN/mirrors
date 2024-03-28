#include <iostream>
#include <memory>
#include <string>

#include "assert.h"
#include "rtsp_server.h"
#include "signal.h"
#include "stdint.h"
#include "stdio.h"
#include "unistd.h"

static bool quit = false;

static void sigterm_handler(int sig)
{
    LOG("signal %d\n", sig);
    quit = true;
}

void deinit_2688p(std::shared_ptr<easymedia::Flow>& video_cap_flow, std::shared_ptr<easymedia::Flow>& video_enc_flow, std::shared_ptr<easymedia::Flow>& video_rtsp_flow)
{
    video_cap_flow->RemoveDownFlow(video_enc_flow);
    video_enc_flow->RemoveDownFlow(video_rtsp_flow);
    video_rtsp_flow.reset();
    video_enc_flow.reset();
    video_cap_flow.reset();
    video_rtsp_flow = nullptr;
    video_enc_flow = nullptr;
    video_cap_flow = nullptr;
}

std::string video_path;

void init_2688p(std::shared_ptr<easymedia::Flow>& video_cap_flow, std::shared_ptr<easymedia::Flow>& video_enc_flow, std::shared_ptr<easymedia::Flow>& video_rtsp_flow)
{
    int width = 2688;
    int height = 1520;
    int fps = 30;
    std::string yuv_format = IMAGE_NV12;
    std::string enc_type = VIDEO_H264;

    const char* video_dev = "rkispp_m_bypass";
    if (!video_path.empty())
    {
        video_dev = video_path.c_str();
    }

    printf("init_2688p video_dev %s\n", video_dev);

    video_cap_flow = create_video_capture_flow(video_dev, yuv_format, width, height);
    video_enc_flow = create_video_enc_flow(yuv_format, enc_type, width, height, fps);
    video_rtsp_flow = create_rtsp_server_flow("main_stream", VIDEO_H264);

    video_enc_flow->AddDownFlow(video_rtsp_flow, 0, 0);
    video_cap_flow->AddDownFlow(video_enc_flow, 0, 0);
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        video_path = argv[1];
    }

    signal(SIGINT, sigterm_handler);
    std::shared_ptr<easymedia::Flow> video_cap_flow;
    std::shared_ptr<easymedia::Flow> video_enc_flow;
    std::shared_ptr<easymedia::Flow> video_rtsp_flow;
    init_2688p(video_cap_flow, video_enc_flow, video_rtsp_flow);
    while (!quit)
    {
        easymedia::msleep(1000);
    }
    deinit_2688p(video_cap_flow, video_enc_flow, video_rtsp_flow);
    return 0;
}
