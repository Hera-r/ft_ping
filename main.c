#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h> // pour host to network et l'inverse htnol/ntohl
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> // convert addr_ip str to int inet_pton()
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

#define BUFSIZE		64


typedef struct s_stats
{
    int     sent;
    int     received;
    long    rtt_min;
    long    rtt_max;
    long    rtt_total;
}   t_stats;



volatile	sig_atomic_t g_stop = 0;

void	handle_sigint(int sig)
{
	(void)sig;
	g_stop = 1;
}

void stats_packet_sent(t_stats *stats)
{
    stats->sent++;
}

void stats_packet_received(t_stats *stats)
{
    stats->received++;
}

double stats_packet_loss(t_stats *stats)
{
    if (stats->sent == 0)
        return 0.0;

    return ((stats->sent - stats->received) * 100.0) / stats->sent;
}

long calculate_rtt_ms(struct timeval *start, struct timeval *end)
{
    long sec = end->tv_sec - start->tv_sec;
    long usec = end->tv_usec - start->tv_usec;

    if (usec < 0)
    {
        sec--;
        usec += 1000000;
    }

    return (sec * 1000) + (usec / 1000);
}

void stats_update_rtt(t_stats *stats, long rtt)
{
    stats->rtt_total += rtt;

    if (rtt < stats->rtt_min)
        stats->rtt_min = rtt;

    if (rtt > stats->rtt_max)
        stats->rtt_max = rtt;
}


double stats_rtt_avg(t_stats *stats)
{
    if (stats->received == 0)
        return 0.0;

    return (double)stats->rtt_total / stats->received;
}


void stats_init(t_stats *stats)
{
    stats->sent = 0;
    stats->received = 0;
    stats->rtt_total = 0;
    stats->rtt_min = LONG_MAX;
    stats->rtt_max = 0;
}


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

u_int16_t	init_icmp_echo_request(struct icmphdr *icmp, char buf[], u_int16_t *seq) {

	
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = getpid() & 0xFFFF;

	u_int16_t current_seq = *seq;
	icmp->un.echo.sequence = htons(current_seq);

	struct timeval *tv = (struct timeval *)(buf + sizeof(struct icmphdr));
	gettimeofday(tv, NULL);

	icmp->checksum = calcul_checksum(buf, sizeof(struct icmphdr) + sizeof(struct timeval));
	(*seq)++;

	return current_seq;
}

int	main()
{
    struct sigaction sa;
    t_stats stats;

	
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
	
	
	
	
	struct sockaddr_in src, dst = {0};
	struct in_addr addr;
	int socket_fd;
	char buf[BUFSIZE];
	u_int16_t seq = 0;
	
	src.sin_family = AF_INET;
	src.sin_addr.s_addr = INADDR_ANY;
	
	dst.sin_family = AF_INET;
	inet_pton(AF_INET, "8.8.8.8", &dst.sin_addr); // gerer manuellement mais sera modifier
	
	socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (socket_fd == -1) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return 1;
	}

	memset(buf, 0, BUFSIZE);
	struct icmphdr *icmp = (struct icmphdr *)buf;
	
	stats_init(&stats);

	printf("PING 8.8.8.8 (8.8.8.8): %lu data bytes\n",
	       sizeof(struct icmphdr) + sizeof(struct timeval)); // gerer manuellement mais sera modifier

	while (!g_stop)
	{

		u_int16_t sent_seq = init_icmp_echo_request(icmp, buf, &seq);

		ssize_t send_ping = sendto(socket_fd, buf,
		                           sizeof(struct icmphdr) + sizeof(struct timeval),
		                           0, (struct sockaddr *)&dst, sizeof(dst));
		if (send_ping < 0) {
			fprintf(stderr, "send error: %s\n", strerror(errno));
			return 1;
		}

		stats_packet_sent(&stats);

		char rbuf[BUFSIZE];
		socklen_t len_dst = sizeof(struct sockaddr_in);
		
		memset(rbuf, 0, BUFSIZE);
		ssize_t recv_ping = recvfrom(socket_fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&src, &len_dst);
		if (recv_ping < 0) {
			fprintf(stderr, "recv error: %s\n", strerror(errno));
			return 1;
		}

		struct iphdr *ip = (struct iphdr *)rbuf;
		struct icmphdr *icmp_reply = (struct icmphdr *)(rbuf + (ip->ihl * 4));
		addr.s_addr = ip->saddr;

		if (icmp_reply->type == ICMP_ECHOREPLY) {
			stats_packet_received(&stats);

			if ((ssize_t)recv_ping >= (ssize_t)(ip->ihl * 4 + sizeof(struct icmphdr) + sizeof(struct timeval))) {
				struct timeval *tv_sent = (struct timeval *)(rbuf + (ip->ihl * 4) + sizeof(struct icmphdr));
				struct timeval tv_now;
				gettimeofday(&tv_now, NULL);

				long rtt_ms = (tv_now.tv_sec - tv_sent->tv_sec) * 1000
				              + (tv_now.tv_usec - tv_sent->tv_usec + 1000000) % 1000000 / 1000;

				long rtt = calculate_rtt_ms(tv_sent, &tv_now);
				stats_update_rtt(&stats, rtt);

				printf("%zd bytes from %s icmp_seq=%d ttl=%d time=%ld ms\n",
				       recv_ping, inet_ntoa(addr), sent_seq, ip->ttl, rtt_ms);
			} else {
				printf("%zd bytes from %s icmp_seq=%d ttl=%d (payload missing)\n",
				       recv_ping, inet_ntoa(addr), sent_seq, ip->ttl);
			}

		} else {
			printf("%zd bytes from %s icmp_type=%d icmp_code=%d\n",
			       recv_ping, inet_ntoa(addr), icmp_reply->type, icmp_reply->code);
		}

		sleep(1);
	}

	printf("\n--- %s ping statistics ---\n",inet_ntoa(addr));
	printf("%d packets transmitted, %d received, %.0f%% packet loss\n",
		stats.sent,
		stats.received,
		stats_packet_loss(&stats));

	printf("rtt min/avg/max = %ld/%.2f/%ld ms\n",
		stats.rtt_min,
		stats_rtt_avg(&stats),
		stats.rtt_max);

	return 0;
}

