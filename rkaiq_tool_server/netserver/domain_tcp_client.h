#ifndef DOMAIN_TCP_CLIENT_H
#define DOMAIN_TCP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <mutex>

#include <string>

#include "logger/log.h"

#ifdef __ANDROID__
    #define LOCAL_SOCKET_PATH "/dev/socket/camera_tool"
#endif

#define SERVER_PORT 5543
#define UNIX_DOMAIN "/tmp/UNIX.domain0"

using namespace std;

class DomainTCPClient
{
  private:
    std::string address;
    int port;
    struct sockaddr_un server;

  public:
    int sock;
    std::timed_mutex m_inetSocketOperationMtx;
    DomainTCPClient();
    virtual ~DomainTCPClient();
    bool Setup(string domainPath);
    bool Send(string data);
    int Send(char* buff, int size);
    string Receive(int size);
    int Receive(char* buff, int size);
    void Close();
};

#endif
