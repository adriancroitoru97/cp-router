# Communication Protocols - Router

Implementation of the *DATA PLANE* component of a router - the packets
forwarding process. The router works with a static routing table, so
the *CONTROL PLANE* (the routing algorithms) was not implemented at this step.


## ARP Protocol

### ARP Request received
When the router receives an `ARP Request`, if it has the same IP as the request
specifies, it *replies* to the sender with the MAC address of the interface on
which the packet arrived. The router modifies accordingly the ethernet
and ARP headers and sends back the packet.

### ARP Reply received
When the router receives an `ARP Reply` due to a previous request, the sender's
MAC address along with its IP are added in a cache. This cache will be used to
store MAC addresses in order to avoid sending requests whenever a packet has to
be forwarded to a next hop. This increases the time efficiency of the router.

Also, every time an `ARP reply` arrives to the router, the entire
*to-be-forwarded* packet queue is traversed and those packets who now
have a valid destination MAC in the cache are forwarded. If the reply
hasn't yet come for a packet, it is added back in the queue, and it will stay
there until the according reply with the destination MAC arrives.

### ARP Request sent
In the forwarding process, the router may not have in cache the next-hop's
MAC address. In this case, an `ARP Request` is broadcasted to the network,
in order to receive the MAC address of the next-hop.


## IP Protocol

If the router receives an `IPv4` protocol, it will process it accordingly.\
The first step is to verify the checksum. If wrong, the packet is thrown. After
that, the TTL is verified and updated. In case of `TTL <= 1`, an `ICMP` message
is sent to the source of the packet.\
Also, the router verifies if it is the final destination of the packet.
If true, the packet is probably an `ICMP Echo request`, and the router wil
send back an `ICMP Echo reply`.

After all the above steps, the router begins the `forwarding` process.
It recalculates the checksum, searches its routing table for the next hop.
If it's not found, a `Host Unreachable ICMP message`
is sent to the source of the packet.\
If the next-hop protocol address was found, the router searches in its cache
for the hardware address in order to forward the packet. If the cache does not
contain this address, an `ARP request` packet will be created and broadcasted
in order to obtain the next-hop's MAC, while the current packet will be put in
a waiting queue. Otherwise, the packet is directly sent to the next-hop.


## ICMP Protocol

### ICMP Echo reply
If an `ICMP Echo request` arrives, the router takes the packet, swap the
addresses, modifies the type and code in the ICMP header
and sends back the packet.\
Of course, checksums are always updated.

### ICMP message
As the `Host unreachable` and `Time exceeded` messages share approximatively
the same structure - the router implements a single function for this task.
The first `64 bytes` of the original packet's data (data = what's after the
Ethernet and IP headers) are temporary saved in a buffer. The ICMP header is
added after the previous 2 headers, and its fields are filled accordingly.
After that, the `64 bytes buffer` is copied at the end of the 3 headers and
the newly modified packet is sent back to its source.


## Efficient Longest Prefix Match
As the static routing table may contain up to 100000 entries, a linear approach
in finding the next-hop for a packet would not be fast enough.\
The solution I found was to sort the routing table only once - when the router
is turned on, and then binary search the next-hop.

I implemented a compare function, used in the `qsort` function. So, the entries
will be sorted ascending by the prefixes and descending by the mask length.

Keeping in mind the previous sort, the `leftmost-match binary search`
will efficiently return the searched next-hop.

This approach has reduced the `O(n)` linear complexity for each search
to `O(n*logn)` - the initial sort + `O(logn)` - each search.


## Incremental Checksum
As in most cases the only `IP header` field which is modified during the
forwarding process is the `TTL`, calculating the checksum over and over again
would not be very efficient. So, as stated in
`Mallory & Kullberg RFC 1141 - Incremental Updating - January 1990`, a simpler
and more efficient formula can be applied. The idea is to increment
the checksum's high byte and add the carry
of this operation to the final checksum.


## License
[Adrian-Valeriu Croitoru, 322CA](https://github.com/adriancroitoru97)