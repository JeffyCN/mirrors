#include "rkaiq_online_protocol.h"
#include "rk-camera-module.h"
#include "rkaiq_protocol.h"
#include "tcp_client.h"
#include "rkaiq_media.h"
#include <regex>
#include <vector>
#include <mutex>
#include "rkaiq_cmdid.h"
#include "rkaiq_socket.h"
#include "rk_aiq_types.h"
#include <thread>
#include "domain_tcp_client.h"
#include <condition_variable>
#include <rkisp2-config.h>
#include <stdarg.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <future>
#include <algorithm>
#include <cstdio>
#include <sys/statfs.h>
#include <string>

using namespace std;

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_width;
extern int g_height;
extern std::string g_stream_dev_name;
extern int g_stream_dev_index;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern int g_device_id;
extern uint32_t g_sensorHdrMode;
extern int g_sensorMemoryMode;
extern int g_sensorSyncMode;
extern int g_usingCaptureCacheFlag; //配置是否使用本地缓存文件
extern std::string g_capture_dev_name;
extern int g_compactModeFlag;
extern std::string g_capture_cache_dir;
extern uint g_lastCapturedSequense;
extern DomainTCPClient g_domainTcpClient;
extern uint32_t g_mmapNumber;

extern int g_offlineRAWCaptureYUVStepCounter;
extern int g_startOfflineRawFlag;

extern int g_sendSpecificFrame;
extern int g_offlineRawSourceFileNumber;

extern int g_inCaptureYUVProcess;
extern std::mutex g_offlineRawEnqueuedMutex;
extern std::unique_lock<std::mutex> g_offlineRawEnqueuedLock;
extern std::condition_variable g_offlineRawEnqueued;
extern std::mutex g_yuvCapturedMutex;
extern std::unique_lock<std::mutex> g_yuvCapturedLock;
extern std::condition_variable g_yuvCaptured;
extern int ConnectAiq();

static uint16_t capture_check_sum;
static int capture_status = READY;
static int capture_frames = 1;
static int capture_frames_index = 0;
static char captureDevNode[128] = {0};

static struct capture_info cap_info;

static struct capture_info cap_info_hdr2_1;
static struct capture_info cap_info_hdr2_2;

static struct capture_info cap_info_hdr3_1;
static struct capture_info cap_info_hdr3_2;
static struct capture_info cap_info_hdr3_3;

static std::timed_mutex get3AStatsMtx;
extern unsigned int GetCRC32(unsigned char* buf, unsigned int len);

#define FMT_NUM_PLANES 1
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#pragma pack(1)
typedef struct RkAiqSocketData
{
    char magic[2] = {'R', 'K'};
    unsigned char packetSize[4];
    int commandID;
    int commandResult;
    unsigned int dataSize;
    char* data;
    unsigned int dataHash;
} RkAiqSocketData;

typedef struct RkAiqSocketDataV2_Write
{
    uint8_t magic[4]; // = { 'R', 0xAA, 0xFF, 'K' };
    int32_t cmd_id;
    int32_t cmd_ret;
    uint32_t sequence;
    uint32_t payload_size;
    uint32_t packet_size;
    uint8_t* data;
    uint32_t data_hash; // different offset in data stream
} RkAiqSocketDataV2_Write;

typedef struct RkAiqSocketDataV2_Receive
{
    uint8_t magic[4]; // = { 'R', 0xAA, 0xFF, 'K' };
    int32_t cmd_id;
    int32_t cmd_ret;
    uint32_t sequence;
    uint32_t payload_size;
    uint32_t packet_size;
    uint8_t note[128];
    uint8_t* data;
    uint32_t data_hash; // different offset in data stream
} RkAiqSocketDataV2_Receive;

typedef struct JsonWriteAnswer
{
    uint16_t writeErrorCode;
    uint16_t aiqErrorCode;
    uint32_t aiqErrorDataLen;
    uint32_t aiqErrorDataCrc;
} JsonWriteAnswer;
#pragma pack()

struct YuvCaptureBuffer
{
    void* start;
    size_t length;
};

static std::string GetFirstDirectory(const std::string& path)
{
    size_t pos = path.find('/', 1);
    if (pos != std::string::npos)
    {
        return path.substr(0, pos);
    }
    else
    {
        return path;
    }
}

static int strcmp_natural(const char* a, const char* b)
{
    if (!a || !b)
        return a ? 1 : b ? -1 : 0;

    if (isdigit(*a) && isdigit(*b))
    {
        char* remainderA;
        char* remainderB;
        long valA = strtol(a, &remainderA, 10);
        long valB = strtol(b, &remainderB, 10);
        if (valA != valB)
        {
            return valA - valB;
        }
        else
        {
            std::ptrdiff_t lengthA = remainderA - a;
            std::ptrdiff_t lengthB = remainderB - b;
            if (lengthA != lengthB)
                return lengthA - lengthB;
            else
                return strcmp_natural(remainderA, remainderB);
        }
    }

    if (isdigit(*a) || isdigit(*b))
        return isdigit(*a) ? -1 : 1;

    while (*a && *b)
    {
        if (isdigit(*a) || isdigit(*b))
            return strcmp_natural(a, b);
        if (*a != *b)
            return *a - *b;
        a++;
        b++;
    }
    return *a ? 1 : *b ? -1 : 0;
}

static bool natural_less(const string& lhs, const string& rhs)
{
    return strcmp_natural(lhs.c_str(), rhs.c_str()) < 0;
}

