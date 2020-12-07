#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#define MAXLINE 4096

static void prepare_set_wifi(void)
{
    int fd, size;
    char buf[200];

    fd = open("/proc/tcp_params", O_RDONLY);

    memset(buf, 0, 200);
    size = read(fd, buf, 200);

    system(buf);
    usleep(300 * 100);
    system("dhd_priv wl tcpka_conn_sess_info 1");
    usleep(300 * 100);
    system("dhd_priv wl tcpka_conn_dump 1");

}

int main(int argc, char** argv)
{
    int   sockfd, n;
    char  recvline[4096], sendline[4096];
    struct sockaddr_in  servaddr;
	int fd, size, send_count = 0;
	char buf[200];
	char recvbuffer[200];
	char sendbuf[2] = {0xc0, 0x00};

    if( argc != 2){
        printf("usage: ./client <ipaddress>\n");
        return 0;
    }

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5150);
    if ( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n",argv[1]);
        return 0;
    }

    char timestamp = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMP, (const char*)&timestamp, sizeof(char));

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s (errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    prepare_set_wifi();

    printf("connected to server \n");

    while (1) {
		if (send_count == 5) {
            system("dhd_priv wl tcpka_conn_sess_info 1");
            usleep(100 * 100);
            system("dhd_priv wl tcpka_conn_dump 0");
            usleep(100 * 100);
            system("dhd_priv wl tcpka_conn_enable 1 1 30 3 8");
            usleep(100 * 100);
            system("dhd_priv setsuspendmode 1");
            usleep(100 * 100);
            system("tb_poweroff &");
            while(1);
        }

        if (send(sockfd, "hellp tcp", 9, 0) != 9) {
			printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
			return 0;
    	}
    	send_count++;
    	sleep(1);
    }
    close(sockfd);
    return 0;
}
