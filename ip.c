#include "ip.h"

void process_ip_packet(packet *m, in_addr_t interface_ip,
                      struct iphdr *ip_hdr, struct ether_header *eth_hdr,
                      struct route_table_entry *rtable, int rtable_len,
                      mac_ip_pair *cache, int cache_len,
                      queue packet_queue, int *queue_len) {
    /* Throw packet if wrong checksum */
    uint16_t temp_checksum = ip_hdr->check;
    ip_hdr->check = htons(0);
    if (ip_checksum((uint8_t*)ip_hdr, sizeof(struct iphdr)) != temp_checksum) {
        return;
    }
    ip_hdr->check = temp_checksum;

    /* Verify and update the TTL */
    if (ip_hdr->ttl <= 1) {
        send_icmp_message(m, ICMP_TIME_EXCEEDED, ICMP_NET_UNREACH);
        return;
    } else {
        ip_hdr->ttl = ip_hdr->ttl - 1;
    }

    /* This router is the destination */
    if (ip_hdr->daddr == interface_ip) {
        send_imcp_echo_reply(m);
        return;
    } else {
        /* Search in the routing table for the best next hop */
        int longest_prefix_index =
                lpm_binary(rtable, rtable_len, ip_hdr->daddr);
        struct route_table_entry *next_hop = NULL;
        if ((ip_hdr->daddr & rtable[longest_prefix_index].mask) == 
            rtable[longest_prefix_index].prefix) {
            next_hop = rtable + longest_prefix_index;
        }
        
        /* There is no entry for the seeked
            destination in the route table */
        if (!next_hop) {
            send_icmp_message(m, ICMP_DEST_UNREACH, ICMP_NET_UNREACH);
            return;
        }

        /* Recalculate the checksum using the incremental
            checksum algorithm, as only the TTL modified */
        ip_hdr->ttl = ip_hdr->ttl + 1;
        ip_hdr->check = incremental_checksum(ip_hdr);

        /* The next hop was found - if its MAC is not found in the cache,
            a ARP request will be sent in order to find the destination MAC */
        uint8_t *destMac =
            find_mac_in_cache(next_hop->next_hop, cache, cache_len);
        if (destMac != NULL) {
            /* rewrite L2 addresses */
            memcpy(eth_hdr->ether_dhost, destMac, ETH_ALEN);
            get_interface_mac(next_hop->interface, eth_hdr->ether_shost);
            m->interface = next_hop->interface;

            send_packet(m);
            return;
        } else {
            /* the current packet is added in the queue
                in order to be processed when the ARP reply arrives */
            packet *q_packet = (packet*)malloc(sizeof(packet));
            memcpy(q_packet, m, sizeof(packet));
            queue_enq(packet_queue, q_packet);
            *queue_len = *queue_len + 1;

            router_arp_send_request(next_hop);
            return;
        }
    }
}