static bool natural_more(const string& lhs, const string& rhs)
{
    return strcmp_natural(lhs.c_str(), rhs.c_str()) > 0;
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

static string GetTime()
{
    string timeString;
    struct tm* tm_t;
    struct timeval time;
    gettimeofday(&time, NULL);
    tm_t = localtime(&time.tv_sec);
    if (NULL != tm_t)
    {
        timeString = string_format("%04d-%02d-%02d %02d:%02d:%02d.%03ld", tm_t->tm_year + 1900, tm_t->tm_mon + 1, tm_t->tm_mday, tm_t->tm_hour, tm_t->tm_min, tm_t->tm_sec, time.tv_usec / 1000);
    }

    return timeString;
}

static void HexDump(const unsigned char* data, size_t size)
{
    printf("####\n");
    int i;
    size_t offset = 0;
    while (offset < size)
    {
        printf("%04x  ", offset);
        for (i = 0; i < 16; i++)
        {
            if (i % 8 == 0)
            {
                putchar(' ');
            }
            if (offset + i < size)
            {
                printf("%02x ", data[offset + i]);
            }
            else
            {
                printf("   ");
            }
        }
        printf("   ");
        for (i = 0; i < 16 && offset + i < size; i++)
        {
            if (isprint(data[offset + i]))
            {
                printf("%c", data[offset + i]);
            }
            else
            {
                putchar('.');
            }
        }
        putchar('\n');
        offset += 16;
    }
    printf("####\n\n");
}

static void write_to_file(std::string filename, void* buffer, std::size_t size)
{
    std::ofstream of(filename, std::ios::binary);
    of.write(static_cast<char*>(buffer), size);
}

static void SendMessageToPC(int sockfd, char* data, unsigned long long dataSize = 0)
{
    if (dataSize == 0)
    {
        dataSize = strlen(data);
    }
    unsigned long long packetSize = strlen("#&#^ToolServerMsg#&#^") + strlen("#&#^@`#`@`#`") + dataSize;
    unsigned long long offSet = 0;
    char* dataToSend = (char*)malloc(packetSize);
    memcpy(dataToSend + offSet, "#&#^ToolServerMsg#&#^", strlen("#&#^ToolServerMsg#&#^"));
    offSet += strlen("#&#^ToolServerMsg#&#^");
    memcpy(dataToSend + offSet, data, dataSize);
    offSet += dataSize;
    memcpy(dataToSend + offSet, "#&#^@`#`@`#`", strlen("#&#^@`#`@`#`"));
    offSet += strlen("#&#^@`#`@`#`");

    send(sockfd, dataToSend, packetSize, 0);
    free(dataToSend);
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void DoAnswer2(int sockfd, CommandData_t* cmd, int cmd_id, uint16_t check_sum, uint32_t result)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = result;
    cmd->dat[1] = check_sum & 0xFF;
    cmd->dat[2] = (check_sum >> 8) & 0xFF;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void DoAiqJsonWriteAnswer(int sockfd, CommandData_t* cmd, int cmd_id, JsonWriteAnswer answer)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_JSON_WRITE;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = sizeof(JsonWriteAnswer);
    memset(cmd->dat, 0, cmd->datLen);
    memcpy(cmd->dat, &answer, cmd->datLen);

    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void DoAiqJsonReadAnswer1(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_JSON_READ;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void DoAiqJsonReadAnswer2(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_JSON_READ;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void DoAiqJsonReadAnswer3(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_JSON_READ;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static int DoCheckSum(int sockfd, uint16_t check_sum)
{
    char recv_data[MAXPACKETSIZE];
    size_t recv_size = 0;
    int param_size = sizeof(CommandData_t);
    int remain_size = param_size;
    int try_count = 3;
    while (remain_size > 0)
    {
        int offset = param_size - remain_size;
        recv_size = recv(sockfd, recv_data + offset, remain_size, 0);
        if (recv_size < 0)
        {
            if (errno == EAGAIN)
            {
                LOG_DEBUG("recv size %zu, do try again, count %d\n", recv_size, try_count);
                try_count--;
                if (try_count < 0)
                {
                    break;
                }
                continue;
            }
            else
            {
                // SendMessageToPC(sockfd, "Error socket recv failed");
                LOG_ERROR("Error socket recv failed %d\n", errno);
            }
        }
        remain_size = remain_size - recv_size;
    }
    LOG_DEBUG("recv_size: 0x%zx expect 0x%x\n", recv_size, sizeof(CommandData_t));

    CommandData_t* cmd = (CommandData_t*)recv_data;
    uint16_t recv_check_sum = 0;
    recv_check_sum += cmd->dat[0] & 0xff;
    recv_check_sum += (cmd->dat[1] & 0xff) << 8;
    LOG_DEBUG("check_sum local: 0x%x pc: 0x%x\n", check_sum, recv_check_sum);

    if (check_sum != recv_check_sum)
    {
        LOG_DEBUG("check_sum fail!\n");
        return -1;
    }
    return 0;
}

static void OnLineSet(int sockfd, CommandData_t* cmd, uint16_t& check_sum, uint32_t& result)
{
    int recv_size = 0;
    int param_size = *(int*)cmd->dat;
    int remain_size = param_size;

    LOG_DEBUG("expect recv param_size 0x%x\n", param_size);
    char* param = (char*)malloc(param_size);
    while (remain_size > 0)
    {
        int offset = param_size - remain_size;
        recv_size = recv(sockfd, param + offset, remain_size, 0);
        remain_size = remain_size - recv_size;
    }

    LOG_DEBUG("recv ready\n");

    for (int i = 0; i < param_size; i++)
    {
        check_sum += param[i];
    }

    LOG_DEBUG("DO Sycn Setting, CmdId: 0x%x, expect ParamSize %d\n", cmd->cmdID, param_size);
#if 0
    if (rkaiq_manager)
    {
        result = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);
    }
#endif
    if (param != NULL)
    {
        free(param);
    }
}

static int OnLineGet(int sockfd, CommandData_t* cmd)
{
    int ret = 0;
    int ioRet = 0;
    int send_size = 0;
    int param_size = *(int*)cmd->dat;
    int remain_size = param_size;
    LOG_DEBUG("ParamSize: 0x%x\n", param_size);

    uint8_t* param = (uint8_t*)malloc(param_size);

    LOG_INFO("DO Get Setting, CmdId: 0x%x, expect ParamSize %d\n", cmd->cmdID, param_size);
#if 0
  if (rkaiq_manager) {
    ioRet = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);
    if (ioRet != 0) {
      LOG_ERROR("DO Get Setting, io get data failed. return\n");
      return 1;
    }
  }
#endif

    while (remain_size > 0)
    {
        int offset = param_size - remain_size;
        send_size = send(sockfd, param + offset, remain_size, 0);
        remain_size = param_size - send_size;
    }

    uint16_t check_sum = 0;
    for (int i = 0; i < param_size; i++)
    {
        check_sum += param[i];
    }

    ret = DoCheckSum(sockfd, check_sum);

    if (param != NULL)
    {
        free(param);
        param = NULL;
    }
    return ret;
}

static void SendYuvData(int socket, int index, void* buffer, int size)
{
    char* buf = NULL;
    int total = size;
    int packet_len = MAXPACKETSIZE;
    int send_size = 0;
    int ret_val;
    uint16_t check_sum = 0;

    // buf = (char*)buffer;
    // for (int i = 0; i < size; i++)
    // {
    //     check_sum += buf[i];
    // }
    // capture_check_sum = check_sum;

    buf = (char*)buffer;
    while (total > 0)
    {
        if (total < packet_len)
        {
            send_size = total;
        }
        else
        {
            send_size = packet_len;
        }
        ret_val = send(socket, buf, send_size, 0);
        total -= send_size;
        buf += ret_val;
    }
}

static void SendYuvDataResult(int sockfd, CommandData_t* cmd, CommandData_t* recv_cmd)
{
    unsigned short* checksum;
    char send_data[MAXPACKETSIZE];
    checksum = (unsigned short*)&recv_cmd->dat[1];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x04;
    if (capture_check_sum == *checksum)
    {
        cmd->dat[1] = RES_SUCCESS;
    }
    else
    {
        cmd->dat[1] = RES_FAILED;
    }
    cmd->checkSum = 0;
    capture_check_sum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void SendOnlineRawDataResult(int sockfd, CommandData_t* cmd, CommandData_t* recv_cmd)
{
    unsigned short* checksum;
    char send_data[MAXPACKETSIZE];
    checksum = (unsigned short*)&recv_cmd->dat[1];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_ONLINE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x04;
    if (capture_check_sum == *checksum)
    {
        cmd->dat[1] = RES_SUCCESS;
    }
    else
    {
        cmd->dat[1] = RES_FAILED;
    }
    cmd->checkSum = 0;
    capture_check_sum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

#ifndef __ANDROID__
// extern std::shared_ptr<easymedia::Flow> video_dump_flow;
#endif
static const uint32_t kSocket_fd = (1 << 0);
static const uint32_t kEnable_Link = (1 << 1);

void LinkCaptureCallBack(unsigned char* buffer, unsigned int buffer_size, int sockfd, uint32_t sequence)
{
    LOG_DEBUG("sockfd %d buffer %p, size %d, sequence:%u\n", sockfd, buffer, buffer_size, sequence);
    SendYuvData(sockfd, capture_frames_index++, buffer, buffer_size);
}

long GetFileSize(char* filename)
{
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

static void ExecuteCMD(const char* cmd, char* result)
{
    char buf_ps[1024];
    char ps[1024] = {0};
    FILE* ptr;
    strcpy(ps, cmd);
    strcat(ps, " 2>&1"); // Redirect stderr to stdout
    if ((ptr = popen(ps, "r")) != NULL)
    {
        while (fgets(buf_ps, 1024, ptr) != NULL)
        {
            strcat(result, buf_ps);
            if (strlen(result) > 1024)
            {
                break;
            }
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
}

static int DoCaptureYuv(int sockfd)
{
    LOG_DEBUG("DoCaptureYuv begin\n");
    g_inCaptureYUVProcess = 1;
    if (g_startOfflineRawFlag == 1)
    {
        if (g_offlineRAWCaptureYUVStepCounter != 0)
        {
            g_offlineRAWCaptureYUVStepCounter = 0;
            usleep(1000 * 200);
        }
        else
        {
            printf("#### offlineRawEnqueue wait begin ####\n");
            g_offlineRawEnqueued.wait(g_offlineRawEnqueuedLock);
            printf("#### offlineRawEnqueue wait end ####\n");
        }
    }

    if (capture_frames_index == 0)
    {
        char tmpPath[100] = {0};
        getcwd(tmpPath, 100);
        string currentDir = tmpPath;
        string tmpCmd = string_format("rm -rf %s/* && sync", g_capture_cache_dir.c_str());
        // LOG_DEBUG("#### exec:%s\n", tmpCmd.c_str());
        system(tmpCmd.c_str());
    }
    bool videoDevNodeFindedFlag = false;
    char cmdResStr[128] = {0};
    memset(captureDevNode, 0, sizeof(captureDevNode));
    if (g_stream_dev_name.length() > 0)
    {
        videoDevNodeFindedFlag = true;
        memcpy(captureDevNode, g_stream_dev_name.c_str(), g_stream_dev_name.length());
        LOG_INFO("DoCaptureYuv,using specific dev node:%s\n", captureDevNode);
    }
    else
    {
        LOG_INFO("DoCaptureYuv,using rkisp_iqtool node.\n");
        if (videoDevNodeFindedFlag == false)
        {
            memset(cmdResStr, 0, sizeof(cmdResStr));
            ExecuteCMD("media-ctl -d /dev/media0 -e rkisp_iqtool", cmdResStr);
            if (strstr(cmdResStr, "/dev/video") != NULL)
            {
                videoDevNodeFindedFlag = true;
                memcpy(captureDevNode, cmdResStr, sizeof(cmdResStr));
            }
        }
        if (videoDevNodeFindedFlag == false)
        {
            memset(cmdResStr, 0, sizeof(cmdResStr));
            ExecuteCMD("media-ctl -d /dev/media1 -e rkisp_iqtool", cmdResStr);
            if (strstr(cmdResStr, "/dev/video") != NULL)
            {
                videoDevNodeFindedFlag = true;
                memcpy(captureDevNode, cmdResStr, sizeof(cmdResStr));
            }
        }
        if (videoDevNodeFindedFlag == false)
        {
            memset(cmdResStr, 0, sizeof(cmdResStr));
            ExecuteCMD("media-ctl -d /dev/media2 -e rkisp_iqtool", cmdResStr);
            if (strstr(cmdResStr, "/dev/video") != NULL)
            {
                videoDevNodeFindedFlag = true;
                memcpy(captureDevNode, cmdResStr, sizeof(cmdResStr));
            }
        }
    }

    if (videoDevNodeFindedFlag == false)
    {
        // SendMessageToPC(sockfd, "Video capture device node not found");
        LOG_ERROR("Video capture device node not found.\n");
        return 1;
    }
    else
    {
        captureDevNode[strcspn(captureDevNode, "\n")] = '\0';
        LOG_DEBUG("Video capture device node:%s\n", captureDevNode);
    }

    //
    struct v4l2_capability capCapability;
    struct v4l2_format capFmt;

    int fd = open(captureDevNode, O_RDWR, 0);
    if (-1 == fd)
    {
        // SendMessageToPC(sockfd, "Open captureDevNode failed");
        LOG_ERROR("Cannot open '%s': %d, %s\n", captureDevNode, errno, strerror(errno));
        return -1;
    }

#ifdef RKISP_CMD_SET_IQTOOL_CONN_ID
    if (g_stream_dev_index == 0)
    {
        int tmpVal = 0;
        if (xioctl(fd, RKISP_CMD_SET_IQTOOL_CONN_ID, &tmpVal) < 0) // 0:mp 1:sp
        {
            // SendMessageToPC(sockfd, "VIDIOC_QUERYCAP failed");
            LOG_ERROR("Failed to ioctl(RKISP_CMD_SET_IQTOOL_CONN_ID)\n");
            return -1;
        }
    }
    else if (g_stream_dev_index == 1)
    {
        int tmpVal = 1;
        if (xioctl(fd, RKISP_CMD_SET_IQTOOL_CONN_ID, &tmpVal) < 0) // 0:mp 1:sp
        {
            // SendMessageToPC(sockfd, "VIDIOC_QUERYCAP failed");
            LOG_ERROR("Failed to ioctl(RKISP_CMD_SET_IQTOOL_CONN_ID)\n");
            return -1;
        }
    }
#endif
    //
    CLEAR(capCapability);
    if (xioctl(fd, VIDIOC_QUERYCAP, &capCapability) < 0)
    {
        // SendMessageToPC(sockfd, "VIDIOC_QUERYCAP failed");
        LOG_ERROR("Failed to ioctl(VIDIOC_QUERYCAP)\n");
        return -1;
    }
    // HexDump((unsigned char*)&capCapability, sizeof(v4l2_capability));
    // LOG_DEBUG("capCapability.capabilities:%u\n", capCapability.capabilities);
    // if (!((capCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE))
    // {
    //     LOG_ERROR("%s, Not a video capture device.\n", captureDevNode);
    //     return -1;
    // }
    // if (!((capCapability.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING))
    // {
    //     LOG_ERROR("%s does not support the streaming I/O method.\n", captureDevNode);
    //     return -1;
    // }

    CLEAR(capFmt);
    capFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (capCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        capFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    capFmt.fmt.pix.width = g_width;
    capFmt.fmt.pix.height = g_height;
    capFmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    if (xioctl(fd, VIDIOC_S_FMT, &capFmt) < 0)
    {
        // SendMessageToPC(sockfd, "VIDIOC_S_FMT failed");
        LOG_ERROR("Failed to ioctl(VIDIOC_S_FMT)\n");
        return -1;
    }
    if (xioctl(fd, VIDIOC_G_FMT, &capFmt) < 0)
    {
        // SendMessageToPC(sockfd, "VIDIOC_G_FMT failed");
        LOG_ERROR("Failed to ioctl(VIDIOC_G_FMT)\n");
        return -1;
    }

    //
    uint32_t file_length = capFmt.fmt.pix.width * capFmt.fmt.pix.height * 1.5; // NV12:x1.5
    LOG_DEBUG("file_length:%u\n", file_length);

    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = g_mmapNumber; // memory map buffer count
    req.type = (v4l2_buf_type)capFmt.type;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0)
    {
        // SendMessageToPC(sockfd, "VIDIOC_REQBUFS failed");
        LOG_ERROR("Failed to ioctl(VIDIOC_REQBUFS)\n");
        return -1;
    }
    if (req.count < 2)
    {
        // SendMessageToPC(sockfd, "Insufficient buffer memory");
        LOG_ERROR("Insufficient buffer memory\n");
        return -1;
    }
    else
    {
        LOG_INFO("YUV capture using %u buffers\n", req.count);
    }
    //

    struct YuvCaptureBuffer* buffers = (YuvCaptureBuffer*)calloc(req.count, sizeof(*buffers));
    unsigned int n_buffers = 0;
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);
        buf.type = (v4l2_buf_type)capFmt.type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == capFmt.type)
        {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            // SendMessageToPC(sockfd, "VIDIOC_QUERYBUF failed");
            LOG_ERROR("VIDIOC_QUERYBUF error\n");
        }
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == capFmt.type)
        {
            buffers[n_buffers].length = buf.m.planes[0].length;
            buffers[n_buffers].start = mmap(NULL, buf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.planes[0].m.mem_offset);
        }
        else
        {
            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        }

        if (MAP_FAILED == buffers[n_buffers].start)
        {
            // SendMessageToPC(sockfd, "memory map failed");
            LOG_ERROR("memory map failed\n");
        }
    }

    for (uint i = 0; i < n_buffers; ++i)
    {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);
        buf.type = (v4l2_buf_type)capFmt.type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == (v4l2_buf_type)capFmt.type)
        {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        {
            // SendMessageToPC(sockfd, "VIDIOC_QBUF failed");
            LOG_ERROR("VIDIOC_QBUF failed\n");
        }
    }

    if (-1 == xioctl(fd, VIDIOC_STREAMON, &capFmt.type))
    {
        // SendMessageToPC(sockfd, "VIDIOC_STREAMON failed");
        LOG_ERROR("VIDIOC_STREAMON failed\n");
        return -1;
    }

    std::vector<std::thread> threads;
    if (g_usingCaptureCacheFlag == 1)
    {
        threads.push_back(std::thread([=]() {
            LOG_DEBUG("DoCapture send yuv file process. start\n");
            LOG_DEBUG("g_startOfflineRawFlag:%d\n", g_startOfflineRawFlag);
            LOG_DEBUG("g_offlineRAWCaptureYUVStepCounter:%d\n", g_offlineRAWCaptureYUVStepCounter);
            while (capture_frames_index < capture_frames)
            {
            START_SEND_YUV:
                printf("capture_frames_index/capture_frames:%d/%d\n", capture_frames_index, capture_frames);
                if (capture_frames_index >= capture_frames)
                {
                    break;
                }
                usleep(1000 * 500);
                if (g_startOfflineRawFlag == 1 && g_offlineRAWCaptureYUVStepCounter == 2)
                {
                    printf("############## send yuv wait.\n");
                    usleep(1000 * 500);
                }
                static std::vector<std::string> yuv_files;
                DIR* dir = opendir(g_capture_cache_dir.c_str());
                struct dirent* dir_ent = NULL;
                if (dir)
                {
                    yuv_files.clear();
                    while ((dir_ent = readdir(dir)))
                    {
                        if (dir_ent->d_type == DT_REG)
                        {
                            // is yuv file
                            if (strstr(dir_ent->d_name, ".yuv"))
                            {
                                yuv_files.push_back(dir_ent->d_name);
                            }
                        }
                    }
                    closedir(dir);
                }
                if (yuv_files.size() == 0)
                {
                    LOG_INFO("No yuv files in %s\n", g_capture_cache_dir.c_str());
                    goto START_SEND_YUV;
                }
                while (yuv_files.size() > 0)
                {
                    // for (auto tmp : yuv_files)
                    // {
                    //     LOG_DEBUG("yuv_file:%s\n", tmp.c_str());
                    // }
                    char tmpPath[100] = {0};
                    getcwd(tmpPath, 100);
                    string tmpDir = tmpPath;

                    std::sort(yuv_files.begin(), yuv_files.end(), natural_more);
                    string tmpyuvFile = yuv_files.back();
                    while (tmpyuvFile != "")
                    {
                        string fileFullpath = string_format("%s/%s", g_capture_cache_dir.c_str(), tmpyuvFile.c_str());
                        LOG_DEBUG("yuv CAPTURED:%s\n", fileFullpath.c_str());

                        FILE* f = fopen(fileFullpath.c_str(), "rb");
                        fseek(f, 0, SEEK_END);
                        long fsize = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        LOG_DEBUG("yuv CAPTURED size:%ld\n", fsize);
                        if (fsize > 0)
                        {
                            char* yuvData = (char*)malloc(fsize);
                            fread(yuvData, fsize, 1, f);

                            LOG_DEBUG("yuv CAPTURED SendyuvData begin\n");
                            SendYuvData(sockfd, 0, yuvData, fsize);
                            LOG_DEBUG("yuv CAPTURED SendyuvData end\n");
                            free(yuvData);
                            yuvData = NULL;
                            string tmpCmd = string_format("rm -f %s", fileFullpath.c_str());
                            // LOG_DEBUG("#### exec:%s\n", tmpCmd.c_str());
                            system(tmpCmd.c_str());
                            yuv_files.pop_back();
                            usleep(1000 * 100);

                            if (yuv_files.size() == 0)
                            {
                                DIR* dir = opendir(g_capture_cache_dir.c_str());
                                struct dirent* dir_ent = NULL;
                                if (dir)
                                {
                                    yuv_files.clear();
                                    while ((dir_ent = readdir(dir)))
                                    {
                                        if (dir_ent->d_type == DT_REG)
                                        {
                                            // is yuv file
                                            if (strstr(dir_ent->d_name, ".yuv"))
                                            {
                                                yuv_files.push_back(dir_ent->d_name);
                                            }
                                        }
                                    }
                                    closedir(dir);
                                    if (yuv_files.size() > 0)
                                    {
                                        std::sort(yuv_files.begin(), yuv_files.end(), natural_more);
                                    }
                                }
                            }
                            if (yuv_files.size() > 0)
                            {
                                tmpyuvFile = yuv_files.back();
                            }
                            else
                            {
                                tmpyuvFile = "";
                            }
                        }
                        else
                        {
                            LOG_DEBUG("yuv CAPTURED size < 0\n");
                        }
                        fclose(f);
                    }
                }
            }
            LOG_DEBUG("DoCapture send yuv file process. end\n");
        }));
    }

    threads.push_back(std::thread([=]() {
        LOG_INFO("YUV capture process begin\n");
        static uint32_t virtual_sequence = 0;
        while (1)
        {
            fd_set fds;
            struct timeval tv;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            // 2 sec Timeout
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            int ret = select(fd + 1, &fds, NULL, NULL, &tv);
            if (-1 == ret)
            {
                if (EINTR == errno)
                    continue;
                // SendMessageToPC(sockfd, "select dev failed");
                LOG_ERROR("select failed\n");
            }
            else if (0 == ret)
            {
                fprintf(stderr, "select timeout\n");
                // SendMessageToPC(sockfd, "select dev timeout");
                LOG_ERROR("select timeout\n");
                // g_inCaptureYUVProcess = 0;
                continue;
                // exit(EXIT_FAILURE);
            }

            //
            if (g_usingCaptureCacheFlag == 1 && 0 != access(g_capture_cache_dir.c_str(), 0))
            {
                if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                {
                    LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                    return;
                }
            }

            virtual_sequence = 0;
            while (capture_frames_index < capture_frames)
            {
                struct v4l2_buffer buf;
                struct v4l2_plane planes[FMT_NUM_PLANES];
                CLEAR(buf);
                CLEAR(planes);
                buf.type = (v4l2_buf_type)capFmt.type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = capture_frames_index;
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == (v4l2_buf_type)capFmt.type)
                {
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
                {
                    // SendMessageToPC(sockfd, "VIDIOC_DQBUF failed");
                    LOG_ERROR("VIDIOC_DQBUF failed\n");
                }
                assert(buf.index < n_buffers);
                // LOG_INFO("YUV capture, sequence:%u\n", buf.sequence);
                // LOG_INFO("g_sendSpecificFrame:%u\n", g_sendSpecificFrame);
                // LOG_INFO("virtual_sequence:%u\n", virtual_sequence);
                // LOG_INFO("capture_frames_index:%u\n", capture_frames_index);

                if (g_sendSpecificFrame == 0 || (g_sendSpecificFrame != 0 && virtual_sequence >= g_sendSpecificFrame))
                {
                    if (g_usingCaptureCacheFlag == 1)
                    {
                        // Check available disk space
                        // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
                        LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
                        struct statfs diskInfo;
                        statfs(g_capture_cache_dir.c_str(), &diskInfo);
                        unsigned long long blocksize = diskInfo.f_bsize;
                        unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
                        if (file_length * 2 > availableDisk)
                        {
                            LOG_DEBUG("file_length/availableDisk:%u/%llu\n", file_length, availableDisk);
                            LOG_ERROR("Insufficient disk space\n");
                            capture_frames_index = capture_frames;
                            return;
                        }

                        string targetFileName = g_capture_cache_dir + "/" + to_string(buf.sequence) + ".yuv";
                        LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                        size_t totalSize = file_length;
                        FILE* f = fopen(targetFileName.c_str(), "wb");
                        if (f)
                        {
                            fwrite(buffers[buf.index].start, 1, totalSize, f);
                            fclose(f);
                            g_yuvCaptured.notify_one();
                        }
                        else
                        {
                            LOG_DEBUG("Failed to open file %s, return\n", targetFileName.c_str());
                            return;
                        }
                        LOG_DEBUG("%s | DoCaptureYuv end save file. sequence:%u\n", GetTime().c_str(), buf.sequence);
                        capture_frames_index++;
                    }
                    else
                    {
                        // LOG_DEBUG("%s | DoCaptureYuv end send data. sequence:%u\n", GetTime().c_str(), buf.sequence);
                        LOG_DEBUG("DoCaptureYuv end send data. sequence:%u\n", buf.sequence);
                        SendYuvData(sockfd, buf.sequence, buffers[buf.index].start, file_length);
                        capture_frames_index++;
                    }
                }

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                {
                    LOG_ERROR("VIDIOC_QBUF failed\n");
                }

                virtual_sequence++;
                // if (virtual_sequence >= g_offlineRawSourceFileNumber)
                // {
                //     virtual_sequence = 0;
                // }
            }
            break;
        }

        // unmap
        for (uint i = 0; i < n_buffers; ++i)
        {
            if (-1 == munmap(buffers[i].start, buffers[i].length))
            {
                LOG_ERROR("munmap error\n");
            }
        }

        free(buffers);
        close(fd);
        LOG_INFO("YUV capture process end\n");
    }));

    for (auto& thread : threads)
    {
        thread.join();
    }
    g_inCaptureYUVProcess = 0;

    LOG_DEBUG("DoCaptureYuv end\n");
    return 0;
}

static void RawCaptureDeinit(struct capture_info* cap_info)
{
    if (cap_info->subdev_fd > 0)
    {
        device_close(cap_info->subdev_fd);
        cap_info->subdev_fd = -1;
        LOG_DEBUG("device_close(cap_info.subdev_fd)\n");
    }
    if (cap_info->dev_fd > 0)
    {
        device_close(cap_info->dev_fd);
        cap_info->dev_fd = -1;
        LOG_DEBUG("device_close(cap_info.dev_fd)\n");
    }
}

static void DumpCapinfo()
{
    if (g_sensorHdrMode == NO_HDR)
    {
        LOG_INFO("DumpCapinfo NO_HDR: \n");
        LOG_INFO("    dev_name ------------- %s\n", cap_info.dev_name);
        LOG_INFO("    dev_fd --------------- %d\n", cap_info.dev_fd);
        LOG_INFO("    io ------------------- %d\n", cap_info.io);
        LOG_INFO("    width ---------------- %d\n", cap_info.width);
        LOG_INFO("    height --------------- %d\n", cap_info.height);
        LOG_INFO("    format --------------- %d\n", cap_info.format);
        LOG_INFO("    capture_buf_type ----- %d\n", cap_info.capture_buf_type);
        LOG_INFO("    out_file ------------- %s\n", cap_info.out_file);
        LOG_INFO("    frame_count ---------- %d\n", cap_info.frame_count);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        if (g_capture_dev_name.length() == 0)
        {
            LOG_INFO("DumpCapinfo HDR_X2: \n");
            LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr2_1.dev_name);
            LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr2_1.dev_fd);
            LOG_INFO("    io ------------------- %d\n", cap_info_hdr2_1.io);
            LOG_INFO("    width ---------------- %d\n", cap_info_hdr2_1.width);
            LOG_INFO("    height --------------- %d\n", cap_info_hdr2_1.height);
            LOG_INFO("    format --------------- %d\n", cap_info_hdr2_1.format);
            LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr2_1.capture_buf_type);
            LOG_INFO("    out_file ------------- %s\n", cap_info_hdr2_1.out_file);
            LOG_INFO("    frame_count ---------- %d\n\n", cap_info_hdr2_1.frame_count);

            LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr2_2.dev_name);
            LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr2_2.dev_fd);
            LOG_INFO("    io ------------------- %d\n", cap_info_hdr2_2.io);
            LOG_INFO("    width ---------------- %d\n", cap_info_hdr2_2.width);
            LOG_INFO("    height --------------- %d\n", cap_info_hdr2_2.height);
            LOG_INFO("    format --------------- %d\n", cap_info_hdr2_2.format);
            LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr2_2.capture_buf_type);
            LOG_INFO("    out_file ------------- %s\n", cap_info_hdr2_2.out_file);
            LOG_INFO("    frame_count ---------- %d\n", cap_info_hdr2_2.frame_count);
        }
        else
        {
            LOG_INFO("DumpCapinfo HDR_X2 read back mode: \n");
            LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr2_1.dev_name);
            LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr2_1.dev_fd);
            LOG_INFO("    io ------------------- %d\n", cap_info_hdr2_1.io);
            LOG_INFO("    width ---------------- %d\n", cap_info_hdr2_1.width);
            LOG_INFO("    height --------------- %d\n", cap_info_hdr2_1.height);
            LOG_INFO("    format --------------- %d\n", cap_info_hdr2_1.format);
            LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr2_1.capture_buf_type);
            LOG_INFO("    out_file ------------- %s\n", cap_info_hdr2_1.out_file);
            LOG_INFO("    frame_count ---------- %d\n\n", cap_info_hdr2_1.frame_count);

            LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr2_2.dev_name);
            LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr2_2.dev_fd);
            LOG_INFO("    io ------------------- %d\n", cap_info_hdr2_2.io);
            LOG_INFO("    width ---------------- %d\n", cap_info_hdr2_2.width);
            LOG_INFO("    height --------------- %d\n", cap_info_hdr2_2.height);
            LOG_INFO("    format --------------- %d\n", cap_info_hdr2_2.format);
            LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr2_2.capture_buf_type);
            LOG_INFO("    out_file ------------- %s\n", cap_info_hdr2_2.out_file);
            LOG_INFO("    frame_count ---------- %d\n", cap_info_hdr2_2.frame_count);
        }
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        LOG_INFO("DumpCapinfo HDR_X3: \n");
        LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr3_1.dev_name);
        LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr3_1.dev_fd);
        LOG_INFO("    io ------------------- %d\n", cap_info_hdr3_1.io);
        LOG_INFO("    width ---------------- %d\n", cap_info_hdr3_1.width);
        LOG_INFO("    height --------------- %d\n", cap_info_hdr3_1.height);
        LOG_INFO("    format --------------- %d\n", cap_info_hdr3_1.format);
        LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr3_1.capture_buf_type);
        LOG_INFO("    out_file ------------- %s\n", cap_info_hdr3_1.out_file);
        LOG_INFO("    frame_count ---------- %d\n\n", cap_info_hdr3_1.frame_count);

        LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr3_2.dev_name);
        LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr3_2.dev_fd);
        LOG_INFO("    io ------------------- %d\n", cap_info_hdr3_2.io);
        LOG_INFO("    width ---------------- %d\n", cap_info_hdr3_2.width);
        LOG_INFO("    height --------------- %d\n", cap_info_hdr3_2.height);
        LOG_INFO("    format --------------- %d\n", cap_info_hdr3_2.format);
        LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr3_2.capture_buf_type);
        LOG_INFO("    out_file ------------- %s\n", cap_info_hdr3_2.out_file);
        LOG_INFO("    frame_count ---------- %d\n\n", cap_info_hdr3_2.frame_count);

        LOG_INFO("    dev_name ------------- %s\n", cap_info_hdr3_3.dev_name);
        LOG_INFO("    dev_fd --------------- %d\n", cap_info_hdr3_3.dev_fd);
        LOG_INFO("    io ------------------- %d\n", cap_info_hdr3_3.io);
        LOG_INFO("    width ---------------- %d\n", cap_info_hdr3_3.width);
        LOG_INFO("    height --------------- %d\n", cap_info_hdr3_3.height);
        LOG_INFO("    format --------------- %d\n", cap_info_hdr3_3.format);
        LOG_INFO("    capture_buf_type ----- %d\n", cap_info_hdr3_3.capture_buf_type);
        LOG_INFO("    out_file ------------- %s\n", cap_info_hdr3_3.out_file);
        LOG_INFO("    frame_count ---------- %d\n", cap_info_hdr3_3.frame_count);
    }
}

