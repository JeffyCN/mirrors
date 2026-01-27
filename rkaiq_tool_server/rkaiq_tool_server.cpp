#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <ctime>
#include <thread>
#include <sys/types.h>
#include <dirent.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdarg.h>
#include <sys/statfs.h>

#include "camera_infohw.h"
#include "domain_tcp_client.h"
#include "rk-camera-module.h"
#if 0
    #include "rkaiq_manager.h"
#endif
#include "rkaiq_protocol.h"
#include "rkaiq_socket.h"
#include "tcp_server.hpp"
#ifdef __ANDROID__
    #include <rtspserver/RtspServer.h>
    #include <cutils/properties.h>
    #include "RtspServer.h"
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

DomainTCPClient g_domainTcpClient;
string g_linuxSocketDomainPath = "/tmp/UNIX.domain0";

std::atomic_bool quit{false};
std::atomic<int> g_app_run_mode(APP_RUN_STATUS_INIT);
int g_width = 1920;
int g_height = 1080;
int g_device_id = 0;
uint32_t g_mmapNumber = 4;
uint32_t g_offlineFrameRate = 10;
int g_sendSpecificFrame = 0;
int g_offlineRawSourceFileNumber = 0;
int g_rtsp_en = 0;
int g_rtsp_en_from_cmdarg = 0;
int g_allow_killapp = 0;
int g_cam_count = 0;
uint32_t g_sensorHdrMode = 0;
std::string g_capture_dev_name = "";
int g_compactModeFlag = 0;
int g_usingCaptureCacheFlag = 0;
std::string g_capture_cache_dir = "/tmp/capture_file_cache";
std::string g_offline_raw_dir = "/data/OfflineRAW";

std::string g_stream_dev_name;
int g_stream_dev_index = -1; // for yuv capture rkisp_iqtool node. RKISP_CMD_SET_IQTOOL_CONN_ID 0:mainpath 1:selfpath
std::string iqfile;
std::string g_sensor_name;
int g_sensorMemoryMode = -1;
int g_sensorSyncMode = -1;

#if 0
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif
std::shared_ptr<RKAiqMedia> rkaiq_media;

void signal_handle(int sig)
{
    quit.store(true, std::memory_order_release);
    RKAiqProtocol::Exit();
    LOG_ERROR("rkaiq_tool_server close\n");
    exit(0);
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

static const char short_options[] = "c:C:s:S:r:i:m:Dd:w:h:n:f:g:v:b:";
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
            case 'g':
                g_sendSpecificFrame = (uint32_t)atoi(optarg);
                if (g_sendSpecificFrame < 0)
                {
                    LOG_INFO("g_sendSpecificFrame < 0, set offline reset index:%u\n", g_sendSpecificFrame);
                    g_sendSpecificFrame = 0;
                }
                else if (g_sendSpecificFrame > 100)
                {
                    LOG_INFO("g_sendSpecificFrame > 100, set offline reset index:%u\n", g_sendSpecificFrame);
                    g_sendSpecificFrame = 100;
                }
                LOG_INFO("set offline reset index:%u\n", g_sendSpecificFrame);
                break;
            case 'C':
                g_capture_dev_name = optarg;
                LOG_INFO("capture image using no compact mode. capture dev name:%s\n", g_capture_dev_name.c_str());
                break;
            case 'c':
                g_capture_dev_name = optarg;
                g_compactModeFlag = 1;
                LOG_INFO("capture image using compact mode. capture dev name:%s\n", g_capture_dev_name.c_str());
                break;
            case 'v':
                g_usingCaptureCacheFlag = 1;
                g_capture_cache_dir = optarg;
                LOG_INFO("set capture cache dir: %s ,use capture cache.\n", g_capture_cache_dir.c_str());
                break;
            case 'b':
                g_offline_raw_dir = optarg;
                LOG_INFO("set offline raw dir: %s\n", g_offline_raw_dir.c_str());
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

static unsigned int g_CRC32_table[256];
void InitCRC32Table()
{
    for (int i = 0; i != 256; i++)
    {
        unsigned int CRC = i;
        for (int j = 0; j != 8; j++)
        {
            if (CRC & 1)
                CRC = (CRC >> 1) ^ 0xEDB88320; // norm:0x04C11DB7  reversed:0xEDB88320  reciprocal:0xDB710641
                                               // reciprocal_reversed:0x82608EDB
            else
                CRC >>= 1;
        }
        g_CRC32_table[i] = CRC;
    }
}
unsigned int GetCRC32(unsigned char* buf, unsigned int len)
{
    unsigned int CRC32_data = 0xFFFFFFFF;
    for (unsigned int i = 0; i != len; ++i)
    {
        unsigned int t = (CRC32_data ^ buf[i]) & 0xFF;
        CRC32_data = ((CRC32_data >> 8) & 0xFFFFFF) ^ g_CRC32_table[t];
    }
    return ~CRC32_data;
}

void PrintLocalIP()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("%s IP: %s\n", ifa->ifa_name, host);
            // break;
        }
    }

    freeifaddrs(ifaddr);
}

