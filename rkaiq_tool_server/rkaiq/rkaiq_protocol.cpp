#include "rkaiq_protocol.h"

#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <regex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <condition_variable>
#include <string.h>
#include <stdarg.h>

using namespace std;

#ifdef __ANDROID__
    #include <cutils/properties.h>
    #include <rtspserver/RtspServer.h>
#endif

#include "domain_tcp_client.h"
#include "tcp_client.h"
#include "rkaiq_socket.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_app_run_mode;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern int g_rtsp_en_from_cmdarg;
extern int g_device_id;
extern int g_allow_killapp;

extern int g_sendSpecificFrame;
extern int g_offlineRawSourceFileNumber;

extern uint32_t g_offlineFrameRate;
extern DomainTCPClient g_domainTcpClient;
extern std::string iqfile;
extern std::string g_sensor_name;
extern std::string g_offline_raw_dir;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::string g_stream_dev_name;
extern int ConnectAiq();
extern unsigned int GetCRC32(unsigned char* buf, unsigned int len);

bool RKAiqProtocol::is_recv_running = false;
std::unique_ptr<std::thread> RKAiqProtocol::forward_thread = nullptr;
std::unique_ptr<std::thread> RKAiqProtocol::offlineRawThread = nullptr;
std::mutex RKAiqProtocol::mutex_;

int g_offlineRawModeControl;
int g_offlineRAWCaptureYUVStepCounter = -1;
int g_startOfflineRawFlag = 0;
int g_inCaptureYUVProcess = 0;
std::mutex g_offlineRawEnqueuedMutex;
std::unique_lock<std::mutex> g_offlineRawEnqueuedLock(g_offlineRawEnqueuedMutex);
std::condition_variable g_offlineRawEnqueued;
std::mutex g_yuvCapturedMutex;
std::unique_lock<std::mutex> g_yuvCapturedLock(g_yuvCapturedMutex);
std::condition_variable g_yuvCaptured;

#define MAX_PACKET_SIZE 8192
#pragma pack(1)
typedef struct FileTransferData_s
{
    uint8_t RKID[8]; // "SendFile"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    int targetDirLen;
    uint8_t targetDir[256];
    int targetFileNameLen;
    uint8_t targetFileName[128];
    unsigned long long dataSize;
    char* data;
    unsigned int dataHash;
} FileTransferData;

typedef struct OfflineRAW_s
{
    uint8_t RKID[8]; // "OffRAW"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    int offlineRawModeControl;
} OfflineRAW;

typedef struct TransportAWBPara_s
{
    uint8_t RKID[8]; // "GetAWBp"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    void* awbParaFile;
} TransportAWBPara_t;
#pragma pack()

extern bool g_crcTableInitFlag;
extern unsigned int g_CRC32_table[256];
extern unsigned int GetCRC32(unsigned char* buf, unsigned int len);

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

static int strcmp_natural(const char* a, const char* b)
{
    if (!a || !b) // if one doesn't exist, it comes first
        return a ? 1 : b ? -1 : 0;

    if (isdigit(*a) && isdigit(*b)) // both start with numbers
    {
        char* remainderA;
        char* remainderB;
        long valA = strtol(a, &remainderA, 10);
        long valB = strtol(b, &remainderB, 10);
        if (valA != valB)
        {
            return valA - valB; // smaller comes first
        }
        else
        {
            std::ptrdiff_t lengthA = remainderA - a;
            std::ptrdiff_t lengthB = remainderB - b;
            if (lengthA != lengthB)
                return lengthA - lengthB; // shorter comes first
            else                          // all being equal, recurse
                return strcmp_natural(remainderA, remainderB);
        }
    }

    if (isdigit(*a) || isdigit(*b))  // if just one is a number
        return isdigit(*a) ? -1 : 1; // numbers always come first

    while (*a && *b) // non-numeric characters
    {
        if (isdigit(*a) || isdigit(*b))
            return strcmp_natural(a, b); // recurse
        if (*a != *b)
            return *a - *b;
        a++;
        b++;
    }
    return *a ? 1 : *b ? -1 : 0;
}

static size_t GetFileSize(const char* fileName)
{
    if (fileName == NULL)
    {
        return 0;
    }
    struct stat statbuf;
    stat(fileName, &statbuf);
    size_t filesize = statbuf.st_size;
    return filesize;
}

static bool natural_less(const string& lhs, const string& rhs)
{
    return strcmp_natural(lhs.c_str(), rhs.c_str()) < 0;
}

static void HexDump(unsigned char* data, size_t size)
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

    // HexDump((unsigned char*)dataToSend, packetSize);
    send(sockfd, dataToSend, packetSize, 0);
    free(dataToSend);
}

static void SendFileToPC(int sockfd, char* fileName, char* fileData, unsigned long long fileDataSize = 0)
{
    char* opStr = (char*)"#&#^ToolServerMsg#&#^SendFile#&#^";
    unsigned long long packetSize = strlen(opStr) + strlen(fileName) + strlen("#&#^") + fileDataSize + strlen("#&#^@`#`@`#`");
    char* dataToSend = (char*)malloc(packetSize);
    unsigned long long offSet = 0;
    memcpy(dataToSend + offSet, opStr, strlen(opStr));
    offSet += strlen(opStr);
    memcpy(dataToSend + offSet, fileName, strlen(fileName));
    offSet += strlen(fileName);
    memcpy(dataToSend + offSet, "#&#^", strlen("#&#^"));
    offSet += strlen("#&#^");
    memcpy(dataToSend + offSet, fileData, fileDataSize);
    offSet += fileDataSize;
    memcpy(dataToSend + offSet, "#&#^@`#`@`#`", strlen("#&#^@`#`@`#`"));
    offSet += strlen("#&#^@`#`@`#`");

    send(sockfd, dataToSend, packetSize, 0);
    free(dataToSend);
}

static int ProcessExists(const char* process_name)
{
    FILE* fp;
    char cmd[1024] = {0};
    char buf[1024] = {0};
    snprintf(cmd, sizeof(cmd), "ps -ef | grep %s | grep -v grep", process_name);
    fp = popen(cmd, "r");
    if (!fp)
    {
        LOG_DEBUG("popen ps | grep %s fail\n", process_name);
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp))
    {
        LOG_DEBUG("ProcessExists %s\n", buf);
        if (strstr(buf, process_name))
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int StopProcess(const char* process, const char* str)
{
    int count = 0;
    while (ProcessExists(process) > 0)
    {
        LOG_DEBUG("StopProcess %s... \n", process);
        system(str);
        sleep(1);
        count++;
        if (count > 3)
        {
            return -1;
        }
    }
    return 0;
}

int WaitProcessExit(const char* process, int sec)
{
    int count = 0;
    LOG_DEBUG("WaitProcessExit %s... \n", process);
    while (ProcessExists(process) > 0)
    {
        LOG_DEBUG("WaitProcessExit %s... \n", process);
        sleep(1);
        count++;
        if (count > sec)
        {
            return -1;
        }
    }
    return 0;
}

void RKAiqProtocol::KillApp()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int RKAiqProtocol::StartApp()
{
    return 0;
}

int RKAiqProtocol::StartRTSP()
{
#ifdef __ANDROID__
    int ret = -1;
    KillApp();
    ret = rkaiq_media->LinkToIsp(true);
    if (ret)
    {
        LOG_ERROR("link isp failed!!!\n");
        return ret;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);

    int readback = 0;
    // std::string isp3a_server_cmd = "/vendor/bin/rkaiq_3A_server -mmedia=";
    // if (mi.isp.media_dev_path.length() > 0)
    // {
    //     isp3a_server_cmd.append(mi.isp.media_dev_path);
    //     LOG_DEBUG("#### using isp dev path.\n");
    // }
    // else if (mi.cif.media_dev_path.length() > 0)
    // {
    //     isp3a_server_cmd.append(mi.cif.media_dev_path);
    //     LOG_DEBUG("#### using cif dev path.\n");
    // }
    // else
    // {
    //     isp3a_server_cmd.append("/dev/media2");
    //     LOG_DEBUG("#### using default dev path.\n");
    // }
    // isp3a_server_cmd.append(" --sensor_index=");
    // isp3a_server_cmd.append(std::to_string(g_device_id));
    // isp3a_server_cmd.append(" &");
    // system("pkill rkaiq_3A_server*");
    // system(isp3a_server_cmd.c_str());
    // std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (true)
    {
        int isp_ver = rkaiq_media->GetIspVer();
        LOG_DEBUG(">>>>>>>> isp ver = %d\n", isp_ver);
        if (isp_ver == 4)
        {
            ret = init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height);
        }
        else if (isp_ver == 5)
        {
            ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
        }
        else
        {
            ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
        }
    }
    else
    {
        ret = init_rtsp(g_stream_dev_name.c_str(), g_width, g_height);
    }
    if (ret)
    {
        LOG_ERROR("init_rtsp failed!!");
        return ret;
    }

    LOG_DEBUG("Started RTSP !!!");
#endif
    return 0;
}

