#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFSIZE 64
#define PACKET_SIZE (sizeof(struct icmphdr) + sizeof(struct timeval))

typedef struct s_stats {
  int sent;
  int received;
  long rtt_min;
  long rtt_max;
  long rtt_total;
} t_stats;

typedef struct s_ping {
  int sockfd;
  struct sockaddr_in dst;
  t_stats stats;
  u_int16_t seq;
  char *hostname;
  char ip_str[INET_ADDRSTRLEN];
  int verbose;
  int count;
} t_ping;

extern volatile sig_atomic_t g_stop;

void handle_sigint(int sig);
void setup_signals(void);

void stats_init(t_stats *stats);
void stats_packet_sent(t_stats *stats);
void stats_packet_received(t_stats *stats);
void stats_update_rtt(t_stats *stats, long rtt);
double stats_packet_loss(t_stats *stats);
double stats_rtt_avg(t_stats *stats);

unsigned short calcul_checksum(void *b, int len);
u_int16_t init_icmp_echo_request(struct icmphdr *icmp, char buf[],
                                 u_int16_t *seq);
long calculate_rtt_ms(struct timeval *start, struct timeval *end);

void ping_loop(t_ping *ping);
void print_usage(void);

#endif
