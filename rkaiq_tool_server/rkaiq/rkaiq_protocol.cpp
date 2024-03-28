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

#ifdef __ANDROID__
    #include <cutils/properties.h>
    #include <rtspserver/RtspServer.h>
#endif

#include "domain_tcp_client.h"
#include "tcp_server.h"
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
extern uint32_t g_offlineFrameRate;
extern DomainTCPClient g_tcpClient;
extern struct ucred* g_aiqCred;
extern std::string iqfile;
extern std::string g_sensor_name;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::string g_stream_dev_name;
extern std::shared_ptr<TCPServer> tcpServer;
extern int ConnectAiq();

bool RKAiqProtocol::is_recv_running = false;
std::unique_ptr<std::thread> RKAiqProtocol::forward_thread = nullptr;
std::unique_ptr<std::thread> RKAiqProtocol::offlineRawThread = nullptr;
std::mutex RKAiqProtocol::mutex_;

int g_offlineRawModeControl;

// int g_offlineRAWCaptureYUVStepCounter = -1;
int g_startOfflineRawFlag = 0;
// int g_inCaptureYUVProcess = 0;
// std::mutex g_offlineRawEnqueuedMutex;
// std::unique_lock<std::mutex> g_offlineRawEnqueuedLock(g_offlineRawEnqueuedMutex);
// std::condition_variable g_offlineRawEnqueued;
// std::mutex g_yuvCapturedMutex;
// std::unique_lock<std::mutex> g_yuvCapturedLock(g_yuvCapturedMutex);
// std::condition_variable g_yuvCaptured;

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
#ifdef __ANDROID__
    if (g_allow_killapp)
    {
        unlink(LOCAL_SOCKET_PATH);
        property_set("ctrl.stop", "cameraserver");
        property_set("ctrl.stop", "vendor.camera-provider-2-4");
        property_set("ctrl.stop", "vendor.camera-provider-2-4-ext");
        system("stop cameraserver");
        system("stop vendor.camera-provider-2-4");
        system("stop vendor.camera-provider-2-4-ext");
    }
#else
    if (g_allow_killapp)
    {
        if (g_aiqCred != nullptr)
        {
            kill(g_aiqCred->pid, SIGTERM);
            delete g_aiqCred;
            g_aiqCred = nullptr;
        }
    }
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int RKAiqProtocol::StartApp()
{
    int ret = -1;
#ifdef __ANDROID__
    if (g_allow_killapp)
    {
        property_set("ctrl.start", "cameraserver");
        system("start cameraserver");
        system("start vendor.camera-provider-2-4");
        system("start vendor.camera-provider-2-4-ext");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif
    return 0;
}

int RKAiqProtocol::StartRTSP()
{
    LOG_DEBUG("Not start RTSP.");
    return -1;
}

int RKAiqProtocol::StopRTSP()
{
    LOG_DEBUG("Stopped RTSP !!!");
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
    LOG_DEBUG("enter\n");

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
    LOG_DEBUG("exit\n");
}

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret = -1;

    LOG_DEBUG("HandlerCheckDevice:\n");

    // for (int i = 0; i < common_cmd->datLen; i++) {
    //   LOG_DEBUG("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
    // }

    if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0)
    {
        LOG_DEBUG("RKID: %s\n", common_cmd->RKID);
    }
    else
    {
        LOG_DEBUG("RKID: Unknow\n");
        return;
    }

    LOG_DEBUG("cmdID: %d\n", common_cmd->cmdID);

    switch (common_cmd->cmdID)
    {
        case CMD_ID_CAPTURE_STATUS:
            LOG_DEBUG("CmdID CMD_ID_CAPTURE_STATUS in\n");
            if (common_cmd->dat[0] == KNOCK_KNOCK)
            {
                InitCommandPingAns(&send_cmd, READY);
                LOG_DEBUG("Device is READY\n");
            }
            else
            {
                LOG_ERROR("Unknow CMD_ID_CAPTURE_STATUS message\n");
            }
            memcpy(send_data, &send_cmd, sizeof(CommandData_t));
            send(sockfd, send_data, sizeof(CommandData_t), 0);
            LOG_DEBUG("cmdID CMD_ID_CAPTURE_STATUS out\n\n");
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
    LOG_DEBUG("FILETRANS receive: receivedData.dataSize:%u\n", receivedData.dataSize);
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
        g_startOfflineRawFlag = 1;
        LOG_DEBUG("offlineRawProcess begin\n");
        while (g_startOfflineRawFlag == 1)
        {
            DIR* dir = opendir("/data/OfflineRAW");
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
                LOG_INFO("No raw files in /data/OfflineRAW\n");
                return;
            }

            std::sort(raw_files.begin(), raw_files.end(), natural_less);
            for (auto raw_file : raw_files)
            {
                cout << raw_file.c_str() << endl;
                if (g_startOfflineRawFlag == 0)
                {
                    break;
                }
                // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
                struct timeval tv;
                struct timezone tz;
                gettimeofday(&tv, &tz);
                long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
                LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
                std::string filePath = "/data/OfflineRAW/" + raw_file;
                LOG_INFO("process raw : %s \n", filePath.c_str());

                g_tcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::seconds(2));
                if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(), (unsigned int)filePath.length() + 1) != 0)
                {
                    LOG_ERROR("########################################################\n");
                    LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
                    LOG_ERROR("########################################################\n\n");
                    // return 1;
                }
                else
                {
                    char tmp[1024];
                    g_tcpClient.Receive(tmp, sizeof(tmp));
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
                g_tcpClient.m_inetSocketOperationMtx.unlock();
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
        system("rm -f /data/OfflineRAW/* && sync");
    }
    else if (receivedData.offlineRawModeControl == 4) // clear raw files
    {
        system("rm -f /data/OfflineRAW/* && sync");
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
        g_tcpClient.Receive(tmp, sizeof(tmp));
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
        std::string pattern{"Isp online"};
        std::regex re(pattern);
        std::smatch results;
        ExecuteCMD("cat /proc/rkisp0-vir0", result);
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
        int recv_len = g_tcpClient.Receive(recv_buffer, MAXPACKETSIZE);
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
                g_tcpClient.m_inetSocketOperationMtx.unlock();
                return -1;
            }
        }
        else if (recv_len <= 0)
        {
            if (errno != EAGAIN)
            {
                g_tcpClient.Close();
                close(sockfd);
                std::lock_guard<std::mutex> lk(mutex_);
                is_recv_running = false;
                g_tcpClient.m_inetSocketOperationMtx.unlock();
                return -1;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopReceiveTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(100))
                {
                    LOG_DEBUG("stop receive\n");
                    std::lock_guard<std::mutex> lk(mutex_);
                    is_recv_running = false;
                }
            }
        }
    }

    std::lock_guard<std::mutex> lk(mutex_);
    is_recv_running = false;
    g_tcpClient.m_inetSocketOperationMtx.unlock();
    return 0;
}

