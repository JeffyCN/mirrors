#include "tcp_server.h"

#include <atomic>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

TCPServer::~TCPServer()
{
    SaveExit();
}

void TCPServer::SaveExit()
{
    quit_.store(true, std::memory_order_release);
    if (accept_thread_ && accept_thread_->joinable())
        accept_thread_->join();
    std::for_each(recv_threads_.begin(), recv_threads_.end(), [](const std::unique_ptr<std::thread>& thrd) {
        if (thrd && thrd->joinable())
            thrd->join();
    });
    close(sockfd);
    sockfd = -1;
}

int TCPServer::Send(int cilent_socket, char* buff, int size)
{
    return send(cilent_socket, buff, size, 0);
}

int TCPServer::Recvieve(int cilent_socket)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    std::thread::id threadID = std::this_thread::get_id();
    LOG_DEBUG("TCPServer::Recvieve enter %d\n", cilent_socket);
    char buffer[MAXPACKETSIZE];
    int size = sizeof(buffer);
    struct timeval timeout = {1, 0};
    setsockopt(cilent_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    while (!quit_.load())
    {
        int length = recv(cilent_socket, buffer, size, 0);
        if (length == 0)
        {
            LOG_DEBUG("socket recvieve exit\n");
            break;
        }
        else if (length < 0 && errno == EAGAIN)
        {
            // LOG_INFO("socket recvieve failed\n");
            continue;
        }
        else if (length < 0)
        {
            break;
        }
        // LOG_DEBUG("socket recvieve length: %d\n", length);

        if (callback_)
        {
            callback_(cilent_socket, buffer, length);
        }
    }

    LOG_DEBUG("TCPServer::Recvieve exit %d\n", cilent_socket);
    close(cilent_socket);
    cilent_socket = -1;
    //
    recv_threads_finished_id_.push_back(threadID);
    LOG_DEBUG("TCPServer::recv_threads_finished_id_ len: %d\n", recv_threads_finished_id_.size());
    return 0;
}

void TCPServer::Accepted()
{
    LOG_INFO("TCPServer::Accepted\n");
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    struct timeval timeout = {1, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    int reuseTrue = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseTrue, sizeof(int));
    while (!quit_)
    {
        int cilent_socket;
        socklen_t sosize = sizeof(clientAddress);
        cilent_socket = accept(sockfd, (struct sockaddr*)&clientAddress, &sosize);

        if (cilent_socket < 0)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                LOG_ERROR("Error socket accept failed %d %d\n", cilent_socket, errno);
                break;
            }
            continue;
        }
        LOG_DEBUG("socket accept ip %s\n", inet_ntoa(clientAddress.sin_addr));

        recv_threads_.push_back(std::unique_ptr<std::thread>(new std::thread(&TCPServer::Recvieve, this, cilent_socket)));
        LOG_DEBUG("socket accept close\n");
    }
    close(sockfd);
    sockfd = -1;
    exited_.store(true, std::memory_order_release);
    LOG_DEBUG("socket accept exit\n");
}

int TCPServer::Process(int port)
{
    exited_.store(false, std::memory_order_release);
    LOG_DEBUG("TCPServer::Process\n");
    int opt = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG_ERROR("Failed to create socket with tunner");
        exited_.store(true, std::memory_order_release);
        return -1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        LOG_ERROR("Error setsockopt\n");
        exited_.store(true, std::memory_order_release);
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    if ((::bind(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) < 0)
    {
        LOG_ERROR("Error bind\n");
        exited_.store(true, std::memory_order_release);
        return -1;
    }
    if (listen(sockfd, 5) < 0)
    {
        LOG_ERROR("Error listen\n");
        exited_.store(true, std::memory_order_release);
        return -1;
    }

    if (accept_thread_)
    {
        // SaveExit();
    }
    quit_.store(false, std::memory_order_release);
    accept_thread_ = std::unique_ptr<std::thread>(new std::thread(&TCPServer::Accepted, this));

    return 0;
}
