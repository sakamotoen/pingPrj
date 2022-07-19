#ifndef _PING_H_
#define _PING_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<error.h>
#include<errno.h>
#include<netdb.h>
#include<netinet/ip.h>
#include<netinet/in.h>
#include<netinet/ip_icmp.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<time.h>

#define PACKET_SIZE     4096
#define MAX_WAIT_TIME   5
#define MAX_NO_PACKETS  10

class ping
{
private:
    char sendPacket[PACKET_SIZE];
    char recvPacket[PACKET_SIZE];
    struct timeval timesend;
    struct timeval timerecv;
    
public:
    void init(int nsend, int nreceived, char *ipAddr);
    static int sockfd;
    static int m_nsend, m_nreceived;
    // static struct timeval begin, end;
    
    static double totalRtt;
    static char *m_Addr;

    pid_t pid;
    int datalen = 56;
    struct sockaddr_in dest_addr;
    struct sockaddr_in from_addr;
    static void statistics(int signo);
    void stat(int signo);
    unsigned short cal_chksum(unsigned short *addr, int len);
    int pack(int pack_no);
    bool send_packet(void);
    bool recv_packet(void);
    int unpack(char *buf, int len);
};


#endif