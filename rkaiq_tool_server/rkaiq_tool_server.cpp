#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <ctime>
#include <thread>
#include <sys/types.h>
#include <dirent.h>
#include <memory>

#include "camera_infohw.h"
#include "domain_tcp_client.h"
#include "rk-camera-module.h"
#if 0
    #include "rkaiq_manager.h"
#endif
#include "rkaiq_protocol.h"
#include "rkaiq_socket.h"
#include "tcp_server.h"
#ifdef __ANDROID__
    #include <rtspserver/RtspServer.h>
    #include <cutils/properties.h>
    #include "RtspServer.h"
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

DomainTCPClient g_tcpClient;
string g_linuxSocketDomainPath = "/tmp/UNIX.domain0";

struct ucred* g_aiqCred = nullptr;
std::atomic_bool quit{false};
std::atomic<int> g_app_run_mode(APP_RUN_STATUS_INIT);
int g_width = 1920;
int g_height = 1080;
int g_device_id = 0;
uint32_t g_mmapNumber = 2;
uint32_t g_offlineFrameRate = 10;
int g_rtsp_en = 0;
int g_rtsp_en_from_cmdarg = 0;
int g_allow_killapp = 0;
int g_cam_count = 0;
uint32_t g_sensorHdrMode = 0;
std::string g_capture_dev_name = "";

std::string g_stream_dev_name;
int g_stream_dev_index = -1; // for yuv capture rkisp_iqtool node. RKISP_CMD_SET_IQTOOL_CONN_ID 0:mainpath 1:selfpath
std::string iqfile;
std::string g_sensor_name;
int g_sensorMemoryMode = -1;
int g_sensorSyncMode = -1;

std::shared_ptr<TCPServer> tcpServer = nullptr;
#if 0
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif
std::shared_ptr<RKAiqMedia> rkaiq_media;

void signal_handle(int sig)
{
    quit.store(true, std::memory_order_release);
    if (tcpServer != nullptr)
        tcpServer->SaveExit();

    RKAiqProtocol::Exit();

    // if (g_rtsp_en)
    //     deinit_rtsp();
}

static int get_env(const char* name, int* value, int default_value)
{
    char* ptr = getenv(name);
    if (NULL == ptr)
    {
        *value = default_value;
    }
    else
    {
        char* endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        errno = 0;
        *value = strtoul(ptr, &endptr, base);
        if (errno || (ptr == endptr))
        {
            errno = 0;
            *value = default_value;
        }
    }
    return 0;
}

static const char short_options[] = "c:s:S:r:i:m:Dd:w:h:n:f:";
static const struct option long_options[] = {{"stream_dev", required_argument, NULL, 's'},
                                             {"enable_rtsp", required_argument, NULL, 'r'},
                                             {"iqfile", required_argument, NULL, 'i'},
                                             {"mode", required_argument, NULL, 'm'},
                                             {"width", no_argument, NULL, 'w'},
                                             {"height", no_argument, NULL, 'h'},
                                             {"device_id", required_argument, NULL, 'd'},
                                             {"mmap_buffer", required_argument, NULL, 'n'},
                                             {"frame_rate", required_argument, NULL, 'f'},
                                             {"help", no_argument, NULL, 'h'},
                                             {0, 0, 0, 0}};

static int parse_args(int argc, char** argv)
{
    int ret = 0;
    for (;;)
    {
        int idx;
        int c;
        c = getopt_long(argc, argv, short_options, long_options, &idx);
        if (-1 == c)
        {
            break;
        }
        switch (c)
        {
            case 0:
                break;
            case 's':
                g_stream_dev_name = optarg;
                break;
            case 'S':
                g_stream_dev_index = atoi(optarg);
                break;
            case 'r':
                g_rtsp_en_from_cmdarg = atoi(optarg);
                if (g_rtsp_en_from_cmdarg != 0 && g_rtsp_en_from_cmdarg != 1)
                {
                    LOG_ERROR("enable_rtsp arg|only equals 0 or 1\n");
                    ret = 1;
                }
                break;
            case 'i':
                iqfile = optarg;
                break;
            case 'm':
                g_app_run_mode = atoi(optarg);
                break;
            case 'w':
                g_width = atoi(optarg);
                break;
            case 'h':
                g_height = atoi(optarg);
                break;
            case 'd':
                g_device_id = atoi(optarg);
                break;
            case 'n':
                g_mmapNumber = (uint32_t)atoi(optarg);
                if (g_mmapNumber < 2)
                {
                    g_mmapNumber = 2;
                    LOG_INFO("mmap Number out of range[2,x], use minimum value 2\n");
                }
                break;
            case 'f':
                g_offlineFrameRate = (uint32_t)atoi(optarg);
                if (g_offlineFrameRate < 1)
                {
                    g_offlineFrameRate = 1;
                }
                else if (g_offlineFrameRate > 100)
                {
                    g_offlineFrameRate = 100;
                }
                LOG_INFO("set framerate:%u\n", g_offlineFrameRate);
                break;
            case 'c':
                g_capture_dev_name = optarg;
                LOG_INFO("capture image using compact mode. capture dev name:%s\n", g_capture_dev_name.c_str());
                break;
            default:
                break;
        }
    }
    if (iqfile.empty())
    {
#ifdef __ANDROID__
        iqfile = "/vendor/etc/camera/rkisp2";
#else
        iqfile = "/oem/etc/iqfiles";
#endif
    }

    return ret;
}

