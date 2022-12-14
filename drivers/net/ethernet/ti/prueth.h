/* SPDX-License-Identifier: GPL-2.0 */

/* PRU ICSS Ethernet driver
 *
 * Copyright (C) 2015-2018 Texas Instruments Incorporated - http://www.ti.com
 */

#ifndef __NET_TI_PRUETH_H
#define __NET_TI_PRUETH_H

#include <linux/types.h>
#include <linux/pruss.h>
#include <linux/types.h>
#include <net/lredev.h>

#include "icss_switch.h"
#include "prueth_ptp.h"

#define PRUETH_NUMQUEUES	5

/* PRUSS local memory map */
#define ICSS_LOCAL_SHARED_RAM   0x00010000

#define EMAC_POLL_WEIGHT	(64) /* Default NAPI poll weight */
#define EMAC_MAX_PKTLEN		(ETH_HLEN + VLAN_HLEN + ETH_DATA_LEN)

/* default timer for NSP and HSR/PRP */
#define PRUETH_TIMER_MS		(10)
#define PRUETH_NSP_TIMER_COUNT	(10)
/* NSP counter refresh every 100 msec */
#define PRUETH_NSP_TIMER_MS	(PRUETH_TIMER_MS * PRUETH_NSP_TIMER_COUNT)

/* PRU Ethernet Type - Ethernet functionality (protocol
 * implemented) provided by the PRU firmware being loaded.
 */
enum pruss_ethtype {
	PRUSS_ETHTYPE_EMAC = 0,
	PRUSS_ETHTYPE_HSR,
	PRUSS_ETHTYPE_PRP,
	PRUSS_ETHTYPE_SWITCH,
	PRUSS_ETHTYPE_MAX,
};

#define PRUETH_IS_EMAC(p)	((p)->eth_type == PRUSS_ETHTYPE_EMAC)
#define PRUETH_IS_SWITCH(p)	((p)->eth_type == PRUSS_ETHTYPE_SWITCH)
#define PRUETH_IS_HSR(p)	((p)->eth_type == PRUSS_ETHTYPE_HSR)
#define PRUETH_IS_PRP(p)	((p)->eth_type == PRUSS_ETHTYPE_PRP)
#define PRUETH_IS_LRE(p)	(PRUETH_IS_HSR(p) || PRUETH_IS_PRP(p))

#define PRUETH_REG_DUMP_VER		1

/* Encoding: 32-16: Reserved, 16-8: Reg dump version, 8-0: Ethertype  */
#define PRUETH_REG_DUMP_GET_VER(x)	((PRUETH_REG_DUMP_VER << 8) | ((x)->eth_type))

/**
 * struct prueth_queue_desc - Queue descriptor
 * @rd_ptr:	Read pointer, points to a buffer descriptor in Shared PRU RAM.
 * @wr_ptr:	Write pointer, points to a buffer descriptor in Shared PRU RAM.
 * @busy_s:	Slave queue busy flag, set by slave(us) to request access from
 *		master(PRU).
 * @status:	Bit field status register, Bits:
 *			0: Master queue busy flag.
 *			1: Packet has been placed in collision queue.
 *			2: Packet has been discarded due to overflow.
 * @max_fill_level:	Maximum queue usage seen.
 * @overflow_cnt:	Count of queue overflows.
 *
 * Each port has up to 4 queues with variable length. The queue is processed
 * as ring buffer with read and write pointers. Both pointers are address
 * pointers and increment by 4 for each buffer descriptor position. Queue has
 * a length defined in constants and a status.
 */
struct prueth_queue_desc {
	u16 rd_ptr;
	u16 wr_ptr;
	u8 busy_s;
	u8 status;
	u8 max_fill_level;
	u8 overflow_cnt;
} __packed;

/**
 * struct prueth_queue - Information about a queue in memory
 * @buffer_offset: buffer offset in OCMC RAM
 * @queue_desc_offset: queue descriptor offset in Shared RAM
 * @buffer_desc_offset: buffer descriptors offset in Shared RAM
 * @buffer_desc_end: end address of buffer descriptors in Shared RAM
 */
