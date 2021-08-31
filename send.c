#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/time.h>

int strupr(char *protocol);
void udpProcess(int pkt_size, int pkt_num, int period, int sleep_microS);
void tcpProcess(int pkt_size, int pkt_num, int period, int sleep_microS);

#define SRC_IP "192.168.2.101"
#define SRC_PORT "8080"
#define DST_IP "192.168.2.201"
#define DST_PORT "8888"

int main(int argc, char *argv[])
{

    if (argc != 6)
    {
        printf("please enter the protocol(UDP/TCP), size of a packet, number of packets, period of sending process(unit:number of packets), sleep time of a period(microsecond)\n");
        return -1;
    }
    int res = strupr(argv[1]);
    if (!res)
    {
        printf("protocol Error!\n");
        return -1;

    }
    printf("%s  ||  src %s : %s ----->  dst %s : %s\n", argv[1], SRC_IP, SRC_PORT, DST_IP, DST_PORT);
    
    // sending process parameters
    int pkt_size = atoi(argv[2]);
    int pkt_num = atoi(argv[3]);
    int period = atoi(argv[4]);
    int sleep_microS = atoi(argv[5]);
    if (!strcmp(argv[1], "TCP"))
    {
        tcpProcess(pkt_size, pkt_num, period, sleep_microS);;
    }
    else if(!strcmp(argv[1], "UDP"))
    {
        udpProcess(pkt_size, pkt_num, period, sleep_microS);
    }
    else
    {
        printf("Only support TCP/UDP!\n");
        return -1;
    }
    
    return 0;
}

// tcp sending process
// paramter: packet_size, packet_num, sending period and sleep(u_seconds)
void tcpProcess(int pkt_size, int pkt_num, int period, int sleep_microS)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("tcp socket error");
        _exit(-1);
    }

    struct sockaddr_in src_addr;
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(atoi(SRC_PORT));
    inet_pton(AF_INET, SRC_IP, &src_addr.sin_addr.s_addr);

    int ret = bind(sockfd, (struct sockaddr *) &src_addr, sizeof(src_addr));
    if (ret < 0)
    {
        perror("tcp bind");
        _exit(-1);
    }
    struct sockaddr_in dst_addr;
    bzero(&dst_addr,sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(atoi(DST_PORT)); 
    inet_pton(AF_INET, DST_IP, &dst_addr.sin_addr.s_addr);
    // tcp connect

    char data[3001] = "";
    for (int i = 0; i < pkt_size; i++)
    {
        data[i] = 1;
    }
    data[pkt_size] = '\0';
    //sending process
    int pkt_count = 0;
    int pkt_sizeSum = 0;
    int pro_time = 0;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    connect(sockfd, (struct sockaddr *)&dst_addr,sizeof(dst_addr));
    for (int i = 0; i < pkt_num; i++)
    {
        int num = send(sockfd, data, strlen(data), 0);
        if (num == -1)
        {
            printf("something is error! Please check carefully!\n");
            break;
        }
        pkt_count++;
        pkt_sizeSum += num;
        if (pkt_count % period == 0)
            usleep(sleep_microS);
    }
    gettimeofday(&end, NULL);
    double total_time = ((start.tv_sec - end.tv_sec) * 1000000 + start.tv_usec - end.tv_usec) / 1000000.0; 
    printf("time : %.3lf \t %d packets(%d byte) have sended successly\n", total_time, pkt_count, pkt_sizeSum);
    close(sockfd);
}

// UDP sending process
// paramter: packet_size, packet_num, sending period and sleep(u_seconds)
void udpProcess(int pkt_size, int pkt_num, int period, int sleep_microS)
{
    //create socket(SOCK_DGRAM)
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    // create src socket address(fixed port)
    struct sockaddr_in src_addr;
    bzero(&src_addr,sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(atoi(SRC_PORT));
    inet_pton(AF_INET, SRC_IP, &src_addr.sin_addr.s_addr);
    int ret = bind(sockfd, (struct sockaddr *) &src_addr, sizeof(src_addr));
    if (ret < 0)
    {
        perror("udp bind");
        _exit(-1);
    }
    // create dst socket address
    struct sockaddr_in dst_addr;
    bzero(&dst_addr,sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(atoi(DST_PORT)); 
    inet_pton(AF_INET, DST_IP, &dst_addr.sin_addr.s_addr);
    //prepare sending data
    char data[3001] = "";
    for (int i = 0; i < pkt_size; i++)
    {
        data[i] = 1;
    }
    data[pkt_size] = '\0';
    //sending process
    int pkt_count = 0;
    int pkt_sizeSum = 0;
    int pro_time = 0;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < pkt_num; i++)
    {
        int num = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
        if (num == -1)
        {
            printf("something is error! Please check carefully!\n");
            break;
        }
        pkt_count++;
        pkt_sizeSum += num;
        if (pkt_count % period == 0)
            usleep(sleep_microS);
    }
    gettimeofday(&end, NULL);
    double total_time = ((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000000.0; 
    printf("time : %.3lf \t %d packets(%d byte) have sended successly\n", total_time, pkt_count, pkt_sizeSum);
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
