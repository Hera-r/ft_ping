#include "ft_ping.h"

void print_usage(void) {
  printf("Usage: ft_ping [-v] [-c count] [-?] destination\n");
  printf("Options:\n");
  printf("  -v          verbose output\n");
  printf("  -c count    stop after sending count packets\n");
  printf("  -?          display this help and exit\n");
}

static void print_stats(t_ping *ping) {
  printf("\n--- %s ping statistics ---\n", ping->hostname);
  printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
         ping->stats.sent, ping->stats.received,
         stats_packet_loss(&ping->stats));
  if (ping->stats.received > 0)
    printf("round-trip min/avg/max = %ld/%.1f/%ld ms\n", ping->stats.rtt_min,
           stats_rtt_avg(&ping->stats), ping->stats.rtt_max);
}

static int resolve_host(t_ping *ping, const char *host) {
  struct addrinfo hints;
  struct addrinfo *res;
  int ret;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;
  ret = getaddrinfo(host, NULL, &hints, &res);
  if (ret != 0) {
    fprintf(stderr, "ft_ping: %s: %s\n", host, gai_strerror(ret));
    return (-1);
  }
  ping->dst = *(struct sockaddr_in *)res->ai_addr;
  inet_ntop(AF_INET, &ping->dst.sin_addr, ping->ip_str, sizeof(ping->ip_str));
  freeaddrinfo(res);
  return (0);
}

static int create_socket(t_ping *ping) {
  ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (ping->sockfd < 0) {
    fprintf(stderr, "ft_ping: socket: %s\n", strerror(errno));
    return (-1);
  }
  return (0);
}

static int parse_args(t_ping *ping, int argc, char **argv) {
  int opt;

  ping->verbose = 0;
  ping->count = 0;
  while ((opt = getopt(argc, argv, "vc:?")) != -1) {
    if (opt == 'v')
      ping->verbose = 1;
    else if (opt == 'c') {
      ping->count = atoi(optarg);
      if (ping->count <= 0) {
        fprintf(stderr, "ft_ping: invalid count: '%s'\n", optarg);
        return (-1);
      }
    } else if (opt == '?') {
      print_usage();
      return (1);
    }
  }
  if (optind >= argc) {
    fprintf(stderr, "ft_ping: missing host operand\n");
    print_usage();
    return (-1);
  }
  ping->hostname = argv[optind];
  return (0);
}

int main(int argc, char **argv) {
  t_ping ping;
  int ret;

  memset(&ping, 0, sizeof(ping));
  ping.seq = 0;
  ret = parse_args(&ping, argc, argv);
  if (ret != 0)
    return (ret < 0 ? 2 : 0);
  if (resolve_host(&ping, ping.hostname) < 0)
    return (2);
  if (create_socket(&ping) < 0)
    return (2);
  setup_signals();
  stats_init(&ping.stats);
  ping_loop(&ping);
  print_stats(&ping);
  close(ping.sockfd);
  return (0);
}
