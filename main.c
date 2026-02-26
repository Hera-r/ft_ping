#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h> // pour host to network et l'inverse htnol/ntohl
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // convert addr_ip str to int inet_pton()
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


#define BUFSIZE		64
#define ICMP_ECHO	8

/*	struct pour IPv4 

	struct sockaddr_in {
		sa_family_t		sin_family;
		in_port_t		sin_port;
		struct in_addr	sin_addr;
	};

	struct in_addr {
		uint32_t	s_addr;
	};

*/

// struct pollfd {
//    int   fd;         /* file descriptor */
//    short events;     /* Événements attendus */
//    short revents;    /* Événements détectés */
// };



// struct icmphdr
// {
//   u_int8_t type;                /* message type */
//   u_int8_t code;                /* type sub-code */
//   u_int16_t checksum;
//   union
//   {
//     struct
//     {
//       u_int16_t        id;
//       u_int16_t        sequence;
//     } echo;                        /* echo datagram */

//     u_int32_t        gateway;        /* gateway address */

//     struct
//     {
//       u_int16_t        __unused;
//       u_int16_t        mtu;
//     } frag;                        /* path mtu discovery */
//   } un;
// };

static unsigned short calcul_checksum(void *b, int len) {
	unsigned short *buf = b;
	unsigned int sum = 0;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}


int	main()
{
	struct sockaddr_in src, dst = {0};
	int socket_fd;
	char buf[BUFSIZE];
	int seq = 1;
	src.sin_family = AF_INET;
	src.sin_addr.s_addr = INADDR_ANY;

	dst.sin_family = AF_INET;
	inet_pton(AF_INET, "8.8.8.8", &dst.sin_addr);

	socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (socket_fd == -1) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return 1;
	}

	memset(buf, 0, BUFSIZE);

	struct icmphdr *icmp = (struct icmphdr *)buf;

	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = getpid() & 0xFFFF;
	icmp->un.echo.sequence = seq;

	icmp->checksum = calcul_checksum(buf, sizeof(struct icmphdr));

	while (1)
	{
		int send_ping = sendto(socket_fd, buf, sizeof(struct icmphdr), 0, (struct sockaddr *)&dst, sizeof(dst));
			
		if (send_ping < 0) {
			fprintf(stderr, "send error: %s\n", strerror(errno));
			return 1;
		}
		
		char rbuf[BUFSIZE];
		socklen_t len_dst = sizeof(struct sockaddr_in);
		
		memset(rbuf, 0, BUFSIZE);
		int recv_ping = recvfrom(socket_fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&src, &len_dst);
		if (recv_ping < 0) {
			fprintf(stderr, "recv error: %s\n", strerror(errno));
			return 1;
		}
		
		struct iphdr *ip = (struct iphdr *)rbuf;
		struct icmphdr *icmp_reply = (struct icmphdr *)(rbuf + (ip->ihl * 4));

		printf("type: %d\n", icmp_reply->type);
		printf("seq: %d\n", icmp_reply->un.echo.sequence);
		printf("ttl: %d\n", ip->ttl);
		sleep(1);
	}

	return 0;
}

// PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
// 64 bytes from 8.8.8.8: icmp_seq=1 ttl=116 time=5.80 ms
// 64 bytes from 8.8.8.8: icmp_seq=2 ttl=116 time=6.93 ms
// ^C
// --- 8.8.8.8 ping statistics ---
// 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
// rtt min/avg/max/mdev = 5.802/6.368/6.934/0.566 ms
