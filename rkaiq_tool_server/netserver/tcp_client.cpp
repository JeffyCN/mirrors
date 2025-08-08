#include "tcp_client.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

TCPClient::TCPClient()
{
    sock = -1;
    port = 0;
    address = "";
}

TCPClient::~TCPClient()
{
    if (sock > 0)
    {
        close(sock);
    }
}

bool TCPClient::Setup(string address, int port)
{
    if (sock == -1)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            LOG_ERROR("Could not create socket\n");
        }
    }
    if ((signed)inet_addr(address.c_str()) == -1)
    {
        struct hostent* he;
        struct in_addr** addr_list;
        if ((he = gethostbyname(address.c_str())) == NULL)
        {
            herror("gethostbyname");
            LOG_ERROR("Failed to resolve hostname\n");
            return false;
        }
        addr_list = (struct in_addr**)he->h_addr_list;
        // for (int i = 0; addr_list[i] != NULL; i++)
        {
            server.sin_addr = *addr_list[0];
            // break;
        }
    }
    else
    {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        LOG_ERROR("connect failed. Error");
        return false;
    }
    return true;
}

int TCPClient::Send(char* buff, int size)
{
    int ret = -1;
    int sendOffset = 0;
    auto stopSendTimer = std::chrono::high_resolution_clock::now();
    while (true)
    {
        int sendSize = send(sock, buff + sendOffset, size, 0);
        if (sendSize > 0)
        {
            stopSendTimer = std::chrono::high_resolution_clock::now();
            sendOffset += sendSize;
        }
        else if (sendSize <= 0)
        {
            if (sendOffset == size)
            {
                LOG_DEBUG("tcp send to pc finish.\n");
                ret = 0;
                break;
            }

            if (errno != EAGAIN)
            {
                close(sock);
                break;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopSendTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(500))
                {
                    LOG_DEBUG("tcp send to pc OK %f\n", waitTime.count());
                    break;
                }
            }
        }
    }

    return ret;
}

int TCPClient::Receive(char* buff, int size)
{
    int ret = -1;
    int receiveOffset = 0;
    auto stopReceiveTimer = std::chrono::high_resolution_clock::now();
    while (true)
    {
        int recvSize = recv(sock, buff + receiveOffset, size, 0);
        if (recvSize > 0)
        {
            stopReceiveTimer = std::chrono::high_resolution_clock::now();
            receiveOffset += recvSize;
        }
        else if (recvSize <= 0)
        {
            if (receiveOffset == size)
            {
                LOG_DEBUG("tcp recv from pc finish.\n");
                ret = 0;
                break;
            }

            if (errno != EAGAIN)
            {
                close(sock);
                break;
            }
            else
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> waitTime = now - stopReceiveTimer;
                if (std::chrono::duration<double, std::milli>(waitTime) > std::chrono::duration<double, std::milli>(500))
                {
                    LOG_DEBUG("tcp recv from pc OK %f\n", waitTime.count());
                    break;
                }
            }
        }
    }

    return ret;
}
