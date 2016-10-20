#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/openflow.h"
#include "../include/openflow131.h"



#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <net/ethernet.h>

#include <netinet/in.h>

#include "../include/benchmark.h"
#include "../include/switch.h"

static int debugMsg(struct fakeswitch * fs, char * msg, ...);
static int makeFeaturesReply(int switch_id, int xid, char * buf, int buflen);
static int makeStatsDescReply(struct ofp_stats_request * req, char * buf, int buflen);
static int parseSetConfig(struct ofp_header * msg);
static int makeConfigReply( int xid, char * buf, int buflen);
static int makeVendorReply(int xid, char * buf, int buflen);
static int makePacketIn(int switch_id, int xid, int buffer_id, char * buf, int buflen, int mac_address);
static int ofp13MakePacketIn(int switch_id, int xid, int buffer_id, char * buf, int buflen, int mac_address);
static int packetOutIsLldp(struct ofp_packet_out * po);
static void switchHandleWrite(struct fakeswitch *fs);
static void switchLearnDstmac(struct fakeswitch *fs);
void switchChangeStatusNow (struct fakeswitch *fs, int new_status);
void switchChangeStatus (struct fakeswitch *fs, int new_status);

static struct ofp_switch_config switch_config = {
	.header = {
		  OFP131_VERSION,
			OFPT_GET_CONFIG_REPLY,
			sizeof(struct ofp_switch_config),
			0},
	.flags = 0,
	.miss_send_len = 0,
};

static inline uint64_t htonll(uint64_t n){
    return htonl(1) == 1 ? n : ((uint64_t) htonl(n) << 32) | htonl(n >> 32);
}

static inline uint64_t ntohll(uint64_t n){
    return htonl(1) == 1 ? n : ((uint64_t) ntohl(n) << 32) | ntohl(n >> 32);
}