int RKAiqProtocol::StopRTSP()
{
#ifdef __ANDROID__
    LOG_DEBUG("Stopping RTSP !!!");
    // deinit_rtsp();
    system("pkill rkaiq_3A_server*");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LOG_DEBUG("Stopped RTSP !!!");
#endif
    return 0;
}

int RKAiqProtocol::DoChangeAppMode(appRunStatus mode)
{
    g_app_run_mode = mode;
    return 0;

    std::lock_guard<std::mutex> lg(mutex_);
    int ret = -1;
    LOG_DEBUG("Switch to mode %d->%d\n", g_app_run_mode, mode);
    if (g_app_run_mode == mode)
    {
        return 0;
    }
    if (mode == APP_RUN_STATUS_CAPTURE)
    {
        LOG_DEBUG("Switch to APP_RUN_STATUS_CAPTURE\n");
        if (g_rtsp_en)
        {
            ret = StopRTSP();
            if (ret)
            {
                LOG_ERROR("stop RTSP failed!!!\n");
                g_app_run_mode = APP_RUN_STATUS_INIT;
                return ret;
            }
        }
        KillApp();
        ret = rkaiq_media->LinkToIsp(false);
        if (ret)
        {
            LOG_ERROR("unlink isp failed!!!\n");
            g_app_run_mode = APP_RUN_STATUS_INIT;
            return ret;
        }
    }
    else
    {
        LOG_DEBUG("Switch to APP_RUN_STATUS_TUNRING\n");
        ret = rkaiq_media->LinkToIsp(true);
        if (ret)
        {
            LOG_ERROR("link isp failed!!!\n");
            g_app_run_mode = APP_RUN_STATUS_INIT;
            // return ret;
        }

        if (!g_rtsp_en)
        {
            ret = StartApp();
            if (ret)
            {
                LOG_ERROR("start app failed!!!\n");
                g_app_run_mode = APP_RUN_STATUS_INIT;
                return ret;
            }
        }
    }
    g_app_run_mode = mode;
    LOG_DEBUG("Change mode to %d exit\n", g_app_run_mode);
    return 0;
}

