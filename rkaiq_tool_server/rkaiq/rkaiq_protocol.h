#ifndef _RKAIQ_PROTOCOL_H__
#define _RKAIQ_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mutex>

#include "logger/log.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_media.h"
#include "rkaiq_online_protocol.h"
#include "rkaiq_raw_protocol.h"

int StopProcess(const char* process, const char* str);
int WaitProcessExit(const char* process, int sec);

class RKAiqProtocol
{
  public:
    RKAiqProtocol() = default;
    virtual ~RKAiqProtocol() = default;
    static int DoChangeAppMode(appRunStatus mode);
    static void HandlerTCPMessage(int sockfd, char* buffer, int size);
    static void HandlerCheckDevice(int sockfd, char* buffer, int size);
    static void HandlerReceiveFile(int sockfd, char* buffer, int size);
    static void HandlerOfflineRawProcess(int sockfd, char* buffer, int size);
    static void HandlerGetAWBParaFileProcess(int sockfd, char* buffer, int size);
    static int MessageForward(int sockfd, char* buffer, int size);
    static int doMessageForward(int sockfd);
    static int offlineRawProcess(int sockfd);
    static void KillApp();
    static int StartApp();
    static int StartRTSP();
    static int StopRTSP();
    static void Exit();
    static std::unique_ptr<std::thread> offlineRawThread;

  private:
    static std::mutex mutex_;
    static bool is_recv_running;
    static std::unique_ptr<std::thread> forward_thread;
};

#endif