static std::string string_format(const std::string fmt_str, ...)
{
    int final_n, n = ((int)fmt_str.size()) * 2;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1)
    {
        formatted.reset(new char[n]);
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

int main(int argc, char** argv)
{
    int ret = -1;
    LOG_ERROR("#### 20240429_153335 ####\n");
    struct sigaction sa;
    sa.sa_handler = signal_handle;
    assert(sigaction(SIGPIPE, NULL, NULL) != -1);
    assert(sigaction(SIGINT, &sa, NULL) != -1);
    assert(sigaction(SIGQUIT, &sa, NULL) != -1);
    assert(sigaction(SIGTERM, &sa, NULL) != -1);

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

    // Check if use capture cache
    LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
    struct statfs diskInfo;
    statfs(g_capture_cache_dir.c_str(), &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
    if (availableDisk < 1024 * 1024 * 200)
    {
        LOG_ERROR("disk space < 200MB, not use capture cache.\n");
    }
    else
    {
        LOG_ERROR("disk space >= 200MB, use capture cache.\n");
        g_usingCaptureCacheFlag = 1;
    }

    string tmpCmd = string_format("rm -rf %s/* && sync", g_capture_cache_dir.c_str());
    system(tmpCmd.c_str());
    mkdir(g_offline_raw_dir.c_str(), 0777);
    mkdir(g_capture_cache_dir.c_str(), 0777);
    sync();

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
        g_linuxSocketDomainPath.clear();
        g_linuxSocketDomainPath = g_linuxSocketDomainPath.append("/dev/socket/camera_tool").append(std::to_string(camIndexInput));
        while (access(g_linuxSocketDomainPath.c_str(), F_OK) == -1)
        {
            camIndexInput = getchar() - '0';
            LOG_INFO("camera index %d:\n", camIndexInput);
            g_linuxSocketDomainPath.clear();
            g_linuxSocketDomainPath = g_linuxSocketDomainPath.append("/dev/socket/camera_tool").append(std::to_string(camIndexInput));
        }
        LOG_INFO("camera socket node %s selected\n", g_linuxSocketDomainPath.c_str());
    }
    else
    {
        if (access("/dev/socket/camera_tool", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /dev/socket/camera_tool\n");
            g_linuxSocketDomainPath = "/dev/socket/camera_tool";
        }
        else if (access("/dev/socket/camera_tool0", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /dev/socket/camera_tool0\n");
            g_linuxSocketDomainPath = "/dev/socket/camera_tool0";
        }
        else if (access("/dev/socket/camera_tool1", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /dev/socket/camera_tool1\n");
            g_linuxSocketDomainPath = "/dev/socket/camera_tool1";
        }
    }

    if (g_domainTcpClient.Setup(g_linuxSocketDomainPath) == false)
    {
        LOG_INFO("#### ToolServer connect AIQ failed, path=%s ####\n", g_linuxSocketDomainPath.c_str());
    }
    else
    {
        LOG_INFO("#### ToolServer connect AIQ success, path=%s ####\n", g_linuxSocketDomainPath.c_str());
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
        g_linuxSocketDomainPath.clear();
        g_linuxSocketDomainPath = g_linuxSocketDomainPath.append("/tmp/UNIX.domain").append(std::to_string(camIndexInput));
        while (access(g_linuxSocketDomainPath.c_str(), F_OK) == -1)
        {
            camIndexInput = getchar() - '0';
            LOG_INFO("camera index %d:\n", camIndexInput);
            g_linuxSocketDomainPath.clear();
            g_linuxSocketDomainPath = g_linuxSocketDomainPath.append("/tmp/UNIX.domain").append(std::to_string(camIndexInput));
        }
        LOG_INFO("camera socket node %s selected\n", g_linuxSocketDomainPath.c_str());
    }
    else
    {
        if (access("/tmp/UNIX.domain", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /tmp/UNIX.domain\n");
            g_linuxSocketDomainPath = "/tmp/UNIX.domain";
        }
        else if (access("/tmp/UNIX.domain0", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /tmp/UNIX.domain0\n");
            g_linuxSocketDomainPath = "/tmp/UNIX.domain0";
        }
        else if (access("/tmp/UNIX.domain1", F_OK) == 0) // Compatible with nodes of older versions
        {
            LOG_INFO("ToolServer using socket node /tmp/UNIX.domain1\n");
            g_linuxSocketDomainPath = "/tmp/UNIX.domain1";
        }
    }

    if (g_domainTcpClient.Setup(g_linuxSocketDomainPath.c_str()) == false)
    {
        LOG_INFO("#### ToolServer connect AIQ failed, path=%s ####\n", g_linuxSocketDomainPath.c_str());
    }
    else
    {
        LOG_INFO("#### ToolServer connect AIQ success, path=%s ####\n", g_linuxSocketDomainPath.c_str());
    }
#endif

    InitCRC32Table();
    PrintLocalIP();

    NewTCPServer<> newTcpServer;

    newTcpServer.onNewConnection = [&](TCPSocket<>* newClient) {
        cout << "New client: [" << newClient->remoteAddress() << ":" << newClient->remotePort() << "]" << endl;
        newClient->onRawMessageReceived = [newClient](const char* message, int length) {
            RKAiqProtocol::HandlerTCPMessage(newClient->sock, (char*)message, length);
        };

        newClient->onSocketClosed = [newClient](int errorCode) {
            cout << "Socket closed:" << newClient->remoteAddress() << ":" << newClient->remotePort() << " -> " << errorCode << endl;
            cout << flush;
        };
    };

    newTcpServer.Bind(5543, [](int errorCode, string errorMessage) {
        cout << errorCode << " : " << errorMessage << endl;
    });

    newTcpServer.Listen([](int errorCode, string errorMessage) {
        cout << errorCode << " : " << errorMessage << endl;
    });

    while (1)
    {
        if (quit.load(std::memory_order_acquire))
        {
            break;
        }
        usleep(1000 * 1000);
    }

    newTcpServer.Close();

    // if (g_rtsp_en)
    // {
    //     system("pkill rkaiq_3A_server*");
    //     deinit_rtsp();
    // }

    return 0;
}