static void InitCommandPingAns(CommandData_t* cmd, int ret_status)
{
    strncpy((char*)cmd->RKID, RKID_CHECK, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_STATUS;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    strncpy((char*)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
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

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret = -1;

    // for (int i = 0; i < common_cmd->datLen; i++) {
    //   LOG_DEBUG("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
    // }

    if (strcmp((char*)common_cmd->RKID, RKID_CHECK) != 0)
    {
        LOG_DEBUG("RKID: unknown\n");
        return;
    }

    switch (common_cmd->cmdID)
    {
        case CMD_ID_CAPTURE_STATUS:
            if (common_cmd->dat[0] == KNOCK_KNOCK)
            {
                InitCommandPingAns(&send_cmd, READY);
                LOG_DEBUG("Device is READY\n");
            }
            else
            {
                LOG_ERROR("unknown CMD_ID_CAPTURE_STATUS message\n");
            }
            memcpy(send_data, &send_cmd, sizeof(CommandData_t));
            send(sockfd, send_data, sizeof(CommandData_t), 0);
            break;
        case CMD_ID_GET_STATUS:
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
            break;
        case CMD_ID_GET_MODE:
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        case CMD_ID_START_RTSP:
            if (g_rtsp_en_from_cmdarg == 1)
            {
                g_rtsp_en = 1;
            }
            ret = StartRTSP();
            if (ret)
            {
                LOG_ERROR("start RTSP failed!!!\n");
            }
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        case CMD_ID_STOP_RTSP:
            if (g_rtsp_en_from_cmdarg == 1)
            {
                g_rtsp_en = 0;
            }
            ret = StopRTSP();
            if (ret)
            {
                LOG_ERROR("stop RTSP failed!!!\n");
            }
            g_app_run_mode = APP_RUN_STATUS_INIT;
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        default:
            break;
    }
}

void RKAiqProtocol::HandlerReceiveFile(int sockfd, char* buffer, int size)
{
    FileTransferData* recData = (FileTransferData*)buffer;
    LOG_DEBUG("HandlerReceiveFile begin\n");
    // HexDump((unsigned char*)buffer, size);
    // parse data
    unsigned long long packetSize = recData->packetSize;
    LOG_DEBUG("FILETRANS receive : sizeof(packetSize):%d\n", sizeof(packetSize));
    // HexDump((unsigned char*)&recData->packetSize, 8);
    LOG_DEBUG("FILETRANS receive : packetSize:%llu\n", packetSize);

    unsigned long long dataSize = recData->dataSize;
    LOG_DEBUG("FILETRANS receive : dataSize:%llu\n", dataSize);
    if (packetSize <= 0 || packetSize - dataSize > 500)
    {
        printf("FILETRANS no data received or packetSize error, return.\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##TransferError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    char* receivedPacket = (char*)malloc(packetSize);
    memset(receivedPacket, 0, packetSize);
    memcpy(receivedPacket, buffer, size);

    unsigned long long remain_size = packetSize - size;
    int recv_size = 0;

    struct timespec startTime = {0, 0};
    struct timespec currentTime = {0, 0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    LOG_DEBUG("FILETRANS get, start receive:%ld\n", startTime.tv_sec);
    while (remain_size > 0)
    {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        if (currentTime.tv_sec - startTime.tv_sec >= 20)
        {
            LOG_DEBUG("FILETRANS receive: receive data timeout, return\n");
            char tmpBuf[200] = {0};
            snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##Timeout##");
            std::string resultStr = tmpBuf;
            send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
            return;
        }

        unsigned long long offset = packetSize - remain_size;

        unsigned long long targetSize = 0;
        if (remain_size > MAX_PACKET_SIZE)
        {
            targetSize = MAX_PACKET_SIZE;
        }
        else
        {
            targetSize = remain_size;
        }
        recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
        remain_size = remain_size - recv_size;

        // LOG_DEBUG("FILETRANS receive,remain_size: %llu\n", remain_size);
    }
    LOG_DEBUG("FILETRANS receive: receive success, need check data\n");

    // HexDump((unsigned char*)receivedPacket, packetSize);
    // Send(receivedPacket, packetSize); //for debug use

    // parse data
    FileTransferData receivedData;
    memset((void*)&receivedData, 0, sizeof(FileTransferData));
    unsigned long long offset = 0;
    // magic
    memcpy(receivedData.RKID, receivedPacket, sizeof(receivedData.RKID));
    // HexDump((unsigned char*)receivedData.RKID, sizeof(receivedData.RKID));
    offset += sizeof(receivedData.RKID);
    // packetSize
    memcpy((void*)&receivedData.packetSize, receivedPacket + offset, sizeof(receivedData.packetSize));
    offset += sizeof(receivedData.packetSize);
    // command id
    memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // command result
    memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // target dir len
    memcpy((void*)&(receivedData.targetDirLen), receivedPacket + offset, sizeof(receivedData.targetDirLen));
    offset += sizeof(receivedData.targetDirLen);
    LOG_DEBUG("FILETRANS receive: receivedData.targetDirLen:%d\n", receivedData.targetDirLen);
    // target dir
    memcpy((void*)&(receivedData.targetDir), receivedPacket + offset, sizeof(receivedData.targetDir));
    // HexDump((unsigned char*)receivedData.targetDir, sizeof(receivedData.targetDir));
    LOG_DEBUG("FILETRANS receive: receivedData.targetDir:%s\n", receivedData.targetDir);
    offset += sizeof(receivedData.targetDir);
    // target file name len
    memcpy((void*)&(receivedData.targetFileNameLen), receivedPacket + offset, sizeof(receivedData.targetFileNameLen));
    offset += sizeof(receivedData.targetFileNameLen);
    LOG_DEBUG("FILETRANS receive: receivedData.targetFileNameLen:%d\n", receivedData.targetFileNameLen);
    // target file name
    memcpy((void*)&(receivedData.targetFileName), receivedPacket + offset, sizeof(receivedData.targetFileName));
    // HexDump((unsigned char*)receivedData.targetFileName, sizeof(receivedData.targetFileName));
    LOG_DEBUG("FILETRANS receive: receivedData.targetFileName:%s\n", receivedData.targetFileName);
    offset += sizeof(receivedData.targetFileName);

    // data size
    memcpy((void*)&(receivedData.dataSize), receivedPacket + offset, sizeof(unsigned long long));
    LOG_DEBUG("FILETRANS receive: receivedData.dataSize:%llu\n", receivedData.dataSize);
    offset += sizeof(unsigned long long);
    // data
    receivedData.data = (char*)malloc(receivedData.dataSize);
    memcpy(receivedData.data, receivedPacket + offset, receivedData.dataSize);
    offset += receivedData.dataSize;
    // data hash
    memcpy((void*)&(receivedData.dataHash), receivedPacket + offset, sizeof(unsigned int));

    if (receivedPacket != NULL)
    {
        free(receivedPacket);
        receivedPacket = NULL;
    }

    // size check
    if (receivedData.dataSize != dataSize)
    {
        LOG_DEBUG("FILETRANS receive: receivedData.dataSize != target data size, return\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##DataSizeError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    // hash check
    unsigned int dataHash = MurMurHash(receivedData.data, receivedData.dataSize);
    LOG_DEBUG("FILETRANS receive 2: dataHash calculated:%x\n", dataHash);
    LOG_DEBUG("FILETRANS receive: receivedData.dataHash:%x\n", receivedData.dataHash);

    if (dataHash == receivedData.dataHash)
    {
        LOG_DEBUG("FILETRANS receive: data hash check pass\n");
    }
    else
    {
        LOG_DEBUG("FILETRANS receive: data hash check failed\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##HashCheckFail##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    // save to file
    std::string dstDir = (char*)receivedData.targetDir;
    std::string dstFileName = (char*)receivedData.targetFileName;
    std::string dstFilePath = dstDir + "/" + dstFileName;

    if (dstFileName != "aiq_offline.ini" && g_offline_raw_dir != "/data/OfflineRAW")
    {
        dstDir = g_offline_raw_dir;
        dstFilePath = dstDir + "/" + dstFileName;
    }

#ifdef __ANDROID__
    if (dstFileName == "aiq_offline.ini")
    {
        dstDir = "/mnt";
        dstFilePath = "/mnt/vendor/aiq_offline.ini";
        property_set("persist.vendor.camera.polltime.debug", "300");
    }
#endif

    DIR* dirPtr = opendir(dstDir.c_str());
    if (dirPtr == NULL)
    {
        LOG_DEBUG("FILETRANS target dir %s not exist, return \n", dstDir.c_str());
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##DirError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }
    else
    {
        closedir(dirPtr);
    }

    FILE* fWrite = fopen(dstFilePath.c_str(), "w");
    if (fWrite != NULL)
    {
        fwrite(receivedData.data, receivedData.dataSize, 1, fWrite);
    }
    else
    {
        LOG_DEBUG("FILETRANS failed to create file %s, return\n", dstFilePath.c_str());
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Failed##FileSaveError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    fclose(fWrite);
    if (receivedData.data != NULL)
    {
        free(receivedData.data);
        receivedData.data = NULL;
    }

    LOG_DEBUG("HandlerReceiveFile process finished.\n");
    LOG_INFO("receive file %s finished.\n", dstFilePath.c_str());

    char tmpBuf[200] = {0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Success##%s##", dstFileName.c_str());
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);

    LOG_DEBUG("g_offlineRawModeControl:%d\n", g_offlineRawModeControl);
    if (g_offlineRawModeControl == 3) // one frame offline raw
    {
        // g_startOfflineRawFlag = 1;
        LOG_DEBUG("offlineRawProcess begin\n");
        // while (g_startOfflineRawFlag == 1)
        {
            DIR* dir = opendir(g_offline_raw_dir.c_str());
            struct dirent* dir_ent = NULL;
            std::vector<std::string> raw_files;
            if (dir)
            {
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
                LOG_INFO("No raw files in %s\n", g_offline_raw_dir.c_str());
                return;
            }

            std::sort(raw_files.begin(), raw_files.end(), natural_less);
            for (auto raw_file : raw_files)
            {
                cout << raw_file.c_str() << endl;
                // if (g_startOfflineRawFlag == 0)
                // {
                //     break;
                // }
                // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
                struct timeval tv;
                struct timezone tz;
                gettimeofday(&tv, &tz);
                long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
                LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
                std::string filePath = g_offline_raw_dir + "/" + raw_file;
                LOG_INFO("process raw : %s \n", filePath.c_str());

                g_domainTcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::seconds(2));
                if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(), (unsigned int)filePath.length() + 1) != 0)
                {
                    LOG_ERROR("########################################################\n");
                    LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
                    LOG_ERROR("########################################################\n\n");
                    // return 1;
                }
                else
                {
                    char tmp[2048];
                    g_domainTcpClient.Receive(tmp, sizeof(tmp));
                }

                uint32_t frameInterval = 1000 / g_offlineFrameRate;
                frameInterval = frameInterval - 50;
                if (frameInterval < 0)
                {
                    frameInterval = 0;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(frameInterval));

                gettimeofday(&tv, &tz);
                long endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
                LOG_DEBUG("end millisecond: %ld\n", endTime);                                                   // ms
                LOG_DEBUG("####################################### time spend: %ld ms\n", endTime - startTime); // ms
                LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE end\n");

                SendMessageToPC(sockfd, (char*)"ENQUEUERKRAWFILE#&#^Success#&#^Success");
                usleep(1000 * 50);
                g_domainTcpClient.m_inetSocketOperationMtx.unlock();
            }
        }
        std::lock_guard<std::mutex> lk(mutex_);
        LOG_DEBUG("offlineRawProcess end\n");
    }
}

void RKAiqProtocol::HandlerOfflineRawProcess(int sockfd, char* buffer, int size)
{
    OfflineRAW* recData = (OfflineRAW*)buffer;
    LOG_DEBUG("HandlerOfflineRawProcess begin\n");
    // HexDump((unsigned char*)buffer, size);
    // parse data
    unsigned long long packetSize = recData->packetSize;
    LOG_DEBUG("receive : sizeof(packetSize):%d\n", sizeof(packetSize));
    // HexDump((unsigned char*)&recData->packetSize, 8);
    LOG_DEBUG("receive : packetSize:%llu\n", packetSize);
    if (packetSize <= 0 || packetSize > 50)
    {
        printf("no data received or packetSize error, return.\n");
        // SendMessageToPC(sockfd, "no data received or packetSize error");
        return;
    }

    char* receivedPacket = (char*)malloc(packetSize);
    memset(receivedPacket, 0, packetSize);
    memcpy(receivedPacket, buffer, size);

    unsigned long long remain_size = packetSize - size;
    int recv_size = 0;

    struct timespec startTime = {0, 0};
    struct timespec currentTime = {0, 0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    LOG_DEBUG("start receive:%ld\n", startTime.tv_sec);
    while (remain_size > 0)
    {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        if (currentTime.tv_sec - startTime.tv_sec >= 5)
        {
            LOG_DEBUG("receive: receive data 5 sec timeout, break\n");
            // SendMessageToPC(sockfd, "receive data timeout");
            break;
        }

        unsigned long long offset = packetSize - remain_size;

        unsigned long long targetSize = 0;
        if (remain_size > MAX_PACKET_SIZE)
        {
            targetSize = MAX_PACKET_SIZE;
        }
        else
        {
            targetSize = remain_size;
        }
        recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
        remain_size = remain_size - recv_size;

        // LOG_DEBUG("FILETRANS receive,remain_size: %llu\n", remain_size);
    }

    // HexDump((unsigned char*)receivedPacket, packetSize);
    // Send(receivedPacket, packetSize); //for debug use

    // parse data
    OfflineRAW receivedData;
    memset((void*)&receivedData, 0, sizeof(OfflineRAW));
    unsigned long long offset = 0;
    // magic
    memcpy(receivedData.RKID, receivedPacket, sizeof(receivedData.RKID));
    offset += sizeof(receivedData.RKID);
    // packetSize
    memcpy((void*)&receivedData.packetSize, receivedPacket + offset, sizeof(receivedData.packetSize));
    offset += sizeof(receivedData.packetSize);
    // command id
    memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // command result
    memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // mode control
    memcpy((void*)&(receivedData.offlineRawModeControl), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);

    // start process offline process
    g_offlineRawModeControl = receivedData.offlineRawModeControl;
    if (receivedData.offlineRawModeControl == 1) // start
    {
        LOG_INFO("#### start offline RAW mode. ####\n");
        forward_thread = std::unique_ptr<std::thread>(new std::thread(&RKAiqProtocol::offlineRawProcess, sockfd));
        forward_thread->detach();
        LOG_INFO("#### offline RAW mode stopped. ####\n");
    }
    else if (receivedData.offlineRawModeControl == 0) // stop
    {
        g_startOfflineRawFlag = 0;
    }
    else if (receivedData.offlineRawModeControl == 2) // remove ini file
    {
        LOG_DEBUG("#### remove offline RAW config file. ####\n");
#ifdef __ANDROID__
        system("rm -f /mnt/vendor/aiq_offline.ini && sync");
        property_set("persist.vendor.camera.polltime.debug", "0");
#else
        system("rm -f /tmp/aiq_offline.ini && sync");
#endif
    }
    else if (receivedData.offlineRawModeControl == 3) // one frame
    {
        LOG_DEBUG("#### offline RAW one frame. ####\n");
        std::string tmpStr = string_format("rm -f %s/* && sync", g_offline_raw_dir.c_str());
        system(tmpStr.c_str());
    }
    else if (receivedData.offlineRawModeControl == 4) // clear raw files
    {
        std::string tmpStr = string_format("rm -f %s/* && sync", g_offline_raw_dir.c_str());
        system(tmpStr.c_str());
    }
    LOG_DEBUG("HandlerOfflineRawProcess process finished.\n");

    // char tmpBuf[200] = {0};
    // snprintf(tmpBuf, sizeof(tmpBuf), "##ToolServerMsg##FileTransfer##Success##%s##", dstFileName.c_str());
    // std::string resultStr = tmpBuf;
    // send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
}

void RKAiqProtocol::HandlerGetAWBParaFileProcess(int sockfd, char* buffer, int size)
{
    system("rm /tmp/awbinput_*");
    usleep(1000 * 100);
    LOG_INFO("call RkAiqSocketClientINETSend(ENUM_ID_AIQ_UAPI2_AWB_WRITEAWBIN);\n");
    char* tmpData = (char*)"0";

    if (RkAiqSocketClientINETSend(ENUM_ID_AIQ_UAPI2_AWB_WRITEAWBIN, (void*)tmpData, 1) != 0)
    {
        LOG_ERROR("########################################################\n");
        LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
        LOG_ERROR("########################################################\n\n");
        return;
    }
    else
    {
        char tmp[256];
        g_domainTcpClient.Receive(tmp, sizeof(tmp));
    }
    usleep(1000 * 500);
    system("ls -l /tmp/awb*");

    char result[2048] = {0};
    std::string pattern{"/tmp/(awbinput_.*)"};
    std::regex re(pattern);
    std::smatch results;
    ExecuteCMD("ls -l /tmp/awb*", result);
    std::string srcStr = result;
    // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
    if (std::regex_search(srcStr, results, re)) // finded
    {
        string awbParaFilaPath = results.str(0);
        string awbParaFileName = results.str(1);
        LOG_DEBUG("AWB para filePath:%s\n", awbParaFilaPath.c_str());
        LOG_DEBUG("AWB para fileName:%s\n", awbParaFileName.c_str());

        std::ifstream t(awbParaFilaPath.c_str());
        t.seekg(0, std::ios::end);
        size_t fileSize = t.tellg();
        std::string fileData(fileSize, ' ');
        t.seekg(0);
        t.read(&fileData[0], fileSize);

        SendFileToPC(sockfd, (char*)awbParaFileName.c_str(), (char*)fileData.c_str(), fileSize);
    }
    else
    {
        LOG_DEBUG("AWB para file not found.\n");
    }
}

static void DoJ2SProcessAnswer(int sockfd, CommandData_t* cmd, int cmd_type, int cmd_id, char* data, uint dataLen)
{
    char send_data[MAXPACKETSIZE];
    LOG_DEBUG("enter\n");

    strncpy((char*)cmd->RKID, RKID_JSON_TO_BIN_PROC, sizeof(cmd->RKID));
    cmd->cmdType = cmd_type;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_J2S_PROCESS_VERSION, sizeof(cmd->version));
    cmd->datLen = min(dataLen, (uint)48);
    memset(cmd->dat, 0, sizeof(cmd->dat));
    memcpy(cmd->dat, data, sizeof(cmd->dat));
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_DEBUG("exit\n");
}

void RKAiqProtocol::HandlerJson2BinProcess(int sockfd, char* buffer, int size)
{
    uint32_t result;
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret = -1;
    LOG_DEBUG("HandlerJson2BinProcess begin\n");
    HexDump((unsigned char*)buffer, size);

    if (strncmp((char*)common_cmd->RKID, RKID_JSON_TO_BIN_PROC, 7) == 0)
    {
        LOG_DEBUG("RKID: %s\n", common_cmd->RKID);
    }
    else
    {
        LOG_DEBUG("RKID: unknown\n");
        return;
    }

    LOG_DEBUG("cmdID: %d\n", common_cmd->cmdID);
    LOG_DEBUG("cmdType: %d\n", common_cmd->cmdType);

    if (common_cmd->cmdType == 0x3001)
    {
        LOG_DEBUG("process request AIQ version begin\n");
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                char cmdResStr[2048] = {0};
                ExecuteCMD("find / -name librkaiq.so -exec strings {} \\; | grep \"AIQ v\" | head -n 1", cmdResStr);
                // string aiqVersion "AIQ v5.0x1.3-rc2";
                string aiqVersion = cmdResStr;
                LOG_DEBUG("aiqVersion:%s\n", aiqVersion.c_str());
                DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)aiqVersion.c_str(), aiqVersion.length());
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process request AIQ version end\n");
    }
    else if (common_cmd->cmdType == 0x3002)
    {
        LOG_DEBUG("process receive j2s tool begin\n");
        HexDump((unsigned char*)common_cmd->dat, sizeof(common_cmd->dat));
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                ////TODO
                uint32_t packetSize = *((uint32_t*)common_cmd->dat);
                uint32_t crc32 = *((uint32_t*)(common_cmd->dat + 4));
                LOG_DEBUG("file size:%u crc32:%x\n", packetSize, crc32);

                uint8_t status = 0x81;
                DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)&status, 1);

                // receive data
                char* receivedPacket = (char*)malloc(packetSize);
                memset(receivedPacket, 0, packetSize);

                uint32_t remain_size = packetSize;
                int recv_size = 0;

                struct timespec startTime = {0, 0};
                struct timespec currentTime = {0, 0};
                clock_gettime(CLOCK_REALTIME, &startTime);
                LOG_DEBUG("FILETRANS get, start receive:%ld\n", startTime.tv_sec);
                while (remain_size > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &currentTime);
                    if (currentTime.tv_sec - startTime.tv_sec >= 20)
                    {
                        LOG_DEBUG("FILETRANS receive: receive data timeout, return\n");
                        status = 0; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                        return;
                    }

                    uint32_t offset = packetSize - remain_size;

                    uint32_t targetSize = 0;
                    if (remain_size > MAX_PACKET_SIZE)
                    {
                        targetSize = MAX_PACKET_SIZE;
                    }
                    else
                    {
                        targetSize = remain_size;
                    }
                    recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
                    remain_size = remain_size - recv_size;

                    LOG_DEBUG("FILETRANS receive,remain_size: %u\n", remain_size);
                }
                LOG_DEBUG("FILETRANS receive: receive success, need check data\n");

                // check data
                uint32_t myCrc32 = GetCRC32((unsigned char*)receivedPacket, packetSize);
                if (crc32 == myCrc32)
                {
                    // save file
                    FILE* fWrite = fopen("/data/j2sToolBin", "w");
                    if (fWrite != NULL)
                    {
                        fwrite(receivedPacket, packetSize, 1, fWrite);
                        fclose(fWrite);
                        LOG_DEBUG("FILETRANS save /data/j2sToolBin success\n");
                        status = 1; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                    }
                    else
                    {
                        LOG_DEBUG("FILETRANS failed to create file /data/j2sToolBin\n");
                        status = 0; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                    }
                }
                else
                {
                    LOG_DEBUG("FILETRANS crc32 check fail. crc32=%u,target=%u\n", myCrc32, crc32);
                    status = 0; // 1:success  0:fail
                    DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                }
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process receive j2s tool end\n");
    }
    else if (common_cmd->cmdType == 0x3003)
    {
        LOG_DEBUG("process receive json file begin\n");
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                ////TODO
                uint32_t packetSize = *((uint32_t*)common_cmd->dat);
                uint32_t crc32 = *((uint32_t*)(common_cmd->dat + 4));
                LOG_DEBUG("file size:%u crc32:%x\n", packetSize, crc32);

                uint8_t status = 0x81;
                DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)&status, 1);

                // receive data
                char* receivedPacket = (char*)malloc(packetSize);
                memset(receivedPacket, 0, packetSize);

                uint32_t remain_size = packetSize;
                int recv_size = 0;

                struct timespec startTime = {0, 0};
                struct timespec currentTime = {0, 0};
                clock_gettime(CLOCK_REALTIME, &startTime);
                LOG_DEBUG("FILETRANS get, start receive:%ld\n", startTime.tv_sec);
                while (remain_size > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &currentTime);
                    if (currentTime.tv_sec - startTime.tv_sec >= 20)
                    {
                        LOG_DEBUG("FILETRANS receive: receive data timeout, return\n");
                        status = 0; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                        return;
                    }

                    uint32_t offset = packetSize - remain_size;

                    uint32_t targetSize = 0;
                    if (remain_size > MAX_PACKET_SIZE)
                    {
                        targetSize = MAX_PACKET_SIZE;
                    }
                    else
                    {
                        targetSize = remain_size;
                    }
                    recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
                    remain_size = remain_size - recv_size;

                    LOG_DEBUG("FILETRANS receive,remain_size: %u\n", remain_size);
                }
                LOG_DEBUG("FILETRANS receive: receive success, need check data\n");

                // check data
                uint32_t myCrc32 = GetCRC32((unsigned char*)receivedPacket, packetSize);
                if (crc32 == myCrc32)
                {
                    // save file
                    FILE* fWrite = fopen("/data/jsonfile.json", "w");
                    if (fWrite != NULL)
                    {
                        fwrite(receivedPacket, packetSize, 1, fWrite);
                        fclose(fWrite);
                        LOG_DEBUG("FILETRANS save /data/jsonfile.json success\n");
                        status = 1; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                    }
                    else
                    {
                        LOG_DEBUG("FILETRANS failed to create file /data/jsonfile.json\n");
                        status = 0; // 1:success  0:fail
                        DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                    }
                }
                else
                {
                    LOG_DEBUG("FILETRANS crc32 check fail. crc32=%u,target=%u\n", myCrc32, crc32);
                    status = 0; // 1:success  0:fail
                    DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&status, 1);
                }
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process receive json file end\n");
    }
    else if (common_cmd->cmdType == 0x3004)
    {
        LOG_DEBUG("process send converted bin file begin\n");
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                LOG_DEBUG("process convert file begin\n");
                system("chmod +x /data/j2sToolBin");
                system("/data/j2sToolBin /data/jsonfile.json /data/convertResult.bin");
                usleep(1000 * 100);
                LOG_DEBUG("process convert file end\n");
                ////TODO
                bool res = true;

                char* sendBuffer = NULL;
                char tmpResult[10] = {0}; //转换结果2byte 文件长度4byte CRC 4byte
                uint16_t result;
                uint32_t fileLen;
                uint32_t crc;
                FILE* file = fopen("/data/convertResult.bin", "rb");
                if (file)
                {
                    fseek(file, 0, SEEK_END);
                    fileLen = ftell(file);
                    rewind(file);

                    sendBuffer = (char*)malloc(fileLen);
                    if (sendBuffer)
                    {
                        fread(sendBuffer, fileLen, 1, file);
                        crc = GetCRC32((unsigned char*)sendBuffer, fileLen);
                    }
                    else
                    {
                        LOG_DEBUG("alloc sendBuffer failed\n");
                        res = false;
                    }
                }
                else
                {
                    res = false;
                    LOG_DEBUG("open file convertResult.bin failed\n");
                }

                //
                if (res == true)
                {
                    uint16_t convertResult = RES_SUCCESS; // success
                    uint32_t offset = 0;
                    *reinterpret_cast<uint16_t*>(tmpResult + offset) = convertResult;
                    offset += sizeof(convertResult);
                    *reinterpret_cast<uint32_t*>(tmpResult + offset) = fileLen;
                    offset += sizeof(fileLen);
                    *reinterpret_cast<uint32_t*>(tmpResult + offset) = crc;
                    offset += sizeof(crc);
                    //
                    LOG_DEBUG("fileLen:%u, crc:%u\n", fileLen, crc);
                    LOG_DEBUG("fileLen:%x, crc:%x\n", fileLen, crc);
                    HexDump((unsigned char*)tmpResult, sizeof(tmpResult));
                    DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, tmpResult, sizeof(tmpResult));
                }
                else
                {
                    uint16_t convertResult = RES_FAILED; // failed
                    uint32_t offset = 0;
                    memcpy(tmpResult + offset, &convertResult, sizeof(convertResult));
                    //
                    DoJ2SProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, tmpResult, sizeof(tmpResult));
                }

                free(sendBuffer);
                fclose(file);
                break;
            }
            case 0x0002: {
                LOG_DEBUG("process send file begin\n");
                char* sendBuffer = NULL;
                uint32_t fileLen;
                FILE* file = fopen("/data/convertResult.bin", "rb");
                if (file)
                {
                    fseek(file, 0, SEEK_END);
                    fileLen = ftell(file);
                    rewind(file);
                    sendBuffer = (char*)malloc(fileLen);
                    if (sendBuffer)
                    {
                        fread(sendBuffer, fileLen, 1, file);
                        //
                        send(sockfd, sendBuffer, fileLen, 0);
                    }
                    else
                    {
                        LOG_DEBUG("alloc sendBuffer failed\n");
                    }
                }
                else
                {
                    LOG_DEBUG("open file convertResult.bin failed\n");
                }

                free(sendBuffer);
                fclose(file);
                LOG_DEBUG("process send file end\n");
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process send converted bin file end\n");
    }
    LOG_DEBUG("HandlerJson2BinProcess end\n");
}

