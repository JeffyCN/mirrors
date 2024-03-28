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

bool TCPClient::Send(string data)
{
    if (sock != -1)
    {
        if (send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
        {
            LOG_ERROR("Send failed : %s\n", data.c_str());
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

int TCPClient::Send(char* buff, int size)
{
    int ret = -1;
    if (sock != -1)
    {
        ret = send(sock, buff, size, 0);
        if (ret <= 0)
        {
            LOG_ERROR("Send buff size %d failed\n", size);
            return ret;
        }
    }
    return ret;
}

string TCPClient::Receive(int size)
{
    char buffer[size];
    memset(&buffer[0], 0, sizeof(buffer));
    string reply;
    if (recv(sock, buffer, size, 0) < 0)
    {
        LOG_ERROR("receive failed %s !\n", strerror(errno));
        return "\0";
    }
    buffer[size - 1] = '\0';
    reply = buffer;
    return reply;
}

int TCPClient::Receive(char* buff, int size)
{
    int ret = -1;
    memset(buff, 0, size);
    ret = recv(sock, buff, size, 0);
    if (ret < 0)
    {
        LOG_ERROR("receive failed %s !\n", strerror(errno));
        return -1;
    }
    return ret;
}