static void SendRawData(int socket, int index, void* buffer, int size)
{
    assert(buffer);

    char* buf = NULL;
    int total = size;
    int packet_len = MAXPACKETSIZE;
    int send_size = 0;
    int ret_val;
    uint16_t check_sum = 0;
    buf = (char*)buffer;
    while (total > 0)
    {
        if (total < packet_len)
        {
            send_size = total;
        }
        else
        {
            send_size = packet_len;
        }
        ret_val = send(socket, buf, send_size, 0);
        total -= send_size;
        buf += ret_val;
    }

    buf = (char*)buffer;
    for (int i = 0; i < size; i++)
    {
        check_sum += buf[i];
    }

    capture_check_sum = check_sum;
    LOG_DEBUG("SendRawData size:%d, sequence:%u\n", size, cap_info.sequence);
}

static void SendRawDataWithExpInfo(int socket, int index, void* buffer, int size, rk_aiq_isp_tool_stats_t* expInfo, int expInfoSize)
{
    assert(buffer);

    char* buf = NULL;
    int total = size;
    int packet_len = MAXPACKETSIZE;
    int send_size = 0;
    int ret_val = 0;
    uint16_t check_sum = 0;
    buf = (char*)buffer;
    while (total > 0)
    {
        if (total < packet_len)
        {
            send_size = total;
        }
        else
        {
            send_size = packet_len;
        }
        ret_val = send(socket, buf, send_size, 0);
        total -= send_size;
        buf += ret_val;
    }

    total = expInfoSize;
    packet_len = MAXPACKETSIZE;
    send_size = 0;
    ret_val = 0;
    buf = (char*)expInfo;
    while (total > 0)
    {
        if (total < packet_len)
        {
            send_size = total;
        }
        else
        {
            send_size = packet_len;
        }
        ret_val = send(socket, buf, send_size, 0);
        total -= send_size;
        buf += ret_val;
    }

    buf = (char*)buffer;
    for (int i = 0; i < size; i++)
    {
        check_sum += buf[i];
    }
    buf = (char*)expInfo;
    for (int i = 0; i < expInfoSize; i++)
    {
        check_sum += buf[i];
    }

    capture_check_sum = check_sum;
    LOG_DEBUG("SendRawDataWithExpInfo size:%d, sequence:%u\n", size, cap_info.sequence);
}

static void OnlineRawCaptureCallBackNoHDR(int socket, int index, void* buffer, int size)
{
    if (g_mmapNumber > 2 && cap_info.ispStatsList.size() == 0)
    {
        usleep(1000 * 50);
        LOG_DEBUG("online raw capture,ispstats list empty.\n");
        return;
    }
    static uint32_t lastSavedSequence = 0;
    if (g_sensorHdrMode == NO_HDR)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackNoHDR NO_HDR\n");
        int width = cap_info.width;
        int height = cap_info.height;

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackNoHDR size error");
            LOG_ERROR("OnlineRawCaptureCallBackNoHDR size error. size:%d. width:%d, height:%d\n", size, width, height);
            return;
        }

        if (cap_info.sequence == 0)
        {
            LOG_ERROR("cap_info.sequence == 0\n");
            return;
        }

        if (g_mmapNumber > 2)
        {
            // LOG_DEBUG("cap_info.sequence:%u\n", cap_info.sequence);
            // LOG_DEBUG("cap_info.ispStatsList.back().frameID:%u\n", cap_info.ispStatsList.back().frameID);
            // LOG_DEBUG("cap_info.ispStatsList.size():%u\n", cap_info.ispStatsList.size());
            while (cap_info.sequence > cap_info.ispStatsList.back().frameID && cap_info.ispStatsList.back().frameID != 0)
            {
                LOG_DEBUG("cap_info.sequence > cap_info.ispStatsList.back().frameID, wait.\n");
                usleep(1000 * 5);
                continue;
            }

            for (vector<rk_aiq_isp_tool_stats_t>::iterator it = cap_info.ispStatsList.begin(); it != cap_info.ispStatsList.end(); it++)
            {
                // LOG_DEBUG("(*it).frameID / cap_info.sequence:%d/%d\n", (*it).frameID, cap_info.sequence);
                if (cap_info.sequence == (*it).frameID || cap_info.ispStatsList.back().frameID == 0)
                {
                    if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
                    {
                        LOG_DEBUG("get 3a lock fail 1\n");
                        assert(NULL);
                    }
                    rk_aiq_isp_tool_stats_t expInfo = *it;
                    expInfo.frameID = cap_info.sequence;
                    get3AStatsMtx.unlock();
                    // HexDump((unsigned char*)&expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    LOG_INFO("expRealParams.integration_time:%f\n", expInfo.linearExp.exp_real_params.integration_time);
                    LOG_INFO("expRealParams.analog_gain:%f\n", expInfo.linearExp.exp_real_params.analog_gain);
                    LOG_INFO("expRealParams.digital_gain:%f\n", expInfo.linearExp.exp_real_params.digital_gain);
                    // LOG_INFO("expRealParams.isp_dgain:%f\n", expInfo.linearExp.exp_real_params.isp_dgain);
                    // LOG_INFO("expRealParams.iso:%d\n", expInfo.linearExp.exp_real_params.iso);
                    // LOG_INFO("expRealParams.dcg_mode:%d\n", expInfo.linearExp.exp_real_params.dcg_mode);
                    // LOG_INFO("expRealParams.longfrm_mode:%d\n", expInfo.linearExp.exp_real_params.longfrm_mode);

                    if (g_usingCaptureCacheFlag == 1)
                    {
                        if (0 != access(g_capture_cache_dir.c_str(), 0))
                        {
                            if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                            {
                                LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                                return;
                            }
                        }

                        uint32_t seq = cap_info.sequence;
                        LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
                        string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info.sequence) + ".raw";
                        LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                        size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                        std::vector<char> tempBuffer(totalSize);
                        // Check available disk space
                        // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
                        LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
                        struct statfs diskInfo;
                        statfs(g_capture_cache_dir.c_str(), &diskInfo);
                        unsigned long long blocksize = diskInfo.f_bsize;
                        unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
                        if (totalSize * 2 > availableDisk)
                        {
                            LOG_DEBUG("totalSize/availableDisk:%zu/%llu\n", totalSize, availableDisk);
                            LOG_ERROR("Insufficient disk space\n");
                            return;
                        }

                        if (!tempBuffer.empty())
                        {
                            memcpy(tempBuffer.data(), buffer, size);
                            memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                            LOG_DEBUG("write file begin:%s\n", GetTime().c_str());
                            std::future<void> result(std::async(std::launch::async, write_to_file, targetFileName.c_str(), tempBuffer.data(), totalSize));
                            // result.wait();
                            LOG_DEBUG("write file end:%s\n", GetTime().c_str());
                            LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                        }
                        else
                        {
                            LOG_DEBUG("Failed to allocate memory for tempBuffer, return\n");
                            return;
                        }
                        LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
                        lastSavedSequence = seq;

                        if (cap_info.ispStatsList.back().frameID != 0)
                        {
                            LOG_DEBUG("SendRawDataWithExpInfo\n");
                        }
                        else
                        {
                            LOG_DEBUG("SendRawData with ExpInfo 0\n");
                        }
                        capture_frames_index++;
                    }
                    else
                    {
                        if (cap_info.ispStatsList.back().frameID != 0)
                        {
                            LOG_DEBUG("SendRawDataWithExpInfo\n");
                        }
                        else
                        {
                            LOG_DEBUG("SendRawData with ExpInfo 0\n");
                        }
                        SendRawDataWithExpInfo(socket, index, buffer, size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                        capture_frames_index++;
                    }
                    break;
                }
                else
                {
                    if (lastSavedSequence != 0 && cap_info.sequence > (*it).frameID && lastSavedSequence > (*it).frameID)
                        it = cap_info.ispStatsList.erase(it);
                }
            }
        }
        else
        {
            LOG_DEBUG("%s | DoCaptureCallBack size %d, sequence:%u\n", GetTime().c_str(), size, cap_info.sequence);

            if (g_usingCaptureCacheFlag == 1)
            {
                uint seq = cap_info.sequence;
                LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
                string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info.sequence) + ".raw";
                LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                std::vector<char> tempBuffer(totalSize);

                // Check available disk space
                // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
                LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
                struct statfs diskInfo;
                statfs(g_capture_cache_dir.c_str(), &diskInfo);
                unsigned long long blocksize = diskInfo.f_bsize;
                unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
                if (totalSize * 2 > availableDisk)
                {
                    LOG_DEBUG("totalSize/availableDisk:%zu/%llu\n", totalSize, availableDisk);
                    LOG_ERROR("Insufficient disk space\n");
                    capture_frames_index = capture_frames;
                    return;
                }

                if (!tempBuffer.empty())
                {
                    rk_aiq_isp_tool_stats_t expInfo;
                    memset(&expInfo, 0, sizeof(rk_aiq_isp_tool_stats_t));
                    memcpy(tempBuffer.data(), buffer, size);
                    memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    LOG_DEBUG("write file begin:%s\n", GetTime().c_str());
                    std::future<void> result(std::async(std::launch::async, write_to_file, targetFileName.c_str(), tempBuffer.data(), totalSize));
                    // result.wait();
                    LOG_DEBUG("write file end:%s\n", GetTime().c_str());
                    LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                }
                else
                {
                    LOG_DEBUG("Failed to allocate memory for tempBuffer, return\n");
                    return;
                }

                LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
                LOG_DEBUG("SendRawData NOExpInfo\n");
                capture_frames_index++;
            }
            else
            {
                size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                std::vector<char> tempBuffer(totalSize);
                rk_aiq_isp_tool_stats_t expInfo;
                memset(&expInfo, 0, sizeof(rk_aiq_isp_tool_stats_t));
                memcpy(tempBuffer.data(), buffer, size);
                memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                SendRawData(socket, index, tempBuffer.data(), totalSize);
                LOG_DEBUG("SendRawData NOExpInfo\n");
                capture_frames_index++;
            }
        }
    }
}

static void OnlineRawCaptureCallBackHDRX2_1(int socket, int index, void* buffer, int size)
{
    if (g_mmapNumber > 2 && cap_info_hdr2_1.ispStatsList.size() == 0)
    {
        usleep(1000 * 50);
        LOG_DEBUG("online raw capture,ispstats list empty.\n");
        return;
    }
    static uint32_t lastSavedSequence = 0;
    if (g_sensorHdrMode == HDR_X2)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackHDRX2_1 HDRX2_1\n");

        int width = cap_info_hdr2_1.width;
        int height = cap_info_hdr2_1.height;

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackHDRX2_1 size error");
            LOG_ERROR("OnlineRawCaptureCallBackHDRX2_1 size error. size:%d. width:%d, height:%d\n", size, width, height);
            return;
        }

        if (cap_info_hdr2_1.sequence == 0)
        {
            LOG_ERROR("cap_info_hdr2_1.sequence == 0\n");
            return;
        }

        if (g_mmapNumber > 2)
        {
            // LOG_DEBUG("cap_info_hdr2_1.sequence:%u\n", cap_info_hdr2_1.sequence);
            // LOG_DEBUG("cap_info_hdr2_1.ispStatsList.back().frameID:%u\n", cap_info_hdr2_1.ispStatsList.back().frameID);
            // LOG_DEBUG("cap_info_hdr2_1.ispStatsList.size():%u\n", cap_info_hdr2_1.ispStatsList.size());
            while (cap_info_hdr2_1.sequence > cap_info_hdr2_1.ispStatsList.back().frameID && cap_info_hdr2_1.ispStatsList.back().frameID != 0)
            {
                LOG_DEBUG("cap_info_hdr2_1.sequence > cap_info_hdr2_1.ispStatsList.back().frameID, wait.\n");
                usleep(1000 * 5);
                continue;
            }

            for (vector<rk_aiq_isp_tool_stats_t>::iterator it = cap_info_hdr2_1.ispStatsList.begin(); it != cap_info_hdr2_1.ispStatsList.end(); it++)
            {
                // LOG_DEBUG("(*it).frameID / cap_info_hdr2_1.sequence:%d/%d\n", (*it).frameID, cap_info_hdr2_1.sequence);
                if (cap_info_hdr2_1.sequence == (*it).frameID || cap_info_hdr2_1.ispStatsList.back().frameID == 0)
                {
                    if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
                    {
                        LOG_DEBUG("get 3a lock fail 1\n");
                        assert(NULL);
                    }
                    rk_aiq_isp_tool_stats_t expInfo = *it;
                    expInfo.frameID = cap_info_hdr2_1.sequence;
                    get3AStatsMtx.unlock();
                    // HexDump((unsigned char*)&expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    for (int i = 0; i < 3; i++)
                    {
                        LOG_INFO("hdrExp[%d].expRealParams.integration_time:%f\n", i, expInfo.hdrExp[i].exp_real_params.integration_time);
                        LOG_INFO("hdrExp[%d].expRealParams.analog_gain:%f\n", i, expInfo.hdrExp[i].exp_real_params.analog_gain);
                        LOG_INFO("hdrExp[%d].expRealParams.digital_gain:%f\n", i, expInfo.hdrExp[i].exp_real_params.digital_gain);
                        LOG_INFO("hdrExp[%d].expRealParams.isp_dgain:%f\n", i, expInfo.hdrExp[i].exp_real_params.isp_dgain);
                        LOG_INFO("hdrExp[%d].expRealParams.iso:%d\n", i, expInfo.hdrExp[i].exp_real_params.iso);
                        LOG_INFO("hdrExp[%d].expRealParams.dcg_mode:%d\n", i, expInfo.hdrExp[i].exp_real_params.dcg_mode);
                        LOG_INFO("hdrExp[%d].expRealParams.longfrm_mode:%d\n", i, expInfo.hdrExp[i].exp_real_params.longfrm_mode);
                    }

                    if (g_usingCaptureCacheFlag == 1)
                    {
                        if (0 != access(g_capture_cache_dir.c_str(), 0))
                        {
                            if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                            {
                                LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                                return;
                            }
                        }

                        uint32_t seq = cap_info_hdr2_1.sequence;
                        LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
                        string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr2_1.sequence) + "-2_2.raw";
                        LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                        size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                        std::vector<char> tempBuffer(totalSize);
                        // Check available disk space
                        // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
                        LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
                        struct statfs diskInfo;
                        statfs(g_capture_cache_dir.c_str(), &diskInfo);
                        unsigned long long blocksize = diskInfo.f_bsize;
                        unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
                        if (totalSize * 2 > availableDisk)
                        {
                            LOG_DEBUG("totalSize/availableDisk:%zu/%llu\n", totalSize, availableDisk);
                            LOG_ERROR("Insufficient disk space\n");
                            return;
                        }

                        if (!tempBuffer.empty())
                        {
                            memcpy(tempBuffer.data(), buffer, size);
                            memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                            LOG_DEBUG("write file begin:%s\n", GetTime().c_str());
                            std::future<void> result(std::async(std::launch::async, write_to_file, targetFileName.c_str(), tempBuffer.data(), totalSize));
                            // result.wait();
                            LOG_DEBUG("write file end:%s\n", GetTime().c_str());
                            LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                        }
                        else
                        {
                            LOG_DEBUG("Failed to allocate memory for tempBuffer, return\n");
                            return;
                        }
                        LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
                        lastSavedSequence = seq;

                        if (cap_info_hdr2_1.ispStatsList.back().frameID != 0)
                        {
                            LOG_DEBUG("SendRawDataWithExpInfo\n");
                        }
                        else
                        {
                            LOG_DEBUG("SendRawData with ExpInfo 0\n");
                        }
                    }
                    else
                    {
                        if (cap_info_hdr2_1.ispStatsList.back().frameID != 0)
                        {
                            LOG_DEBUG("SendRawDataWithExpInfo\n");
                        }
                        else
                        {
                            LOG_DEBUG("SendRawData with ExpInfo 0\n");
                        }
                        SendRawDataWithExpInfo(socket, index, buffer, size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    }
                    break;
                }
                else
                {
                    if (lastSavedSequence != 0 && cap_info_hdr2_1.sequence > (*it).frameID && lastSavedSequence > (*it).frameID)
                        it = cap_info_hdr2_1.ispStatsList.erase(it);
                }
            }
        }
        else
        {
            LOG_DEBUG("%s | DoCaptureCallBack size %d, sequence:%u\n", GetTime().c_str(), size, cap_info_hdr2_1.sequence);

            if (g_usingCaptureCacheFlag == 1)
            {
                uint seq = cap_info_hdr2_1.sequence;
                LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
                string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr2_1.sequence) + "-2_2.raw";
                LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                std::vector<char> tempBuffer(totalSize);

                // Check available disk space
                // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
                LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
                struct statfs diskInfo;
                statfs(g_capture_cache_dir.c_str(), &diskInfo);
                unsigned long long blocksize = diskInfo.f_bsize;
                unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
                if (totalSize * 2 > availableDisk)
                {
                    LOG_DEBUG("totalSize/availableDisk:%zu/%llu\n", totalSize, availableDisk);
                    LOG_ERROR("Insufficient disk space\n");
                    capture_frames_index = capture_frames;
                    return;
                }

                if (!tempBuffer.empty())
                {
                    rk_aiq_isp_tool_stats_t expInfo;
                    memset(&expInfo, 0, sizeof(rk_aiq_isp_tool_stats_t));
                    memcpy(tempBuffer.data(), buffer, size);
                    memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    LOG_DEBUG("write file begin:%s\n", GetTime().c_str());
                    std::future<void> result(std::async(std::launch::async, write_to_file, targetFileName.c_str(), tempBuffer.data(), totalSize));
                    // result.wait();
                    LOG_DEBUG("write file end:%s\n", GetTime().c_str());
                    LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                }
                else
                {
                    LOG_DEBUG("Failed to allocate memory for tempBuffer, return\n");
                    return;
                }

                LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
                LOG_DEBUG("SendRawData NOExpInfo\n");
            }
            else
            {
                size_t totalSize = size + sizeof(rk_aiq_isp_tool_stats_t);
                std::vector<char> tempBuffer(totalSize);
                rk_aiq_isp_tool_stats_t expInfo;
                memset(&expInfo, 0, sizeof(rk_aiq_isp_tool_stats_t));
                memcpy(tempBuffer.data(), buffer, size);
                memcpy(tempBuffer.data() + size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                SendRawData(socket, index, tempBuffer.data(), totalSize);
                LOG_DEBUG("SendRawData NOExpInfo\n");
            }
        }
    }
}