static void DoI2CProcessAnswer(int sockfd, CommandData_t* cmd, int cmd_type, int cmd_id, char* data, uint dataLen)
{
    char send_data[MAXPACKETSIZE];
    LOG_DEBUG("enter\n");

    strncpy((char*)cmd->RKID, RKID_I2C_TRANSFER_PROC, sizeof(cmd->RKID));
    cmd->cmdType = cmd_type;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_I2C_PROCESS_VERSION, sizeof(cmd->version));
    cmd->datLen = min(dataLen, (uint)48);
    memset(cmd->dat, 0, sizeof(cmd->dat));
    memcpy(cmd->dat, data, sizeof(cmd->dat));
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_DEBUG("exit\n");
}

void RKAiqProtocol::HandlerI2CTransferProcess(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret = -1;
    LOG_DEBUG("HandlerI2CTransferProcess begin\n");
    HexDump((unsigned char*)buffer, size);

    if (strncmp((char*)common_cmd->RKID, RKID_I2C_TRANSFER_PROC, 7) == 0)
    {
        LOG_DEBUG("RKID: %s\n", common_cmd->RKID);
    }
    else
    {
        LOG_DEBUG("RKID: unknown\n");
        return;
    }

    LOG_DEBUG("cmdID: %d\n", common_cmd->cmdID);
    LOG_DEBUG("cmdType: %d\n", common_cmd->cmdType);

    if (common_cmd->cmdType == 0x4001)
    {
        LOG_DEBUG("process quiry i2c status begin\n");
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                uint16_t status = 0x81; // ready  0x82 busy
                DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)&status, sizeof(status));
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process quiry i2c status end\n");
    }
    else if (common_cmd->cmdType == 0x4002)
    {
        LOG_DEBUG("process write i2c data begin\n");
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                //
                uint32_t offset = 0;
                uint32_t i2cCommandLen = *((uint32_t*)common_cmd->dat);
                offset += sizeof(i2cCommandLen);
                uint32_t crc32 = *((uint32_t*)(common_cmd->dat + offset));
                //
                uint16_t status = 0x81; // ready  0x82 busy
                DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)&status, sizeof(status));

                // receive data
                string i2cCommand;
                char* receivedPacket = (char*)malloc(i2cCommandLen + 1); // add 1 for \0
                memset(receivedPacket, 0, i2cCommandLen + 1);

                uint32_t remain_size = i2cCommandLen;
                int recv_size = 0;

                struct timespec startTime = {0, 0};
                struct timespec currentTime = {0, 0};
                clock_gettime(CLOCK_REALTIME, &startTime);
                LOG_DEBUG("command get, start receive:%ld\n", startTime.tv_sec);
                while (remain_size > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &currentTime);
                    if (currentTime.tv_sec - startTime.tv_sec >= 20)
                    {
                        LOG_DEBUG("command receive: receive data timeout, return\n");
                        uint16_t res = RES_FAILED; // 1:success  0:fail
                        DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&res, sizeof(res));
                        return;
                    }

                    uint32_t offset = i2cCommandLen - remain_size;

                    uint32_t targetSize = 0;
                    if (remain_size > MAX_PACKET_SIZE)
                    {
                        targetSize = MAX_PACKET_SIZE;
                    }
                    else
                    {
                        targetSize = remain_size;
                    }
                    recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
                    remain_size = remain_size - recv_size;

                    LOG_DEBUG("command receive,remain_size: %u\n", remain_size);
                }
                LOG_DEBUG("command receive: receive success, need check data\n");
                LOG_DEBUG("command size:%u crc32:%x\n", i2cCommandLen, crc32);
                receivedPacket[i2cCommandLen] = '\0'; // add \0 to string end
                i2cCommand = receivedPacket;
                LOG_DEBUG("i2cCommand:%s\n", i2cCommand.c_str());
                HexDump((unsigned char*)receivedPacket, i2cCommandLen);
                free(receivedPacket);

                //开始执行命令
                char cmdResStr[2048] = {0};
                ExecuteCMD(i2cCommand.c_str(), cmdResStr);
                LOG_DEBUG("i2c transfer command result:%s\n", cmdResStr);

                //
                uint16_t res = RES_SUCCESS;
                DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&res, sizeof(res));
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process write i2c data end\n");
    }
    else if (common_cmd->cmdType == 0x4003)
    {
        LOG_DEBUG("process read i2c data begin\n");
        static char cmdResStr[4096] = {0};
        switch (common_cmd->cmdID)
        {
            case 0x0001: {
                //
                uint32_t offset = 0;
                uint32_t i2cCommandLen = *((uint32_t*)common_cmd->dat);
                offset += sizeof(i2cCommandLen);
                uint32_t crc32 = *((uint32_t*)(common_cmd->dat + offset));
                //
                uint16_t status = 0x81; // ready  0x82 busy
                DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, common_cmd->cmdID, (char*)&status, sizeof(status));

                // receive data
                string i2cCommand;
                char* receivedPacket = (char*)malloc(i2cCommandLen + 1); // add 1 for \0
                memset(receivedPacket, 0, i2cCommandLen + 1);

                uint32_t remain_size = i2cCommandLen;
                int recv_size = 0;

                struct timespec startTime = {0, 0};
                struct timespec currentTime = {0, 0};
                clock_gettime(CLOCK_REALTIME, &startTime);
                LOG_DEBUG("command get, start receive:%ld\n", startTime.tv_sec);
                while (remain_size > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &currentTime);
                    if (currentTime.tv_sec - startTime.tv_sec >= 20)
                    {
                        LOG_DEBUG("command receive: receive data timeout, return\n");
                        uint16_t res = RES_FAILED; // 1:success  0:fail
                        DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, (char*)&res, sizeof(res));
                        return;
                    }

                    uint32_t offset = i2cCommandLen - remain_size;

                    uint32_t targetSize = 0;
                    if (remain_size > MAX_PACKET_SIZE)
                    {
                        targetSize = MAX_PACKET_SIZE;
                    }
                    else
                    {
                        targetSize = remain_size;
                    }
                    recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
                    remain_size = remain_size - recv_size;

                    LOG_DEBUG("command receive,remain_size: %u\n", remain_size);
                }
                LOG_DEBUG("command receive: receive success, need check data\n");
                LOG_DEBUG("command size:%u crc32:%x\n", i2cCommandLen, crc32);
                receivedPacket[i2cCommandLen] = '\0'; // add \0 to string end
                i2cCommand = receivedPacket;
                LOG_DEBUG("i2cCommand:%s\n", i2cCommand.c_str());
                HexDump((unsigned char*)receivedPacket, i2cCommandLen);
                free(receivedPacket);

                //开始执行命令
                ExecuteCMD(i2cCommand.c_str(), cmdResStr);
                LOG_DEBUG("i2c transfer command result:%s\n", cmdResStr);

                //
                char tmpResult[10] = {0};               //结果2byte 数据长度4byte CRC 4byte
                uint16_t exei2cCmdResult = RES_SUCCESS; // success
                crc32 = GetCRC32((unsigned char*)cmdResStr, strlen(cmdResStr));
                offset = 0;
                *reinterpret_cast<uint16_t*>(tmpResult + offset) = exei2cCmdResult;
                offset += sizeof(exei2cCmdResult);
                *reinterpret_cast<uint32_t*>(tmpResult + offset) = strlen(cmdResStr);
                offset += 4;
                *reinterpret_cast<uint32_t*>(tmpResult + offset) = crc32;
                offset += sizeof(crc32);
                //
                LOG_DEBUG("cmdResStr:%u, crc:%u\n", strlen(cmdResStr), crc32);
                LOG_DEBUG("cmdResStr:%x, crc:%x\n", strlen(cmdResStr), crc32);
                HexDump((unsigned char*)cmdResStr, strlen(cmdResStr));
                DoI2CProcessAnswer(sockfd, common_cmd, common_cmd->cmdType, 0x0002, tmpResult, sizeof(tmpResult));
                break;
            }
            case 0x0003: {
                LOG_DEBUG("process send i2c result begin\n");
                send(sockfd, cmdResStr, strlen(cmdResStr), 0);
                LOG_DEBUG("process send i2c result end\n");
                memset(cmdResStr, 0, sizeof(cmdResStr));
                break;
            }
            default:
                break;
        }
        LOG_DEBUG("process read i2c data end\n");
    }
    LOG_DEBUG("HandlerI2CTransferProcess end\n");
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    // LOG_DEBUG("HandlerTCPMessage:\n");
    // LOG_DEBUG("HandlerTCPMessage CommandData_t: 0x%lx\n", sizeof(CommandData_t));
    // LOG_DEBUG("HandlerTCPMessage RKID: %s\n", (char*)common_cmd->RKID);

    int resetThreadFlag = 1;
    // TODO Check APP Mode
    if (strcmp((char*)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0)
    {
        char result[2048] = {0};
        std::string pattern{"Output"};
        std::regex re(pattern);
        std::smatch results;
        ExecuteCMD("cat /proc/rkisp*", result);
        std::string srcStr = result;
        // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
        if (std::regex_search(srcStr, results, re)) // finded
        {
            LOG_INFO("Isp online, please use online raw capture.\n");
            // SendMessageToPC(sockfd, "Isp online, please use online raw capture.");
            return;
        }
        RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
    }
    else if (strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0)
    {
        RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
    }
    else if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0)
    {
        HandlerCheckDevice(sockfd, buffer, size);
    }
    else if (memcmp((char*)common_cmd->RKID, RKID_SEND_FILE, 8) == 0)
    {
        HandlerReceiveFile(sockfd, buffer, size);
    }
    else if (memcmp((char*)common_cmd->RKID, RKID_OFFLINE_RAW, 6) == 0)
    {
        HandlerOfflineRawProcess(sockfd, buffer, size);
    }
    else if (memcmp((char*)common_cmd->RKID, RKID_GET_AWB_PARA_FILE, 6) == 0)
    {
        HandlerGetAWBParaFileProcess(sockfd, buffer, size);
    }
    else if (memcmp((char*)common_cmd->RKID, RKID_JSON_TO_BIN_PROC, 7) == 0)
    {
        HandlerJson2BinProcess(sockfd, buffer, size);
    }
    else if (memcmp((char*)common_cmd->RKID, RKID_I2C_TRANSFER_PROC, 7) == 0)
    {
        HandlerI2CTransferProcess(sockfd, buffer, size);
    }
    else
    {
        resetThreadFlag = 0;
        if (!DoChangeAppMode(APP_RUN_STATUS_TUNRING))
            MessageForward(sockfd, buffer, size);
    }
}

