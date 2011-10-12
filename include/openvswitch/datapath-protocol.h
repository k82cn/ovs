/*
 * Copyright (c) 2009, 2010, 2011 Nicira Networks.
 *
 * This file is offered under your choice of two licenses: Apache 2.0 or GNU
 * GPL 2.0 or later.  The permission statements for each of these licenses is
 * given below.  You may license your modifications to this file under either
 * of these licenses or both.  If you wish to license your modifications under
 * only one of these licenses, delete the permission text for the other
 * license.
 *
 * ----------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ----------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * ----------------------------------------------------------------------
 */

#ifndef OPENVSWITCH_DATAPATH_PROTOCOL_H
#define OPENVSWITCH_DATAPATH_PROTOCOL_H 1

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/socket.h>
#define ovs_be16 __be16
#define ovs_be32 __be32
#define ovs_be64 __be64
#else
#include "openvswitch/types.h"
#include <sys/socket.h>
#endif

#include <linux/netlink.h>

/* datapaths. */

#define OVS_DATAPATH_FAMILY  "ovs_datapath"
#define OVS_DATAPATH_MCGROUP "ovs_datapath"

enum ovs_datapath_cmd {
	OVS_DP_CMD_UNSPEC,
	OVS_DP_CMD_NEW,
	OVS_DP_CMD_DEL,
	OVS_DP_CMD_GET,
	OVS_DP_CMD_SET
};

/**
 * struct ovs_header - header for OVS Generic Netlink messages.
 * @dp_ifindex: ifindex of local port for datapath (0 to make a request not
 * specific to a datapath).
 *
 * Attributes following the header are specific to a particular OVS Generic
 * Netlink family, but all of the OVS families use this header.
 */
struct ovs_header {
	int dp_ifindex;
};

/**
 * enum ovs_datapath_attr - attributes for %OVS_DP_* commands.
 * @OVS_DP_ATTR_NAME: Name of the network device that serves as the "local
 * port".  This is the name of the network device whose dp_ifindex is given in
 * the &struct ovs_header.  Always present in notifications.  Required in
 * %OVS_DP_NEW requests.  May be used as an alternative to specifying
 * dp_ifindex in other requests (with a dp_ifindex of 0).
 * @OVS_DP_ATTR_UPCALL_PID: The Netlink socket in userspace that is initially
 * set on the datapath port (for OVS_ACTION_ATTR_MISS).  Only valid on
 * %OVS_DP_CMD_NEW requests. A value of zero indicates that upcalls should
 * not be sent.
 * @OVS_DP_ATTR_STATS: Statistics about packets that have passed through the
 * datapath.  Always present in notifications.
 * @OVS_DP_ATTR_IPV4_FRAGS: One of %OVS_DP_FRAG_*.  Always present in
 * notifications.  May be included in %OVS_DP_NEW or %OVS_DP_SET requests to
 * change the fragment handling policy.
 *
 * These attributes follow the &struct ovs_header within the Generic Netlink
 * payload for %OVS_DP_* commands.
 */
enum ovs_datapath_attr {
	OVS_DP_ATTR_UNSPEC,
	OVS_DP_ATTR_NAME,       /* name of dp_ifindex netdev */
	OVS_DP_ATTR_UPCALL_PID, /* Netlink PID to receive upcalls */
	OVS_DP_ATTR_STATS,      /* struct ovs_dp_stats */
	OVS_DP_ATTR_IPV4_FRAGS,	/* 32-bit enum ovs_frag_handling */
	__OVS_DP_ATTR_MAX
};

#define OVS_DP_ATTR_MAX (__OVS_DP_ATTR_MAX - 1)

/**
 * enum ovs_frag_handling - policy for handling received IPv4 fragments.
 * @OVS_DP_FRAG_ZERO: Treat IP fragments as IP protocol 0 and transport ports
 * zero.
 * @OVS_DP_FRAG_DROP: Drop IP fragments.  Do not pass them through the flow
 * table or up to userspace.
 */
enum ovs_frag_handling {
	OVS_DP_FRAG_UNSPEC,
	OVS_DP_FRAG_ZERO,	/* Treat IP fragments as transport port 0. */
	OVS_DP_FRAG_DROP	/* Drop IP fragments. */
};

struct ovs_dp_stats {
    uint64_t n_frags;           /* Number of dropped IP fragments. */
    uint64_t n_hit;             /* Number of flow table matches. */
    uint64_t n_missed;          /* Number of flow table misses. */
    uint64_t n_lost;            /* Number of misses not sent to userspace. */
    uint64_t n_flows;           /* Number of flows present */
};