// cap_info_hdr2_1 中发送相关曝光信息，这里不用发送曝光信息，也就不需要匹配
static void OnlineRawCaptureCallBackHDRX2_2(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("OnlineRawCaptureCallBackHDRX2_2 size %d\n", size);
    if (g_sensorHdrMode == HDR_X2)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackHDRX2_2 HDRX2_2\n");

        int width = cap_info_hdr2_2.width;
        int height = cap_info_hdr2_2.height;
        LOG_DEBUG("cap_info_hdr2_2.width %d\n", cap_info_hdr2_2.width);
        LOG_DEBUG("cap_info_hdr2_2.height %d\n", cap_info_hdr2_2.height);
        LOG_DEBUG("cap_info_hdr2_2.sequence %u\n", cap_info_hdr2_2.sequence);

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackHDRX2_2 size error");
            LOG_ERROR("OnlineRawCaptureCallBackHDRX2_2 size error\n");
            return;
        }

        if (cap_info_hdr2_2.sequence == 0)
        {
            LOG_ERROR("cap_info_hdr2_2.sequence == 0\n");
            return;
        }

        if (g_usingCaptureCacheFlag == 1)
        {
            if (0 != access(g_capture_cache_dir.c_str(), 0))
            {
                if (g_usingCaptureCacheFlag == 1 && mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                {
                    LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                    return;
                }
            }

            LOG_DEBUG("DoCaptureCallBack size %d, sequence:%u\n", size, cap_info_hdr2_2.sequence);
            uint seq = cap_info_hdr2_2.sequence;
            LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
            string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr2_2.sequence) + "-2_1.raw";
            LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

            size_t totalSize = size;
            std::vector<char> tempBuffer(totalSize);

            // Check available disk space
            // std::string tmpStr = GetFirstDirectory(g_capture_cache_dir.c_str());
            LOG_DEBUG("capture cache dir:%s\n", g_capture_cache_dir.c_str());
            struct statfs diskInfo;
            statfs(g_capture_cache_dir.c_str(), &diskInfo);
            unsigned long long blocksize = diskInfo.f_bsize;
            unsigned long long availableDisk = diskInfo.f_bavail * blocksize;
            if (totalSize * 2 > availableDisk)
            {
                LOG_DEBUG("totalSize/availableDisk:%zu/%llu\n", totalSize, availableDisk);
                LOG_ERROR("Insufficient disk space\n");
                capture_frames_index = capture_frames;
                return;
            }

            if (!tempBuffer.empty())
            {
                memcpy(tempBuffer.data(), buffer, size);
                LOG_DEBUG("write file begin:%s\n", GetTime().c_str());
                std::future<void> result(std::async(std::launch::async, write_to_file, targetFileName.c_str(), tempBuffer.data(), totalSize));
                // result.wait();
                LOG_DEBUG("write file end:%s\n", GetTime().c_str());
                LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
            }
            else
            {
                LOG_DEBUG("Failed to allocate memory for tempBuffer, return\n");
                return;
            }
            LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
            LOG_DEBUG("SendRawData NOExpInfo\n");
            capture_frames_index++;
        }
        else
        {
            LOG_DEBUG("SendRawData NOExpInfo\n");
            SendRawData(socket, index, buffer, size);
            capture_frames_index++;
        }
    }
}

static void OnlineRawCaptureCallBackHDRX3_1(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_1 size %d\n", size);
    if (g_sensorHdrMode == HDR_X2)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_1 HDRX3_1\n");

        int width = cap_info_hdr3_1.width;
        int height = cap_info_hdr3_1.height;
        LOG_DEBUG("cap_info_hdr3_1.width %d\n", cap_info_hdr3_1.width);
        LOG_DEBUG("cap_info_hdr3_1.height %d\n", cap_info_hdr3_1.height);
        LOG_DEBUG("cap_info_hdr3_1.sequence %u\n", cap_info_hdr3_1.sequence);

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackHDRX3_1 size error");
            LOG_ERROR("OnlineRawCaptureCallBackHDRX3_1 size error\n");
            return;
        }

        if (cap_info_hdr3_1.sequence == 0)
        {
            LOG_ERROR("cap_info_hdr3_1.sequence == 0\n");
            return;
        }
        if (g_mmapNumber > 2)
        {
            LOG_DEBUG("HDRX3_1 sequence match process begin.\n");
            // while (cap_info_hdr3_1.ispStatsList.size() == 0 || cap_info_hdr3_1.sequence >
            // cap_info_hdr3_1.ispStatsList.back().frameID)
            // {
            //     LOG_DEBUG("cap_info_hdr3_1.sequence:%u\n", cap_info_hdr3_1.sequence);
            //     if (cap_info_hdr3_1.ispStatsList.size() > 0)
            //     {
            //         LOG_DEBUG("cap_info_hdr3_1.sequence > cap_info_hdr3_1.ispStatsList.back().frameID:%u\n",
            //         cap_info_hdr3_1.ispStatsList.back().frameID);
            //     }
            //     else
            //     {
            //         LOG_DEBUG("cap_info_hdr3_1.ispStatsList.size()==0\n");
            //     }

            //     LOG_DEBUG("HDRX3_1 sequence match process .\n");
            //     rk_aiq_isp_tool_stats_t attr;
            //     RkAiqSocketClientINETReceive(ENUM_ID_AIQ_UAPI_SYSCTL_GETTOOLSERVER3ASTATSBLK, (void*)&attr,
            //     sizeof(rk_aiq_isp_tool_stats_t)); LOG_DEBUG("HDRX3_1 sequence match process ..\n");

            //     if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
            //     {
            //         LOG_DEBUG("get 3a lock fail 4\n");
            //         assert(NULL);
            //     }
            //     LOG_DEBUG("HDRX3_1 sequence match process ...\n");
            //     cap_info_hdr3_1.ispStatsList.push_back(attr);
            //     get3AStatsMtx.unlock();

            //     LOG_DEBUG("HDRX3_1 sequence match process finish\n");
            //     usleep(1000 * 1);
            //     continue;
            // }
            LOG_DEBUG("HDRX3_1 sequence match process end.\n");
            LOG_DEBUG("cap_info_hdr3_1.sequence:%u\n", cap_info_hdr3_1.sequence);
            LOG_DEBUG("cap_info_hdr3_1.ispStatsList.back().frameID:%u\n", cap_info_hdr3_1.ispStatsList.back().frameID);

            for (vector<rk_aiq_isp_tool_stats_t>::iterator it = cap_info_hdr3_1.ispStatsList.begin(); it != cap_info_hdr3_1.ispStatsList.end(); it++)
            {
                if (cap_info_hdr3_1.sequence == (*it).frameID)
                {
                    if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
                    {
                        LOG_DEBUG("get 3a lock fail 1\n");
                        assert(NULL);
                    }
                    LOG_INFO("cap_info_hdr3_1.ispStatsList[x].frameID:%u\n", (*it).frameID);
                    rk_aiq_isp_tool_stats_t expInfo;
                    memcpy((void*)&expInfo, (void*)&(*it), sizeof(rk_aiq_isp_tool_stats_t));
                    expInfo.frameID = cap_info_hdr3_1.sequence;
                    get3AStatsMtx.unlock();

                    HexDump((unsigned char*)&expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    for (int i = 0; i < 3; i++)
                    {
                        LOG_INFO("hdrExp[%d].expRealParams.integration_time:%f\n", i, expInfo.hdrExp[i].exp_real_params.integration_time);
                        LOG_INFO("hdrExp[%d].expRealParams.analog_gain:%f\n", i, expInfo.hdrExp[i].exp_real_params.analog_gain);
                        LOG_INFO("hdrExp[%d].expRealParams.digital_gain:%f\n", i, expInfo.hdrExp[i].exp_real_params.digital_gain);
                        LOG_INFO("hdrExp[%d].expRealParams.isp_dgain:%f\n", i, expInfo.hdrExp[i].exp_real_params.isp_dgain);
                        LOG_INFO("hdrExp[%d].expRealParams.iso:%d\n", i, expInfo.hdrExp[i].exp_real_params.iso);
                        LOG_INFO("hdrExp[%d].expRealParams.dcg_mode:%d\n", i, expInfo.hdrExp[i].exp_real_params.dcg_mode);
                        LOG_INFO("hdrExp[%d].expRealParams.longfrm_mode:%d\n", i, expInfo.hdrExp[i].exp_real_params.longfrm_mode);
                    }

                    if (g_usingCaptureCacheFlag == 1 && 0 != access(g_capture_cache_dir.c_str(), 0))
                    {
                        if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                        {
                            LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                            return;
                        }
                    }

                    LOG_DEBUG("DoCaptureCallBack size %d, sequence:%u\n", size, cap_info_hdr3_1.sequence);
                    uint seq = cap_info_hdr3_1.sequence;
                    LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
                    string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr3_1.sequence) + "-3_1.raw";
                    LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

                    FILE* fWrite = fopen(targetFileName.c_str(), "w");
                    if (fWrite != NULL)
                    {
                        fwrite(buffer, size, 1, fWrite);
                        fwrite(&expInfo, sizeof(rk_aiq_isp_tool_stats_t), 1, fWrite);
                        fflush(fWrite);
                        LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                        fclose(fWrite);
                    }
                    else
                    {
                        LOG_DEBUG("DoCaptureCallBack failed to create file %s, return\n", targetFileName.c_str());
                        fclose(fWrite);
                        return;
                    }
                    LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);

                    LOG_DEBUG("SendRawDataWithExpInfo\n");
                    // SendRawDataWithExpInfo(socket, index, buffer, size, &expInfo, sizeof(rk_aiq_isp_tool_stats_t));
                    break;
                }
            }
        }
        else
        {
            if (g_usingCaptureCacheFlag == 1 && 0 != access(g_capture_cache_dir.c_str(), 0))
            {
                if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                {
                    LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                    return;
                }
            }

            LOG_DEBUG("DoCaptureCallBack size %d, sequence:%u\n", size, cap_info_hdr3_1.sequence);
            uint seq = cap_info_hdr3_1.sequence;
            LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
            string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr3_1.sequence) + "-3_1.raw";
            LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

            FILE* fWrite = fopen(targetFileName.c_str(), "w");
            if (fWrite != NULL)
            {
                rk_aiq_isp_tool_stats_t expInfo;
                fwrite(buffer, size, 1, fWrite);
                fwrite(&expInfo, sizeof(rk_aiq_isp_tool_stats_t), 1, fWrite);
                fflush(fWrite);
                LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                fclose(fWrite);
            }
            else
            {
                LOG_DEBUG("DoCaptureCallBack failed to create file %s, return\n", targetFileName.c_str());
                fclose(fWrite);
                return;
            }
            LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
            LOG_DEBUG("SendRawData NOExpInfo\n");
        }
    }
}

// cap_info_hdr3_1 中发送相关曝光信息，这里不用发送曝光信息，也就不需要匹配
static void OnlineRawCaptureCallBackHDRX3_2(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_2 size %d\n", size);
    if (g_sensorHdrMode == HDR_X2)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_2 HDRX3_2\n");

        int width = cap_info_hdr3_2.width;
        int height = cap_info_hdr3_2.height;
        LOG_DEBUG("cap_info_hdr3_2.width %d\n", cap_info_hdr3_2.width);
        LOG_DEBUG("cap_info_hdr3_2.height %d\n", cap_info_hdr3_2.height);
        LOG_DEBUG("cap_info_hdr3_2.sequence %u\n", cap_info_hdr3_2.sequence);

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackHDRX3_2 size error");
            LOG_ERROR("OnlineRawCaptureCallBackHDRX3_2 size error\n");
            return;
        }

        if (cap_info_hdr3_2.sequence == 0)
        {
            LOG_ERROR("cap_info_hdr3_2.sequence == 0\n");
            return;
        }

        {
            if (g_usingCaptureCacheFlag == 1 && 0 != access(g_capture_cache_dir.c_str(), 0))
            {
                if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                {
                    LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                    return;
                }
            }

            LOG_DEBUG("DoCaptureCallBack size %d, sequence:%u\n", size, cap_info_hdr3_2.sequence);
            uint seq = cap_info_hdr3_2.sequence;
            LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
            string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr3_2.sequence) + "-3_2.raw";
            LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

            FILE* fWrite = fopen(targetFileName.c_str(), "w");
            if (fWrite != NULL)
            {
                fwrite(buffer, size, 1, fWrite);
                fflush(fWrite);
                LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                fclose(fWrite);
            }
            else
            {
                LOG_DEBUG("DoCaptureCallBack failed to create file %s, return\n", targetFileName.c_str());
                fclose(fWrite);
                return;
            }
            LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
            LOG_DEBUG("SendRawData NOExpInfo\n");
        }
    }
}

// cap_info_hdr3_1 中发送相关曝光信息，这里不用发送曝光信息，也就不需要匹配
static void OnlineRawCaptureCallBackHDRX3_3(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_3 size %d\n", size);
    if (g_sensorHdrMode == HDR_X2)
    {
        LOG_DEBUG("OnlineRawCaptureCallBackHDRX3_3 HDRX3_3\n");

        int width = cap_info_hdr3_3.width;
        int height = cap_info_hdr3_3.height;
        LOG_DEBUG("cap_info_hdr3_3.width %d\n", cap_info_hdr3_3.width);
        LOG_DEBUG("cap_info_hdr3_3.height %d\n", cap_info_hdr3_3.height);
        LOG_DEBUG("cap_info_hdr3_3.sequence %u\n", cap_info_hdr3_3.sequence);

        if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
        {
            // SendMessageToPC(socket, "OnlineRawCaptureCallBackHDRX3_3 size error");
            LOG_ERROR("OnlineRawCaptureCallBackHDRX3_3 size error\n");
            return;
        }

        if (cap_info_hdr3_3.sequence == 0)
        {
            LOG_ERROR("cap_info_hdr3_3.sequence == 0\n");
            return;
        }

        {
            if (g_usingCaptureCacheFlag == 1 && 0 != access(g_capture_cache_dir.c_str(), 0))
            {
                if (mkdir(g_capture_cache_dir.c_str(), 0777) != 0)
                {
                    LOG_ERROR("Create folder %s failed.\n", g_capture_cache_dir.c_str());
                    return;
                }
            }

            LOG_DEBUG("DoCaptureCallBack size %d, sequence:%u\n", size, cap_info_hdr3_3.sequence);
            uint seq = cap_info_hdr3_3.sequence;
            LOG_DEBUG("%s | DoCaptureCallBack start save file. sequence:%u\n", GetTime().c_str(), seq);
            string targetFileName = g_capture_cache_dir + "/" + to_string(cap_info_hdr3_3.sequence) + "-3_3.raw";
            LOG_ERROR("Capture image :%s\n", targetFileName.c_str());

            FILE* fWrite = fopen(targetFileName.c_str(), "w");
            if (fWrite != NULL)
            {
                fwrite(buffer, size, 1, fWrite);
                fflush(fWrite);
                LOG_ERROR("Capture image save to :%s\n", targetFileName.c_str());
                fclose(fWrite);
            }
            else
            {
                LOG_DEBUG("DoCaptureCallBack failed to create file %s, return\n", targetFileName.c_str());
                fclose(fWrite);
                return;
            }
            LOG_DEBUG("%s | DoCaptureCallBack end save file. sequence:%u\n", GetTime().c_str(), seq);
            LOG_DEBUG("SendRawData NOExpInfo\n");
        }
    }
}

