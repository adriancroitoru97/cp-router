#ifndef _IP_H_
#define _IP_H_

#include "skel.h"
#include "utils.h"
#include "queue.h"
#include "icmp.h"
#include "arp.h"

/*
 * Processes a received IP packet.
 * The function verifies and updates the TTL and checksum.
 * In case of error, it sends ICMP messages back to the source.
 * 
 * It also responds to echo request messages destinated to the current router.
 * 
 * Afterall, it forwards the packet if it knows the next-hop's MAC address.
 * Otherwise, it puts the current to-be-forwarded packet in a queue and
 * sends an ARP request.
 * The current packet will be processed when the ARP reply arrives along with
 * the MAC destination address.
 */
void process_ip_packet(packet *m, in_addr_t interface_ip,
                      struct iphdr *ip_hdr, struct ether_header *eth_hdr,
                      struct route_table_entry *rtable, int rtable_len,
                      mac_ip_pair *cache, int cache_len,
                      queue packet_queue, int *queue_len);

#endif /* _IP_H_ */
