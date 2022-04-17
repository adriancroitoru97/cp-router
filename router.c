#include "queue.h"
#include "skel.h"

#include "utils.h"
#include "icmp.h"
#include "arp.h"
#include "ip.h"

int main(int argc, char *argv[])
{
	packet m;
	int rc;

	// Do not modify this line
	init(argc - 2, argv + 2);

	/* Loading the route table in the memory.
	 * Sorting it ascendingly by prefixes and descendingly
	 * by the masks.
	 */
	struct route_table_entry *rtable =
				malloc(sizeof(struct route_table_entry) * MAX_RT_ENTRIES);
	DIE(rtable == NULL, "Route table malloc failed!");
	int rtable_len = read_rtable(argv[1], rtable);
	qsort(rtable, rtable_len,
				sizeof(struct route_table_entry), rt_compare_function);

	/* Intialising the router's cache.
	 * MAC-IP pairs mentained in network byte order. */
	mac_ip_pair *cache =
				(mac_ip_pair*)malloc(sizeof(mac_ip_pair) * MAX_RT_ENTRIES);
	DIE(cache == NULL, "Cache malloc failed!");
	int cache_len = 0;

	/* Intialising the router's packets queue. */
	queue packet_queue = queue_create();
	int queue_len = 0;

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_packet");

		struct ether_header *eth_hdr = (struct ether_header *) m.payload;
		uint8_t* interface_mac = (uint8_t *)malloc(ETH_ALEN * sizeof(uint8_t));
		get_interface_mac(m.interface, interface_mac);

		/* Throw packets with wrong destination MAC */
		if (!equal_macs(eth_hdr->ether_dhost, interface_mac) &&
			!is_broadcast_mac(eth_hdr->ether_dhost)) {
			free(interface_mac);
			continue;
		}
		free(interface_mac);

		/* The current interface IP address */
		in_addr_t interface_ip = inet_addr(get_interface_ip(m.interface));

		/* ARP protocol */
		if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP) {
			struct arp_header *arp_hdr =
				(struct arp_header *)(m.payload + sizeof(struct ether_header));

			/* The packet is destinated to this router */
			if (arp_hdr->tpa == interface_ip) {
				/* ARP request received */
				if (ntohs(arp_hdr->op) == 1) {
					router_arp_reply(&m, eth_hdr, arp_hdr);
				}
				
				/* ARP reply received */
				if (ntohs(arp_hdr->op) == 2) {
					router_arp_request(cache, &cache_len,
									   packet_queue, &queue_len, arp_hdr,
									   rtable, rtable_len);
				}
			}

			continue;
		}

		/* IPv4 protocol */
		if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP) {
			struct iphdr *ip_hdr =
					(struct iphdr *)(m.payload + sizeof(struct ether_header));

			process_ip_packet(&m, interface_ip, ip_hdr, eth_hdr,
							  rtable, rtable_len, cache, cache_len,
							  packet_queue, &queue_len);
			continue;
		}
	}

	free(cache);
	free(rtable);
}