struct prueth_queue_info {
	u16 buffer_offset;
	u16 queue_desc_offset;
	u16 buffer_desc_offset;
	u16 buffer_desc_end;
} __packed;

/**
 * struct prueth_packet_info - Info about a packet in buffer
 * @start_offset: start offset of the frame in the buffer for HSR/PRP
 * @shadow: this packet is stored in the collision queue
 * @port: port packet is on
 * @length: length of packet
 * @broadcast: this packet is a broadcast packet
 * @error: this packet has an error
 * @sv_frame: indicate if the frame is a SV frame for HSR/PRP
 * @lookup_success: src mac found in FDB
 * @flood: packet is to be flooded
 * @timstamp: Specifies if timestamp is appended to the packet
 */
struct prueth_packet_info {
	bool start_offset;
	bool shadow;
	unsigned int port;
	unsigned int length;
	bool broadcast;
	bool error;
	bool sv_frame;
	bool lookup_success;
	bool flood;
	bool timestamp;
};

/**
 * struct port_statistics - Statistics structure for capturing statistics
 *			    on PRUs
 * @tx_bcast: Number of broadcast packets sent
 * @tx_mcast:Number of multicast packets sent
 * @tx_ucast:Number of unicast packets sent
 *
 * @tx_octets:Number of undersized frames rcvd
 *
 * @rx_bcast:Number of broadcast packets rcvd
 * @rx_mcast:Number of multicast packets rcvd
 * @rx_ucast:Number of unicast packets rcvd
 *
 * @rx_octets:Number of Rx packets
 *
 * @tx64byte:Number of 64 byte packets sent
 * @tx65_127byte:Number of 65-127 byte packets sent
 * @tx128_255byte:Number of 128-255 byte packets sent
 * @tx256_511byte:Number of 256-511 byte packets sent
 * @tx512_1023byte:Number of 512-1023 byte packets sent
 * @tx1024byte:Number of 1024 and larger size packets sent
 *
 * @rx64byte:Number of 64 byte packets rcvd
 * @rx65_127byte:Number of 65-127 byte packets rcvd
 * @rx128_255byte:Number of 128-255 byte packets rcvd
 * @rx256_511byte:Number of 256-511 byte packets rcvd
 * @rx512_1023byte:Number of 512-1023 byte packets rcvd
 * @rx1024byte:Number of 1024 and larger size packets rcvd
 *
 * @late_coll:Number of late collisions(Half Duplex)
 * @single_coll:Number of single collisions (Half Duplex)
 * @multi_coll:Number of multiple collisions (Half Duplex)
 * @excess_coll:Number of excess collisions(Half Duplex)
 *
 * @rx_misalignment_frames:Number of non multiple of 8 byte frames rcvd
 * @stormprev_counter:Number of packets dropped because of Storm Prevention
 * @mac_rxerror:Number of MAC receive errors
 * @sfd_error:Number of invalid SFD
 * @def_tx:Number of transmissions deferred
 * @mac_txerror:Number of MAC transmit errors
 * @rx_oversized_frames:Number of oversized frames rcvd
 * @rx_undersized_frames:Number of undersized frames rcvd
 * @rx_crc_frames:Number of CRC error frames rcvd
 * @dropped_packets:Number of packets dropped due to link down on opposite port
 *
 * @tx_hwq_overflow:Hardware Tx Queue (on PRU) over flow count
 * @tx_hwq_underflow:Hardware Tx Queue (on PRU) under flow count
 *
 * @u32 cs_error: Number of carrier sense errors
 * @sqe_test_error: Number of MAC receive errors
 *
 * Above fields are aligned so that it's consistent
 * with the memory layout in PRU DRAM, this is to facilitate easy
 * memcpy. Don't change the order of the fields.
 *
 * @vlan_dropped: Number of VLAN tagged packets dropped
 * @multicast_dropped: Number of multicast packets dropped
 */
