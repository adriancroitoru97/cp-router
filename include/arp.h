#ifndef _ARP_H_
#define _ARP_H_

#include "skel.h"
#include "utils.h"
#include "queue.h"

/*
 * Router replies when an ARP request is destinated to one of its interfaces.
 * The function modifies the original packet, swaping the addresses and
 * adding the requested MAC address.
 */
void router_arp_reply(packet *m, struct ether_header *eth_hdr,
					  struct arp_header *arp_hdr);

/*
 * Router gets a reply to a request it did previously.
 * The new obtained MAC address is added in the cache, along with its IP.
 * The packets queue is after traversed and all packets, for which the router
 * knows now the MAC destination addresses, are forwarded.
 */
void router_arp_request(mac_ip_pair *cache, int *cache_len,
						queue packet_queue, int *queue_len,
						struct arp_header *arp_hdr,
						struct route_table_entry *rtable, int rtable_len);

/*
 * Router sends an ARP request in order to obtain the MAC address of
 * the next hop.
 */
void router_arp_send_request(struct route_table_entry *next_hop);

#endif /* _ARP_H_ */
