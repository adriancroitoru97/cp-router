#ifndef _UTILS_H_
#define _UTILS_H_

#include <math.h>
#include "skel.h"

#define MAX_RT_ENTRIES 100000

/* MAC - IPV4 pair, in the network bytes order */
typedef struct {
	uint8_t mac[ETH_ALEN];
	uint32_t ip;
} mac_ip_pair;

/*
 * Returns a pointer to the MAC address of the given IP,
 * if it exists in the cache and NULL, otherwise.
 */
uint8_t* find_mac_in_cache (uint32_t ip, mac_ip_pair *cache, int cache_len);

/*
 * Returns 1 if mac_a and mac_b are equal.
 * Returns 0 otherwise.
 */
int equal_macs (uint8_t *mac_a, uint8_t *mac_b);

/*
 * Sets all bytes of mac parameter to 255.
 */
void set_broadcast_mac (uint8_t *mac);

/*
 * Returns 1 if all bytes of mac_a are 255.
 * Returns 0 otherwise.
 */
int is_broadcast_mac (uint8_t *mac_a);

/*
 * Does what the name says.
 */
void swap_uints(uint32_t *a, uint32_t *b);

/*
 * Swaps 2 mac addresses.
 */
void swap_macs(uint8_t *mac_a, uint8_t *mac_b);


/*
 * Compare function used to sort the route table ascendingly
 * by prefixes and descendingly by masks.
 */
int rt_compare_function(const void* a, const void* b);

/*
 * Leftmost-match binary search. As the route table was sorted before
 * specifically, the leftmost-match will be the one with good prefix and
 * maximum mask. 
 */
int lpm_binary(struct route_table_entry *rtable,
			   int rtable_len, uint32_t dest);


/*
 * Incremental update of the checksum. Returns the new checksum.
 * Faster and useful when only the TTL field of an IP header modifies.
 * Idea and implementation taken from
 * Mallory & Kullberg RFC 1141 - Incremental Updating - January 1990.
 */
uint16_t incremental_checksum(struct iphdr* ip_hdr);

#endif /* _UTILS_H_ */
