#include <csignal>
#include <ctime>

#include "tcp_server.h"

int quit = 0;

void sigterm_handler(int sig)
{
    fprintf(stderr, "sigterm_handler signal %d\n", sig);
    quit = 1;
    exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./server port\n");
        return 0;
    }

    signal(SIGQUIT, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGXCPU, sigterm_handler);
    signal(SIGIO, sigterm_handler);
    signal(SIGPIPE, SIG_IGN);

    TCPServer tcpServer;
    tcpServer.Process(atoi(argv[1]));

    while (!quit)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