static int DoCaptureOnlineRaw(int sockfd)
{
    if (capture_frames_index >= capture_frames)
    {
        LOG_DEBUG("DoCaptureOnlineRaw done. %d/%d return.\n", capture_frames_index, capture_frames);
        return 0;
    }

    DumpCapinfo();

    if (g_sensorHdrMode == NO_HDR)
    {
        start_capturing(&cap_info);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        std::vector<std::thread> captureThreads;
        captureThreads.push_back(std::thread([&]() {
            start_capturing(&cap_info_hdr2_1);
        }));
        usleep(1000 * 2);
        captureThreads.push_back(std::thread([&]() {
            start_capturing(&cap_info_hdr2_2);
        }));
        for (auto& thread : captureThreads)
        {
            thread.join();
        }
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        std::vector<std::thread> captureThreads;
        captureThreads.push_back(std::thread([&]() {
            start_capturing(&cap_info_hdr3_1);
        }));
        usleep(1000 * 2);
        captureThreads.push_back(std::thread([&]() {
            start_capturing(&cap_info_hdr3_2);
        }));
        usleep(1000 * 2);
        captureThreads.push_back(std::thread([&]() {
            start_capturing(&cap_info_hdr3_3);
        }));
        for (auto& thread : captureThreads)
        {
            thread.join();
        }
    }

    if (capture_frames_index == 0)
    {
        char tmpPath[100] = {0};
        getcwd(tmpPath, 100);
        string currentDir = tmpPath;
        string tmpCmd = string_format("rm -rf %s/* && sync", g_capture_cache_dir.c_str());
        // LOG_DEBUG("#### exec:%s\n", tmpCmd.c_str());
        system(tmpCmd.c_str());
    }

    std::vector<std::thread> threads;
    threads.push_back(std::thread([&]() {
        LOG_DEBUG("get 3A stats begin\n");
        while (1)
        {
            if (capture_frames_index >= capture_frames)
            {
                printf("capture_frames_index/capture_frames:%u/%u", capture_frames_index, capture_frames);
                break;
            }
            if (g_mmapNumber <= 2)
            {
                usleep(1000 * 100);
                break;
            }

            static uint32_t lastFrameID = 0;
            rk_aiq_isp_tool_stats_t attr;
            memset(&attr, 0, sizeof(rk_aiq_isp_tool_stats_t));
            if (RkAiqSocketClientINETReceive(ENUM_ID_AIQ_UAPI_SYSCTL_GETTOOLSERVER3ASTATSBLK, (void*)&attr, sizeof(rk_aiq_isp_tool_stats_t)) == 0)
            {
                if (lastFrameID != attr.frameID)
                {
                    lastFrameID = attr.frameID;
                    if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
                    {
                        LOG_DEBUG("get 3a lock fail 4\n");
                        assert(NULL);
                    }
                    // LOG_DEBUG("#### attr.version:%u, attr.frameID=%u\n", attr.version, attr.frameID);
                    cap_info.ispStatsList.push_back(attr);
                    cap_info_hdr2_1.ispStatsList.push_back(attr);
                    get3AStatsMtx.unlock();
                }
            }
            else
            {
                attr.frameID = 0;
                lastFrameID = attr.frameID;
                if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
                {
                    LOG_DEBUG("get 3a lock fail 4.1\n");
                    assert(NULL);
                }
                // LOG_DEBUG("#### attr.version:%u, attr.frameID=%u\n", attr.version, attr.frameID);
                cap_info.ispStatsList.push_back(attr);
                cap_info_hdr2_1.ispStatsList.push_back(attr);
                get3AStatsMtx.unlock();
            }
            usleep(1000 * 10);
            continue;
        }
        LOG_DEBUG("get 3A stats end\n");
    }));
    usleep(1000 * 100);

    int skip_frame = 2;
    if (capture_frames_index == 0)
    {
        for (int i = 0; i < skip_frame; i++)
        {
            if (g_sensorHdrMode == NO_HDR)
            {
                read_frame(sockfd, capture_frames_index, &cap_info, nullptr);
            }
            else if (g_sensorHdrMode == HDR_X2)
            {
                read_frame(sockfd, capture_frames_index, &cap_info_hdr2_2, nullptr); // short frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr2_1, nullptr); // long frame
            }
            else if (g_sensorHdrMode == HDR_X3)
            {
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_1, nullptr); // long frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_2, nullptr); // mid frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_3, nullptr); // short frame
            }
            LOG_DEBUG("DoCapture skip frame %d ...\n", i);
        }
    }

    threads.push_back(std::thread([&, sockfd]() {
        LOG_DEBUG("read_frame begin\n");
        while (capture_frames_index < capture_frames)
        {
            if (g_sensorHdrMode == NO_HDR)
            {
                read_frame(sockfd, capture_frames_index, &cap_info, OnlineRawCaptureCallBackNoHDR);
            }
            else if (g_sensorHdrMode == HDR_X2)
            {
                read_frame(sockfd, capture_frames_index, &cap_info_hdr2_2,
                           OnlineRawCaptureCallBackHDRX2_2); // short frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr2_1,
                           OnlineRawCaptureCallBackHDRX2_1); // long frame
            }
            else if (g_sensorHdrMode == HDR_X3)
            {
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_1,
                           OnlineRawCaptureCallBackHDRX3_1); // long frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_2,
                           OnlineRawCaptureCallBackHDRX3_2); // mid frame
                usleep(1000 * 2);
                read_frame(sockfd, capture_frames_index, &cap_info_hdr3_3,
                           OnlineRawCaptureCallBackHDRX3_3); // short frame
            }
            usleep(1000 * 10);
        }
        LOG_DEBUG("read_frame end\n");
    }));

    if (g_usingCaptureCacheFlag == 1)
    {
        usleep(1000 * 500);

        threads.push_back(std::thread([=]() {
            LOG_DEBUG("DoCapture send raw file process. start\n");
            static std::vector<std::string> raw_files;
            int startCapFlag = 1;
            while (startCapFlag == 1 || (capture_frames_index < capture_frames && raw_files.size() > 0))
            {
                for (auto tmp : raw_files)
                {
                    LOG_DEBUG("raw_file:%s\n", tmp.c_str());
                }
                startCapFlag = 0;
                char tmpPath[100] = {0};
                getcwd(tmpPath, 100);
                string tmpDir = tmpPath;

                usleep(1000 * 200);
                DIR* dir = opendir(g_capture_cache_dir.c_str());
                struct dirent* dir_ent = NULL;
                if (dir)
                {
                    raw_files.clear();
                    while ((dir_ent = readdir(dir)))
                    {
                        if (dir_ent->d_type == DT_REG)
                        {
                            // is raw file
                            if (strstr(dir_ent->d_name, ".raw"))
                            {
                                raw_files.push_back(dir_ent->d_name);
                            }
                            // is raw file
                            if (strstr(dir_ent->d_name, ".rkraw"))
                            {
                                raw_files.push_back(dir_ent->d_name);
                            }
                        }
                    }
                    closedir(dir);
                }
                if (raw_files.size() == 0)
                {
                    LOG_INFO("No raw files in %s\n", g_capture_cache_dir.c_str());
                    return;
                }

                std::sort(raw_files.begin(), raw_files.end(), natural_more);
                string tmpRawFile = raw_files.back();
                while (tmpRawFile != "")
                {
                    string fileFullpath = string_format("%s/%s", g_capture_cache_dir.c_str(), tmpRawFile.c_str());
                    LOG_DEBUG("RAW CAPTURED:%s\n", fileFullpath.c_str());

                    FILE* f = fopen(fileFullpath.c_str(), "r");
                    fseek(f, 0, SEEK_END);
                    long fsize = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    LOG_DEBUG("RAW CAPTURED size:%ld\n", fsize);
                    if (fsize > 0)
                    {
                        char* rawData = (char*)malloc(fsize);
                        fread(rawData, fsize, 1, f);

                        LOG_DEBUG("RAW CAPTURED SendRawData begin\n");
                        SendRawData(sockfd, capture_frames_index, rawData, fsize);
                        usleep(1000 * 100);
                        LOG_DEBUG("RAW CAPTURED SendRawData end\n");
                        free(rawData);
                        rawData = NULL;
                        string tmpCmd = string_format("rm -f %s", fileFullpath.c_str());
                        // LOG_DEBUG("#### exec:%s\n", tmpCmd.c_str());
                        system(tmpCmd.c_str());
                        raw_files.pop_back();

                        if (raw_files.size() == 0)
                        {
                            usleep(1000 * 200);
                            DIR* dir = opendir(g_capture_cache_dir.c_str());
                            struct dirent* dir_ent = NULL;
                            if (dir)
                            {
                                raw_files.clear();
                                while ((dir_ent = readdir(dir)))
                                {
                                    if (dir_ent->d_type == DT_REG)
                                    {
                                        // is raw file
                                        if (strstr(dir_ent->d_name, ".raw"))
                                        {
                                            raw_files.push_back(dir_ent->d_name);
                                        }
                                        // is raw file
                                        if (strstr(dir_ent->d_name, ".rkraw"))
                                        {
                                            raw_files.push_back(dir_ent->d_name);
                                        }
                                    }
                                }
                                closedir(dir);
                                if (raw_files.size() > 0)
                                {
                                    std::sort(raw_files.begin(), raw_files.end(), natural_more);
                                }
                            }
                        }
                        if (raw_files.size() > 0)
                        {
                            tmpRawFile = raw_files.back();
                        }
                        else
                        {
                            tmpRawFile = "";
                        }
                    }
                    else
                    {
                        LOG_DEBUG("RAW CAPTURED size < 0, remove\n");
                        int result = std::remove(fileFullpath.c_str());
                        if (result != 0)
                        {
                            LOG_DEBUG("remove file %s failed\n", fileFullpath);
                        }
                        raw_files.pop_back();
                        if (raw_files.size() == 0)
                        {
                            usleep(1000 * 200);
                            DIR* dir = opendir(g_capture_cache_dir.c_str());
                            struct dirent* dir_ent = NULL;
                            if (dir)
                            {
                                raw_files.clear();
                                while ((dir_ent = readdir(dir)))
                                {
                                    if (dir_ent->d_type == DT_REG)
                                    {
                                        // is raw file
                                        if (strstr(dir_ent->d_name, ".raw"))
                                        {
                                            raw_files.push_back(dir_ent->d_name);
                                        }
                                        // is raw file
                                        if (strstr(dir_ent->d_name, ".rkraw"))
                                        {
                                            raw_files.push_back(dir_ent->d_name);
                                        }
                                    }
                                }
                                closedir(dir);
                                if (raw_files.size() > 0)
                                {
                                    std::sort(raw_files.begin(), raw_files.end(), natural_more);
                                }
                            }
                        }
                        if (raw_files.size() > 0)
                        {
                            tmpRawFile = raw_files.back();
                        }
                        else
                        {
                            tmpRawFile = "";
                        }
                    }
                    fclose(f);
                }
            }
            LOG_DEBUG("DoCapture capture_frames_index/capture_frames:%d/%d\n", capture_frames_index, capture_frames);
            LOG_DEBUG("DoCapture raw_files.size():%d\n", raw_files.size());
            LOG_DEBUG("DoCapture send raw file process. end\n");
        }));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    //
    if (!get3AStatsMtx.try_lock_for(std::chrono::milliseconds(2000)))
    {
        LOG_DEBUG("get 3a lock fail 2\n");
        assert(NULL);
    }
    cap_info.ispStatsList.clear();
    cap_info_hdr2_1.ispStatsList.clear();
    get3AStatsMtx.unlock();

    if (g_sensorHdrMode == NO_HDR)
    {
        stop_capturing(&cap_info);
        uninit_device(&cap_info);
        RawCaptureDeinit(&cap_info);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        stop_capturing(&cap_info_hdr2_1);
        uninit_device(&cap_info_hdr2_1);
        RawCaptureDeinit(&cap_info_hdr2_1);

        stop_capturing(&cap_info_hdr2_2);
        uninit_device(&cap_info_hdr2_2);
        RawCaptureDeinit(&cap_info_hdr2_2);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        stop_capturing(&cap_info_hdr3_1);
        uninit_device(&cap_info_hdr3_1);
        RawCaptureDeinit(&cap_info_hdr3_1);

        stop_capturing(&cap_info_hdr3_2);
        uninit_device(&cap_info_hdr3_2);
        RawCaptureDeinit(&cap_info_hdr3_2);

        stop_capturing(&cap_info_hdr3_3);
        uninit_device(&cap_info_hdr3_3);
        RawCaptureDeinit(&cap_info_hdr3_3);
    }

    //
    if (g_compactModeFlag == 0 && g_sensorHdrMode == NO_HDR && g_sensorMemoryMode != -1)
    {
        int fd = open(cap_info.dev_name, O_RDWR, 0);
        LOG_INFO("fd: %d\n", fd);
        if (fd < 0)
        {
            LOG_ERROR("Open dev %s failed.\n", cap_info.dev_name);
        }
        else
        {
            int ret = ioctl(fd, RKCIF_CMD_SET_CSI_MEMORY_MODE, &g_sensorMemoryMode); // set to origional mode
            if (ret > 0)
            {
                LOG_ERROR("set cif node %s compact mode failed.\n", cap_info.dev_name);
            }
            else
            {
                LOG_ERROR("set cif node %s compact mode success.\n", cap_info.dev_name);
            }
        }

        // recover sync mode for dual camera
        int sensorfd = open(cap_info.sd_path.device_name, O_RDWR, 0);
        int ret = ioctl(sensorfd, RKMODULE_SET_SYNC_MODE, &g_sensorSyncMode); // recover sync mode
        if (ret > 0)
        {
            LOG_ERROR("set cif node %s sync mode failed.\n", cap_info.dev_name);
        }
        close(sensorfd);
    }

    return 0;
}