struct port_statistics {
	u32 tx_bcast;
	u32 tx_mcast;
	u32 tx_ucast;

	u32 tx_octets;

	u32 rx_bcast;
	u32 rx_mcast;
	u32 rx_ucast;

	u32 rx_octets;

	u32 tx64byte;
	u32 tx65_127byte;
	u32 tx128_255byte;
	u32 tx256_511byte;
	u32 tx512_1023byte;
	u32 tx1024byte;

	u32 rx64byte;
	u32 rx65_127byte;
	u32 rx128_255byte;
	u32 rx256_511byte;
	u32 rx512_1023byte;
	u32 rx1024byte;

	u32 late_coll;
	u32 single_coll;
	u32 multi_coll;
	u32 excess_coll;

	u32 rx_misalignment_frames;
	u32 stormprev_counter_bc;
	u32 stormprev_counter_mc;
	u32 stormprev_counter_uc;
	u32 mac_rxerror;
	u32 sfd_error;
	u32 def_tx;
	u32 mac_txerror;
	u32 rx_oversized_frames;
	u32 rx_undersized_frames;
	u32 rx_crc_frames;
	u32 dropped_packets;

	u32 tx_hwq_overflow;
	u32 tx_hwq_underflow;

	u32 cs_error;
	u32 sqe_test_error;

	u32 vlan_dropped;
	u32 multicast_dropped;
} __packed;

/* In switch mode there are 3 real ports i.e. 3 mac addrs.
 * however Linux sees only the host side port. The other 2 ports
 * are the switch ports.
 * In emac mode there are 2 real ports i.e. 2 mac addrs.
 * Linux sees both the ports.
 */
enum prueth_port {
	PRUETH_PORT_HOST = 0,	/* host side port */
	PRUETH_PORT_MII0,	/* physical port MII 0 */
	PRUETH_PORT_MII1,	/* physical port MII 1 */
};

enum prueth_mac {
	PRUETH_MAC0 = 0,
	PRUETH_MAC1,
	PRUETH_NUM_MACS,
};

/* In both switch & emac modes there are 3 port queues
 * EMAC mode:
 *	RX packets for both MII0 & MII1 ports come on
 *	QUEUE_HOST.
 *	TX packets for MII0 go on QUEUE_MII0, TX packets
 *	for MII1 go on QUEUE_MII1.
 * Switch mode:
 *	Host port RX packets come on QUEUE_HOST
 *	TX packets might have to go on MII0 or MII1 or both.
 *	MII0 TX queue is QUEUE_MII0 and MII1 TX queue is
 *	QUEUE_MII1.
 */
enum prueth_port_queue_id {
	PRUETH_PORT_QUEUE_HOST = 0,
	PRUETH_PORT_QUEUE_MII0,
	PRUETH_PORT_QUEUE_MII1,
	PRUETH_PORT_QUEUE_MAX,
};

/* Each port queue has 4 queues and 1 collision queue */
enum prueth_queue_id {
	PRUETH_QUEUE1 = 0,
	PRUETH_QUEUE2,
	PRUETH_QUEUE3,
	PRUETH_QUEUE4,
	PRUETH_COLQUEUE,	/* collision queue */
};

/* PRUeth memory range identifiers */
enum prueth_mem {
	PRUETH_MEM_DRAM0 = 0,
	PRUETH_MEM_DRAM1,
	PRUETH_MEM_SHARED_RAM,
	PRUETH_MEM_OCMC,
	PRUETH_MEM_MAX,
};

