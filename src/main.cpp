#include"../include/ping.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("error_argc!\n");
        exit(-1);
    }

    // Init class
    ping *myPing = new ping;
    myPing->init(0, 0, argv[1]);

    struct hostent *host;
    struct protoent *proto;
 
    unsigned long inaddr = 01;
    // Buff size
    int size = 50*1024;
    if((proto = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobuname!");
        exit(1);
    }

    

    // Init socket
    if((ping::sockfd = socket(AF_INET, SOCK_RAW, proto->p_proto)) == -1)
    {
        perror("socket!");
        exit(1);
    }

    // Set Uid
    setuid(getuid());

    // Set socket option
    setsockopt(myPing->sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&myPing->dest_addr, sizeof(myPing->dest_addr));

    myPing->dest_addr.sin_family = AF_INET;

    // Input is domain name or IPaddr?
    if(inaddr = inet_addr(argv[1]) == INADDR_NONE)
    {
        if((host = gethostbyname(argv[1])) == NULL)
        {
            perror("gethostbyname!");
            exit(1);
        }
        memcpy((char *)&myPing->dest_addr.sin_addr, host->h_addr, host->h_length);
    }
    else
    {
        myPing->dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
    }
    // Get id to set ICMP
    myPing->pid = getpid();
    printf("PING %s: %d bytes data in ICMP packets.\n", argv[1], myPing->datalen);

    // Loop to ping
    int loop = 0;
    // gettimeofday(&ping::begin, NULL);

    while(loop < MAX_NO_PACKETS)
    {
        // It can always be sent successfully, but could not received successfully.
        myPing->send_packet();
        ++ping::m_nsend;
        myPing->recv_packet() == true ? ping::m_nreceived++ : ping::m_nreceived;
        ++loop;
        sleep(1);
    }
    // gettimeofday(&ping::end, NULL);

    myPing->statistics(SIGALRM);
    delete myPing;
    return 0;
}