static void ReplyStatus(int sockfd, CommandData_t* cmd, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = DATA_ID_CAPTURE_RAW_STATUS; // ProcessID
    cmd->dat[1] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void ReplyOnlineRawStatus(int sockfd, CommandData_t* cmd, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_ONLINE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = DATA_ID_CAPTURE_ONLINE_RAW_STATUS; // ProcessID
    cmd->dat[1] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void ReplySensorPara(int sockfd, CommandData_t* cmd)
{
    char cmdResStr[2048] = {0};
    bool videoDevNodeFindedFlag = false;
    memset(captureDevNode, 0, sizeof(captureDevNode));

    if (g_stream_dev_index == -1)
    {
        if (g_stream_dev_name.length() > 0)
        {
            videoDevNodeFindedFlag = true;
            memcpy(captureDevNode, g_stream_dev_name.c_str(), g_stream_dev_name.length());
            LOG_INFO("DoCaptureYuv,using specific dev node:%s\n", captureDevNode);
        }
        else
        {
            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media0 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media1 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media2 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
        }
    }
    else
    {
        if (g_stream_dev_index == 0)
        {
            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media0 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media1 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media2 -e rkisp_mainpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
        }
        else if (g_stream_dev_index == 1)
        {
            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media0 -e rkisp_selfpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media1 -e rkisp_selfpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }

            if (videoDevNodeFindedFlag == false)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media2 -e rkisp_selfpath", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
        }
    }

    if (videoDevNodeFindedFlag == false)
    {
        // SendMessageToPC(sockfd, "Video capture device node not found");
        LOG_ERROR("Video capture device node not found.\n");
        return;
    }
    else
    {
        captureDevNode[strcspn(captureDevNode, "\n")] = '\0';
        LOG_DEBUG("Video capture device node:%s\n", captureDevNode);
    }
    if (strlen(captureDevNode) == 0)
    {
        // SendMessageToPC(sockfd, "Video capture device node not found");
        LOG_ERROR("Video capture device node not found.\n");
        return;
    }

    char tmpCmd[128] = {0};
    memset(tmpCmd, 0, sizeof(tmpCmd));
    sprintf(tmpCmd, "v4l2-ctl -d %s --set-fmt-video=width=%d,height=%d", captureDevNode, g_width, g_height);
    memset(cmdResStr, 0, sizeof(cmdResStr));
    if (g_stream_dev_index == -1)
    {
        ExecuteCMD(tmpCmd, cmdResStr);
    }

    memset(tmpCmd, 0, sizeof(tmpCmd));
    sprintf(tmpCmd, "v4l2-ctl -d %s --get-fmt-video", captureDevNode);
    memset(cmdResStr, 0, sizeof(cmdResStr));
    ExecuteCMD(tmpCmd, cmdResStr);

    //
    std::string srcStr = cmdResStr;
    std::smatch results;
    // get width / height
    std::string resolutionStrPattern{"Width/Height.*: (.*)/(.*)"};
    std::regex reResolution(resolutionStrPattern);
    results.empty();
    std::regex_search(srcStr, results, reResolution);
    assert(results.length() >= 2);
    string width = results.str(1);
    string height = results.str(2);
    if (g_stream_dev_name.length() == 0)
    {
        g_width = atoi(width.c_str());
        g_height = atoi(height.c_str());
    }
    if (g_width == 0 || g_height == 0)
    {
        // SendMessageToPC(sockfd, "get output resolution failed");
        LOG_ERROR("Captrure YUV, get output resolution failed.\n");
    }
    else
    {
        LOG_ERROR("Captrure YUV, get resolution %d x %d\n", g_width, g_height);
    }
    // get pixel format
    std::string pixelStrPattern{"Pixel Format.*'(.*)'"};
    std::regex rePixelStrPattern(pixelStrPattern);
    results.empty();
    std::regex_search(srcStr, results, rePixelStrPattern);
    assert(results.length() >= 2);
    string pixelFormat = results.str(1);
    if (pixelFormat.length() == 0)
    {
        // SendMessageToPC(sockfd, "get pixel format failed");
        LOG_ERROR("Captrure YUV, get pixel format failed.\n");
    }
    else
    {
        LOG_ERROR("Captrure YUV, get pixel format:%s.\n", pixelFormat.c_str());
    }

    char send_data[MAXPACKETSIZE];
    memset(cmd, 0, sizeof(CommandData_t));
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
    cmd->datLen = 3;
    Sensor_Yuv_Params_t* param = (Sensor_Yuv_Params_t*)(cmd->dat);
    param->data_id = DATA_ID_CAPTURE_YUV_GET_PARAM;
    param->width = g_width;
    param->height = g_height;
    if (pixelFormat == "YUYV")
    {
        param->format = RKISP_FORMAT_YUYV;
    }
    else
    {
        param->format = RKISP_FORMAT_NV12;
    }

    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void GetSensorHDRMode()
{
    char cmdResStr[2048] = {0};
    bool videoDevNodeFindedFlag = false;

    char tmpCmd[128] = {0};
    memset(tmpCmd, 0, sizeof(tmpCmd));
    sprintf(tmpCmd, "v4l2-ctl -d /dev/video0 --get-fmt-video");
    memset(cmdResStr, 0, sizeof(cmdResStr));
    ExecuteCMD(tmpCmd, cmdResStr);

    //
    std::string srcStr = cmdResStr;
    std::smatch results;
    // get width / height
    std::string resolutionStrPattern{"Width/Height.*: (.*)/(.*)"};
    std::regex reResolution(resolutionStrPattern);
    results.empty();
    std::regex_search(srcStr, results, reResolution);
    assert(results.length() >= 2);
    string width = results.str(1);
    string height = results.str(2);

    if (g_capture_dev_name.length() == 0)
    {
        g_width = atoi(width.c_str());
        g_height = atoi(height.c_str());
    }

    if (g_width == 0 || g_height == 0)
    {
        LOG_ERROR("Captrure online raw, get output resolution failed.\n");
    }
    else
    {
        LOG_ERROR("Captrure online raw, get resolution %d x %d\n", g_width, g_height);
    }

    // get pixel format
    std::string pixelStrPattern{"Pixel Format.*'(.*)'"};
    std::regex rePixelStrPattern(pixelStrPattern);
    results.empty();
    std::regex_search(srcStr, results, rePixelStrPattern);
    assert(results.length() >= 2);
    string pixelFormat = results.str(1);
    if (pixelFormat.length() == 0)
    {
        LOG_ERROR("Captrure online raw, get pixel format failed.\n");
    }
    else
    {
        LOG_ERROR("Captrure online raw, get pixel format:%s.\n", pixelFormat.c_str());
    }

    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
    //
    if (mi.cif.linked_sensor)
    {
        strcpy(cap_info.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
    }
    else if (mi.dvp.linked_sensor)
    {
        strcpy(cap_info.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
    }
    else
    {
        strcpy(cap_info.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
    }

    int fd = device_open(cap_info.sd_path.device_name);
    LOG_DEBUG("sensor subdev path: %s\n", cap_info.sd_path.device_name);
    LOG_DEBUG("cap_info.subdev_fd: %d\n", fd);
    if (fd < 0)
    {
        LOG_ERROR("Open %s failed.\n", cap_info.sd_path.device_name);
    }
    else
    {
        rkmodule_hdr_cfg hdrCfg;
        int ret = ioctl(fd, RKMODULE_GET_HDR_CFG, &hdrCfg);
        if (ret > 0)
        {
            g_sensorHdrMode = NO_HDR;
            LOG_ERROR("Get sensor hdr mode failed, use default, No HDR\n");
        }
        else
        {
            g_sensorHdrMode = hdrCfg.hdr_mode;
            LOG_INFO("Get sensor hdr mode:%u\n", g_sensorHdrMode);
            if (g_sensorHdrMode != HDR_X2 && g_sensorHdrMode != HDR_X3)
            {
                g_sensorHdrMode = NO_HDR;
                LOG_INFO("sensor hdr mode not in HDRx2 or HDRx3,set to no HDR:%u\n", g_sensorHdrMode);
            }
        }
    }
    close(fd);
    //
    //
    if (g_sensorHdrMode == NO_HDR)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info.link = link_to_vicap;
            strcpy(cap_info.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info.link = link_to_dvp;
            strcpy(cap_info.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else
        {
            cap_info.link = link_to_isp;
            strcpy(cap_info.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        strcpy(cap_info.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info.lens_path.lens_device_name[0] = '\0';
        }
        cap_info.dev_fd = -1;
        cap_info.subdev_fd = -1;
        cap_info.lensdev_fd = -1;
        LOG_DEBUG("cap_info.link: %d \n", cap_info.link);
        LOG_DEBUG("cap_info.dev_name: %s \n", cap_info.dev_name);
        LOG_DEBUG("cap_info.isp_media_path: %s \n", cap_info.vd_path.media_dev_path);
        LOG_DEBUG("cap_info.vd_path.isp_sd_path: %s \n", cap_info.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info.sd_path.device_name: %s \n", cap_info.sd_path.device_name);
        LOG_DEBUG("cap_info.lens_path.lens_dev_name: %s \n", cap_info.lens_path.lens_device_name);

        cap_info.io = IO_METHOD_MMAP;
        cap_info.height = g_height;
        cap_info.width = g_width;
        // cap_info.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info.width, cap_info.height);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr2_1.link = link_to_vicap;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr2_1.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info_hdr2_1.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr2_1.link = link_to_dvp;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_1.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr2_1.link = link_to_isp;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_1.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.vd_path.isp_main_path;
        }
        strcpy(cap_info_hdr2_1.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info_hdr2_1.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info_hdr2_1.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info_hdr2_1.lens_path.lens_device_name[0] = '\0';
        }
        cap_info_hdr2_1.dev_fd = -1;
        cap_info_hdr2_1.subdev_fd = -1;
        cap_info_hdr2_1.lensdev_fd = -1;
        LOG_DEBUG("cap_info_hdr2_1.link: %d \n", cap_info_hdr2_1.link);
        LOG_DEBUG("cap_info_hdr2_1.dev_name: %s \n", cap_info_hdr2_1.dev_name);
        LOG_DEBUG("cap_info_hdr2_1.isp_media_path: %s \n", cap_info_hdr2_1.vd_path.media_dev_path);
        LOG_DEBUG("cap_info_hdr2_1.vd_path.isp_sd_path: %s \n", cap_info_hdr2_1.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info_hdr2_1.sd_path.device_name: %s \n", cap_info_hdr2_1.sd_path.device_name);
        LOG_DEBUG("cap_info_hdr2_1.lens_path.lens_dev_name: %s \n", cap_info_hdr2_1.lens_path.lens_device_name);

        cap_info_hdr2_1.io = IO_METHOD_MMAP;
        cap_info_hdr2_1.height = g_height;
        cap_info_hdr2_1.width = g_width;
        // cap_info_hdr2_1.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info_hdr2_1.width, cap_info_hdr2_1.height);

        //
        //
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr2_2.link = link_to_vicap;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());

            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr2_2.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            else if (g_capture_dev_name.length() > 0)
            {
                std::string pattern{"/dev/video(.*)"};
                std::regex re(pattern);
                std::smatch results;
                std::string srcStr = g_capture_dev_name;
                std::string captureDevNodeStr;
                if (std::regex_search(srcStr, results, re)) // finded
                {
                    LOG_DEBUG("#### results: %s \n", results.str(1).c_str());
                    int devNodeNum = atoi(results.str(1).c_str()) + 1;
                    char buff[100];
                    snprintf(buff, sizeof(buff), "%s%d", "/dev/video", devNodeNum);
                    captureDevNodeStr = buff;
                }
                else
                {
                    LOG_DEBUG("#### not finded\n");
                }
                strcpy(cap_info_hdr2_2.cif_path.cif_video_path, captureDevNodeStr.c_str());
            }
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr2_2.link = link_to_dvp;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_2.cif_path.cif_video_path, mi.dvp.dvp_id1.c_str());
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr2_2.link = link_to_isp;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_2.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.vd_path.isp_main_path;
        }
        strcpy(cap_info_hdr2_2.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info_hdr2_2.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info_hdr2_2.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info_hdr2_2.lens_path.lens_device_name[0] = '\0';
        }
        cap_info_hdr2_2.dev_fd = -1;
        cap_info_hdr2_2.subdev_fd = -1;
        cap_info_hdr2_2.lensdev_fd = -1;
        LOG_DEBUG("cap_info_hdr2_2.link: %d \n", cap_info_hdr2_2.link);
        LOG_DEBUG("cap_info_hdr2_2.dev_name: %s \n", cap_info_hdr2_2.dev_name);
        LOG_DEBUG("cap_info_hdr2_2.isp_media_path: %s \n", cap_info_hdr2_2.vd_path.media_dev_path);
        LOG_DEBUG("cap_info_hdr2_2.vd_path.isp_sd_path: %s \n", cap_info_hdr2_2.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info_hdr2_2.sd_path.device_name: %s \n", cap_info_hdr2_2.sd_path.device_name);
        LOG_DEBUG("cap_info_hdr2_2.lens_path.lens_dev_name: %s \n", cap_info_hdr2_2.lens_path.lens_device_name);

        cap_info_hdr2_2.io = IO_METHOD_MMAP;
        cap_info_hdr2_2.height = g_height;
        cap_info_hdr2_2.width = g_width;
        // cap_info_hdr2_2.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info_hdr2_2.width, cap_info_hdr2_2.height);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_1.link = link_to_vicap;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr3_1.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info_hdr3_1.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_1.link = link_to_dvp;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_1.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_1.link = link_to_isp;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_1.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.vd_path.isp_main_path;
        }
        strcpy(cap_info_hdr3_1.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info_hdr3_1.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info_hdr3_1.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info_hdr3_1.lens_path.lens_device_name[0] = '\0';
        }
        cap_info_hdr3_1.dev_fd = -1;
        cap_info_hdr3_1.subdev_fd = -1;
        cap_info_hdr3_1.lensdev_fd = -1;
        LOG_DEBUG("cap_info_hdr3_1.link: %d \n", cap_info_hdr3_1.link);
        LOG_DEBUG("cap_info_hdr3_1.dev_name: %s \n", cap_info_hdr3_1.dev_name);
        LOG_DEBUG("cap_info_hdr3_1.isp_media_path: %s \n", cap_info_hdr3_1.vd_path.media_dev_path);
        LOG_DEBUG("cap_info_hdr3_1.vd_path.isp_sd_path: %s \n", cap_info_hdr3_1.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info_hdr3_1.sd_path.device_name: %s \n", cap_info_hdr3_1.sd_path.device_name);
        LOG_DEBUG("cap_info_hdr3_1.lens_path.lens_dev_name: %s \n", cap_info_hdr3_1.lens_path.lens_device_name);
        cap_info_hdr3_1.io = IO_METHOD_MMAP;
        cap_info_hdr3_1.height = g_height;
        cap_info_hdr3_1.width = g_width;
        // cap_info_hdr3_2.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info_hdr3_2.width, cap_info_hdr3_2.height);

        //
        //
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_2.link = link_to_vicap;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_2.link = link_to_dvp;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.cif_path.cif_video_path, mi.dvp.dvp_id1.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_2.link = link_to_isp;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.vd_path.isp_main_path;
        }
        strcpy(cap_info_hdr3_2.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info_hdr3_2.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info_hdr3_2.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info_hdr3_2.lens_path.lens_device_name[0] = '\0';
        }
        cap_info_hdr3_2.dev_fd = -1;
        cap_info_hdr3_2.subdev_fd = -1;
        cap_info_hdr3_2.lensdev_fd = -1;
        LOG_DEBUG("cap_info_hdr3_2.link: %d \n", cap_info_hdr3_2.link);
        LOG_DEBUG("cap_info_hdr3_2.dev_name: %s \n", cap_info_hdr3_2.dev_name);
        LOG_DEBUG("cap_info_hdr3_2.isp_media_path: %s \n", cap_info_hdr3_2.vd_path.media_dev_path);
        LOG_DEBUG("cap_info_hdr3_2.vd_path.isp_sd_path: %s \n", cap_info_hdr3_2.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info_hdr3_2.sd_path.device_name: %s \n", cap_info_hdr3_2.sd_path.device_name);
        LOG_DEBUG("cap_info_hdr3_2.lens_path.lens_dev_name: %s \n", cap_info_hdr3_2.lens_path.lens_device_name);
        cap_info_hdr3_2.io = IO_METHOD_MMAP;
        cap_info_hdr3_2.height = g_height;
        cap_info_hdr3_2.width = g_width;
        // cap_info_hdr3_2.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info_hdr3_2.width, cap_info_hdr3_2.height);

        //
        //
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_3.link = link_to_vicap;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.cif_path.cif_video_path, mi.cif.mipi_id2.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_3.link = link_to_dvp;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.cif_path.cif_video_path, mi.dvp.dvp_id2.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_3.link = link_to_isp;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.vd_path.isp_main_path;
        }
        strcpy(cap_info_hdr3_3.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
        strcpy(cap_info_hdr3_3.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
        if (mi.lens.module_lens_dev_name.length())
        {
            strcpy(cap_info_hdr3_3.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
        }
        else
        {
            cap_info_hdr3_3.lens_path.lens_device_name[0] = '\0';
        }
        cap_info_hdr3_3.dev_fd = -1;
        cap_info_hdr3_3.subdev_fd = -1;
        cap_info_hdr3_3.lensdev_fd = -1;
        LOG_DEBUG("cap_info_hdr3_3.link: %d \n", cap_info_hdr3_3.link);
        LOG_DEBUG("cap_info_hdr3_3.dev_name: %s \n", cap_info_hdr3_3.dev_name);
        LOG_DEBUG("cap_info_hdr3_3.isp_media_path: %s \n", cap_info_hdr3_3.vd_path.media_dev_path);
        LOG_DEBUG("cap_info_hdr3_3.vd_path.isp_sd_path: %s \n", cap_info_hdr3_3.vd_path.isp_sd_path);
        LOG_DEBUG("cap_info_hdr3_3.sd_path.device_name: %s \n", cap_info_hdr3_3.sd_path.device_name);
        LOG_DEBUG("cap_info_hdr3_3.lens_path.lens_dev_name: %s \n", cap_info_hdr3_3.lens_path.lens_device_name);
        cap_info_hdr3_3.io = IO_METHOD_MMAP;
        cap_info_hdr3_3.height = g_height;
        cap_info_hdr3_3.width = g_width;
        // cap_info_hdr3_3.format = v4l2_fourcc('B', 'G', '1', '2');
        LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info_hdr3_3.width, cap_info_hdr3_3.height);
    }
}

static void InitOnlineRawCapture(struct capture_info* cap_info)
{
    // Get Sensor Para
    struct v4l2_queryctrl ctrl;
    struct v4l2_subdev_frame_interval finterval;
    struct v4l2_subdev_format fmt;
    struct v4l2_format format;

    int hblank, vblank;
    int vts, hts, ret;
    float fps;
    int endianness = 0;

    cap_info->subdev_fd = device_open(cap_info->sd_path.device_name);
    LOG_DEBUG("sensor subdev path: %s\n", cap_info->sd_path.device_name);

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (device_getblank(cap_info->subdev_fd, &ctrl) < 0)
    {
        goto end;
    }
    hblank = ctrl.minimum;
    LOG_DEBUG("get hblank: %d\n", hblank);

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (device_getblank(cap_info->subdev_fd, &ctrl) < 0)
    {
        goto end;
    }
    vblank = ctrl.minimum;
    LOG_DEBUG("get vblank: %d\n", vblank);

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    if (device_getsubdevformat(cap_info->subdev_fd, &fmt) < 0)
    {
        goto end;
    }
    vts = vblank + fmt.format.height;
    hts = hblank + fmt.format.width;
    LOG_DEBUG("get hts: %d  vts: %d\n", hts, vts);
    cap_info->format = convert_to_v4l2fmt(cap_info, fmt.format.code);
    cap_info->sd_path.sen_fmt = fmt.format.code;
    cap_info->sd_path.width = fmt.format.width;
    cap_info->sd_path.height = fmt.format.height;

    LOG_DEBUG("get sensor code: %d  bits: %d, cap_info->format:  %d\n", cap_info->sd_path.sen_fmt, cap_info->sd_path.bits, cap_info->format);

    /* set isp subdev fmt to bayer raw*/
    if (cap_info->link == link_to_isp)
    {
        ret = rkisp_set_ispsd_fmt(cap_info, fmt.format.width, fmt.format.height, fmt.format.code, cap_info->width, cap_info->height, fmt.format.code);
        endianness = 1;
        LOG_DEBUG("rkisp_set_ispsd_fmt: %d endianness = %d\n", ret, endianness);

        if (ret)
        {
            LOG_ERROR("subdev choose the best fit fmt: %dx%d, 0x%08x\n", fmt.format.width, fmt.format.height, fmt.format.code);
            goto end;
        }
    }

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;
    if (device_getsensorfps(cap_info->subdev_fd, &finterval) < 0)
    {
        goto end;
    }
    fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
    LOG_DEBUG("get fps: %f\n", fps);

    if (cap_info->subdev_fd > 0)
    {
        device_close(cap_info->subdev_fd);
        cap_info->subdev_fd = -1;
    }
    if (cap_info->dev_fd > 0)
    {
        device_close(cap_info->dev_fd);
        cap_info->dev_fd = -1;
    }
    return;

end:
    if (cap_info->subdev_fd > 0)
    {
        device_close(cap_info->subdev_fd);
        cap_info->subdev_fd = -1;
    }
}

static void ReplyOnlineRawSensorPara(int sockfd, CommandData_t* cmd)
{
    char cmdResStr[2048] = {0};
    bool videoDevNodeFindedFlag = false;
    memset(captureDevNode, 0, sizeof(captureDevNode));

    if (videoDevNodeFindedFlag == false)
    {
        if (g_capture_dev_name.length() == 0)
        {
            if (g_device_id == 0)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media0 -e stream_cif_mipi_id0", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
            else if (g_device_id == 1)
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media1 -e stream_cif_mipi_id0", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
            else
            {
                memset(cmdResStr, 0, sizeof(cmdResStr));
                ExecuteCMD("media-ctl -d /dev/media0 -e stream_cif_mipi_id0", cmdResStr);
                if (strstr(cmdResStr, "/dev/video") != NULL)
                {
                    videoDevNodeFindedFlag = true;
                    memcpy(captureDevNode, cmdResStr, strlen(cmdResStr) + 1);
                }
            }
        }
        else
        {
            videoDevNodeFindedFlag = true;
            memcpy(captureDevNode, g_capture_dev_name.c_str(), g_capture_dev_name.length());
        }
    }

    if (videoDevNodeFindedFlag == false)
    {
        // SendMessageToPC(sockfd, "Video capture device node not found");
        LOG_ERROR("Video capture device node not found.\n");
        return;
    }
    else
    {
        captureDevNode[strcspn(captureDevNode, "\n")] = '\0';
        LOG_DEBUG("Video capture device node:%s\n", captureDevNode);
    }
    if (strlen(captureDevNode) == 0)
    {
        // SendMessageToPC(sockfd, "Video capture device node not found");
        LOG_ERROR("Video capture device node not found.\n");
        return;
    }

    if (g_compactModeFlag == 0)
    {
        int fd = open(cap_info.dev_name, O_RDWR, 0);
        LOG_INFO("fd: %d\n", fd);
        if (fd < 0)
        {
            LOG_ERROR("Open dev %s failed.\n", cap_info.dev_name);
        }
        else
        {
            int ret = ioctl(fd, RKCIF_CMD_GET_CSI_MEMORY_MODE, &g_sensorMemoryMode); // get original memory mode
            if (ret > 0)
            {
                LOG_ERROR("get cif node %s memory mode failed.\n", cap_info.dev_name);
            }

            //
            int value = CSI_LVDS_MEM_WORD_LOW_ALIGN;
            ret = ioctl(fd, RKCIF_CMD_SET_CSI_MEMORY_MODE, &value); // set to no compact
            if (ret > 0)
            {
                LOG_ERROR("set cif node %s compact mode failed.\n", cap_info.dev_name);
            }
        }

        // set sync mode to no sync for dual camera
        int sensorfd = open(cap_info.sd_path.device_name, O_RDWR, 0);
        int ret = ioctl(sensorfd, RKMODULE_GET_SYNC_MODE, &g_sensorSyncMode);
        if (ret > 0)
        {
            LOG_ERROR("get cif node %s sync mode failed.\n", cap_info.dev_name);
        }
        int value = NO_SYNC_MODE;
        ret = ioctl(sensorfd, RKMODULE_SET_SYNC_MODE, &value); // set to no sync
        if (ret > 0)
        {
            LOG_ERROR("set cif node %s sync mode failed.\n", cap_info.dev_name);
        }
        close(sensorfd);
    }

    //
    if (g_sensorHdrMode == NO_HDR)
    {
        InitOnlineRawCapture(&cap_info);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        InitOnlineRawCapture(&cap_info_hdr2_1);
        InitOnlineRawCapture(&cap_info_hdr2_2);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        InitOnlineRawCapture(&cap_info_hdr3_1);
        InitOnlineRawCapture(&cap_info_hdr3_2);
        InitOnlineRawCapture(&cap_info_hdr3_3);
    }

    char send_data[MAXPACKETSIZE];
    memset(cmd, 0, sizeof(CommandData_t));
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = CMD_ID_CAPTURE_ONLINE_RAW_CAPTURE;
    cmd->datLen = 3;
    Sensor_Online_Raw_Params_t* param = (Sensor_Online_Raw_Params_t*)(cmd->dat);
    param->data_id = DATA_ID_CAPTURE_ONLINE_RAW_GET_PARAM;
    param->width = g_width;
    param->height = g_height;

    if (g_sensorHdrMode == NO_HDR)
    {
        param->bits = cap_info.sd_path.bits;
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        param->bits = cap_info_hdr2_1.sd_path.bits;
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        param->bits = cap_info_hdr3_1.sd_path.bits;
    }

    uint16_t sensorFormat;
    // set capture image data format
    if (g_sensorHdrMode == NO_HDR)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_NON_COMPACT_LINEAR;
        LOG_INFO("NO_HDR | sensorFormat:%d\n", sensorFormat);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN256;
        LOG_INFO("HDR_X2 | sensorFormat:%d\n", sensorFormat);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN256;
        LOG_INFO("HDR_X3 | sensorFormat:%d\n", sensorFormat);
    }
    else
    {
        LOG_ERROR("Get sensor hdr mode failed, hdr mode:%u, use default.No HDR\n", g_sensorHdrMode);
        sensorFormat = PROC_ID_CAPTURE_RAW_NON_COMPACT_LINEAR;
        LOG_INFO("NO_HDR | sensorFormat:%d\n", sensorFormat);
    }

    if (g_capture_dev_name.length() > 0)
    {
        if (g_sensorHdrMode == HDR_X2)
        {
            param->sensorImageFormat = PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN256;
        }
        else if (g_compactModeFlag == 1)
        {
            param->sensorImageFormat = PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN256;
        }
        else
        {
            param->sensorImageFormat = sensorFormat;
        }
    }
    else
    {
        param->sensorImageFormat = sensorFormat;
    }

    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void SetSensorPara(int sockfd, CommandData_t* recv_cmd, CommandData_t* cmd)
{
    char send_data[MAXPACKETSIZE];

    Capture_Yuv_Params_t* CapParam = (Capture_Yuv_Params_t*)(recv_cmd->dat + 1);
    LOG_DEBUG(" set gain        : %d\n", CapParam->gain);
    LOG_DEBUG(" set exposure    : %d\n", CapParam->time);
    LOG_DEBUG(" set fmt        : %d\n", CapParam->fmt);
    LOG_DEBUG(" set framenumber : %d\n", CapParam->framenumber);

    capture_frames = CapParam->framenumber;
    capture_frames_index = 0;
    capture_check_sum = 0;

    memset(cmd, 0, sizeof(CommandData_t));
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = CMD_ID_CAPTURE_YUV_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = DATA_ID_CAPTURE_RAW_SET_PARAM;
    cmd->dat[1] = RES_SUCCESS;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

static void SetOnlineRawSensorPara(int sockfd, CommandData_t* recv_cmd, CommandData_t* cmd)
{
    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);

    if (g_sensorHdrMode == NO_HDR)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info.link = link_to_vicap;
            strcpy(cap_info.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info.link = link_to_dvp;
            strcpy(cap_info.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else
        {
            cap_info.link = link_to_isp;
            strcpy(cap_info.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info.dev_name = cap_info.vd_path.isp_main_path;
        }
        cap_info.io = IO_METHOD_MMAP;
        init_device(&cap_info);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr2_1.link = link_to_vicap;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr2_1.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info_hdr2_1.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr2_1.link = link_to_dvp;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_1.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr2_1.link = link_to_isp;
            strcpy(cap_info_hdr2_1.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_1.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr2_1.dev_name = cap_info_hdr2_1.vd_path.isp_main_path;
        }
        cap_info_hdr2_1.io = IO_METHOD_MMAP;
        init_device(&cap_info_hdr2_1);

        if (mi.cif.linked_sensor)
        {
            cap_info_hdr2_2.link = link_to_vicap;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());

            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr2_2.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            else if (g_capture_dev_name.length() > 0)
            {
                std::string pattern{"/dev/video(.*)"};
                std::regex re(pattern);
                std::smatch results;
                std::string srcStr = g_capture_dev_name;
                std::string captureDevNodeStr;
                if (std::regex_search(srcStr, results, re)) // finded
                {
                    LOG_DEBUG("#### results: %s \n", results.str(1).c_str());
                    int devNodeNum = atoi(results.str(1).c_str()) + 1;
                    char buff[100];
                    snprintf(buff, sizeof(buff), "%s%d", "/dev/video", devNodeNum);
                    captureDevNodeStr = buff;
                }
                else
                {
                    LOG_DEBUG("#### not finded\n");
                }
                strcpy(cap_info_hdr2_2.cif_path.cif_video_path, captureDevNodeStr.c_str());
            }
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr2_2.link = link_to_dvp;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_2.cif_path.cif_video_path, mi.dvp.dvp_id1.c_str());
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr2_2.link = link_to_isp;
            strcpy(cap_info_hdr2_2.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr2_2.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr2_2.dev_name = cap_info_hdr2_2.vd_path.isp_main_path;
        }
        cap_info_hdr2_2.io = IO_METHOD_MMAP;
        init_device(&cap_info_hdr2_2);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_1.link = link_to_vicap;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            if (g_capture_dev_name.length() == 0)
                strcpy(cap_info_hdr3_1.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            else if (g_capture_dev_name.length() > 0)
                strcpy(cap_info_hdr3_1.cif_path.cif_video_path, g_capture_dev_name.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_1.link = link_to_dvp;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_1.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_1.link = link_to_isp;
            strcpy(cap_info_hdr3_1.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_1.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_1.dev_name = cap_info_hdr3_1.vd_path.isp_main_path;
        }
        cap_info_hdr3_1.io = IO_METHOD_MMAP;
        init_device(&cap_info_hdr3_1);

        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_2.link = link_to_vicap;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_2.link = link_to_dvp;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.cif_path.cif_video_path, mi.dvp.dvp_id1.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_2.link = link_to_isp;
            strcpy(cap_info_hdr3_2.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_2.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_2.dev_name = cap_info_hdr3_2.vd_path.isp_main_path;
        }
        cap_info_hdr3_2.io = IO_METHOD_MMAP;
        init_device(&cap_info_hdr3_2);

        if (mi.cif.linked_sensor)
        {
            cap_info_hdr3_3.link = link_to_vicap;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.cif_path.cif_video_path, mi.cif.mipi_id2.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.cif_path.cif_video_path;
        }
        else if (mi.dvp.linked_sensor)
        {
            cap_info_hdr3_3.link = link_to_dvp;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.cif_path.cif_video_path, mi.dvp.dvp_id2.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.cif_path.cif_video_path;
        }
        else
        {
            cap_info_hdr3_3.link = link_to_isp;
            strcpy(cap_info_hdr3_3.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
            strcpy(cap_info_hdr3_3.vd_path.isp_main_path, mi.isp.main_path.c_str());
            cap_info_hdr3_3.dev_name = cap_info_hdr3_3.vd_path.isp_main_path;
        }
        cap_info_hdr3_3.io = IO_METHOD_MMAP;
        init_device(&cap_info_hdr3_3);
    }

    char send_data[MAXPACKETSIZE];

    Capture_Yuv_Params_t* CapParam = (Capture_Yuv_Params_t*)(recv_cmd->dat + 1);
    // LOG_DEBUG(" set gain        : %d\n", CapParam->gain);
    // LOG_DEBUG(" set exposure    : %d\n", CapParam->time);
    // LOG_DEBUG(" set fmt        : %d\n", CapParam->fmt);
    LOG_DEBUG(" set framenumber : %d\n", CapParam->framenumber);

    capture_frames = CapParam->framenumber;
    capture_frames_index = 0;
    capture_check_sum = 0;

    memset(cmd, 0, sizeof(CommandData_t));
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = CMD_ID_CAPTURE_ONLINE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = DATA_ID_CAPTURE_ONLINE_RAW_SET_PARAM;
    cmd->dat[1] = RES_SUCCESS;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
}

int Send(int sockfd, char* buffer, int size)
{
    int ret = -1;
    int sendOffset = 0;
    auto stopSendTimer = std::chrono::high_resolution_clock::now();
    while (true)
    {
        int sendSize = send(sockfd, buffer + sendOffset, size, 0);
        if (sendSize > 0)
        {
            stopSendTimer = std::chrono::high_resolution_clock::now();
            sendOffset += sendSize;
        }
        else if (sendSize <= 0)
        {
            if (errno != EAGAIN)
            {
                LOG_DEBUG("errno != EAGAIN skip close sockfd\n");
                // close(sockfd);
                break;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopSendTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(200))
                {
                    break;
                }
            }
        }
        if (sendOffset >= size)
        {
            LOG_DEBUG("tcp send to pc finish. send/size: %d/%d\n", sendOffset, size);
            ret = sendOffset;
            break;
        }
    }
    return ret;
}

int Receive(int sockfd, char* buffer, int size)
{
    int ret = -1;
    int receiveOffset = 0;
    auto stopReceiveTimer = std::chrono::high_resolution_clock::now();
    while (true)
    {
        int recvSize = recv(sockfd, buffer + receiveOffset, size, 0);
        if (recvSize > 0)
        {
            stopReceiveTimer = std::chrono::high_resolution_clock::now();
            receiveOffset += recvSize;
        }
        else if (recvSize <= 0)
        {
            if (errno != EAGAIN)
            {
                LOG_DEBUG("errno != EAGAIN, skip close sockfd\n");
                // close(sockfd);
                break;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopReceiveTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(50))
                {
                    ret = receiveOffset;
                    break;
                }
            }
        }
        if (receiveOffset >= size)
        {
            LOG_DEBUG("tcp recv finish. receive/size: %d/%d\n", receiveOffset, size);
            ret = receiveOffset;
            break;
        }
    }

    return ret;
}

void RKAiqOLProtocol::HandlerOnLineMessage(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    int ret_val, ret;

    if (strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) != 0)
    {
        LOG_DEBUG("RKID: Unknown\n");
        return;
    }

    switch (common_cmd->cmdType)
    {
        case CMD_TYPE_JSON_WRITE: {
            printf("CMD_TYPE_JSON_WRITE\n");
            JsonRW_Dat_t* dat = (JsonRW_Dat_t*)common_cmd->dat;
            uint32_t uuid = dat->UUID;
            uint32_t jsonDataLen = dat->jsonDataLen;
            uint32_t crc = dat->crc32;
            uint16_t camID = dat->CamID;
            // LOG_DEBUG("uuid: %d, jsonDataLen: %d, crc: %d, camID: %d\n", uuid, jsonDataLen, crc, camID);
            //
            char* tmpBuffer = (char*)malloc(jsonDataLen);
            if (tmpBuffer == NULL)
            {
                LOG_ERROR("Receive json data from pc malloc failed\n");
                free(tmpBuffer);
                tmpBuffer = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_UNKNOWN_ERROR;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            int ret = Receive(sockfd, tmpBuffer, jsonDataLen);
            if (ret < 0)
            {
                LOG_ERROR("Receive json data from pc failed\n");
                JsonWriteAnswer answer;
                answer.writeErrorCode = 0x00E1;
                DoAiqJsonWriteAnswer(sockfd, &send_cmd, common_cmd->cmdID, answer);
                free(tmpBuffer);
                tmpBuffer = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            // aiq交互write
            // typedef struct RkAiqSocketDataV2_Write
            // {
            //     uint8_t magic[4]; // = { 'R', 0xAA, 0xFF, 'K' };
            //     int32_t cmd_id;
            //     int32_t cmd_ret;
            //     uint32_t sequence;
            //     uint32_t payload_size;
            //     uint32_t packet_size;
            //     uint8_t* data;
            //     uint32_t data_hash;
            // } RkAiqSocketDataV2_Write;
            uint32_t offset = 0;
            uint32_t dataToSendSize = jsonDataLen + sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*);
            uint8_t* dataToSend = (unsigned char*)malloc(dataToSendSize);
            if (dataToSend == NULL)
            {
                LOG_ERROR("malloc failed\n");
                free(dataToSend);
                dataToSend = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_UNKNOWN_ERROR;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            RkAiqSocketDataV2_Write tempData;
            tempData.magic[0] = 'R';
            tempData.magic[1] = 0xAA;
            tempData.magic[2] = 0xFF;
            tempData.magic[3] = 'K';
            tempData.cmd_id = 0;
            tempData.cmd_ret = 0;
            tempData.sequence = 0;
            tempData.payload_size = jsonDataLen;
            tempData.packet_size = dataToSendSize;
            memcpy(dataToSend + offset, &tempData, sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*) - sizeof(uint32_t));

            offset += sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*) - sizeof(uint32_t);
            // json
            memcpy(dataToSend + offset, tmpBuffer, jsonDataLen);
            // data hash 好像没有实际使用
            uint32_t check_sum = 0;
            for (int i = 0; i < jsonDataLen; i++)
            {
                check_sum += tmpBuffer[i];
            }
            offset += jsonDataLen;
            memcpy(dataToSend + offset, &check_sum, sizeof(uint32_t));

            ret = Send(g_domainTcpClient.sock, (char*)dataToSend, dataToSendSize);

            free(dataToSend);
            dataToSend = NULL;
            if (ret < 0 && errno != EINTR)
            {
                if (ConnectAiq() < 0)
                {
                    g_domainTcpClient.Close();
                    LOG_ERROR("########################################################\n");
                    LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
                    LOG_ERROR("########################################################\n\n");
                    close(sockfd);
                    free(tmpBuffer);
                    tmpBuffer = NULL;

                    LOG_DEBUG("prepare annswer pc\n");
                    JsonReadAns_Dat_t readAnserDat;
                    readAnserDat.readErrorCode = MSG_READ_AIQ_NOT_CONNECTED;
                    readAnserDat.aiqErrorCode = IPC_RET_OK;
                    memset(&send_cmd, 0, sizeof(CommandData_t));
                    strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                    send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                    strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                    send_cmd.datLen = sizeof(readAnserDat);
                    memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                    send_cmd.checkSum = 0;
                    for (int i = 0; i < send_cmd.datLen; i++)
                    {
                        send_cmd.checkSum += send_cmd.dat[i];
                    }
                    send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                    LOG_DEBUG("answer pc\n");
                    return;
                }
                else
                {
                    LOG_ERROR("########################################################\n");
                    LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
                    LOG_ERROR("########################################################\n\n");
                    Send(g_domainTcpClient.sock, tmpBuffer, jsonDataLen);
                }
            }

            free(tmpBuffer);
            tmpBuffer = NULL;

            // 从aiq收返回的数据
            LOG_DEBUG("begin to receive from aiq\n");
            RkAiqSocketDataV2_Receive rkAiqSocketDataV2_Receive;
            int firstPackReceived = 0;
            firstPackReceived = Receive(g_domainTcpClient.sock, (char*)&rkAiqSocketDataV2_Receive, sizeof(RkAiqSocketDataV2_Receive) - sizeof(uint8_t*) - sizeof(uint32_t));
            if (firstPackReceived <= 0)
            {
                LOG_ERROR("first pack receive failed\n");
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            if (rkAiqSocketDataV2_Receive.magic[0] != 'R' || rkAiqSocketDataV2_Receive.magic[1] != 0xAA || rkAiqSocketDataV2_Receive.magic[2] != 0xFF || rkAiqSocketDataV2_Receive.magic[3] != 'K')
            {
                LOG_ERROR("get magic failed\n");
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            int readTotalSize = rkAiqSocketDataV2_Receive.packet_size;
            if (readTotalSize <= 0)
            {
                LOG_ERROR("get readTotalSize failed\n");
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            int readOffset = 0;
            char* readTotalBuffer = (char*)malloc(readTotalSize);
            memcpy(readTotalBuffer, &rkAiqSocketDataV2_Receive, firstPackReceived);
            readOffset += firstPackReceived;
            while (readOffset < readTotalSize)
            {
                int recvCount = recv(g_domainTcpClient.sock, readTotalBuffer + readOffset, readTotalSize - readOffset, 0);
                if (recvCount <= 0)
                {
                    break;
                }
                readOffset += recvCount;
                if (readOffset + 1 >= readTotalSize)
                {
                    break;
                }
            }

            //从aiq获取的数据解析校验
            uint8_t magic[4] = {0};
            memcpy(&magic, ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->magic, 4);
            int32_t cmd_id = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->cmd_id;
            int32_t cmd_ret = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->cmd_ret;
            uint32_t sequence = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->sequence;
            // uint32_t payload_size = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->payload_size;
            // uint32_t packet_size = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->packet_size;
            uint8_t aiqRetNote[128];
            memcpy(aiqRetNote, ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->note, 128);
            if (magic[0] != 'R' || magic[1] != 0xAA || magic[2] != 0xFF || magic[3] != 'K')
            {
                LOG_ERROR("get magic failed\n");
                free(readTotalBuffer);
                readTotalBuffer = NULL;
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_WRITE;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            LOG_DEBUG("cmd_id:%d, cmd_ret:0x%X, sequence:%u\n", cmd_id, cmd_ret, sequence);

            // 发回给PC的反馈
            JsonWriteAnswer answer;
            answer.writeErrorCode = 0x00E0; // test return value, always success
            answer.aiqErrorCode = cmd_ret;  // aiq return value
            answer.aiqErrorDataLen = 128;
            uint32_t aiqErrorDataCrc = GetCRC32((unsigned char*)aiqRetNote, 128);
            answer.aiqErrorDataCrc = aiqErrorDataCrc;
            //
            DoAiqJsonWriteAnswer(sockfd, &send_cmd, common_cmd->cmdID, answer);
            LOG_DEBUG("Answer pc\n");
            usleep(1000 * 20);
            // send aiq note
            send(sockfd, aiqRetNote, 128, 0);
            LOG_DEBUG("Answer pc aiq note\n");
        }
        break;
        case CMD_TYPE_JSON_READ: {
            printf("CMD_TYPE_JSON_READ\n");
            JsonRW_Dat_t* dat = (JsonRW_Dat_t*)common_cmd->dat;
            uint32_t uuid = dat->UUID;
            uint32_t jsonDataLen = dat->jsonDataLen;
            uint32_t crc = dat->crc32;
            uint16_t camID = dat->CamID;
            //
            char* jsonDataBuffer = (char*)malloc(jsonDataLen);
            if (jsonDataBuffer == NULL)
            {
                LOG_ERROR("Receive json data from pc malloc failed\n");
                free(jsonDataBuffer);
                jsonDataBuffer = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_UNKNOWN_ERROR;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            int ret = Receive(sockfd, jsonDataBuffer, jsonDataLen);
            if (ret < 0)
            {
                LOG_ERROR("Receive json data from pc failed\n");
                DoAiqJsonReadAnswer1(sockfd, &send_cmd, common_cmd->cmdID, ret);
                free(jsonDataBuffer);
                jsonDataBuffer = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            // aiq交互read
            //组包
            size_t offset = 0;
            size_t dataToSendSize = jsonDataLen + sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*);
            uint8_t* dataToSend = (unsigned char*)malloc(dataToSendSize);
            if (dataToSend == NULL)
            {
                LOG_ERROR("malloc failed\n");
                free(dataToSend);
                dataToSend = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_UNKNOWN_ERROR;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            RkAiqSocketDataV2_Write tempData;
            tempData.magic[0] = 'R';
            tempData.magic[1] = 0xAA;
            tempData.magic[2] = 0xFF;
            tempData.magic[3] = 'K';
            tempData.cmd_id = 1;
            tempData.cmd_ret = 1;
            tempData.sequence = 1;
            tempData.payload_size = jsonDataLen;
            tempData.packet_size = dataToSendSize;
            memcpy(dataToSend + offset, &tempData, sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*) - sizeof(uint32_t));
            offset += sizeof(RkAiqSocketDataV2_Write) - sizeof(uint8_t*) - sizeof(uint32_t);
            // json
            memcpy(dataToSend + offset, jsonDataBuffer, jsonDataLen);
            // data hash 好像没有实际使用
            offset += jsonDataLen;
            memcpy(dataToSend + offset, &tempData.data_hash, sizeof(uint32_t));

            ret = Send(g_domainTcpClient.sock, (char*)dataToSend, dataToSendSize);

            free(jsonDataBuffer);
            jsonDataBuffer = NULL;

            /*
            // typedef struct RkAiqSocketDataV2_Receive
            // {
            //     uint8_t magic[4]; // = { 'R', 0xAA, 0xFF, 'K' };
            //     int32_t cmd_id;
            //     int32_t cmd_ret;
            //     uint32_t sequence;
            //     uint32_t payload_size;
            //     uint32_t packet_size;
            //     uint8_t note[128];
            //     uint8_t* data;
            //     uint32_t data_hash;
            // } RkAiqSocketDataV2;
            */
            //从aiq收数据，收完再转发给PC
            RkAiqSocketDataV2_Receive rkAiqSocketDataV2_Receive;
            int firstPackReceived = 0;
            firstPackReceived = Receive(g_domainTcpClient.sock, (char*)&rkAiqSocketDataV2_Receive, sizeof(RkAiqSocketDataV2_Receive) - sizeof(uint8_t*) - sizeof(uint32_t));
            if (firstPackReceived <= 0)
            {
                LOG_ERROR("first pack receive failed\n");

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_AIQ_ERROR;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            if (rkAiqSocketDataV2_Receive.magic[0] != 'R' || rkAiqSocketDataV2_Receive.magic[1] != 0xAA || rkAiqSocketDataV2_Receive.magic[2] != 0xFF || rkAiqSocketDataV2_Receive.magic[3] != 'K')
            {
                LOG_ERROR("get magic failed\n");
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            int readTotalSize = rkAiqSocketDataV2_Receive.packet_size;
            if (readTotalSize <= 0)
            {
                LOG_ERROR("get readTotalSize failed\n");
                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }

            int readOffset = 0;
            char* readTotalBuffer = (char*)malloc(readTotalSize);
            memcpy(readTotalBuffer, &rkAiqSocketDataV2_Receive, firstPackReceived);
            readOffset += firstPackReceived;
            while (readOffset < readTotalSize)
            {
                int recvCount = recv(g_domainTcpClient.sock, readTotalBuffer + readOffset, readTotalSize - readOffset, 0);
                if (recvCount <= 0)
                {
                    break;
                }
                readOffset += recvCount;
                if (readOffset + 1 >= readTotalSize)
                {
                    break;
                }
            }

            //从aiq获取的数据解析校验
            uint8_t magic[4] = {0};
            memcpy(&magic, ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->magic, 4);
            int32_t cmd_id = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->cmd_id;
            int32_t cmd_ret = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->cmd_ret;
            uint32_t sequence = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->sequence;
            uint32_t payload_size = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->payload_size;
            uint32_t packet_size = ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->packet_size;
            uint8_t aiqRetNote[128];
            memcpy(aiqRetNote, ((RkAiqSocketDataV2_Receive*)readTotalBuffer)->note, 128);
            if (magic[0] != 'R' || magic[1] != 0xAA || magic[2] != 0xFF || magic[3] != 'K')
            {
                LOG_ERROR("get magic failed\n");
                free(readTotalBuffer);
                readTotalBuffer = NULL;

                LOG_DEBUG("prepare annswer pc\n");
                JsonReadAns_Dat_t readAnserDat;
                readAnserDat.readErrorCode = MSG_READ_JSON_NOT_COMPLETE;
                readAnserDat.aiqErrorCode = IPC_RET_OK;
                memset(&send_cmd, 0, sizeof(CommandData_t));
                strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
                send_cmd.cmdType = CMD_TYPE_JSON_READ;
                strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
                send_cmd.datLen = sizeof(readAnserDat);
                memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

                send_cmd.checkSum = 0;
                for (int i = 0; i < send_cmd.datLen; i++)
                {
                    send_cmd.checkSum += send_cmd.dat[i];
                }
                send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
                LOG_DEBUG("answer pc\n");
                return;
            }
            else
            {
                LOG_DEBUG("get magic success\n");
            }
            uint32_t data_hash = *(uint32_t*)(&readTotalBuffer[readTotalSize - sizeof(uint32_t)]);
            LOG_DEBUG("cmd_id:%d, cmd_ret:%d, sequence:%d, payload_size:%d, packet_size:%d, data_hash:%d\n", cmd_id, cmd_ret, sequence, payload_size, packet_size, data_hash);

            //计算数据crc32, 包括json和aiq note
            uint8_t* dataToCrc = (uint8_t*)malloc(payload_size + 128);
            memcpy(dataToCrc, readTotalBuffer + firstPackReceived, payload_size);
            memcpy(dataToCrc + payload_size, aiqRetNote, 128);
            uint32_t crc32 = GetCRC32(dataToCrc, payload_size + 128);
            free(dataToCrc);
            dataToCrc = NULL;

            LOG_DEBUG("crc32:%u ,data_hash:%u\n", crc32, data_hash);

            // toolserver与pc交互，answer
            LOG_DEBUG("prepare annswer pc 1\n");
            JsonReadAns_Dat_t readAnserDat;
            readAnserDat.readErrorCode = 0x00E0;
            readAnserDat.aiqErrorCode = cmd_ret;
            readAnserDat.jsonDataLen = payload_size;
            readAnserDat.crc32 = crc32; // checksum
            //
            memset(&send_cmd, 0, sizeof(CommandData_t));
            strncpy((char*)send_cmd.RKID, RKID_ISP_ON, sizeof(send_cmd.RKID));
            send_cmd.cmdType = CMD_TYPE_JSON_READ;
            strncpy((char*)send_cmd.version, RKAIQ_TOOL_VERSION, sizeof(send_cmd.version));
            send_cmd.datLen = sizeof(readAnserDat);
            memcpy(send_cmd.dat, &readAnserDat, send_cmd.datLen);

            send_cmd.checkSum = 0;
            for (int i = 0; i < send_cmd.datLen; i++)
            {
                send_cmd.checkSum += send_cmd.dat[i];
            }
            send(sockfd, (void*)&send_cmd, sizeof(CommandData_t), 0);
            LOG_DEBUG("answer pc 1\n");

            usleep(1000 * 20);
            ret = Send(sockfd, (char*)(readTotalBuffer + firstPackReceived), payload_size);
            LOG_DEBUG("answer pc 2\n");
            usleep(1000 * 20);

            ret = Send(sockfd, (char*)(readTotalBuffer + firstPackReceived - 128), 128);
            LOG_DEBUG("answer pc 3, aiq note\n");

            free(readTotalBuffer);
            readTotalBuffer = NULL;
        }
        break;
        case CMD_TYPE_STREAMING: {
            RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_TUNRING);
            if (common_cmd->cmdID == 0xffff)
            {
                uint16_t check_sum;
                uint32_t result;
                DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
                OnLineSet(sockfd, common_cmd, check_sum, result);
                DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum, result ? RES_FAILED : RES_SUCCESS);
            }
        }
        break;
        case CMD_TYPE_STATUS: {
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
        }
        break;
        case CMD_TYPE_UAPI_SET: {
            uint16_t check_sum;
            uint32_t result;
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
            OnLineSet(sockfd, common_cmd, check_sum, result);
            DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum, result ? RES_FAILED : RES_SUCCESS);
        }
        break;
        case CMD_TYPE_UAPI_GET: {
            ret = OnLineGet(sockfd, common_cmd);
            if (ret == 0)
            {
                DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_SUCCESS);
            }
            else
            {
                DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_FAILED);
            }
        }
        break;
        case CMD_TYPE_CAPTURE: {
            char* datBuf = (char*)(common_cmd->dat);
            switch (datBuf[0])
            {
                // online yuv
                case DATA_ID_CAPTURE_YUV_STATUS: {
                    if (capture_status == BUSY)
                    {
                        LOG_DEBUG("Capture in process.\n");
                        return;
                    }
                    if (access("/tmp/aiq_offline.ini", F_OK) == 0 || access("/mnt/vendor/aiq_offline.ini", F_OK) == 0)
                    {
                        while (g_startOfflineRawFlag == 0)
                        {
                            usleep(1 * 1000);
                        }
                    }
                    ReplyStatus(sockfd, &send_cmd, READY);
                }
                break;
                case DATA_ID_CAPTURE_YUV_GET_PARAM: {
                    ReplySensorPara(sockfd, &send_cmd);
                }
                break;
                case DATA_ID_CAPTURE_YUV_SET_PARAM: {
                    SetSensorPara(sockfd, common_cmd, &send_cmd);
                }
                break;
                case DATA_ID_CAPTURE_YUV_START: {
                    if (capture_status != BUSY && capture_frames_index < capture_frames)
                    {
                        capture_status = BUSY;
                        DoCaptureYuv(sockfd);
                        capture_status = READY;
                    }
                }
                break;
                case DATA_ID_CAPTURE_YUV_CHECKSUM: {
                    LOG_DEBUG("DATA_ID_CAPTURE_YUV_CHECKSUM SKIP\n");
                    // SendYuvDataResult(sockfd, &send_cmd, common_cmd);
                }
                break;

                // online raw
                case DATA_ID_CAPTURE_ONLINE_RAW_STATUS: {
                    if (capture_status == BUSY)
                    {
                        LOG_DEBUG("Capture in process.\n");
                        return;
                    }
                    bool nodeFindedFlag = false;
                    vector<string> targetNodeList = {"/proc/rkisp*"};
                    for (string tmpStr : targetNodeList)
                    {
                        char result[2048] = {0};
                        std::string pattern{"Isp online"};
                        std::string pattern2{"Output.*frameloss"};
                        std::regex re(pattern);
                        std::regex re2(pattern2);
                        std::smatch results;
                        char temp[50] = {0};
                        snprintf(temp, 50, "cat %s 2>/dev/null", tmpStr.c_str());
                        ExecuteCMD(temp, result);
                        std::string srcStr = result;
                        if (std::regex_search(srcStr, results, re)) // finded
                        {
                            LOG_INFO("ISP online:%s\n", srcStr.c_str());
                            nodeFindedFlag = true;
                            break;
                        }
                        if (std::regex_search(srcStr, results, re2)) // finded
                        {
                            LOG_INFO("ISP online:%s\n", srcStr.c_str());
                            nodeFindedFlag = true;
                            break;
                        }
                    }
                    if (nodeFindedFlag == false)
                    {
                        LOG_DEBUG("Isp not online, online raw capture not available\n");
                        return;
                    }

                    if (capture_status == READY)
                    {
                        ReplyOnlineRawStatus(sockfd, &send_cmd, READY);
                        GetSensorHDRMode();
                    }
                }
                break;
                case DATA_ID_CAPTURE_ONLINE_RAW_GET_PARAM: {
                    ReplyOnlineRawSensorPara(sockfd, &send_cmd);
                }
                break;
                case DATA_ID_CAPTURE_ONLINE_RAW_SET_PARAM: {
                    SetOnlineRawSensorPara(sockfd, common_cmd, &send_cmd);
                }
                break;
                case DATA_ID_CAPTURE_ONLINE_RAW_START: {
                    capture_status = BUSY;
                    LOG_DEBUG("DoCaptureOnlineRaw begin\n");
                    DoCaptureOnlineRaw(sockfd);
                    LOG_DEBUG("DoCaptureOnlineRaw end\n");
                    capture_status = READY;
                }
                break;
                case DATA_ID_CAPTURE_ONLINE_RAW_CHECKSUM: {
                    LOG_DEBUG("DATA_ID_CAPTURE_ONLINE_RAW_CHECKSUM SKIP\n");
                    // SendYuvDataResult(sockfd, &send_cmd, common_cmd);
                }
                break;
                default:
                    break;
            }
        }
        break;
        default: {
            LOG_INFO("cmdID: unknown\n");
        }
        break;
    }
}
