#include "domain_tcp_client.h"

#include <errno.h>

#ifdef __ANDROID__
    #include <android-base/file.h>
    #include <cutils/sockets.h>
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern DomainTCPClient g_domainTcpClient;

DomainTCPClient::DomainTCPClient()
{
    sock = -1;
}

DomainTCPClient::~DomainTCPClient()
{
    Close();
}

void DomainTCPClient::Close()
{
    if (sock > 0)
    {
        close(sock);
        sock = -1;
    }
}

bool DomainTCPClient::Setup(string domainPath)
{
    LOG_ERROR("DomainTCPClient::Setup, domainPath:%s\n", domainPath.c_str());
    if (sock == -1)
    {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1)
        {
            LOG_ERROR("Could not create domain socket\n");
            return false;
        }
        else
        {
            LOG_DEBUG("Linux,Create domain socket success.\n");
        }
    }

    server.sun_family = AF_UNIX;
    // strcpy(server.sun_path, domainPath.c_str());
    strncpy(server.sun_path, domainPath.c_str(), sizeof(server.sun_path) - 1);
    server.sun_path[sizeof(server.sun_path) - 1] = '\0';

    struct timeval timeout = {0, 50 * 1000};
    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    LOG_ERROR("server.sun_path:%s\n", server.sun_path);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        LOG_ERROR("connect domain server failed. Error\n");
        close(sock);
        sock = -1;
        return false;
    }

    return true;
}

bool DomainTCPClient::Send(string data)
{
    if (sock != -1)
    {
        if (send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
        {
            LOG_ERROR("Send failed : %s\n", data.c_str());
            Close();
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

int DomainTCPClient::Send(char* buff, int size)
{
    int ret = -1;
    if (sock != -1)
    {
        ret = send(sock, buff, size, 0);
        if (ret < 0 && (errno != EAGAIN && errno != EINTR))
        {
            LOG_ERROR("Send buff size %d failed\n", size);
            Close();
            return ret;
        }
    }
    return ret;
}

string DomainTCPClient::Receive(int size)
{
    char buffer[size];
    memset(&buffer[0], 0, sizeof(buffer));
    string reply;
    if (sock < 0)
    {
        return "\0";
    }
    ssize_t ret = recv(sock, buffer, size, 0);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR))
    {
        LOG_ERROR("domain receive 1 failed %s!\n", strerror(errno));
        Close();
        return "\0";
    }
    buffer[size - 1] = '\0';
    reply = buffer;
    return reply;
}

int DomainTCPClient::Receive(char* buff, int size)
{
    ssize_t ret = -1;
    if (sock < 0)
    {
        return -1;
    }
    memset(buff, 0, size);
    ret = recv(sock, buff, size, 0);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR))
    {
        LOG_ERROR("domain receive 2 failed %s!\n", strerror(errno));
        Close();
        return -1;
    }
    return ret;
}