/* Firmware offsets/size information */
struct prueth_fw_offsets {
	u32 index_array_offset;
	u32 bin_array_offset;
	u32 nt_array_offset;
	u32 index_array_loc;
	u32 bin_array_loc;
	u32 nt_array_loc;
	u32 index_array_max_entries;
	u32 bin_array_max_entries;
	u32 nt_array_max_entries;
	u32 vlan_ctrl_byte;
	u32 vlan_filter_tbl;
	u32 mc_ctrl_byte;
	u32 mc_filter_mask;
	u32 mc_filter_tbl;
	/* IEP wrap is used in the rx packet ordering logic and
	 * is different for ICSSM v1.0 vs 2.1
	 */
	u32 iep_wrap;
	u16 hash_mask;
};

/**
 * @fw_name: firmware names of firmware to run on PRU
 */
struct prueth_firmware {
	const char *fw_name[PRUSS_ETHTYPE_MAX];
};

/**
 * struct prueth_private_data - PRU Ethernet private data
 * @fw_names: firmware names to be used for PRUSS ethernet usecases
 */
struct prueth_private_data {
	const struct prueth_firmware fw_pru[PRUSS_NUM_PRUS];
	/* TODO: need to get this from firmware */
	bool rev_2_1;
	bool support_lre;
	bool support_switch;
};

struct nsp_counter {
	unsigned long cookie;
	u16 credit;
};

/* data for each emac port */
struct prueth_emac {
	struct prueth *prueth;
	struct net_device *ndev;
	u8 mac_addr[6];
	struct napi_struct napi;
	u32 msg_enable;

	int link;
	int speed;
	int duplex;

	const char *phy_id;
	struct device_node *phy_node;
	int phy_if;
	struct phy_device *phydev;
	struct rproc *pru;

	enum prueth_port port_id;
	enum prueth_port_queue_id tx_port_queue;

	enum prueth_queue_id rx_queue_start;
	enum prueth_queue_id rx_queue_end;

	enum prueth_mem dram;

	int rx_irq;
	int tx_irq;

	struct prueth_queue_desc __iomem *rx_queue_descs;
	struct prueth_queue_desc __iomem *tx_queue_descs;

	struct port_statistics stats; /* stats holder when i/f is down */
	unsigned char mc_filter_mask[ETH_ALEN];	/* for multicast filtering */

	spinlock_t lock;	/* serialize access */
	spinlock_t nsp_lock;	/* serialize access to nsp_counters */
	spinlock_t addr_lock;	/* serialize access to VLAN/MC filter table */

	u8 nsp_timer_count;
	struct nsp_counter nsp_bc;
	struct nsp_counter nsp_mc;
	struct nsp_counter nsp_uc;
	bool nsp_enabled;

	int offload_fwd_mark;

	struct sk_buff *ptp_skb[PRUETH_PTP_TS_EVENTS];
	struct sk_buff *ptp_ct_skb[PRUETH_PTP_TS_EVENTS];
	spinlock_t ptp_skb_lock;	/* serialize access */
	int emac_ptp_tx_irq;
	int hsr_ptp_tx_irq;
	bool ptp_tx_enable;
	bool ptp_rx_enable;
};

struct prueth_ndev_priority {
	struct net_device *ndev;
	int priority;
};

/**
 * struct prueth - PRUeth structure
 * @dev: device
 * @pruss: pruss handle
 * @pru0: rproc instance to PRU0
 * @pru1: rproc instance to PRU1
 * @mem: PRUSS memory resources we need to access
 * @sram_pool: OCMC ram pool for buffers
 * @mii_rt: regmap to mii_rt block
 * @iep: Pointer to ICSS IEP data
 * @tbl_check_timer: HR timer for refreshing NSP counters
 *
 * @eth_node: node for each emac node
 * @emac: emac data for three ports, one host and two physical
 * @registered_netdevs: net device for each registered emac
 *
 * @hw_bridge_dev: pointer to hw_bridge device
 * @fdb_tbl: pointer to FDB table struct
 *
 * @prueth_ndev_nb: netdev notifier block
 * @prueth_sw_switchdev_notifier: non blocking switchdev notifier block
 * @prueth_sw_switchdev_bl_notifier: blocking switchdev notifier block
 *
 * @emac_configured: bit mask to configured ports
 * @br_members: bit mask indicating ports that are part of the bridge
 * @eth_type: flag indicate firmware mode (Dual emac vs Switch etc)
 * @base_mac: random mac used as physical ID for each port of a switch
 */
