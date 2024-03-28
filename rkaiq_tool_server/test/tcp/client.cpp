#include <signal.h>

#include "tcp_client.h"

void sig_exit(int s)
{
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./client ip port message");
        return 0;
    }
    signal(SIGINT, sig_exit);

    TCPClient tcpClient;
    tcpClient.Setup(argv[1], atoi(argv[2]));
    while (1)
    {
        tcpClient.Send(argv[3]);
        sleep(1);
    }
    return 0;
}
