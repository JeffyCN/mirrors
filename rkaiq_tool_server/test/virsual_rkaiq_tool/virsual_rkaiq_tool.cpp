#include <signal.h>

#include "rkaiq_protocol.h"
#include "tcp_client.h"

#define SERVER_PORT 5543
#define MAX_BUFFER_SIZE 8192

#pragma pack(1)
typedef struct Common_Cmd_s
{
    uint8_t RKID[8];
    uint16_t cmdType;
    uint16_t cmdID;
    uint16_t datLen;
    uint8_t dat[48]; // defined by command
    uint16_t checkSum;
} Common_Cmd_t;
#pragma pack()

int capture_mode = 0;
int capture_count = 1;
void sig_exit(int s)
{
    exit(0);
}

static void ICMD_CheckStatus(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = CHECK_DEVICE_STATUS;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x80;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_GetVideoDevStatus(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x00;
    cmd->dat[1] = 0x80;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_GetPclkHtsVts(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x01;
    cmd->dat[1] = 0x00;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_GetSetParam(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x02;
    cmd->dat[1] = 0x10;
    cmd->dat[3] = 0x20;
    cmd->dat[5] = 0;
    cmd->dat[6] = 16;
    cmd->dat[7] = capture_count;
    cmd->dat[8] = capture_mode;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_DoCapture(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x03;
    cmd->dat[1] = 0x80;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_DoSumCheck(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x04;
    cmd->dat[1] = 0x00;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_GetAppStatus(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = VIDEO_APP_STATUS_REQ;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x80;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void ICMD_SetAppStatus(Common_Cmd_t* cmd)
{
    strncpy((char*)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = VIDEO_APP_STATUS_SET;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x80;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

void DumpCommand(char* buff)
{
    Common_Cmd_t* common_cmd = (Common_Cmd_t*)buff;
    for (int i = 0; i < common_cmd->datLen; i++)
    {
        fprintf(stderr, "data[%d]: 0x%x\n", i, common_cmd->dat[i]);
    }

    char rkid[9];
    memcpy(rkid, common_cmd->RKID, 8);
    rkid[8] = '\0';
    if (strcmp("RKISP-AS", rkid) == 0)
    {
        fprintf(stderr, "common_cmd RKID: %s\n", common_cmd->RKID);
    }
    else
    {
        fprintf(stderr, "Unknow command RKID\n");
    }
}

void CMD_CheckStatus(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_CheckStatus\n");
    ICMD_CheckStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_SetAppStatus(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_SetAppStatus\n");
    ICMD_SetAppStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_GetAppStatus(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_GetAppStatus\n");
    ICMD_GetAppStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_GetVideoDevStatus(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_GetVideoDevStatus\n");
    ICMD_GetVideoDevStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_GetPclkHtsVts(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_GetPclkHtsVts\n");
    ICMD_GetPclkHtsVts(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_GetSetParam(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_GetSetParam\n");
    ICMD_GetSetParam(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_DoCapture(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_DoCapture\n");
    ICMD_DoCapture(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    int ret_val = tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    while (sizeof(Common_Cmd_s) == MAX_BUFFER_SIZE)
    {
        tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    }
}

void CMD_DoSumCheck(TCPClient& tcpClient)
{
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];
    fprintf(stderr, "CMD_DoSumCheck\n");
    ICMD_DoSumCheck(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    tcpClient.Send(send_data, sizeof(Common_Cmd_s));
    tcpClient.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
}

void CMD_CaptureRaw(TCPClient& tcpClient)
{
    CMD_SetAppStatus(tcpClient);
    CMD_GetVideoDevStatus(tcpClient);
    CMD_GetPclkHtsVts(tcpClient);
    CMD_GetSetParam(tcpClient);
    CMD_DoCapture(tcpClient);
    CMD_DoSumCheck(tcpClient);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./%s ip msg_id\n", argv[0]);
        return 0;
    }
    signal(SIGINT, sig_exit);

    int msg_id = 0;
    int proc_id = 0;
    TCPClient tcpClient;
    tcpClient.Setup(argv[1], SERVER_PORT);
    msg_id = atoi(argv[2]);

    if (argc == 5)
    {
        capture_mode = atoi(argv[3]);
        capture_count = atoi(argv[4]);
    }

    int ret_val;
    Common_Cmd_t send_cmd;
    char send_data[MAX_BUFFER_SIZE];

    switch (msg_id)
    {
        case CHECK_DEVICE_STATUS:
            CMD_CheckStatus(tcpClient);
            break;
        case RAW_CAPTURE:
            CMD_CaptureRaw(tcpClient);
            break;
        default:
            break;
    }

    return 0;
}
