#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void *deal_client(void *sockfd);
int strupr(char *protocol);
void udpServer();
void tcpServer();

#define IP "192.168.2.101"
#define PORT "8888"

int tcpStreamLen = 0;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("please enter the protocol(UDP/TCP) to listen\n");
        return -1;
    }
    int res = strupr(argv[1]);
    if (!res)
    {
        printf("protocol Error!\n");
        return -1;

    }
    printf("%s:%s\n", IP, PORT);
    if (!strcmp(argv[1], "TCP"))
    {
        tcpServer();
    }
    else if(!strcmp(argv[1], "UDP"))
    {
        udpServer();
    }
    else
    {
        printf("Only support TCP/UDP!\n");
        return -1;
    }
    return 0;
}


void tcpServer()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in myaddr;
    bzero(&myaddr,sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(atoi(PORT));
    inet_pton(AF_INET, IP, &myaddr.sin_addr.s_addr);

    int on = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0 )
	{
	perror("setSockOpt");
	_exit(-1);
	}
    
    int ret = bind(sockfd,(struct sockaddr *)&myaddr,sizeof(myaddr));
    if (ret < 0)
    {
        perror("tcp bind");
        _exit(-1);
    }
    listen(sockfd, 1);

    while (1)
    {
        struct sockaddr_in from;
        bzero(&from, sizeof(from));
        socklen_t from_len = sizeof(from);

        int new_fd = accept(sockfd, (struct sockaddr *)&from, &from_len);
        if (new_fd == -1) 
        {
            printf("accept error!\n");
            _exit(-1);
        }
        unsigned short port = ntohs(from.sin_port);
        char ip[16] = "";
        inet_ntop(AF_INET, &from.sin_addr.s_addr, ip, 16);
        printf("src : %s : %hu\n", ip, port);

        pthread_t tid;
        pthread_create(&tid, NULL, deal_client, (void *)&new_fd);
        pthread_detach(tid);
	printf("tcp sum byte received is %d\n", tcpStreamLen);
        
    }
    close(sockfd);
}

void *deal_client(void *sockfd)
{
    int fd = *(int *)sockfd;
    int count  = 0;
    while(1)
    {
        char buf[3001] = "";
        int len = recv(fd, buf, sizeof(buf), 0);
        if (len == 0) break;
	tcpStreamLen += len;
        count++;
        printf("tcp byte received is %d\n", tcpStreamLen);
    }
}

void udpServer()
{
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);

    struct sockaddr_in myaddr;
    bzero(&myaddr,sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(atoi(PORT));
    inet_pton(AF_INET, IP, &myaddr.sin_addr .s_addr);

    int ret = bind(sockfd,(struct sockaddr *)&myaddr,sizeof(myaddr));
    if (ret < 0)
    {
        perror("bind");
        _exit(-1);
    }
    int count = 0;
    while (1)
    {
        char buf[3001] = "";
        
        struct sockaddr_in from;
        bzero(&from, sizeof(from));
        socklen_t from_len = sizeof(from);

        int len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len);
        if (len == -1)
        {
            printf("something is error!\n");
            break;
        }
        unsigned short port = ntohs(from.sin_port);
        char ip[16] = "";
        inet_ntop(AF_INET, &from.sin_addr.s_addr, ip, 16);

        count++;
        //if(count % 10000 == 0)
	printf("NO.%d\tsrc : %s : %hu\tthe length is : %d\n", count, ip, port, len);
    }
    close(sockfd);
}

int strupr(char *protocol)
{
    int i = 0;
    while(protocol[i] != '\0')
    {
        if (protocol[i] >= 'a' && protocol[i] <= 'z')
        {
            protocol[i] -= 32;
        }
        else if(protocol[i] >= 'A' && protocol[i] <= 'Z')
        {
            i++;
            continue;
        }
        else{
            return 0;
        }
        i++;
    }
    return 1;
}
