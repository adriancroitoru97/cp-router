#ifndef _ICMP_H_
#define _ICMP_H_

#include "skel.h"

/*
 * Sends an ICMP message - destination unreacheable / time exceeded
 * It takes the original packet and modifies it accordingly before
 * sending it back to the source.
 */
void send_icmp_message(packet *m, uint8_t type, uint8_t code);

/*
 * Sends an ICMP echo reply.
 * It firstly checks if the packet is an ICMP echo request
 * and then modifies the original packet accordingly and sents it back.
 */ 
void send_imcp_echo_reply(packet *m);

#endif /* _ICMP_H_ */
