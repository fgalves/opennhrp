/* nhrp_peer.h - NHRP peer cache definitions
 *
 * Copyright (C) 2007-2009 Timo Teräs <timo.teras@iki.fi>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or later as
 * published by the Free Software Foundation.
 *
 * See http://www.gnu.org/ for details.
 */

#ifndef NHRP_PEER_H
#define NHRP_PEER_H

#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include "nhrp_address.h"
#include "libev.h"
#include "list.h"

#define NHRP_PEER_TYPE_INCOMPLETE	0x00	/* Resolution request sent */
#define NHRP_PEER_TYPE_NEGATIVE		0x01	/* Negative cached */
#define NHRP_PEER_TYPE_CACHED		0x02	/* Received/relayed resolution reply */
#define NHRP_PEER_TYPE_CACHED_ROUTE	0x03	/* Received/relayed resolution for route */
#define NHRP_PEER_TYPE_DYNAMIC		0x04	/* NHC registration */
#define NHRP_PEER_TYPE_DYNAMIC_NHS	0x05	/* Dynamic NHS from dns-map */
#define NHRP_PEER_TYPE_STATIC		0x06	/* Static mapping from config file */
#define NHRP_PEER_TYPE_STATIC_DNS	0x07	/* Static dns-map from config file */
#define NHRP_PEER_TYPE_LOCAL		0x08	/* Learned from interface config */
#define NHRP_PEER_TYPE_MAX		(NHRP_PEER_TYPE_LOCAL+1)

#define NHRP_PEER_TYPEMASK_ADJACENT \
	(BIT(NHRP_PEER_TYPE_CACHED) | \
	 BIT(NHRP_PEER_TYPE_DYNAMIC) | \
	 BIT(NHRP_PEER_TYPE_DYNAMIC_NHS) | \
	 BIT(NHRP_PEER_TYPE_STATIC) | \
	 BIT(NHRP_PEER_TYPE_LOCAL))

#define NHRP_PEER_TYPEMASK_REMOVABLE \
	(BIT(NHRP_PEER_TYPE_INCOMPLETE) | \
	 BIT(NHRP_PEER_TYPE_NEGATIVE) | \
	 BIT(NHRP_PEER_TYPE_CACHED) | \
	 BIT(NHRP_PEER_TYPE_CACHED_ROUTE) | \
	 BIT(NHRP_PEER_TYPE_DYNAMIC))

#define NHRP_PEER_TYPEMASK_PURGEABLE \
	(NHRP_PEER_TYPEMASK_REMOVABLE | \
	 BIT(NHRP_PEER_TYPE_DYNAMIC_NHS) | \
	 BIT(NHRP_PEER_TYPE_STATIC) | \
	 BIT(NHRP_PEER_TYPE_STATIC_DNS))

#define NHRP_PEER_TYPEMASK_ALL \
	(NHRP_PEER_TYPEMASK_PURGEABLE | \
	 BIT(NHRP_PEER_TYPE_LOCAL))

#define NHRP_PEER_FLAG_UNIQUE		0x01	/* Peer is unique; see RFC2332 */
#define NHRP_PEER_FLAG_REGISTER		0x02	/* For TYPE_STATIC: send registration */
#define NHRP_PEER_FLAG_CISCO		0x04	/* For TYPE_STATIC: peer is Cisco */
#define NHRP_PEER_FLAG_USED		0x10	/* Peer is in kernel ARP table */
#define NHRP_PEER_FLAG_LOWER_UP		0x20	/* Script executed succesfully */
#define NHRP_PEER_FLAG_UP		0x40	/* Can send all packets (registration ok) */
#define NHRP_PEER_FLAG_REPLACED		0x80	/* Peer has been replaced */
#define NHRP_PEER_FLAG_REMOVED		0x100	/* Deleted, but not removed from cache yet */
#define NHRP_PEER_FLAG_MARK		0x200	/* Can be used to temporarily mark peers */

#define NHRP_PEER_FIND_ROUTE		0x01
#define NHRP_PEER_FIND_EXACT		0x02
#define NHRP_PEER_FIND_SUBNET		0x04
#define NHRP_PEER_FIND_UP		0x10
#define NHRP_PEER_FIND_MARK		0x20