int RKAiqProtocol::doMessageForward(int sockfd)
{
    auto stopReceiveTimer = std::chrono::high_resolution_clock::now();
    is_recv_running = true;
    while (is_recv_running)
    {
        char recv_buffer[MAXPACKETSIZE] = {0};
        int recv_len = g_domainTcpClient.Receive(recv_buffer, MAXPACKETSIZE);
        // if (recv_len != -1)
        // {
        //     LOG_DEBUG("recv_len:%d\n", recv_len);
        // }
        // else
        // {
        //     LOG_DEBUG("recv_chk\n");
        // }
        if (recv_len > 0)
        {
            stopReceiveTimer = std::chrono::high_resolution_clock::now();
            ssize_t ret = send(sockfd, recv_buffer, recv_len, 0);
            if (ret < 0)
            {
                LOG_ERROR("#########################################################\n");
                LOG_ERROR("## Forward socket %d failed, please check AIQ status.####\n", sockfd);
                LOG_ERROR("#########################################################\n\n");

                close(sockfd);
                std::lock_guard<std::mutex> lk(mutex_);
                is_recv_running = false;
                g_domainTcpClient.m_inetSocketOperationMtx.unlock();
                return -1;
            }
        }
        else if (recv_len <= 0)
        {
            if (errno != EAGAIN)
            {
                g_domainTcpClient.Close();
                close(sockfd);
                std::lock_guard<std::mutex> lk(mutex_);
                is_recv_running = false;
                g_domainTcpClient.m_inetSocketOperationMtx.unlock();
                return -1;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopReceiveTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(100))
                {
                    LOG_DEBUG("receive OK %f\n", waitTime.count());
                    std::lock_guard<std::mutex> lk(mutex_);
                    is_recv_running = false;
                }
            }
        }
    }

    std::lock_guard<std::mutex> lk(mutex_);
    is_recv_running = false;
    g_domainTcpClient.m_inetSocketOperationMtx.unlock();
    return 0;
}

