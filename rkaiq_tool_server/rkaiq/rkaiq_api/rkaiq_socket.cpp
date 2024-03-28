#include "rkaiq_socket.h"

#include <time.h>

#include "domain_tcp_client.h"
#if 0
    #include "rkaiq_manager.h"
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

#define MAX_PACKET_SIZE 8192

extern DomainTCPClient g_tcpClient;
extern int g_app_run_mode;
extern string g_linuxSocketDomainPath;

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

typedef struct RkAiqSocketDataV2
{
    uint8_t magic[4]; // = { 'R', 0xAA, 0xFF, 'K' };
    int32_t cmd_id;
    int32_t cmd_ret;
    uint32_t sequence;
    uint32_t payload_size;
    uint32_t packet_size;
    uint8_t* data;
    uint32_t data_hash; // different offset in data stream
} RkAiqSocketDataV2;
#pragma pack()

unsigned int MurMurHash(const void* key, int len)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char* data = (const unsigned char*)key;
    while (len >= 4)
    {
        unsigned int k = *(unsigned int*)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch (len)
    {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
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

int ConnectAiq()
{
#ifdef __ANDROID__
    if (g_tcpClient.Setup(LOCAL_SOCKET_PATH) == false)
    {
        LOG_DEBUG("domain connect failed\n");
        g_tcpClient.Close();
        return -1;
    }
#else
    if (g_tcpClient.Setup(g_linuxSocketDomainPath.c_str()) == false)
    {
        LOG_INFO("domain connect failed\n");
        g_tcpClient.Close();
        return -1;
    }
    else
    {
        LOG_INFO("#### Socket connect AIQ success ####\n");
    }
#endif
    return 0;
}

int RkAiqSocketClientINETSend(int commandID, void* data, unsigned int dataSize)
{
    int commandReturnValue = 0;

    struct timeval startTime;
    struct timeval currentTime;

    // |sendTotalSize|command id|command data|data hash|
    RkAiqSocketData packetData;
    // packetData.magic = xxxx;
    packetData.commandID = commandID;
    packetData.commandResult = -1;
    packetData.dataSize = dataSize;
    packetData.data = (char*)malloc(dataSize);
    memcpy(packetData.data, data, dataSize);
    packetData.dataHash = MurMurHash(packetData.data, dataSize);

    unsigned int packetSize = sizeof(packetData) + dataSize - sizeof(char*);
    memcpy(packetData.packetSize, &packetSize, 4);

    char* dataToSend = (char*)malloc(packetSize);
    int offset = 0;
    memset(dataToSend, 0, packetSize);
    memcpy(dataToSend, packetData.magic, sizeof(packetData.magic));
    offset += sizeof(packetData.magic);
    memcpy(dataToSend + offset, packetData.packetSize, 4);
    offset += 4;
    memcpy(dataToSend + offset, (void*)&packetData.commandID, sizeof(packetData.commandID));
    offset += sizeof(packetData.commandID);
    memcpy(dataToSend + offset, (void*)&packetData.commandResult, sizeof(packetData.commandResult));
    offset += sizeof(packetData.commandResult);
    memcpy(dataToSend + offset, (void*)&packetData.dataSize, sizeof(packetData.dataSize));
    offset += sizeof(packetData.dataSize);
    memcpy(dataToSend + offset, packetData.data, packetData.dataSize);
    offset += packetData.dataSize;
    memcpy(dataToSend + offset, (void*)&packetData.dataHash, sizeof(packetData.dataHash));

    // LOG_DEBUG("INET send: dataHash %08x\n", packetData.dataHash);
    // LOG_DEBUG("INET send: packetSize %d\n", packetSize);
    // LOG_DEBUG("INET send: dataSize %d\n", dataSize);

    int ret = g_tcpClient.Send(dataToSend, packetSize);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR))
    {
        if (ConnectAiq() < 0)
        {
            g_tcpClient.Close();
            g_app_run_mode = -1; // APP_RUN_STATUS_INIT = -1
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
            LOG_ERROR("########################################################\n\n");
            return 1;
        }
        else
        {
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
            LOG_ERROR("########################################################\n\n");
        }
    }

    // LOG_DEBUG("############# inet send data ###################\n");
    // HexDump((unsigned char*)dataToSend, packetSize);

    free(packetData.data);
    packetData.data = NULL;
    free(dataToSend);
    dataToSend = NULL;

    // // receive data
    // RkAiqSocketData retPacket;
    // memset((void*)&retPacket, 0, sizeof(RkAiqSocketData));
    // g_tcpClient.Receive((char*)&retPacket, sizeof(RkAiqSocketData));

    // LOG_DEBUG("############# inet return data ###################\n");
    // HexDump((unsigned char*)&retPacket, sizeof(RkAiqSocketData));
    // if (memcmp(retPacket.magic, "RK", 2) != 0)
    // {
    //     LOG_DEBUG("INET send: return value maigc check failed,return\n");
    //     g_tcpClient.Receive(MAX_PACKET_SIZE);
    //     return 1;
    // }

    // commandReturnValue = retPacket.commandResult;
    // LOG_DEBUG("INET send: return value:%d\n", commandReturnValue);

    return commandReturnValue;
}