void switchInit(struct fakeswitch *fs, int dpid, int sock, int bufsize, int debug, int delay, enum testMode mode, int nMacAddresses, int learnDstMac, int version){
    char buf[BUFLEN];
    struct ofp_header ofph;
    fs->sock = sock;
    fs->debug = debug;
    fs->id = dpid;
    fs->inbuf = msgbufNew(bufsize);
    fs->outbuf = msgbufNew(bufsize);
    fs->probeState = 0;
    fs->mode = mode;
    fs->probeSize = ofp13MakePacketIn(fs->id, 0, 0, buf, BUFLEN, fs->macAddress++);
    fs->count = 0;
    fs->switchStatus = START;
    fs->delay = delay;
    fs->nMacAddresses = nMacAddresses;
    fs->macAddress = 0;
    fs->xid = 1;
    fs->learnDstMac = learnDstMac;
    fs->bufferId = 1;
		fs->version = version;

		// Send HELLO
    ofph.version = fs->version;
    ofph.type = OFPT_HELLO;
    ofph.length = htons(sizeof(ofph));
    ofph.xid   = htonl(1);

    msgbufPush(fs->outbuf,(char * ) &ofph, sizeof(ofph));
    debugMsg(fs, "Sent hello Version %04x", ofph.version);
}
/***********************************************************************/
void switchLearnDstmac(struct fakeswitch *fs){
    // thanks wireshark
    char gratuitous_arp_reply [] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c,
        0x29, 0x1a, 0x29, 0x1a, 0x08, 0x06, 0x00, 0x01,
        0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0x00, 0x0c,
        0x29, 0x1a, 0x29, 0x1a, 0x7f, 0x00, 0x00, 0x01,
        0x00, 0x0c, 0x29, 0x1a, 0x29, 0x1a, 0x7f, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    char mac_address_to_learn[] = { 0x80, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x01 };
    char ip_address_to_learn[] = { 192, 168 , 1, 40 };

    char buf [512];
    int len = sizeof( struct ofp_packet_in ) + sizeof(gratuitous_arp_reply);
    struct ofp_packet_in *pkt_in;
    struct ether_header * eth;
    void * arp_reply;

    memset(buf, 0, sizeof(buf));
    pkt_in = ( struct ofp_packet_in *) buf;

    pkt_in->header.version = OFP_VERSION;
    pkt_in->header.type = OFPT_PACKET_IN;
    pkt_in->header.length = htons(len);
    pkt_in->header.xid = htonl(fs->xid++);

    pkt_in->buffer_id = -1;
    pkt_in->total_len = htons(sizeof(gratuitous_arp_reply));
    pkt_in->in_port = htons(2);
    pkt_in->reason = OFPR_NO_MATCH;

    memcpy(pkt_in->data, gratuitous_arp_reply, sizeof(gratuitous_arp_reply));

    mac_address_to_learn[5] = fs->id;
    ip_address_to_learn[2] = fs->id;

    eth = (struct ether_header * ) pkt_in->data;
    memcpy (eth->ether_shost, mac_address_to_learn, 6);

    arp_reply =  ((void *)  eth) + sizeof (struct ether_header);
    memcpy ( arp_reply + 8, mac_address_to_learn, 6);
    memcpy ( arp_reply + 14, ip_address_to_learn, 4);
    memcpy ( arp_reply + 18, mac_address_to_learn, 6);
    memcpy ( arp_reply + 24, ip_address_to_learn, 4);

    msgbufPush(fs->outbuf,(char * ) pkt_in, len);
    debugMsg(fs, " Sent gratuitous ARP reply to learn about Mac Address: version %d length %d type %d eth: %x arp: %x ", pkt_in->header.version, len, buf[1], eth, arp_reply);
}
/***********************************************************************/
void ofp13SwitchLearnDstmac(struct fakeswitch *fs){
    // thanks wireshark
    char gratuitous_arp_reply [] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 0x29, 0x1a, 0x29, 0x1a, 0x08, 0x06, 0x00, 0x01,
        0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0x00, 0x0c, 0x29, 0x1a, 0x29, 0x1a, 0x7f, 0x00, 0x00, 0x01,
        0x00, 0x0c, 0x29, 0x1a, 0x29, 0x1a, 0x7f, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    char
		mac_address_to_learn[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    char ip_address_to_learn[] = { 192, 168 , 233, 6 };

    char buf [512];
    int len = sizeof( struct ofp131_packet_in )+  sizeof(gratuitous_arp_reply) + 32; //32 aligned
		debugMsg(fs, "Sizes gratuitous_arp_reply: %d packet_in: %d len: %d ", sizeof(gratuitous_arp_reply), sizeof( struct ofp131_packet_in ), len);
		unsigned short int op_code = htons(0x0002);
		uint64_t arp_header= htonll(0x0001080006040002);
    struct ofp131_packet_in *pkt_in;
    struct ether_header * eth;
		struct ofp_oxm_header *oxm_ptr;
    void * arp_reply;

    memset(buf, 0, sizeof(buf));
    pkt_in = ( struct ofp131_packet_in *) buf; //change this ofp131_packet_in

    pkt_in->header.version = OFP131_VERSION;
    pkt_in->header.type = OFPT131_PACKET_IN;
    pkt_in->header.xid = htonl(fs->xid++);
		pkt_in->header.length = htons(len);

    pkt_in->buffer_id = htonl(fs->bufferId);
		pkt_in->total_len = htons(sizeof(gratuitous_arp_reply));
    pkt_in->table_id = 0;
    pkt_in->cookie = 0;
		pkt_in->reason = OFPR_NO_MATCH;
    pkt_in->match.type = htons(1); // OFPMT_OXM
    pkt_in->match.length = htons(0x0A); // (sizeof (oxm_match)  + omx_fields)

		oxm_ptr = (void *)(pkt_in->match.oxm_fields);
  	oxm_ptr->length = 2;
    oxm_ptr->oxm_class = OFPXMC_OPENFLOW_BASIC;
    OFP_OXM_SHDR_FIELD(oxm_ptr,0x15);  //OFPXMT_OFB_ARP_OP = 21, ARP
		memcpy(oxm_ptr->data,&op_code,2); //ARP_REPLY OpCode
		HTON_OXM_HDR(oxm_ptr);

    memcpy( &pkt_in[32+10] , gratuitous_arp_reply, sizeof(gratuitous_arp_reply)); //24 + paddings

    mac_address_to_learn[5] = fs->id; //change addresses index according to switchId
    ip_address_to_learn[3] = fs->id; //commented while the tests involve only 1 switch and

    eth = (struct ether_header * ) ((char*)pkt_in + sizeof(struct
                ofp131_packet_in)+10);
    memcpy (eth->ether_shost, mac_address_to_learn, 6);
		eth->ether_type = htons(0x0806);

    arp_reply =  ((void *)  eth) + sizeof (struct ether_header); //offset from eth begin pointer
		memcpy ( arp_reply, &arp_header ,8); //HTYPE+PTYPE+HLEN+PLEN+OPCODE
    memcpy ( arp_reply + 8, mac_address_to_learn, 6);
    memcpy ( arp_reply + 14, ip_address_to_learn, 4);
    memcpy ( arp_reply + 18, mac_address_to_learn, 6);
    memcpy ( arp_reply + 24, ip_address_to_learn, 4);

    msgbufPush(fs->outbuf,(char * ) pkt_in, len);
    debugMsg(fs, "Sent gratuitous ARP reply in PacketIn message to learn about Mac Address: version: %d length: %d type: %d EtherType: 0x%hx eth: %x arp: %x ", pkt_in->header.version, len, buf[1], eth->ether_type, arp_reply);
}
/***********************************************************************/
void switchSetPollfd(struct fakeswitch *fs, struct pollfd *pfd){
    pfd->events = POLLIN|POLLOUT;
    pfd->fd = fs->sock;
}
/***********************************************************************/
int switchGetCount(struct fakeswitch *fs){
    int ret = fs->count;
    int count;
    int msglen;
    struct ofp_header * ofph;
    fs->count = 0;
    fs->probeState = 0;        // reset packet state
    // keep reading until there is nothing to clear out the queue
    while( (count = msgbufRead(fs->inbuf,fs->sock)) > 0) {
        while(count > 0) {
            // need to read msg by msg to ensure framing isn't broken
            ofph = msgbufPeek(fs->inbuf);
            msglen = ntohs(ofph->length);
            if(count < msglen)
                break;     // msg not all there yet;
            msgbufPull(fs->inbuf, NULL, ntohs(ofph->length));
            count -= msglen;
        }
    }
    return ret;
}
/***********************************************************************/
static int parseSetConfig(struct ofp_header * msg) {
	struct ofp_switch_config * sc;
	assert(msg->type == OFPT_SET_CONFIG);
	sc = (struct ofp_switch_config *) msg;
	memcpy(&switch_config, sc, sizeof(struct ofp_switch_config));

	return 0;
}
/***********************************************************************/
static int makeConfigReply( int xid, char * buf, int buflen){
	int len = sizeof(struct ofp_switch_config);
	assert(buflen >= len);
	switch_config.header.type = OFPT_GET_CONFIG_REPLY;
	switch_config.header.xid = xid;
	memcpy(buf, &switch_config, len);

	return len;
}
/***********************************************************************/
static int makeFeaturesReply(int id, int xid, char * buf, int buflen){
    struct ofp_switch_features * features;
    const char fake[] =     // stolen from wireshark
    {

      0x97,0x06,0x00,0xe0,0x04,0x01,0x00,0x00,0x00,0x00,0x76,0xa9,
      0xd4,0x0d,0x25,0x48,0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x07,0xff,0x00,0x01,0x1a,0xc1,0x51,0xff,0xef,0x8a,0x76,0x65,0x74,0x68,
      0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x02,0xce,0x2f,0xa2,0x87,0xf6,0x70,0x76,0x65,0x74,0x68,
      0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x03,0xca,0x8a,0x1e,0xf3,0x77,0xef,0x76,0x65,0x74,0x68,
      0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x04,0xfa,0xbc,0x77,0x8d,0x7e,0x0b,0x76,0x65,0x74,0x68,
      0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00
    };

    assert(buflen> sizeof(fake));
    memcpy(buf, fake, sizeof(fake));
    features = (struct ofp_switch_features *) buf;
    features->header.version = OFP_VERSION;
    features->header.xid = xid;
    features->datapath_id = htonll(id);
    return sizeof(fake);
}
/***********************************************************************/
static int  ofp13MakeFeaturesReply(int id, int xid, char * buf, int buflen){
    struct ofp131_switch_features * features;
    features = (struct ofp131_switch_features *) buf;
    features->header.version = OFP131_VERSION;
    features->header.xid = xid;
    features->header.length = htons(sizeof(struct ofp131_switch_features));
    features->header.type = OFPT131_FEATURES_REPLY;
    features->datapath_id = htonll(id);
    features->n_buffers  = 1;
		features->n_buffers  = 1;
		features->auxiliary_id = 0;
    features->n_tables  = 1;
    features->capabilities  = 0;
    return sizeof(struct ofp131_switch_features);
}
/***********************************************************************/
static int makeStatsReply(struct ofp_stats_request * req,
        int reply_type, char * buf, int buflen) {

    struct ofp_stats_reply * reply;
    int len = sizeof(struct ofp_stats_reply);
    assert(BUFLEN > len);

    reply = (struct ofp_stats_reply *) buf;
    reply->header.type = OFPT_STATS_REPLY;
    reply->header.length = htons(len);
    reply->type = ntohs(reply_type);
    reply->flags = 0;

    return len;
}
/***********************************************************************/
static int makeStatsDescReply(struct ofp_stats_request * req,
        char * buf, int buflen) {
    static struct ofp_desc_stats ofcbench_desc = {
        .mfr_desc = "OFC Benchmark - OpenFlow Controller Benchmark",
        .hw_desc  = "Software based on KulCloud Version of Cbench",
        .sw_desc  = "Version 1.0",
        .serial_num= "fsociety",
        .dp_desc  = "Virtual Switch from OfcBenchmarking Tool OF 1.3"
    };
    struct ofp_stats_reply * reply;
    int len = sizeof(struct ofp_stats_reply) +
                sizeof(struct ofp_desc_stats);
    assert(BUFLEN > len);
    assert(ntohs(req->type) == OFPST_DESC);

    memcpy( buf, req, sizeof(*req));
    reply = (struct ofp_stats_reply *) buf;
    reply->header.type = OFPT_STATS_REPLY;
    reply->header.length = htons(len);
		reply->type = ntohs(OFPST_DESC);
    reply->flags = 0;
    memcpy(reply->body, &ofcbench_desc, sizeof(ofcbench_desc));

    return len;
}
/***********************************************************************/
static int makeVendorReply(int xid, char * buf, int buflen){
    struct ofp_error_msg * e;
    assert(buflen> sizeof(struct ofp_error_msg));
    e = (struct ofp_error_msg *) buf;
    e->header.type = OFPT_ERROR;
    e->header.version = OFP_VERSION;
    e->header.length = htons(sizeof(struct ofp_error_msg));
    e->header.xid = xid;
    e->type = htons(OFPET_BAD_REQUEST);
    e->code = htons(OFPBRC_BAD_VENDOR);
    return sizeof(struct ofp_error_msg);
}
/***********************************************************************
 *  return 1 if the embedded packet in the packet_out is lldp
 *
 */

#ifndef ETHERTYPE_LLDP
#define ETHERTYPE_LLDP 0x88cc
#endif

static int packetOutIsLldp(struct ofp_packet_out * po){
	char * ptr = (char *) po;
	ptr += sizeof(struct ofp_packet_out) + ntohs(po->actions_len);
	struct ether_header * ethernet = (struct ether_header *) ptr;
	unsigned short ethertype = ntohs(ethernet->ether_type);
	if (ethertype == ETHERTYPE_VLAN) {
		ethernet = (struct ether_header *) ((char *) ethernet) +4;
		ethertype = ntohs(ethernet->ether_type);
	}

	return ethertype == ETHERTYPE_LLDP;
}
/***********************************************************************/
static int ofp13MakePacketIn(int switch_id, int xid, int buffer_id, char * buf, int buflen, int mac_address){
    struct ofp131_packet_in * pi;
    struct ofp_oxm_header *oxm_ptr;
    struct ether_header * eth;
    unsigned int in_port = htonl(4);
    const char fake[] = {
                0x97,0x0a,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x40,0x00,0x47,
								0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x45,0x00,0x00,0x32,
								0x00,0x00,0x00,0x00,0x40,0xff,0xf7,0x2c,0x45,0xa8,0x00,0x1a,0xc0,0xa8,0x01,0x28,
								0x7a,0x18,0x58,0x6b,0x11,0x08,0x97,0xf5,0x19,0xe2,0x65,0x7e,0x07,0xcc,0x31,0xc3,
								0x11,0xc7};
    memcpy(buf, fake, sizeof(fake));
    pi = (struct ofp131_packet_in *) buf;
    pi->header.version = OFP131_VERSION;
    pi->header.type = OFPT131_PACKET_IN;
    pi->header.xid = htonl(xid);
    pi->header.length = htons(sizeof(fake));
    pi->buffer_id = htonl(buffer_id);
		pi->total_len = htons(0x28); //size of payload, ty wireshark
    pi->table_id = 0;
    pi->cookie = 0;
    pi->match.type = htons(1); // OFPMT_OXM
    pi->match.length = htons(0x0c); // (sizeof (oxm_match) + omx_fields)

    oxm_ptr = (void *)(pi->match.oxm_fields);
    oxm_ptr->length = 4;
    oxm_ptr->oxm_class = OFPXMC_OPENFLOW_BASIC;
    OFP_OXM_SHDR_FIELD(oxm_ptr,0); //OFPXMT_OFB_IN_PORT
    memcpy(oxm_ptr->data,&in_port,4);
		HTON_OXM_HDR(oxm_ptr);

    eth = (struct ether_header * ) ((char*)pi + sizeof(struct
                ofp131_packet_in) + 10);
    // copy into src mac addr; only 4 bytes, but should suffice to not confuse
    // the controller; don't overwrite first byte
    memcpy(&eth->ether_shost[1], &mac_address, sizeof(mac_address));
    // mark this as coming from us, mostly for debug
    eth->ether_dhost[5] = switch_id;
    eth->ether_shost[5] = switch_id;
    eth->ether_type = htons(0x0800);
    return sizeof(fake);
}
/***********************************************************************/
static int makePacketIn(int switch_id, int xid, int buffer_id, char * buf, int buflen, int mac_address){
    struct ofp_packet_in * pi;
    struct ether_header * eth;
    const char fake[] = {
                0x97,0x0a,0x00,0x52,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x40,0x00,0x01,
								0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x08,0x00,
								0x45,0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x40,0xff,0xf7,0x2c,0xc0,0xa8,0x00,0x28,
								0xc0,0xa8,0x01,0x28,0x7a,0x18,0x58,0x6b,0x11,0x08,0x97,0xf5,0x19,0xe2,0x65,0x7e,
								0x07,0xcc,0x31,0xc3,0x11,0xc7,0xc4,0x0c,0x8b,0x95,0x51,0x51,0x33,0x54,0x51,0xd5,
								0x00,0x36};
    assert(buflen> sizeof(fake));
    memcpy(buf, fake, sizeof(fake));
    pi = (struct ofp_packet_in *) buf;
    pi->header.version = OFP_VERSION;
    pi->header.xid = htonl(xid);
    pi->buffer_id = htonl(buffer_id);
    eth = (struct ether_header * ) pi->data;
    // copy into src mac addr; only 4 bytes, but should suffice to not confuse
    // the controller; don't overwrite first byte
    memcpy(&eth->ether_shost[1], &mac_address, sizeof(mac_address));
    // mark this as coming from us, mostly for debug
    eth->ether_dhost[5] = switch_id;
    eth->ether_shost[5] = switch_id;
    return sizeof(fake);
}
/***********************************************************************/
static int ofp13MakeMultipartReply(int mp_type, int switch_id, int xid, char * buf, int buflen){
    struct ofp131_multipart_reply *mp_reply;
		struct ofp131_table_features *tf_reply;
		struct ofp131_table_feature_prop_oxm *tf_prop_h;
		struct ofp_oxm_header *oxm_ptr;

    int length = 0;
		static char reserved_mem [512];
		unsigned short int op_code = htons(0x0002);

    const char fake[] =     // OFPMP_PORT_DESC
    {
      0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x1a,0xc1,0x51,0xff,0xef,0x8a,0x00,0x00,
			0x76,0x65,0x74,0x68,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0xce,0x2f,0xa2,0x87,0xf6,0x70,0x00,0x00,
			0x76,0x65,0x74,0x68,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0xca,0x8a,0x1e,0xf3,0x77,0xef,0x00,0x00,
			0x76,0x65,0x74,0x68,0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0xfa,0xbc,0x77,0x8d,0x7e,0x0b,0x00,0x00,
			0x76,0x65,0x74,0x68,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };


    mp_reply = (struct ofp131_multipart_reply *) buf;
    mp_reply->header.version = OFP131_VERSION;
    mp_reply->header.xid = xid;
    mp_reply->header.type = OFPT131_MULTIPART_REPLY;
    mp_reply->type = htons(mp_type);
    mp_reply->flags = 0;

    switch(mp_type) {
        case OFPMP_PORT_DESC:
            mp_reply->header.length = htons(sizeof(struct ofp131_multipart_reply) +
                sizeof(fake));
            memcpy(mp_reply->body, fake, sizeof(fake));
            length = sizeof(struct ofp131_multipart_reply) + sizeof(fake);
            break;
			 case OFPMP_DESC:
			 			mp_reply->header.length = htons(sizeof(struct ofp131_multipart_reply)+ sizeof(struct ofp_desc_stats));
						static struct ofp_desc_stats mp_desc = {
								.mfr_desc = "OFC Benchmark - OpenFlow Controller Benchmark",
								.hw_desc  = "Software based on KulCloud Version of Cbench",
								.sw_desc  = "Version 1.0",
								.serial_num= "fsociety",
								.dp_desc  = "Virtual Switch from OfcBenchmarking Tool OF 1.3"
						};
						memcpy(mp_reply->body, &mp_desc, sizeof(mp_desc));
						length = sizeof(struct ofp131_multipart_reply) + sizeof( struct ofp_desc_stats);
						break;
				case OFPMP_TABLE_FEATURES:
						mp_reply->header.length = htons(sizeof(struct ofp131_multipart_reply)+ 72);
						memset(reserved_mem, 0, sizeof(reserved_mem));
						tf_reply = (struct ofp131_table_features *) reserved_mem;

						tf_reply->length=htons(0x0048); //sizeof(ofp131_table_features) + properties->length
						tf_reply->table_id=0x00;

						memset(tf_reply->name, '\0', OFP_MAX_TABLE_NAME_LEN);
						strcpy(tf_reply->name, "First Table");

						tf_reply->metadata_match= htonll(0xffffffffffffffff);
						tf_reply->metadata_write= htonll(0xffffffffffffffff);
						tf_reply->config= htonl(OFPTC_DEPRECATED_MASK);
						tf_reply->max_entries= htonl(0xffffffff);

						tf_prop_h = (void *)(tf_reply->properties);
						tf_prop_h->type= htons(OFPTFPT_MATCH);
						tf_prop_h->length=htons(0x08); //sizeof(ofp131_table_feature_prop_oxm) + oxm->length

						oxm_ptr = (void *)(tf_prop_h->oxm_ids);
						oxm_ptr->length = 2;
						oxm_ptr->oxm_class = OFPXMC_OPENFLOW_BASIC;
						OFP_OXM_SHDR_FIELD(oxm_ptr,0x15);  //OFPXMT_OFB_ARP_OP = 21, ARP
						HTON_OXM_HDR(oxm_ptr);
						memcpy(mp_reply->body, tf_reply, ntohs(tf_reply->length));

						length=sizeof(struct ofp131_multipart_reply) +72;
						break;

        default:
            mp_reply->header.length = htons(sizeof(struct ofp131_multipart_reply));
            length = sizeof(struct ofp131_multipart_reply);
            break;

    }
    return length;
}
/***********************************************************************/
void switchChangeStatusNow (struct fakeswitch *fs, int new_status) {
    fs->switchStatus = new_status;
    if(new_status == READY_TO_SEND) {
        fs->count = 0;
        fs->probeState = 0;
    }
}
/***********************************************************************/
void switchChangeStatus(struct fakeswitch *fs, int new_status) {
    if( fs->delay == 0) {
        switchChangeStatusNow(fs, new_status);
        debugMsg(fs, "Switched to next status %d", new_status);
    } else {
        fs->switchStatus = WAITING;
        fs->nextStatus = new_status;
        gettimeofday(&fs->delay_start, NULL);
        fs->delay_start.tv_sec += fs->delay / 1000;
        fs->delay_start.tv_usec += (fs->delay % 1000 ) * 1000;
        debugMsg(fs, "Delaying next status %d by %d ms", new_status, fs->delay);
    }
}
/***********************************************************************/
void switchHandleRead(struct fakeswitch *fs){
    int count;
    struct ofp_header * ofph;
    struct ofp_header echo;
    struct ofp_header barrier;
    char buf[BUFLEN];
    count = msgbufRead(fs->inbuf, fs->sock);   // read any queued data
    if (count <= 0)
    {
        fprintf(stderr, "Reading queued data. Controller msgbufRead() = %d:  ", count);
        if(count < 0)
            perror("msgbufRead");
        else
            fprintf(stderr, " Closed connection ");
        fprintf(stderr, "Exiting\n");
        exit(1);
    }
    while((count= msgbufCountBuffered(fs->inbuf)) >= sizeof(struct ofp_header ))
    {
        ofph = msgbufPeek(fs->inbuf);
        if(count < ntohs(ofph->length))
            return;     // msg not all there yet
        msgbufPull(fs->inbuf, NULL, ntohs(ofph->length));
        switch(ofph->type)
        {
            struct ofp_flow_mod * fm;
            struct ofp_packet_out *po;
            struct ofp_stats_request * stats_req;
            case OFPT_PACKET_OUT:
                po = (struct ofp_packet_out *) ofph;
                if ( fs->switchStatus == READY_TO_SEND && ! packetOutIsLldp(po)) {
                    // assume this is in response to what we sent
                    fs->count++;        // got response to what we went
                    fs->probeState--;
                }
                break;
            case OFPT_FLOW_MOD:
                fm = (struct ofp_flow_mod *) ofph;
                if(fs->switchStatus == READY_TO_SEND && (fm->command == htons(OFPFC_ADD) ||
                        fm->command == htons(OFPFC_MODIFY_STRICT)))
                {
                    fs->count++;        // got response to what we went
                    fs->probeState--;
                }
                break;
            case OFPT_FEATURES_REQUEST:
                // pull msgs out of buffer
                debugMsg(fs, "Got feature_req");
                // Send features reply
                count = makeFeaturesReply(fs->id, ofph->xid, buf, BUFLEN);
                msgbufPush(fs->outbuf, buf, count);
                debugMsg(fs, "Sent feature_rsp");
                switchChangeStatus(fs, fs->learnDstMac ? LEARN_DSTMAC : READY_TO_SEND);
                break;
            case OFPT_SET_CONFIG:
                // pull msgs out of buffer
                debugMsg(fs, "Parsing set_config");
								parseSetConfig(ofph);
                break;
            case OFPT_GET_CONFIG_REQUEST:
                // pull msgs out of buffer
                debugMsg(fs, "Got get_config_request");
                count = makeConfigReply(ofph->xid, buf, BUFLEN);
                msgbufPush(fs->outbuf, buf, count);
								if ((fs->mode == MODE_LATENCY)  && ( fs->probeState == 1 )) {
							    fs->probeState = 0;
									// restart probe state b/c some
					        // controllers block on config
                	debugMsg(fs, "Reset probe state b/c of get_config_reply");
								}
                debugMsg(fs, "Sent get_config_reply");
                break;
            case OFPT_VENDOR:
                // pull msgs out of buffer
                debugMsg(fs, "Got vendor");
                count = makeVendorReply(ofph->xid, buf, BUFLEN);
                msgbufPush(fs->outbuf, buf, count);
                debugMsg(fs, "Sent vendor");
                // apply nox hack; nox ignores packet_in until this msg is sent
                fs->probeState=0;
                break;
            case OFPT_HELLO:
								debugMsg(fs, "Got hello version %d",ofph->version);
                // we already sent our own HELLO; don't respond
                break;
            case OFPT_ECHO_REQUEST:
                debugMsg(fs, "Got echo, sent echo_resp");
                echo.version= OFP_VERSION;
                echo.length = htons(sizeof(echo));
                echo.type   = OFPT_ECHO_REPLY;
                echo.xid = ofph->xid;
                msgbufPush(fs->outbuf,(char *) &echo, sizeof(echo));
                break;
            case OFPT_BARRIER_REQUEST:
                debugMsg(fs, "Got barrier, sent barrier_resp");
                barrier.version= OFP_VERSION;
                barrier.length = htons(sizeof(barrier));
                barrier.type   = OFPT_BARRIER_REPLY;
                barrier.xid = ofph->xid;
                msgbufPush(fs->outbuf,(char *) &barrier, sizeof(barrier));
                break;
            case OFPT_STATS_REQUEST:
								stats_req  = (struct ofp_stats_request *) ofph;
								if ( ntohs(stats_req->type) == OFPST_DESC ) {
										count = makeStatsDescReply(stats_req, buf, BUFLEN);
										msgbufPush(fs->outbuf, buf, count);
										debugMsg(fs, "Sent description stats_reply");
										if ((fs->mode == MODE_LATENCY)  && ( fs->probeState == 1 )) {
												fs->probeState = 0;
												 // restart probe state b/c some
												 // controllers block on config
												debugMsg(fs, "Reset probe state b/c of desc_stats_request");
										}
								} else if(ntohs(stats_req->type) == OFPST_FLOW) {
										count = makeStatsReply(stats_req, OFPST_FLOW, buf, BUFLEN);
										msgbufPush(fs->outbuf, buf, count);
								}
								else {
										debugMsg(fs, "Silently ignoring non-desc stats_request msg\n");
								}
								break;
            default:
                fprintf(stderr, "Ignoring OpenFlow message type %d\n", ofph->type);
        };
        if(fs->probeState < 0)
        {
                debugMsg(fs, "WARN: Got more responses than probes!!: : %d",
                            fs->probeState);
                fs->probeState =0;
        }
    }
}
/***********************************************************************/
static void switchHandleWrite(struct fakeswitch *fs){
    char buf[BUFLEN];
    int count ;
    int send_count = 0 ;
    int throughput_buffer = BUFLEN;
    int i;
    if( fs->switchStatus == READY_TO_SEND)
    {
        if ((fs->mode == MODE_LATENCY)  && ( fs->probeState == 0 ))
            send_count = 1;                 // just send one packet
        else if ((fs->mode == MODE_THROUGHPUT) && (msgbufCountBuffered(fs->outbuf) < throughput_buffer))  // keep buffer full
            send_count = (throughput_buffer - msgbufCountBuffered(fs->outbuf)) / fs->probeSize;
        for (i = 0; i < send_count; i++)
        {
            // queue up packet
            fs->probeState++;
            // TODO come back and remove this copy
            count = makePacketIn(fs->id, fs->xid++, fs->bufferId, buf, BUFLEN, fs->macAddress);
            fs->macAddress = ( fs->macAddress + 1 ) % fs->nMacAddresses;
            fs->bufferId =  ( fs->bufferId + 1 ) % NUM_BUFFER_IDS;
            msgbufPush(fs->outbuf, buf, count);
            debugMsg(fs, "Send message %d", i);
        }
    } else if( fs->switchStatus == WAITING)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (timercmp(&now, &fs->delay_start, > ))
        {
            switchChangeStatusNow(fs, fs->nextStatus);
            debugMsg(fs, "Delay is over: switching to state %d", fs->nextStatus);
        }
    } else if (  fs->switchStatus == LEARN_DSTMAC)
    {
        // we should learn the dst mac addresses
        switchLearnDstmac(fs);
        switchChangeStatus(fs, READY_TO_SEND);
    }
    // send any data if it's queued
    if( msgbufCountBuffered(fs->outbuf) > 0)
        msgbufWrite(fs->outbuf, fs->sock, 0);
}
/***********************************************************************/
void switchHandleIo(struct fakeswitch *fs, const struct pollfd *pfd){//consider delete this and leave solo 131 handler
    if(pfd->revents & POLLIN)
        switchHandleRead(fs);
    if(pfd->revents & POLLOUT)
        switchHandleWrite(fs);
}
/***********************************************************************/
void ofp13SwitchHandleRead(struct fakeswitch *fs){
    int count;
    struct ofp_header * ofph;
    struct ofp_header echo;
		struct ofp_header barrier;
		struct ofp131_role_request role;
    char buf[BUFLEN];
    enum ofp131_multipart_types mp_req_type;
    count = msgbufRead(fs->inbuf, fs->sock);   // read any queued data
    if (count <= 0)
    {
        fprintf(stderr, "Reading Queued data OF1.3 Controller msgbufRead() = %d:  ", count);
        if(count < 0)
            perror("msgbufRead");
        else
            fprintf(stderr, "Closed connection ");
        fprintf(stderr, "Exiting\n");
        exit(1);
    }
    while((count= msgbufCountBuffered(fs->inbuf)) >= sizeof(struct ofp_header ))
    {
        ofph = msgbufPeek(fs->inbuf);
        if(count < ntohs(ofph->length))
            return;     // msg not all there yet
        msgbufPull(fs->inbuf, NULL, ntohs(ofph->length));
        switch(ofph->type)
        {
            struct ofp_flow_mod * fm;
            struct ofp_packet_out *po;
            struct ofp131_multipart_request * mp_req;
            case OFPT131_PACKET_OUT:
                po = (struct ofp_packet_out *) ofph;
                if ( fs->switchStatus == READY_TO_SEND && !packetOutIsLldp(po)) {
                    // assume this is in response to what we sent
                    fs->count++;        // got response to what we went
                    fs->probeState--;
                }
                break;
            case OFPT131_FLOW_MOD: //check
                fm = (struct ofp_flow_mod *) ofph;
                if(fs->switchStatus == READY_TO_SEND && (fm->command == htons(OFPFC_ADD) ||
                        fm->command == htons(OFPFC_MODIFY_STRICT)))
                {
                    fs->count++;        // got response to what we went
                    fs->probeState--;
                }
                break;
						case OFPT131_GET_CONFIG_REQUEST:
								// pull msgs out of buffer
								debugMsg(fs, "OF1.3 Got get_config_request");
								count = makeConfigReply(ofph->xid, buf, BUFLEN);
								msgbufPush(fs->outbuf, buf, count);
								if ((fs->mode == MODE_LATENCY)  && ( fs->probeState == 1 )) {
									fs->probeState = 0;
									// restart probe state b/c some
									// controllers block on config
									debugMsg(fs, "Reset probe state b/c of get_config_reply");
								}
								debugMsg(fs, "OF1.3 Sent get_config_reply");
								break;
						case OFPT131_SET_CONFIG:
                // pull msgs out of buffer
                debugMsg(fs, "OF1.3 Parsing set_config");
								parseSetConfig(ofph);
                break;
						case OFPT131_BARRIER_REQUEST:
						  	debugMsg(fs, "OF1.3 Got barrier_req, sent barrier_rep");
								barrier.version= OFP131_VERSION;
								barrier.length = htons(sizeof(barrier));
								barrier.type   = OFPT131_BARRIER_REPLY;
								barrier.xid = ofph->xid;
								msgbufPush(fs->outbuf,(char *) &barrier, sizeof(barrier));
								break;
            case OFPT131_FEATURES_REQUEST:
                // pull msgs out of buffer
                debugMsg(fs, "OF1.3 Got feature_req");
                // Send features reply
                count = ofp13MakeFeaturesReply(fs->id, ofph->xid, buf, BUFLEN);
                msgbufPush(fs->outbuf, buf, count);
                debugMsg(fs, "OF1.3 Sent feature_rsp, len %d",count);
                switchChangeStatus(fs, SEND_PORT_DESC);
                break;
            case OFPT131_HELLO:
                debugMsg(fs, "OF1.3 Got hello");
                // we already sent our own HELLO; don't respond
                break;
            case OFPT131_ECHO_REQUEST:
                debugMsg(fs, "OF1.3 Got echo, sent echo_resp");
                echo.version= OFP131_VERSION;
                echo.length = htons(sizeof(echo));
                echo.type   = OFPT_ECHO_REPLY;
                echo.xid = ofph->xid;
                msgbufPush(fs->outbuf,(char *) &echo, sizeof(echo));
                break;
            case OFPT131_MULTIPART_REQUEST:
                mp_req = (struct ofp131_multipart_request *) ofph;//changed from ofp_multipart_req
                mp_req_type = ntohs(mp_req->type);
                switch (mp_req_type)
                {
                    case OFPMP_PORT_DESC:
                        debugMsg(fs, "OF1.3 Got MP port desc req");
                        count = ofp13MakeMultipartReply(mp_req_type,
                                fs->id, ofph->xid, buf, BUFLEN);
                        fs->sendPortDesc = 1;
                        msgbufPush(fs->outbuf, buf, count);
                        debugMsg(fs, "OF1.3 Sent port description");
                        break;
                    case OFPMP_GROUP_FEATURES:
                        debugMsg(fs, "OF1.3 Got MP group feature req,"
                                " ignored");
                        break;
                    case OFPMP_METER_FEATURES:
                        debugMsg(fs, "OF1.3 Got MP meter feature req,"
                                " ignored");
                        break;
                    default:
										debugMsg(fs, "OF1.3 Got MP req of type: %d",mp_req_type	);
                        count = ofp13MakeMultipartReply(mp_req_type,
                                fs->id, ofph->xid, buf, BUFLEN);
                        msgbufPush(fs->outbuf, buf, count);
                        break;
                }
                break;
            case OFPT131_GROUP_MOD:
                debugMsg(fs, "OF1.3 got group mod, ignored");
                break;
            case OFPT131_METER_MOD:
                debugMsg(fs, "OF1.3 got meter mod, ignored");
                break;
            case OFPT131_ROLE_REQUEST:
                debugMsg(fs, "OF1.3 got role request, sent role reply");
								role.header.version= OFP131_VERSION;
								role.header.type = OFPT131_ROLE_REPLY;
						    role.header.xid = ofph->xid;
								role.header.length = htons(sizeof(role));
								role.role= htonl(OPFCR_ROLE_MASTER);
								role.generation_id = 0;
                msgbufPush(fs->outbuf,(char *) &role, sizeof(role));

                break;

            default:
                    fprintf(stderr, "Ignoring OpenFlow message type %d\n", ofph->type);
                break;
        };
        if(fs->probeState < 0)
        {
                debugMsg(fs, "WARN: Got more responses than probes!!: : %d",
                            fs->probeState);
                fs->probeState =0;
        }
    }
}
/***********************************************************************/
static void ofp13SwitchHandleWrite(struct fakeswitch *fs){
    char buf[BUFLEN];
    int count = 0;
    int send_count = 0 ;
    int throughput_buffer = BUFLEN;
    int i;
    if( fs->switchStatus == READY_TO_SEND)
    {
        if ((fs->mode == MODE_LATENCY)  && ( fs->probeState == 0 ))
            send_count = 1;                 // just send one packet
        else if ((fs->mode == MODE_THROUGHPUT) &&
                (msgbufCountBuffered(fs->outbuf) < throughput_buffer))  // keep buffer full
            send_count = (throughput_buffer - msgbufCountBuffered(fs->outbuf)) / fs->probeSize;
        for (i = 0; i < send_count; i++)
        {
            // queue up packet
            fs->probeState++;
            // TODO come back and remove this copy
            count = ofp13MakePacketIn(fs->id, fs->xid++, fs->bufferId, buf, BUFLEN, fs->macAddress);
            fs->macAddress = ( fs->macAddress + 1 ) % fs->nMacAddresses;
            fs->bufferId =  ( fs->bufferId + 1 ) % NUM_BUFFER_IDS;
            msgbufPush(fs->outbuf, buf, count);
            debugMsg(fs, "Send message %d from %d", i, send_count);
        }
    } else if( fs->switchStatus == WAITING)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (timercmp(&now, &fs->delay_start, > ))
        {
            switchChangeStatusNow (fs, fs->nextStatus);
            debugMsg(fs, "Delay is over: switching to state %d", fs->nextStatus);
        }
    } else if (  fs->switchStatus == LEARN_DSTMAC)
    {
        // we should learn the dst mac addresses
        ofp13SwitchLearnDstmac(fs);
        switchChangeStatus(fs, READY_TO_SEND);

    } else if (  fs->switchStatus == SEND_PORT_DESC)
    {
        // we should wait for Multipart (Port desc message) from controller
        if(fs->sendPortDesc)
				{
					debugMsg(fs, "Waited for Multipart OFPMP_PORT_DESC message to send pkt_in");
        	switchChangeStatus(fs, fs->learnDstMac ? LEARN_DSTMAC : READY_TO_SEND);
				}
    }
    // send any data if it's queued
    if( msgbufCountBuffered(fs->outbuf) > 0)
        msgbufWrite(fs->outbuf, fs->sock, 0);
}
/***********************************************************************/
void ofp13SwitchHandleIo(struct fakeswitch *fs, const struct pollfd *pfd){
    if(pfd->revents & POLLIN)
        ofp13SwitchHandleRead(fs);
    if(pfd->revents & POLLOUT)
        ofp13SwitchHandleWrite(fs);
}
/************************************************************************/
static int debugMsg(struct fakeswitch * fs, char * msg, ...){
    va_list aq;
    if(fs->debug == 0 )
        return 0;
    fprintf(stderr,"\n-------Switch %d: ", fs->id);
    va_start(aq,msg);
    vfprintf(stderr,msg,aq);
    if(msg[strlen(msg)-1] != '\n')
        fprintf(stderr, "\n");
     fflush(stderr); 
    return 1;
}
