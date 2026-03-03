#include "ft_ping.h"

unsigned short calcul_checksum(void *b, int len) {
  unsigned short *buf;
  unsigned int sum;

  buf = b;
  sum = 0;
  while (len > 1) {
    sum += *buf++;
    len -= 2;
  }
  if (len == 1)
    sum += *(unsigned char *)buf;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  return ((unsigned short)(~sum));
}

long calculate_rtt_ms(struct timeval *start, struct timeval *end) {
  long sec;
  long usec;

  sec = end->tv_sec - start->tv_sec;
  usec = end->tv_usec - start->tv_usec;
  if (usec < 0) {
    sec--;
    usec += 1000000;
  }
  return ((sec * 1000) + (usec / 1000));
}

u_int16_t init_icmp_echo_request(struct icmphdr *icmp, char buf[],
                                 u_int16_t *seq) {
  u_int16_t current_seq;
  struct timeval *tv;

  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->checksum = 0;
  icmp->un.echo.id = getpid() & 0xFFFF;
  current_seq = *seq;
  icmp->un.echo.sequence = htons(current_seq);
  tv = (struct timeval *)(buf + sizeof(struct icmphdr));
  gettimeofday(tv, NULL);
  icmp->checksum = calcul_checksum(buf, PACKET_SIZE);
  (*seq)++;
  return (current_seq);
}