struct nhrp_interface;
struct nhrp_packet;

union __attribute__ ((__transparent_union__)) nhrp_peer_event {
	struct ev_timer *timer;
	struct ev_child *child;
};

struct nhrp_peer {
	unsigned int ref;
	unsigned int flags;

	struct list_head peer_list_entry;
	struct list_head mcast_list_entry;
	struct hlist_node nbma_hash_entry;

	struct nhrp_interface *interface;
	struct nhrp_peer *parent;
	union {
		struct nhrp_packet *queued_packet;
		struct nhrp_pending_request *request;
	};

	struct ev_timer timer;
	struct ev_child child;
	struct nhrp_address_query address_query;

	uint8_t type;
	uint8_t prefix_length;
	uint16_t afnum;
	uint16_t protocol_type;
	uint16_t mtu, my_nbma_mtu;
	ev_tstamp expire_time;
	ev_tstamp last_used;
	struct nhrp_address my_nbma_address;
	struct nhrp_address protocol_address;
	unsigned int holding_time;

	char *nbma_hostname;
	/* NHRP_PEER_TYPE_ROUTE: protocol addr., others: NBMA addr. */
	struct nhrp_address next_hop_address;
	struct nhrp_address next_hop_nat_oa;
};

struct nhrp_peer_selector {
	int flags; /* NHRP_PEER_FIND_xxx */
	int type_mask;

	struct nhrp_interface *interface;
	struct nhrp_peer *parent;
	const char *hostname;

	int prefix_length;
	struct nhrp_address protocol_address;
	struct nhrp_address next_hop_address;
};

const char * const nhrp_peer_type[NHRP_PEER_TYPE_MAX];
typedef int (*nhrp_peer_enumerator)(void *ctx, struct nhrp_peer *peer);

struct nhrp_peer *nhrp_peer_alloc(struct nhrp_interface *iface);
struct nhrp_peer *nhrp_peer_get(struct nhrp_peer *peer);
int nhrp_peer_put(struct nhrp_peer *peer);

void nhrp_peer_insert(struct nhrp_peer *peer);
void nhrp_peer_remove(struct nhrp_peer *peer);
void nhrp_peer_purge(struct nhrp_peer *peer);

int nhrp_peer_match(struct nhrp_peer *peer, struct nhrp_peer_selector *sel);

int nhrp_peer_foreach(nhrp_peer_enumerator e, void *ctx,
		      struct nhrp_peer_selector *sel);
int nhrp_peer_remove_matching(void *count, struct nhrp_peer *peer);
int nhrp_peer_purge_matching(void *count, struct nhrp_peer *peer);
int nhrp_peer_set_used_matching(void *ctx, struct nhrp_peer *peer);

int nhrp_peer_event_ok(union nhrp_peer_event e, int revents);
char *nhrp_peer_event_reason(union nhrp_peer_event e, int revents,
			     size_t buflen, char *buf);
struct nhrp_peer *nhrp_peer_from_event(union nhrp_peer_event e, int revents);
void nhrp_peer_run_script(struct nhrp_peer *peer, char *action,
			  void (*cb)(union nhrp_peer_event, int));
int nhrp_peer_discover_nhs(struct nhrp_peer *peer,
			   struct nhrp_address *newaddr);

struct nhrp_peer *nhrp_peer_route_full(struct nhrp_interface *iface,
				       struct nhrp_address *dest,
				       int flags, int type_mask,
				       struct nhrp_address *source,
				       struct list_head *exclude_cie_list);

static inline struct nhrp_peer *nhrp_peer_route(struct nhrp_interface *iface,
						struct nhrp_address *dest,
						int flags, int type_mask)
{
	return nhrp_peer_route_full(iface, dest, flags, type_mask, NULL, NULL);
}

void nhrp_peer_traffic_indication(struct nhrp_interface *iface,
				  uint16_t afnum, struct nhrp_address *dst);
void nhrp_peer_dump_cache(void);

#endif
