#include "icmp.h"
#include "utils.h"

void send_icmp_message(packet *m, uint8_t type, uint8_t code) {
    /* temporary store the first 64 bytes after the ip header */
	void* temp_64bytes = malloc(64);
	memcpy(temp_64bytes, m->payload + sizeof(struct ether_header) + sizeof(struct iphdr), 64);

    /* set the ethernet header */
	struct ether_header *eth_hdr = (struct ether_header *) m->payload;
	swap_macs(eth_hdr->ether_dhost, eth_hdr->ether_shost);

    /* set the IP header */
	struct iphdr *ip_hdr = (struct iphdr *) (m->payload + sizeof(struct ether_header));
	ip_hdr->protocol = IPPROTO_ICMP;
    /* if the ICMP type is time exceeded, refresh the TTL */
	if (type == 11) {
		ip_hdr->ttl = 64;
	}
	ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
	ip_hdr->daddr = ip_hdr->saddr;
	ip_hdr->saddr = inet_addr(get_interface_ip(m->interface));
	ip_hdr->check = htons(0);
	ip_hdr->check = ip_checksum((uint8_t*)ip_hdr, sizeof(struct iphdr));

    /* set the ICMP header and add the 64 bytes after */
	struct icmphdr *icmp_hdr = (struct icmphdr*)
			(m->payload + sizeof(struct ether_header) + sizeof(struct iphdr));
	icmp_hdr->type = type;
	icmp_hdr->code = code;
	memcpy(m->payload + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr), temp_64bytes, 64);
    /* also update the total packet length */
	m->len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr) + 64;
	icmp_hdr->checksum = htons(0);
	icmp_hdr->checksum = icmp_checksum((uint16_t*)icmp_hdr,
					m->len - sizeof(struct ether_header) - sizeof(struct iphdr));
	
	send_packet(m);
	free(temp_64bytes);
}

void send_imcp_echo_reply(packet *m) {
    /* set the ethernet header */
	struct ether_header *eth_hdr = (struct ether_header *) m->payload;
	swap_macs(eth_hdr->ether_dhost, eth_hdr->ether_shost);

    /* set the IP header */
	struct iphdr *ip_hdr = (struct iphdr *) (m->payload + sizeof(struct ether_header));
	ip_hdr->protocol = IPPROTO_ICMP;
	ip_hdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
	ip_hdr->daddr = ip_hdr->saddr;
	ip_hdr->saddr = inet_addr(get_interface_ip(m->interface));
	ip_hdr->check = htons(0);
	ip_hdr->check = ip_checksum((uint8_t*)ip_hdr, sizeof(struct iphdr));

    /* set the ICMP header */
	struct icmphdr *icmp_hdr = (struct icmphdr*)
			(m->payload + sizeof(struct ether_header) + sizeof(struct iphdr));
	if (icmp_hdr->type != 8 || icmp_hdr->code != 0) {
		return;
	}
	icmp_hdr->type = 0;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = htons(0);
	icmp_hdr->checksum = icmp_checksum((uint16_t*)icmp_hdr,
					m->len - sizeof(struct ether_header) - sizeof(struct iphdr));
	
	send_packet(m);
}
