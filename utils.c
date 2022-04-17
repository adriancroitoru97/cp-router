#include "utils.h"

uint8_t* find_mac_in_cache (uint32_t ip, mac_ip_pair *cache, int cache_len) {
	for (int i = 0; i < cache_len; i++) {
		if (cache[i].ip == ip) {
			return cache[i].mac;
		}
	}

	return NULL;
}

int equal_macs (uint8_t *mac_a, uint8_t *mac_b) {
	for (int i = 0; i < ETH_ALEN; i++) {
		if (*(mac_a + i) != *(mac_b + i)) {
			return 0;
		}
	}

	return 1;
}

void set_broadcast_mac (uint8_t *mac) {
	for (int i = 0; i < ETH_ALEN; i++) {
		*(mac + i) = 255;
	}
}

int is_broadcast_mac (uint8_t *mac_a) {
	for (int i = 0; i < ETH_ALEN; i++) {
		if (*(mac_a + i) != 255) {
			return 0;
		}
	}

	return 1;
}

void swap_uints(uint32_t *a, uint32_t *b) {
	uint32_t aux = *a;
	*a = *b;
	*b = aux;
}

void swap_macs(uint8_t *mac_a, uint8_t *mac_b) {
	uint8_t* temp = (uint8_t *)malloc(ETH_ALEN * sizeof(uint8_t));
	memcpy(temp, mac_a, ETH_ALEN);
	memcpy(mac_a, mac_b, ETH_ALEN);
	memcpy(mac_b, temp, ETH_ALEN);
	free(temp);
}

int rt_compare_function(const void* a, const void* b) {
	const struct route_table_entry* rt_a = a;
	const struct route_table_entry* rt_b = b;
	uint32_t common_mask = ntohl(rt_a->mask) & ntohl(rt_b->mask);

	if ((ntohl(rt_a->prefix) & common_mask) <
		(ntohl(rt_b->prefix) & common_mask)) {
		return -1;
	} else if ((ntohl(rt_a->prefix) & common_mask) >
			   (ntohl(rt_b->prefix) & common_mask)) {
		return 1;
	}

	if (ntohl(rt_a->mask) < ntohl(rt_b->mask)) {
		return 1;
	} else if ((ntohl(rt_a->prefix) & common_mask) >
			   (ntohl(rt_b->prefix) & common_mask)) {
		return -1;
	}

	return 0;
}

int lpm_binary(struct route_table_entry *rtable,
			   int rtable_len, uint32_t dest) {
	int left = 0;
	int right = rtable_len;
	
	while (left < right) {
		int middle = (left + right) / 2;
		if (ntohl(rtable[middle].prefix) <
			(ntohl(dest) & ntohl(rtable[middle].mask))) {
			left = middle + 1;
		} else {
			right = middle;
		}
	}

	return left;
}

uint16_t incremental_checksum(struct iphdr* ip_hdr) {
	uint64_t sum;
	uint16_t old_ttl;

	old_ttl = ntohs(*(uint16_t*)&ip_hdr->ttl);
	ip_hdr->ttl = ip_hdr->ttl - 1;
	sum = old_ttl + (~ntohs(*(uint16_t*)&ip_hdr->ttl) & 0xffff);
	sum += ntohs(ip_hdr->check);
	sum = (sum & 0xffff) + (sum>>16);

	return htons(sum + (sum>>16));
}