struct ovs_vport_stats {
	uint64_t   rx_packets;		/* total packets received       */
	uint64_t   tx_packets;		/* total packets transmitted    */
	uint64_t   rx_bytes;		/* total bytes received         */
	uint64_t   tx_bytes;		/* total bytes transmitted      */
	uint64_t   rx_errors;		/* bad packets received         */
	uint64_t   tx_errors;		/* packet transmit problems     */
	uint64_t   rx_dropped;		/* no space in linux buffers    */
	uint64_t   tx_dropped;		/* no space available in linux  */
};

/* Logical ports. */
#define OVSP_LOCAL      ((uint16_t)0)

#define OVS_PACKET_FAMILY "ovs_packet"

enum ovs_packet_cmd {
        OVS_PACKET_CMD_UNSPEC,

        /* Kernel-to-user notifications. */
        OVS_PACKET_CMD_MISS,    /* Flow table miss. */
        OVS_PACKET_CMD_ACTION,  /* OVS_ACTION_ATTR_USERSPACE action. */

        /* User commands. */
        OVS_PACKET_CMD_EXECUTE  /* Apply actions to a packet. */
};

/**
 * enum ovs_packet_attr - attributes for %OVS_PACKET_* commands.
 * @OVS_PACKET_ATTR_PACKET: Present for all notifications.  Contains the entire
 * packet as received, from the start of the Ethernet header onward.  For
 * %OVS_PACKET_CMD_ACTION, %OVS_PACKET_ATTR_PACKET reflects changes made by
 * actions preceding %OVS_ACTION_ATTR_USERSPACE, but %OVS_PACKET_ATTR_KEY is
 * the flow key extracted from the packet as originally received.
 * @OVS_PACKET_ATTR_KEY: Present for all notifications.  Contains the flow key
 * extracted from the packet as nested %OVS_KEY_ATTR_* attributes.  This allows
 * userspace to adapt its flow setup strategy by comparing its notion of the
 * flow key against the kernel's.
 * @OVS_PACKET_ATTR_ACTIONS: Contains actions for the packet.  Used
 * for %OVS_PACKET_CMD_EXECUTE.  It has nested %OVS_ACTION_ATTR_* attributes.
 * @OVS_PACKET_ATTR_USERDATA: Present for an %OVS_PACKET_CMD_ACTION
 * notification if the %OVS_ACTION_ATTR_USERSPACE action specified an
 * %OVS_USERSPACE_ATTR_USERDATA attribute.
 *
 * These attributes follow the &struct ovs_header within the Generic Netlink
 * payload for %OVS_PACKET_* commands.
 */
enum ovs_packet_attr {
	OVS_PACKET_ATTR_UNSPEC,
	OVS_PACKET_ATTR_PACKET,      /* Packet data. */
	OVS_PACKET_ATTR_KEY,         /* Nested OVS_KEY_ATTR_* attributes. */
	OVS_PACKET_ATTR_ACTIONS,     /* Nested OVS_ACTION_ATTR_* attributes. */
	OVS_PACKET_ATTR_USERDATA,    /* u64 OVS_ACTION_ATTR_USERSPACE arg. */
	__OVS_PACKET_ATTR_MAX
};

#define OVS_PACKET_ATTR_MAX (__OVS_PACKET_ATTR_MAX - 1)

enum ovs_vport_type {
	OVS_VPORT_TYPE_UNSPEC,
	OVS_VPORT_TYPE_NETDEV,   /* network device */
	OVS_VPORT_TYPE_INTERNAL, /* network device implemented by datapath */
	OVS_VPORT_TYPE_PATCH,    /* virtual tunnel connecting two vports */
	OVS_VPORT_TYPE_GRE,      /* GRE tunnel */
	OVS_VPORT_TYPE_CAPWAP,   /* CAPWAP tunnel */
	__OVS_VPORT_TYPE_MAX
};

#define OVS_VPORT_TYPE_MAX (__OVS_VPORT_TYPE_MAX - 1)

#define OVS_VPORT_FAMILY  "ovs_vport"
#define OVS_VPORT_MCGROUP "ovs_vport"

enum ovs_vport_cmd {
	OVS_VPORT_CMD_UNSPEC,
	OVS_VPORT_CMD_NEW,
	OVS_VPORT_CMD_DEL,
	OVS_VPORT_CMD_GET,
	OVS_VPORT_CMD_SET
};

