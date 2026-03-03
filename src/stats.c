#include "ft_ping.h"

void stats_init(t_stats *stats) {
  stats->sent = 0;
  stats->received = 0;
  stats->rtt_total = 0;
  stats->rtt_min = LONG_MAX;
  stats->rtt_max = 0;
}

void stats_packet_sent(t_stats *stats) { stats->sent++; }

void stats_packet_received(t_stats *stats) { stats->received++; }

void stats_update_rtt(t_stats *stats, long rtt) {
  stats->rtt_total += rtt;
  if (rtt < stats->rtt_min)
    stats->rtt_min = rtt;
  if (rtt > stats->rtt_max)
    stats->rtt_max = rtt;
}

double stats_packet_loss(t_stats *stats) {
  if (stats->sent == 0)
    return (0.0);
  return (((stats->sent - stats->received) * 100.0) / stats->sent);
}

double stats_rtt_avg(t_stats *stats) {
  if (stats->received == 0)
    return (0.0);
  return ((double)stats->rtt_total / stats->received);
}