int RKAiqProtocol::doWriteAiqData(int sockfd, char* buffer, int size)
{
    std::lock_guard<std::mutex> lk(mutex_);
    // HexDump((unsigned char*)buffer, size);
    int ret = g_domainTcpClient.Send((char*)buffer, size);
    if (ret < 0 && errno != EINTR)
    {
        if (ConnectAiq() < 0)
        {
            g_domainTcpClient.Close();
            g_app_run_mode = APP_RUN_STATUS_INIT;
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
            LOG_ERROR("########################################################\n\n");
            close(sockfd);
            is_recv_running = false;
            return -1;
        }
        else
        {
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
            LOG_ERROR("########################################################\n\n");
            g_domainTcpClient.Send((char*)buffer, size);
        }
    }
    return 0;
}

int RKAiqProtocol::doReadAiqData(int sockfd, char* buffer, int size)
{
    if (buffer == NULL)
    {
        LOG_DEBUG("buffer is null,stop read aiq data.\n");
        return -1;
    }

    auto stopReceiveTimer = std::chrono::high_resolution_clock::now();
    is_recv_running = true;
    int recv_len = 0;
    int targetLen = size;
    while (is_recv_running)
    {
        recv_len = g_domainTcpClient.Receive(buffer + recv_len, targetLen);
        if (recv_len > 0)
        {
            stopReceiveTimer = std::chrono::high_resolution_clock::now();
            targetLen -= recv_len;
            if (targetLen <= 0)
            {
                LOG_DEBUG("do read aiq data target len finish.\n");
                break;
            }
        }
        else if (recv_len <= 0)
        {
            if (errno != EAGAIN)
            {
                g_domainTcpClient.Close();
                close(sockfd);
                std::lock_guard<std::mutex> lk(mutex_);
                is_recv_running = false;
                g_domainTcpClient.m_inetSocketOperationMtx.unlock();
                return -1;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopReceiveTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(100))
                {
                    LOG_DEBUG("receive OK %f\n", waitTime.count());
                    std::lock_guard<std::mutex> lk(mutex_);
                    is_recv_running = false;
                }
            }
        }
    }

    std::lock_guard<std::mutex> lk(mutex_);
    is_recv_running = false;
    g_domainTcpClient.m_inetSocketOperationMtx.unlock();
    return 0;
}