/**
 * enum ovs_vport_attr - attributes for %OVS_VPORT_* commands.
 * @OVS_VPORT_ATTR_PORT_NO: 32-bit port number within datapath.
 * @OVS_VPORT_ATTR_TYPE: 32-bit %OVS_VPORT_TYPE_* constant describing the type
 * of vport.
 * @OVS_VPORT_ATTR_NAME: Name of vport.  For a vport based on a network device
 * this is the name of the network device.  Maximum length %IFNAMSIZ-1 bytes
 * plus a null terminator.
 * @OVS_VPORT_ATTR_UPCALL_PID: The Netlink socket in userspace that
 * OVS_PACKET_CMD_MISS upcalls will be directed to for packets received on
 * this port.  A value of zero indicates that upcalls should not be sent.
 * @OVS_VPORT_ATTR_STATS: A &struct ovs_vport_stats giving statistics for
 * packets sent or received through the vport.
 * @OVS_VPORT_ATTR_ADDRESS: A 6-byte Ethernet address for the vport.
 *
 * These attributes follow the &struct ovs_header within the Generic Netlink
 * payload for %OVS_VPORT_* commands.
 *
 * For %OVS_VPORT_CMD_NEW requests, the %OVS_VPORT_ATTR_TYPE and
 * %OVS_VPORT_ATTR_NAME attributes are required.  %OVS_VPORT_ATTR_PORT_NO is
 * optional; if not specified a free port number is automatically selected.
 * Whether %OVS_VPORT_ATTR_OPTIONS is required or optional depends on the type
 * of vport.  %OVS_VPORT_ATTR_STATS and %OVS_VPORT_ATTR_ADDRESS are optional,
 * and other attributes are ignored.
 *
 * For other requests, if %OVS_VPORT_ATTR_NAME is specified then it is used to
 * look up the vport to operate on; otherwise dp_idx from the &struct
 * ovs_header plus %OVS_VPORT_ATTR_PORT_NO determine the vport.
 */
enum ovs_vport_attr {
	OVS_VPORT_ATTR_UNSPEC,
	OVS_VPORT_ATTR_PORT_NO,	/* port number within datapath */
	OVS_VPORT_ATTR_TYPE,	/* 32-bit OVS_VPORT_TYPE_* constant. */
	OVS_VPORT_ATTR_NAME,	/* string name, up to IFNAMSIZ bytes long */
	OVS_VPORT_ATTR_UPCALL_PID, /* Netlink PID to receive upcalls */
	OVS_VPORT_ATTR_STATS,	/* struct ovs_vport_stats */
	OVS_VPORT_ATTR_ADDRESS, /* hardware address */
	OVS_VPORT_ATTR_OPTIONS, /* nested attributes, varies by vport type */
	__OVS_VPORT_ATTR_MAX
};

#define OVS_VPORT_ATTR_MAX (__OVS_VPORT_ATTR_MAX - 1)

/* OVS_VPORT_ATTR_OPTIONS attributes for patch vports. */
enum {
	OVS_PATCH_ATTR_UNSPEC,
	OVS_PATCH_ATTR_PEER,	/* name of peer vport, as a string */
	__OVS_PATCH_ATTR_MAX
};

#define OVS_PATCH_ATTR_MAX (__OVS_PATCH_ATTR_MAX - 1)

/* Flows. */

#define OVS_FLOW_FAMILY  "ovs_flow"
#define OVS_FLOW_MCGROUP "ovs_flow"

enum ovs_flow_cmd {
	OVS_FLOW_CMD_UNSPEC,
	OVS_FLOW_CMD_NEW,
	OVS_FLOW_CMD_DEL,
	OVS_FLOW_CMD_GET,
	OVS_FLOW_CMD_SET
};

struct ovs_flow_stats {
    uint64_t n_packets;         /* Number of matched packets. */
    uint64_t n_bytes;           /* Number of matched bytes. */
};