int RKAiqProtocol::offlineRawProcess(int sockfd)
{
    g_startOfflineRawFlag = 1;
    LOG_DEBUG("offlineRawProcess begin\n");
    while (g_startOfflineRawFlag == 1)
    {
        DIR* dir = opendir("/data/OfflineRAW");
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
            LOG_INFO("No raw files in /data/OfflineRAW\n");
            return 1;
        }

        std::sort(raw_files.begin(), raw_files.end(), natural_less);
        for (auto raw_file : raw_files)
        // for (int i = 0; i < raw_files.size(); i++)
        {
            // cout << raw_file.c_str() << endl;
            if (g_startOfflineRawFlag == 0)
            {
                break;
            }
            // if (g_inCaptureYUVProcess == 1)
            // if (false)
            // {
            //     // if (g_offlineRAWCaptureYUVStepCounter == 0)
            //     // {
            //     //     g_offlineRAWCaptureYUVStepCounter = 1;
            //     //     i = 0;
            //     // }
            //     // else if (g_offlineRAWCaptureYUVStepCounter == 1)
            //     // {
            //     //     g_yuvCaptured.wait(g_yuvCapturedLock);
            //     // }

            //     // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
            //     // struct timeval tv;
            //     // struct timezone tz;
            //     // gettimeofday(&tv, &tz);
            //     // long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            //     // LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
            //     std::string filePath = "/data/OfflineRAW/" + raw_files.at(i);
            //     LOG_INFO("process raw 1: %s \n", filePath.c_str());

            //     g_tcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::seconds(2));
            //     if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(), (unsigned int)filePath.length() + 1) != 0)
            //     {
            //         LOG_ERROR("########################################################\n");
            //         LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
            //         LOG_ERROR("########################################################\n\n");
            //         // return 1;
            //     }
            //     else
            //     {
            //         char tmp[1024];
            //         g_tcpClient.Receive(tmp, sizeof(tmp));
            //     }
            //     g_tcpClient.m_inetSocketOperationMtx.unlock();

            //     uint32_t frameInterval = 1000 / g_offlineFrameRate;
            //     frameInterval = frameInterval - 50;
            //     if (frameInterval < 0)
            //     {
            //         frameInterval = 0;
            //     }
            //     std::this_thread::sleep_for(std::chrono::milliseconds(frameInterval));

            //     // gettimeofday(&tv, &tz);
            //     // long endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            //     // LOG_DEBUG("end millisecond: %ld\n", endTime);                                                   // ms
            //     // LOG_DEBUG("####################################### time spend: %ld ms\n", endTime - startTime); // ms
            //     // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE end\n");
            //     // g_offlineRawEnqueued.notify_one();
            // }
            // else
            {
                // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
                // struct timeval tv;
                // struct timezone tz;
                // gettimeofday(&tv, &tz);
                // long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
                // LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
                // std::string filePath = "/data/OfflineRAW/" + raw_files.at(i);
                std::string filePath = "/data/OfflineRAW/" + raw_file;
                LOG_INFO("process raw 2: %s \n", filePath.c_str());

                g_tcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::seconds(2));
                if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(), (unsigned int)filePath.length() + 1) != 0)
                {
                    LOG_ERROR("########################################################\n");
                    LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
                    LOG_ERROR("########################################################\n\n");
                    // return 1;
                }
                else
                {
                    char tmp[1024];
                    g_tcpClient.Receive(tmp, sizeof(tmp));
                }
                g_tcpClient.m_inetSocketOperationMtx.unlock();

                uint32_t frameInterval = 1000 / g_offlineFrameRate;
                frameInterval = frameInterval - 50;
                if (frameInterval < 0)
                {
                    frameInterval = 0;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(frameInterval));

                // gettimeofday(&tv, &tz);
                // long endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
                // LOG_DEBUG("end millisecond: %ld\n", endTime);                                                   // ms
                // LOG_DEBUG("####################################### time spend: %ld ms\n", endTime - startTime); // ms
                // LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE end\n");
            }
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
    int ret = g_tcpClient.Send((char*)buffer, size);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR))
    {
        if (ConnectAiq() < 0)
        {
            g_tcpClient.Close();
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
        }
    }

#if 0
  if (forward_thread && forward_thread->joinable()) forward_thread->join();
#endif

    if (g_tcpClient.m_inetSocketOperationMtx.try_lock_for(std::chrono::milliseconds(1)))
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