int main(int argc, char** argv)
{
    int ret = -1;
    LOG_ERROR("#### 2023-7-31 11:17:41 ####\n");
    signal(SIGPIPE, SIG_IGN);

#ifdef _WIN32
    signal(SIGINT, signal_handle);
    signal(SIGQUIT, signal_handle);
    signal(SIGTERM, signal_handle);
#else
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    struct sigaction new_action, old_action;
    new_action.sa_handler = signal_handle;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);
    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGTERM, &new_action, NULL);
#endif

#ifdef __ANDROID__
    char property_value[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.aiqtool.log", property_value, "5");
    log_level = strtoull(property_value, nullptr, 16);
    property_get("persist.vendor.aiqtool.killapp", property_value, "1");
    g_allow_killapp = strtoull(property_value, nullptr, 16);
    // property_get("persist.vendor.rkisp_no_read_back", property_value, "-1");
    // readback = strtoull(property_value, nullptr, 16);
#else
    get_env("rkaiq_tool_server_log_level", &log_level,
            LOG_LEVEL_DEBUG); // LOG_LEVEL_ERROR   LOG_LEVEL_WARN  LOG_LEVEL_INFO  LOG_LEVEL_DEBUG
    get_env("rkaiq_tool_server_kill_app", &g_allow_killapp, 0);
#endif

    if (parse_args(argc, argv) != 0)
    {
        LOG_ERROR("Tool server args parse error.\n");
        return 1;
    }

    LOG_DEBUG("iqfile cmd_parser.get  %s\n", iqfile.c_str());
    LOG_DEBUG("g_mode cmd_parser.get  %d\n", g_app_run_mode.load());
    LOG_DEBUG("g_width cmd_parser.get  %d\n", g_width);
    LOG_DEBUG("g_height cmd_parser.get  %d\n", g_height);
    LOG_DEBUG("g_device_id cmd_parser.get  %d\n", g_device_id);

    rkaiq_media = std::make_shared<RKAiqMedia>();
    rkaiq_media->GetMediaInfo();
    rkaiq_media->DumpMediaInfo();

    LOG_DEBUG("================== %d =====================\n", g_app_run_mode.load());
    system("mkdir -p /data/OfflineRAW && sync");

    // return 0;
    if (g_stream_dev_name.length() > 0)
    {
        if (0 > access(g_stream_dev_name.c_str(), R_OK | W_OK))
        {
            LOG_DEBUG("Could not access streaming device\n");
            if (g_rtsp_en_from_cmdarg == 1)
            {
                g_rtsp_en = 0;
            }
        }
        else
        {
            LOG_DEBUG("Access streaming device\n");
            if (g_rtsp_en_from_cmdarg == 1)
            {
                g_rtsp_en = 1;
            }
        }
    }

    // if (g_rtsp_en && g_stream_dev_name.length() > 0)
    // {
    //     ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_STREAMING);
    //     if (ret != 0)
    //     {
    //         LOG_ERROR("Failed set mode to tunning mode, does app started?\n");
    //     }
    // }
    // else
    // {
    //     ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_TUNRING);
    //     if (ret != 0)
    //     {
    //         LOG_ERROR("Failed set mode to tunning mode, does app started?\n");
    //     }
    // }

#ifdef __ANDROID__
    DIR* dir = opendir("/dev/socket");
    struct dirent* dir_ent = NULL;
    std::vector<std::string> domainSocketNodes;
    if (dir)
    {
        while ((dir_ent = readdir(dir)))
        {
            if (dir_ent->d_type == DT_SOCK)
            {
                if (strstr(dir_ent->d_name, "camera_tool") != NULL)
                {
                    domainSocketNodes.push_back(dir_ent->d_name);
                }
            }
        }
        closedir(dir);
    }
    std::sort(domainSocketNodes.begin(), domainSocketNodes.end());
    // for (string socketNode : domainSocketNodes)
    // {
    //     LOG_INFO("socketNode:%s\n", socketNode.c_str());
    // }

    if (domainSocketNodes.size() > 1)
    {
        LOG_INFO("################ Please input camera index to connect\n");
        for (int i = 0; i < domainSocketNodes.size(); i++)
        {
            string tmpStr = domainSocketNodes[i];
            tmpStr = tmpStr.replace(tmpStr.find("camera_tool"), strlen("camera_tool"), "");
            LOG_INFO("camera %d ,please input %s\n", i, tmpStr.c_str());
        }
        LOG_INFO("----\n");
        LOG_INFO("PLEASE INPUT CAMERA INDEX:");

        int camIndexInput = getchar() - '0';
        LOG_INFO("camera index %d:\n", camIndexInput);
        sprintf((char*)g_linuxSocketDomainPath.c_str(), "/dev/socket/camera_tool%d", camIndexInput);
        while (access(g_linuxSocketDomainPath.c_str(), F_OK) == -1)
        {
            camIndexInput = getchar() - '0';
            LOG_INFO("camera index %d:\n", camIndexInput);
            sprintf((char*)g_linuxSocketDomainPath.c_str(), "/dev/socket/camera_tool%d", camIndexInput);
        }
        LOG_INFO("camera socket node %s selected\n", g_linuxSocketDomainPath.c_str());
    }

    if (access("/dev/socket/camera_tool", F_OK) == 0) // Compatible with nodes of older versions
    {
        LOG_INFO("ToolServer using old socket node\n");
        g_linuxSocketDomainPath = "/dev/socket/camera_tool";
    }

    if (g_tcpClient.Setup(g_linuxSocketDomainPath) == false)
    {
        LOG_INFO("#### ToolServer connect AIQ failed ####\n");
    }
    else
    {
        LOG_INFO("#### ToolServer connect AIQ success ####\n");
    }
#else
    DIR* dir = opendir("/tmp");
    struct dirent* dir_ent = NULL;
    std::vector<std::string> domainSocketNodes;
    if (dir)
    {
        while ((dir_ent = readdir(dir)))
        {
            if (dir_ent->d_type == DT_SOCK)
            {
                if (strstr(dir_ent->d_name, "UNIX.domain") != NULL)
                {
                    domainSocketNodes.push_back(dir_ent->d_name);
                }
            }
        }
        closedir(dir);
    }
    std::sort(domainSocketNodes.begin(), domainSocketNodes.end());
    // for (string socketNode : domainSocketNodes)
    // {
    //     LOG_INFO("socketNode:%s\n", socketNode.c_str());
    // }

    if (domainSocketNodes.size() > 1)
    {
        LOG_INFO("################ Please input camera index to connect\n");
        for (int i = 0; i < domainSocketNodes.size(); i++)
        {
            string tmpStr = domainSocketNodes[i];
            tmpStr = tmpStr.replace(tmpStr.find("UNIX.domain"), strlen("UNIX.domain"), "");
            LOG_INFO("camera %d ,please input %s\n", i, tmpStr.c_str());
        }
        LOG_INFO("----\n");
        LOG_INFO("PLEASE INPUT CAMERA INDEX:");

        int camIndexInput = getchar() - '0';
        LOG_INFO("camera index %d:\n", camIndexInput);
        sprintf((char*)g_linuxSocketDomainPath.c_str(), "/tmp/UNIX.domain%d", camIndexInput);
        while (access(g_linuxSocketDomainPath.c_str(), F_OK) == -1)
        {
            camIndexInput = getchar() - '0';
            LOG_INFO("camera index %d:\n", camIndexInput);
            sprintf((char*)g_linuxSocketDomainPath.c_str(), "/tmp/UNIX.domain%d", camIndexInput);
        }
        LOG_INFO("camera socket node %s selected\n", g_linuxSocketDomainPath.c_str());
    }

    if (access("/tmp/UNIX.domain", F_OK) == 0) // Compatible with nodes of older versions
    {
        LOG_INFO("ToolServer using old socket node\n");
        g_linuxSocketDomainPath = "/tmp/UNIX.domain";
    }

    if (g_tcpClient.Setup(g_linuxSocketDomainPath.c_str()) == false)
    {
        LOG_INFO("#### ToolServer connect AIQ failed ####\n");
    }
    else
    {
        LOG_INFO("#### ToolServer connect AIQ success ####\n");
    }
#endif

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    tcpServer = std::make_shared<TCPServer>();
    tcpServer->RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
    tcpServer->Process(SERVER_PORT);
    while (!quit.load() && !tcpServer->Exited())
    {
        pause();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    tcpServer->SaveExit();

    if (g_aiqCred != nullptr)
    {
        delete g_aiqCred;
        g_aiqCred = nullptr;
    }

    // if (g_rtsp_en)
    // {
    //     system("pkill rkaiq_3A_server*");
    //     deinit_rtsp();
    // }

#if 0
  rkaiq_manager.reset();
  rkaiq_manager = nullptr;
#endif
    return 0;
}