enum ovs_key_type {
	OVS_KEY_ATTR_UNSPEC,
	OVS_KEY_ATTR_TUN_ID,    /* 64-bit tunnel ID */
	OVS_KEY_ATTR_IN_PORT,   /* 32-bit OVS dp port number */
	OVS_KEY_ATTR_ETHERNET,  /* struct ovs_key_ethernet */
	OVS_KEY_ATTR_8021Q,     /* struct ovs_key_8021q */
	OVS_KEY_ATTR_ETHERTYPE,	/* 16-bit Ethernet type */
	OVS_KEY_ATTR_IPV4,      /* struct ovs_key_ipv4 */
	OVS_KEY_ATTR_IPV6,      /* struct ovs_key_ipv6 */
	OVS_KEY_ATTR_TCP,       /* struct ovs_key_tcp */
	OVS_KEY_ATTR_UDP,       /* struct ovs_key_udp */
	OVS_KEY_ATTR_ICMP,      /* struct ovs_key_icmp */
	OVS_KEY_ATTR_ICMPV6,    /* struct ovs_key_icmpv6 */
	OVS_KEY_ATTR_ARP,       /* struct ovs_key_arp */
	OVS_KEY_ATTR_ND,        /* struct ovs_key_nd */
	__OVS_KEY_ATTR_MAX
};

#define OVS_KEY_ATTR_MAX (__OVS_KEY_ATTR_MAX - 1)

struct ovs_key_ethernet {
	uint8_t	 eth_src[6];
	uint8_t	 eth_dst[6];
};

struct ovs_key_8021q {
	ovs_be16 q_tpid;
	ovs_be16 q_tci;
};

struct ovs_key_ipv4 {
	ovs_be32 ipv4_src;
	ovs_be32 ipv4_dst;
	uint8_t  ipv4_proto;
	uint8_t  ipv4_tos;
};

struct ovs_key_ipv6 {
	ovs_be32 ipv6_src[4];
	ovs_be32 ipv6_dst[4];
	uint8_t  ipv6_proto;
	uint8_t  ipv6_tos;
};

struct ovs_key_tcp {
	ovs_be16 tcp_src;
	ovs_be16 tcp_dst;
};

struct ovs_key_udp {
	ovs_be16 udp_src;
	ovs_be16 udp_dst;
};

struct ovs_key_icmp {
	uint8_t icmp_type;
	uint8_t icmp_code;
};

struct ovs_key_icmpv6 {
	uint8_t icmpv6_type;
	uint8_t icmpv6_code;
};

struct ovs_key_arp {
	ovs_be32 arp_sip;
	ovs_be32 arp_tip;
	ovs_be16 arp_op;
	uint8_t  arp_sha[6];
	uint8_t  arp_tha[6];
};

struct ovs_key_nd {
	uint32_t nd_target[4];
	uint8_t  nd_sll[6];
	uint8_t  nd_tll[6];
};

/**
 * enum ovs_flow_attr - attributes for %OVS_FLOW_* commands.
 * @OVS_FLOW_ATTR_KEY: Nested %OVS_KEY_ATTR_* attributes specifying the flow
 * key.  Always present in notifications.  Required for all requests (except
 * dumps).
 * @OVS_FLOW_ATTR_ACTIONS: Nested %OVS_ACTION_ATTR_* attributes specifying
 * the actions to take for packets that match the key.  Always present in
 * notifications.  Required for %OVS_FLOW_CMD_NEW requests, optional
 * @OVS_FLOW_ATTR_STATS: &struct ovs_flow_stats giving statistics for this
 * flow.  Present in notifications if the stats would be nonzero.  Ignored in
 * requests.
 * @OVS_FLOW_ATTR_TCP_FLAGS: An 8-bit value giving the OR'd value of all of the
 * TCP flags seen on packets in this flow.  Only present in notifications for
 * TCP flows, and only if it would be nonzero.  Ignored in requests.
 * @OVS_FLOW_ATTR_USED: A 64-bit integer giving the time, in milliseconds on
 * the system monotonic clock, at which a packet was last processed for this
 * flow.  Only present in notifications if a packet has been processed for this
 * flow.  Ignored in requests.
 * @OVS_FLOW_ATTR_CLEAR: If present in a %OVS_FLOW_CMD_SET request, clears the
 * last-used time, accumulated TCP flags, and statistics for this flow.
 * Otherwise ignored in requests.  Never present in notifications.
 *
 * These attributes follow the &struct ovs_header within the Generic Netlink
 * payload for %OVS_FLOW_* commands.
 */
enum ovs_flow_attr {
	OVS_FLOW_ATTR_UNSPEC,
	OVS_FLOW_ATTR_KEY,       /* Sequence of OVS_KEY_ATTR_* attributes. */
	OVS_FLOW_ATTR_ACTIONS,   /* Nested OVS_ACTION_ATTR_* attributes. */
	OVS_FLOW_ATTR_STATS,     /* struct ovs_flow_stats. */
	OVS_FLOW_ATTR_TCP_FLAGS, /* 8-bit OR'd TCP flags. */
	OVS_FLOW_ATTR_USED,      /* u64 msecs last used in monotonic time. */
	OVS_FLOW_ATTR_CLEAR,     /* Flag to clear stats, tcp_flags, used. */
	__OVS_FLOW_ATTR_MAX
};