int RKAiqProtocol::offlineRawProcess(int sockfd)
{
    static int limitCounter;
    g_startOfflineRawFlag = 1;
    LOG_DEBUG("offlineRawProcess begin\n");
    while (g_startOfflineRawFlag == 1)
    {
        DIR* dir = opendir(g_offline_raw_dir.c_str());
        struct dirent* dir_ent = NULL;
        std::vector<std::string> raw_files;
        if (dir)
        {
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
            LOG_INFO("No raw files in %s\n", g_offline_raw_dir.c_str());
            return 1;
        }
        g_offlineRawSourceFileNumber = raw_files.size();

        std::sort(raw_files.begin(), raw_files.end(), natural_less);
        // for (auto raw_file : raw_files)
        for (int i = 0; i < raw_files.size(); i++)
        {
            if (g_startOfflineRawFlag == 0)
            {
                break;
            }

            // LOG_DEBUG("############# 1\n");
            // LOG_DEBUG("############# g_inCaptureYUVProcess:%d\n", g_inCaptureYUVProcess);
            // LOG_DEBUG("############# g_offlineRAWCaptureYUVStepCounter:%d\n", g_offlineRAWCaptureYUVStepCounter);
            if (g_inCaptureYUVProcess)
            {
                if (g_offlineRAWCaptureYUVStepCounter == 0)
                {
                    limitCounter = 0;
                    i = 0;
                    g_offlineRAWCaptureYUVStepCounter = 1;
                }
                else if (g_offlineRAWCaptureYUVStepCounter == 1)
                {
                    std::cv_status tmpStat = g_yuvCaptured.wait_for(g_yuvCapturedLock, std::chrono::milliseconds(250));
                    if (tmpStat == std::cv_status::no_timeout)
                    {
                        g_offlineRAWCaptureYUVStepCounter = 2;
                    }
                    else
                    {
                        limitCounter++;
                        if (limitCounter <= 3)
                        {

                            i = 0;
                            usleep(1000 * 10);
                        }
                        else
                        {
                            g_offlineRAWCaptureYUVStepCounter = 2;
                        }
                    }

                    printf("tmpStat:%d\n", tmpStat);
                }
            }

            // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
            // struct timeval tv;
            // struct timezone tz;
            // gettimeofday(&tv, &tz);
            // long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            // LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
            std::string filePath = g_offline_raw_dir + "/" + raw_files.at(i);
            LOG_INFO("process raw 2: %s \n", filePath.c_str());

            if (g_offlineRAWCaptureYUVStepCounter == 0)
            {
                i = 0;
                g_offlineRAWCaptureYUVStepCounter = 1;
                continue;
            }
            g_domainTcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::seconds(2));
            if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(), (unsigned int)filePath.length() + 1) != 0)
            {
                LOG_ERROR("########################################################\n");
                LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
                LOG_ERROR("########################################################\n\n");
                // return 1;
            }
            else
            {
                char tmp[2048];
                g_domainTcpClient.Receive(tmp, sizeof(tmp));
            }
            g_domainTcpClient.m_inetSocketOperationMtx.unlock();
            g_offlineRawEnqueued.notify_one();

            //
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / g_offlineFrameRate));

            // gettimeofday(&tv, &tz);
            // long endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            // LOG_DEBUG("end millisecond: %ld\n", endTime);                                                   // ms
            // LOG_DEBUG("####################################### time spend: %ld ms\n", endTime - startTime); // ms
            // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE end\n");
        }
    }
    std::lock_guard<std::mutex> lk(mutex_);
    LOG_DEBUG("offlineRawProcess end\n");
    return 0;
}

