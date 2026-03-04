#include "ft_ping.h"

static void print_echo_reply(ssize_t recv_len, struct iphdr *ip, u_int16_t seq,
                             long rtt) {
  struct in_addr addr;

  addr.s_addr = ip->saddr;
  printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%ld ms\n",
         recv_len - (ip->ihl * 4), inet_ntoa(addr), seq, ip->ttl, rtt);
}

static void print_verbose_error(ssize_t recv_len, struct iphdr *ip,
                                struct icmphdr *reply) {
  struct in_addr addr;

  addr.s_addr = ip->saddr;
  printf("%zd bytes from %s: type=%d code=%d\n", recv_len, inet_ntoa(addr),
         reply->type, reply->code);
}

static int process_reply(t_ping *ping, char *rbuf, ssize_t recv_len) {
  struct iphdr *ip;
  struct icmphdr *reply;
  struct timeval *tv_sent;
  struct timeval tv_now;
  long rtt;
  u_int16_t reply_id;

  ip = (struct iphdr *)rbuf;
  reply = (struct icmphdr *)(rbuf + (ip->ihl * 4));
  reply_id = getpid() & 0xFFFF;
  if (reply->type == ICMP_ECHOREPLY) {
    if (reply->un.echo.id != reply_id)
      return (0);
    stats_packet_received(&ping->stats);
    rtt = 0;
    if (recv_len >= (ssize_t)(ip->ihl * 4 + PACKET_SIZE)) {
      tv_sent = (struct timeval *)((char *)reply + sizeof(struct icmphdr));
      gettimeofday(&tv_now, NULL);
      rtt = calculate_rtt_ms(tv_sent, &tv_now);
      stats_update_rtt(&ping->stats, rtt);
    }
    print_echo_reply(recv_len, ip, ntohs(reply->un.echo.sequence), rtt);
    return (1);
  } else if (ping->verbose)
    print_verbose_error(recv_len, ip, reply);
  return (0);
}

static int handle_reply(t_ping *ping) {
  char rbuf[BUFSIZE + sizeof(struct iphdr) + 64];
  struct sockaddr_in src;
  socklen_t len_src;
  ssize_t recv_len;
  struct pollfd pfd;
  struct timeval start;
  struct timeval now;
  long elapsed;
  int timeout;
  int ret;

  gettimeofday(&start, NULL);
  timeout = 1000;
  while (1) {
    pfd.fd = ping->sockfd;
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, timeout);
    if (ret <= 0)
      return (ret);
    len_src = sizeof(src);
    memset(rbuf, 0, sizeof(rbuf));
    recv_len = recvfrom(ping->sockfd, rbuf, sizeof(rbuf), 0,
                        (struct sockaddr *)&src, &len_src);
    if (recv_len < 0) {
      if (errno == EINTR)
        return (0);
      fprintf(stderr, "ft_ping: recvfrom: %s\n", strerror(errno));
      return (-1);
    }
    if (process_reply(ping, rbuf, recv_len))
      return (1);
    gettimeofday(&now, NULL);
    elapsed = calculate_rtt_ms(&start, &now);
    timeout = 1000 - (int)elapsed;
    if (timeout <= 0)
      return (0);
  }
}

void ping_loop(t_ping *ping) {
  char buf[BUFSIZE];
  struct icmphdr *icmp;
  ssize_t send_ret;

  memset(buf, 0, BUFSIZE);
  icmp = (struct icmphdr *)buf;
  printf("PING %s (%s): %lu data bytes\n", ping->hostname, ping->ip_str,
         PACKET_SIZE);
  while (!g_stop) {
    if (ping->count > 0 && ping->stats.sent >= ping->count)
      break;
    init_icmp_echo_request(icmp, buf, &ping->seq);
    send_ret = sendto(ping->sockfd, buf, PACKET_SIZE, 0,
                      (struct sockaddr *)&ping->dst, sizeof(ping->dst));
    if (send_ret < 0) {
      fprintf(stderr, "ft_ping: sendto: %s\n", strerror(errno));
      break;
    }
    stats_packet_sent(&ping->stats);
    handle_reply(ping);
    if (g_stop)
      break;
    if (ping->count > 0 && ping->stats.sent >= ping->count)
      break;
    sleep(1);
  }
}