#define OVS_FLOW_ATTR_MAX (__OVS_FLOW_ATTR_MAX - 1)

/**
 * enum ovs_sample_attr - Attributes for %OVS_ACTION_ATTR_SAMPLE action.
 * @OVS_SAMPLE_ATTR_PROBABILITY: 32-bit fraction of packets to sample with
 * @OVS_ACTION_ATTR_SAMPLE.  A value of 0 samples no packets, a value of
 * %UINT32_MAX samples all packets and intermediate values sample intermediate
 * fractions of packets.
 * @OVS_SAMPLE_ATTR_ACTIONS: Set of actions to execute in sampling event.
 * Actions are passed as nested attributes.
 *
 * Executes the specified actions with the given probability on a per-packet
 * basis.
 */
enum ovs_sample_attr {
	OVS_SAMPLE_ATTR_UNSPEC,
	OVS_SAMPLE_ATTR_PROBABILITY, /* u32 number */
	OVS_SAMPLE_ATTR_ACTIONS,     /* Nested OVS_ACTION_ATTR_* attributes. */
	__OVS_SAMPLE_ATTR_MAX,
};

#define OVS_SAMPLE_ATTR_MAX (__OVS_SAMPLE_ATTR_MAX - 1)

/**
 * enum ovs_userspace_attr - Attributes for %OVS_ACTION_ATTR_USERSPACE action.
 * @OVS_USERSPACE_ATTR_PID: u32 Netlink PID to which the %OVS_PACKET_CMD_ACTION
 * message should be sent.  Required.
 * @OVS_USERSPACE_ATTR_USERDATA: If present, its u64 argument is copied to the
 * %OVS_PACKET_CMD_ACTION message as %OVS_PACKET_ATTR_USERDATA,
 */
enum ovs_userspace_attr {
	OVS_USERSPACE_ATTR_UNSPEC,
	OVS_USERSPACE_ATTR_PID,	      /* u32 Netlink PID to receive upcalls. */
	OVS_USERSPACE_ATTR_USERDATA,  /* u64 optional user-specified cookie. */
	__OVS_USERSPACE_ATTR_MAX
};

#define OVS_USERSPACE_ATTR_MAX (__OVS_USERSPACE_ATTR_MAX - 1)

/* Action types. */
enum ovs_action_type {
	OVS_ACTION_ATTR_UNSPEC,
	OVS_ACTION_ATTR_OUTPUT,	      /* Output to switch port. */
	OVS_ACTION_ATTR_USERSPACE,    /* Nested OVS_USERSPACE_ATTR_*. */
	OVS_ACTION_ATTR_PUSH_VLAN,    /* Set the 802.1q TCI value. */
	OVS_ACTION_ATTR_POP_VLAN,     /* Strip the 802.1q header. */
	OVS_ACTION_ATTR_SET_DL_SRC,   /* Ethernet source address. */
	OVS_ACTION_ATTR_SET_DL_DST,   /* Ethernet destination address. */
	OVS_ACTION_ATTR_SET_NW_SRC,   /* IPv4 source address. */
	OVS_ACTION_ATTR_SET_NW_DST,   /* IPv4 destination address. */
	OVS_ACTION_ATTR_SET_NW_TOS,   /* IP ToS/DSCP field (6 bits). */
	OVS_ACTION_ATTR_SET_TP_SRC,   /* TCP/UDP source port. */
	OVS_ACTION_ATTR_SET_TP_DST,   /* TCP/UDP destination port. */
	OVS_ACTION_ATTR_SET_TUNNEL,   /* Set the encapsulating tunnel ID. */
	OVS_ACTION_ATTR_SET_PRIORITY, /* Set skb->priority. */
	OVS_ACTION_ATTR_POP_PRIORITY, /* Restore original skb->priority. */
	OVS_ACTION_ATTR_SAMPLE,       /* Nested OVS_SAMPLE_ATTR_*. */
	__OVS_ACTION_ATTR_MAX
};

#define OVS_ACTION_ATTR_MAX (__OVS_ACTION_ATTR_MAX - 1)

#endif  /* openvswitch/datapath-protocol.h */