struct prueth {
	struct device *dev;
	struct pruss *pruss;
	struct rproc *pru0, *pru1;
	struct pruss_mem_region mem[PRUETH_MEM_MAX];
	struct gen_pool *sram_pool;
	struct regmap *mii_rt;
	struct icss_iep *iep;
	struct prueth_ecap *ecap;
	struct hrtimer tbl_check_timer;
	const struct prueth_private_data *fw_data;
	struct prueth_fw_offsets *fw_offsets;

	/* HSR-PRP */
	bool support_lre;
	struct prueth_ndev_priority *hp, *lp;
	/* NAPI for lp and hp queue scans */
	struct napi_struct napi_lpq;
	struct napi_struct napi_hpq;
	int rx_lpq_irq;
	int rx_hpq_irq;
	unsigned int hsr_mode;
	unsigned int tbl_check_period;
	unsigned int node_table_clear;
	unsigned int node_table_clear_last_cmd;
	unsigned int tbl_check_mask;
	enum iec62439_3_tr_modes prp_tr_mode;
	struct node_tbl	*nt;
	struct nt_queue_t *mac_queue;
	struct kthread_worker *nt_kworker;
	struct kthread_work nt_work;
	u32 rem_cnt;
	/* lock between kthread worker and rx packet processing code */
	spinlock_t nt_lock;
	struct lre_statistics *lre_stats;

	struct device_node *eth_node[PRUETH_NUM_MACS];
	struct prueth_emac *emac[PRUETH_NUM_MACS];
	struct net_device *registered_netdevs[PRUETH_NUM_MACS];
	struct device_node *prueth_np;

	struct net_device *hw_bridge_dev;
	struct fdb_tbl *fdb_tbl;

	struct notifier_block prueth_ndev_nb;
	struct notifier_block prueth_sw_switchdev_notifier;
	struct notifier_block prueth_sw_switchdev_bl_notifier;

	unsigned int eth_type;
	/* mutex to enter critical region in ndo_open() and
	 * ndo_kill() as common resources for switch based firmware is
	 * to be initialized for the first port in ndo_open() and
	 * cleaned up on last port in ndo_stop().
	 */
	struct mutex mlock;
	u8 emac_configured;
	u8 br_members;
	u8 base_mac[ETH_ALEN];
};

void prueth_enable_nsp(struct prueth_emac *emac);
void prueth_start_timer(struct prueth_emac *emac);
int emac_ndo_setup_tc(struct net_device *dev, enum tc_setup_type type,
		      void *type_data);
void parse_packet_info(struct prueth *prueth, u32 buffer_descriptor,
		       struct prueth_packet_info *pkt_info);
int emac_rx_packet(struct prueth_emac *emac, u16 *bd_rd_ptr,
		   struct prueth_packet_info pkt_info,
		   const struct prueth_queue_info *rxqueue);
int emac_add_del_vid(struct prueth_emac *emac,
		     bool add, __be16 proto, u16 vid);
irqreturn_t prueth_ptp_tx_irq_handle(int irq, void *dev);
irqreturn_t prueth_ptp_tx_irq_work(int irq, void *dev);

extern const struct prueth_queue_desc queue_descs[][NUM_QUEUES];

static inline void emac_finish_napi(struct prueth_emac *emac,
				    struct napi_struct *napi,
				    int irq)
{
	napi_complete(napi);
	enable_irq(irq);
}

void emac_mc_filter_bin_allow(struct prueth_emac *emac, u8 hash);
void emac_mc_filter_bin_disallow(struct prueth_emac *emac, u8 hash);
u8 emac_get_mc_hash(u8 *mac, u8 *mask);
#endif /* __NET_TI_PRUETH_H */