// return:  0-failed 1-success
int RkAiqSocketClientINETReceive(int commandID, void* data, unsigned int dataSize)
{
    // |sendTotalSize|command id|command data|data hash|
    // send request
    RkAiqSocketData packetData;
    // packetData.magic = xxxx;
    packetData.commandID = commandID;
    packetData.commandResult = -1;
    packetData.dataSize = 3;
    packetData.data = (char*)malloc(3);
    memcpy(packetData.data, "GET", 3);
    packetData.dataHash = MurMurHash(packetData.data, 3);

    unsigned int packetSize = sizeof(packetData) + 3 - sizeof(char*);
    memcpy(packetData.packetSize, &packetSize, 4);

    char* dataToSend = (char*)malloc(packetSize);
    int offset = 0;
    memset(dataToSend, 0, packetSize);
    memcpy(dataToSend, packetData.magic, sizeof(packetData.magic));
    offset += sizeof(packetData.magic);
    memcpy(dataToSend + offset, packetData.packetSize, 4);
    offset += 4;
    memcpy(dataToSend + offset, (void*)&packetData.commandID, sizeof(packetData.commandID));
    offset += sizeof(packetData.commandID);
    memcpy(dataToSend + offset, (void*)&packetData.commandResult, sizeof(packetData.commandResult));
    offset += sizeof(packetData.commandResult);
    memcpy(dataToSend + offset, (void*)&packetData.dataSize, sizeof(packetData.dataSize));
    offset += sizeof(packetData.dataSize);
    memcpy(dataToSend + offset, packetData.data, packetData.dataSize);
    offset += packetData.dataSize;
    memcpy(dataToSend + offset, (void*)&packetData.dataHash, sizeof(packetData.dataHash));

    // LOG_DEBUG("INET receive 1: dataHash %08x\n", packetData.dataHash);

    int ret = g_tcpClient.Send(dataToSend, packetSize);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR))
    {
        if (ConnectAiq() < 0)
        {
            g_tcpClient.Close();
            g_app_run_mode = -1; // APP_RUN_STATUS_INIT = -1
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
            LOG_ERROR("########################################################\n\n");
            return 1;
        }
        else
        {
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
            LOG_ERROR("########################################################\n\n");
        }
    }

    if (packetData.data != NULL)
    {
        free(packetData.data);
        packetData.data = NULL;
    }
    if (dataToSend != NULL)
    {
        free(dataToSend);
        dataToSend = NULL;
    }

    // receive data
    char tmpStr[6] = {};
    g_tcpClient.Receive(tmpStr, 6);
    // LOG_DEBUG("INET receive : magic & header received\n");
    if (tmpStr[0] != 'R' || tmpStr[1] != 'K')
    {
        // LOG_DEBUG("INET receive : packet magic check failed. return\n");
        g_tcpClient.Receive(MAX_PACKET_SIZE);
        return 1;
    }
    // LOG_DEBUG("INET receive : packet magic check pass.\n");

    packetSize = (tmpStr[2] & 0xff) | ((tmpStr[3] & 0xff) << 8) | ((tmpStr[4] & 0xff) << 16) | ((tmpStr[5] & 0xff) << 24);
    // LOG_DEBUG("INET receive : packetSize:%u\n", packetSize);
    // LOG_DEBUG("INET receive : dataSize:%u\n", dataSize);
    if (packetSize <= 0 || packetSize - dataSize > 200)
    {
        printf("INET no data received or packetSize error, return.\n");
        return 1;
    }

    char* receivedPacket = (char*)malloc(packetSize);
    memset(receivedPacket, 0, packetSize);
    memcpy(receivedPacket, tmpStr, 6);

    int remain_size = packetSize - 6;
    int recv_size = 0;

    struct timespec startTime = {0, 0};
    struct timespec currentTime = {0, 0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    // printf("INET get, start receive:%ld\n", startTime.tv_sec);
    while (remain_size > 0)
    {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        if (currentTime.tv_sec - startTime.tv_sec >= 2)
        {
            LOG_DEBUG("INET receive: receive data timeout, return\n");
            return 1;
        }

        int offset = packetSize - remain_size;
        int targetSize = 0;
        if (remain_size > MAX_PACKET_SIZE)
        {
            targetSize = MAX_PACKET_SIZE;
        }
        else
        {
            targetSize = remain_size;
        }
        recv_size = g_tcpClient.Receive(&receivedPacket[offset], targetSize);
        remain_size = remain_size - recv_size;
    }
    // LOG_DEBUG("INET receive: receive success, need check data\n");

    // hexdump(receivedPacket, packetSize);

    // g_tcpClient.Send(receivedPacket, packetSize); //for debug use

    // parse data
    RkAiqSocketData receivedData;
    offset = 0;
    // magic
    memcpy(receivedData.magic, receivedPacket, 2);
    offset += 2;
    // packetSize
    memcpy(receivedData.packetSize, receivedPacket + offset, 4);
    offset += 4;
    // command id
    memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // command result
    memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // data size
    memcpy((void*)&(receivedData.dataSize), receivedPacket + offset, sizeof(unsigned int));
    // LOG_DEBUG("INET receive: receivedData.dataSize:%u\n", receivedData.dataSize);
    offset += sizeof(unsigned int);
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
        LOG_DEBUG("INET receive: receivedData.dataSize != target data size, return\n");
        return 1;
    }

    // hash check
    unsigned int dataHash = MurMurHash(receivedData.data, receivedData.dataSize);
    // LOG_DEBUG("INET receive 2: dataHash calculated:%x\n", dataHash);
    // LOG_DEBUG("INET receive: receivedData.dataHash:%x\n", receivedData.dataHash);

    if (dataHash == receivedData.dataHash)
    {
        // LOG_DEBUG("INET receive: data hash check pass\n");
    }
    else
    {
        LOG_DEBUG("INET receive: data hash check failed\n");
    }

    // return data value
    memcpy(data, (void*)receivedData.data, receivedData.dataSize);
    if (receivedData.data != NULL)
    {
        free(receivedData.data);
        receivedData.data = NULL;
    }
    return 0;
}