int RKAiqProtocol::MessageForward(int sockfd, char* buffer, int size)
{
    std::lock_guard<std::mutex> lk(mutex_);
    // HexDump((unsigned char*)buffer, size);
    int ret = g_domainTcpClient.Send((char*)buffer, size);
    if (ret < 0 && errno != EINTR)
    {
        if (ConnectAiq() < 0)
        {
            g_domainTcpClient.Close();
            g_app_run_mode = APP_RUN_STATUS_INIT;
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
            LOG_ERROR("########################################################\n\n");
            close(sockfd);
            is_recv_running = false;
            return -1;
        }
        else
        {
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
            LOG_ERROR("########################################################\n\n");
            g_domainTcpClient.Send((char*)buffer, size);
        }
    }

#if 0
  if (forward_thread && forward_thread->joinable()) forward_thread->join();
#endif

    if (g_domainTcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::milliseconds(1)))
    {
        forward_thread = std::unique_ptr<std::thread>(new std::thread(&RKAiqProtocol::doMessageForward, sockfd));
        forward_thread->detach();
    }
    else
    {
        // LOG_DEBUG("message in process.\n");
    }

    return 0;
}

void RKAiqProtocol::Exit()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        is_recv_running = false;
    }
#if 0
  if (forward_thread && forward_thread->joinable()) {
    forward_thread->join();
  }
#endif
}
