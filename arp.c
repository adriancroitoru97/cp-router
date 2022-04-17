#include "arp.h"

void router_arp_reply(packet *m, struct ether_header *eth_hdr,
					  struct arp_header *arp_hdr) {
	arp_hdr->op = htons(2);
	memcpy(arp_hdr->tha, arp_hdr->sha, ETH_ALEN);
	swap_uints(&arp_hdr->tpa, &arp_hdr->spa);
	get_interface_mac(m->interface, arp_hdr->sha);

	swap_macs(eth_hdr->ether_dhost, eth_hdr->ether_shost);
	memcpy(eth_hdr->ether_shost, arp_hdr->sha, ETH_ALEN);

	send_packet(m);
}

void router_arp_request(mac_ip_pair *cache, int *cache_len,
						queue packet_queue, int *queue_len,
						struct arp_header *arp_hdr,
						struct route_table_entry *rtable, int rtable_len) {
	/* MAC-IP pair added in the cache. */
	cache[*cache_len].ip = arp_hdr->spa;
	memcpy(cache[*cache_len].mac, arp_hdr->sha, ETH_ALEN);
	*cache_len = *cache_len + 1;

	/* Traverse the queue and forward all packets
       with determinated destination MAC. */
	for (int i = 0; i < *queue_len; i++) {
		packet *temp = (packet*)queue_deq(packet_queue);
		struct ether_header *temp_eth_hdr =
                (struct ether_header *)(temp->payload);
		struct iphdr *temp_ip_hdr =
                (struct iphdr *)(temp->payload + sizeof(struct ether_header));

        /* Continues if the current packet doesn't have a route table entry */
		int longest_prefix_index =
                lpm_binary(rtable, rtable_len, temp_ip_hdr->daddr);
		if ((temp_ip_hdr->daddr & rtable[longest_prefix_index].mask) != 
			rtable[longest_prefix_index].prefix) {
			continue;
		}
		struct route_table_entry *next_hop = rtable + longest_prefix_index;
		uint8_t *destMac =
                find_mac_in_cache(next_hop->next_hop, cache, *cache_len);

        /* If current packet has determinated destination MAC, it is forwarded.
           Otherwise, it is added back in the queue
           until the ARP reply arrives. */
		if (destMac != NULL) {
			/* Rewrite L2 addresses */
			memcpy(temp_eth_hdr->ether_dhost, destMac, ETH_ALEN);
			get_interface_mac(next_hop->interface, temp_eth_hdr->ether_shost);
			temp->interface = next_hop->interface;

			send_packet(temp);
			free(temp);
			*queue_len = *queue_len - 1;
		} else {
			queue_enq(packet_queue, temp);
		}
	}
}

void router_arp_send_request(struct route_table_entry *next_hop) {
	packet *arp_request = (packet*)malloc(sizeof(packet));
	arp_request->interface = next_hop->interface;
	arp_request->len = sizeof(struct ether_header) + sizeof(struct arp_header);

	/* Setting the ethernet header */
	struct ether_header *arp_req_eth_hdr =
			(struct ether_header *) arp_request->payload;
	set_broadcast_mac(arp_req_eth_hdr->ether_dhost); 
	get_interface_mac(next_hop->interface, arp_req_eth_hdr->ether_shost);
	arp_req_eth_hdr->ether_type = htons(ETHERTYPE_ARP);

	/* Setting the ARP header */
	struct arp_header *arp_req_hdr = (struct arp_header *)
				(arp_request->payload + sizeof(struct ether_header));
	arp_req_hdr->htype = htons(1);
	arp_req_hdr->ptype = htons(ETHERTYPE_IP);
	arp_req_hdr->hlen = 6;
	arp_req_hdr->plen = 4;
	arp_req_hdr->op = htons(1);
	memcpy(arp_req_hdr->sha, arp_req_eth_hdr->ether_shost, ETH_ALEN);
	arp_req_hdr->spa = inet_addr(get_interface_ip(next_hop->interface));
	memset(arp_req_hdr->tha, 0, ETH_ALEN);
	arp_req_hdr->tpa = next_hop->next_hop;

	send_packet(arp_request);
	free(arp_request);
}
