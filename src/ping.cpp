#include"../include/ping.h"



int ping::sockfd = 0;
struct timeval ping::begin = {0, 0};
struct timeval ping::end = {0, 0};
int ping::m_nsend = 0;
int ping::m_nreceived = 0;
char *ping::m_Addr = NULL;
void tv_sub(struct timeval *out, struct timeval *in);

// Package send and recv statistics
void ping::statistics(int signo)
{
    printf("\n--- %s ping statistics ---\n", ping::m_Addr);
    tv_sub(&end, &begin);
    double rtt = end.tv_sec*1000 + end.tv_usec / 1000;
    printf("%d packets transmitted, %d received, %d%% packet loss, time %.0lfms\n", m_nsend, m_nreceived, (m_nsend - m_nreceived)/m_nsend*100, rtt);
    close(sockfd);
    exit(1);
}

void ping::init(int nsend, int nreceived, char *ipAddr)
{
    sockfd = 0;
    m_nsend = nsend;
    m_nreceived = nreceived;
    m_Addr = ipAddr;
}

// Calculate checksum
unsigned short ping::cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *i = addr;
    unsigned short answer = 0;

    // Accumulate ICMP header binary data in units of 2 bytes
    while(nleft>1)
    {
        sum += *i++;
        nleft -=2;
    }

    // When the ICMP header is odd, the last byte is regarded as a two-byte high bit, and the low bit is filled with a '0' byte
    if(nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)i;
        sum += answer;
    }

    sum = (sum>>16)+(sum&0xFFFF);
    sum += (sum>>16);
    answer =~ sum;
    return answer;
}

// Set header
int ping::pack(int pack_no)
{
    int i, packsize;
    struct icmp *icmp;
    struct timeval *timeval;
    icmp = (struct icmp*)(sendPacket);
    icmp->icmp_type = ICMP_ECHO;        //icmp type is ECHO
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = pack_no;
    icmp->icmp_id = pid;
    packsize = 8 + datalen;
    timeval = (struct timeval*)(icmp->icmp_data);
    gettimeofday(timeval, NULL);
    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);
    return packsize;
}

// Send package
// void ping::send_packet(void)
// {
//     int packetsize;
//     while(m_nsend < MAX_NO_PACKETS)
//     {
//         m_nsend++;
//         packetsize = pack(m_nsend);
//         if(sendto(sockfd, sendPacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
//         {
//             perror("sendto!");
//             continue;
//         }
//         sleep(1);       // Every 1 sec send a package to target host
//     }
// }




// Send Package
bool ping::send_packet(void)
{
    int packetsize;
    gettimeofday(&timesend, NULL);
    packetsize = pack(m_nsend);
    if(sendto(sockfd, sendPacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("sendto!");
        return false;
    }
    //sleep(1);       // Every 1 sec send a package to target host
    return true;
}



// Recv package
// void ping::recv_packet(void)
// {
//     int n, fromlen;
    
//     signal(SIGALRM, statistics);
//     fromlen = sizeof(from_addr);
//     while(m_nreceived < m_nsend)
//     {
//         alarm(MAX_WAIT_TIME);
//         if((n = recvfrom(sockfd, recvPacket, sizeof(recvPacket),0,(struct sockaddr *)&from_addr, (socklen_t *)&fromlen)) < 0)
//         {
//             if(errno == EINTR)
//                 continue;
//             perror("recvfrom!");
//             continue;
//         }
//         gettimeofday(&timerecv, NULL);
//         if(unpack(recvPacket, n) == -1)
//             continue;
        
//         m_nreceived++;
//     }
// }






// Recv package
bool ping::recv_packet(void)
{
    int n, fromlen;
    
    signal(SIGALRM, statistics);
    fromlen = sizeof(from_addr);

    alarm(MAX_WAIT_TIME);
    if((n = recvfrom(sockfd, recvPacket, sizeof(recvPacket),0,(struct sockaddr *)&from_addr, (socklen_t *)&fromlen)) < 0)
    {
        if(errno == EINTR)
            return false;
        perror("recvfrom!");
        return false;
    }
    gettimeofday(&timerecv, NULL);
    if(unpack(recvPacket, n) == -1)
        return false;
    
    return true;
    
}




// Deal with header
int ping::unpack(char *buf, int len)
{
    int i, iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    
    double rtt;
    ip = (struct ip*)buf;
    iphdrlen = ip->ip_hl<<2;     // Length of ip header
    icmp = (struct icmp*)(buf + iphdrlen);
    len -= iphdrlen;

    if(len < 8)
    {
        printf("ICMP packet\'s length is less than 8.\n");
        return -1;
    }

    // Guaranteed to receive an ICMP response
    if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
    {
        //timesend = (struct timeval*)(icmp->icmp_data);
        tv_sub(&timerecv, &timesend);
        rtt = timerecv.tv_sec * 1000 + timerecv.tv_usec / 1000;       //ms
        printf("%d bytes from %s (%s): icmp_seq=%u ttl=%d time=%.0lfms\n", len, ping::m_Addr, inet_ntoa(from_addr.sin_addr),
                icmp->icmp_seq, ip->ip_ttl, rtt);
    }
    else
    {
        return -1;
    }
    return 0;
}

// Cal time
void tv_sub(struct timeval *out, struct timeval *in)
{
    if((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec +=1000000;
    }
    out->tv_sec -= in->tv_sec;
}