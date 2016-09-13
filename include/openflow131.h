#ifndef __OPENFLOW131_H
#define __OPENFLOW131_H


/***********************************************************************/

#define HTON_OXM_HDR(oxm) (*(uint32_t *)(oxm) = htonl(*(uint32_t *)(oxm)))
#define NTOH_OXM_HDR(oxm) (*(uint32_t *)(oxm) = ntohl(*(uint32_t *)(oxm)))

/* OXM Class IDs.
 * The high order bit differentiate reserved classes from member classes.
 * Classes 0x0000 to 0x7FFF are member classes, allocated by ONF.
 * Classes 0x8000 to 0xFFFE are reserved classes, reserved for
 * standardisation.
 */
enum ofp_oxm_class {
    OFPXMC_NXM_0 = 0x0000, /* Backward compatibility with NXM */
    OFPXMC_NXM_1 = 0x0001, /* Backward compatibility with NXM */
    OFPXMC_OPENFLOW_BASIC = 0x8000, /* Basic class for OpenFlow
                                     */
    OFPXMC_EXPERIMENTER = 0xFFFF, /* Experimenter class */
};

/* Fields to match against flows */
struct ofp131_match {
    uint16_t type; /* One of OFPMT_* */
    uint16_t length; /* Length of ofp_match (excluding padding) */
    /* Followed by:
       * - Exactly (length - 4) (possibly 0) bytes containing OXM TLVs, then
       * - Exactly ((length + 7)/8*8 - length) (between 0 and 7) bytes of
       * all-zero bytes
       * In summary, ofp_match is padded as needed, to make its overall size
       * a multiple of 8, to preserve alignement in structures using it.
     */
    uint8_t oxm_fields[4]; /* OXMs start here - Make compiler happy */
};
struct ofp_oxm_header {
    uint8_t length;
#define OFP_OXM_GHDR_FIELD(oxm) (((oxm)->oxm_field_hm >> 1) & 0xff)
#define OFP_OXM_GHDR_HM(oxm) ((oxm)->oxm_field_hm & 0x1)
#define OFP_OXM_GHDR_TYPE(oxm)  ((oxm)->oxm_class << 15 |\
        OFP_OXM_GHDR_FIELD(oxm))
#define OFP_OXM_SHDR_FIELD(oxm, field) ((oxm)->oxm_field_hm = ((field & \
                0x7f) << 1) | \
        OFP_OXM_GHDR_HM(oxm))
#define OFP_OXM_SHDR_HM(oxm, hm) ((oxm)->oxm_field_hm = (hm & 0x1)| \
        OFP_OXM_GHDR_FIELD(oxm))
uint8_t oxm_field_hm;
uint16_t oxm_class;
uint8_t  data[0];
};


/* Packet received on port (datapath -> controller). */
struct ofp131_packet_in {
    struct ofp_header header;
    uint32_t buffer_id; /* ID assigned by datapath. */
    uint16_t total_len; /* Full length of frame. */
    uint8_t reason; /* Reason packet is being sent (one of OFPR_*) */
    uint8_t table_id; /* ID of the table that was looked up */
    uint64_t cookie; /* Cookie of the flow entry that was looked up. */
    struct ofp131_match match; /* Packet metadata. Variable size. */
    /* Followed by:
       * - Exactly 2 all-zero padding bytes, then
       * - An Ethernet frame whose length is inferred from header.length.
       * The padding bytes preceding the Ethernet frame ensure that the IP
       * header (if any) following the Ethernet header is 32-bit aligned.
       */
    //uint8_t pad[2]; /* Align to 64 bit + 16 bit */
    //uint8_t data[0]; /* Ethernet frame */
};
enum ofp131_type {
    /* Immutable messages. */
    OFPT131_HELLO = 0,              /* Symmetric message */
    OFPT131_ERROR = 1,              /* Symmetric message */
    OFPT131_ECHO_REQUEST = 2,       /* Symmetric message */
    OFPT131_ECHO_REPLY = 3,         /* Symmetric message */
    OFPT131_EXPERIMENTER = 4,       /* Symmetric message */

    /* Switch configuration messages. */
    OFPT131_FEATURES_REQUEST = 5,   /* Controller/switch message */
    OFPT131_FEATURES_REPLY = 6,     /* Controller/switch message */
    OFPT131_GET_CONFIG_REQUEST = 7, /* Controller/switch message */
    OFPT131_GET_CONFIG_REPLY = 8,   /* Controller/switch message */
    OFPT131_SET_CONFIG = 9,         /* Controller/switch message */

    /* Asynchronous messages. */
    OFPT131_PACKET_IN = 10,         /* Async message */
    OFPT131_FLOW_REMOVED = 11,      /* Async message */
    OFPT131_PORT_STATUS = 12,       /* Async message */

    /* Controller command messages. */
    OFPT131_PACKET_OUT = 13,        /* Controller/switch message */
    OFPT131_FLOW_MOD = 14,          /* Controller/switch message */
    OFPT131_GROUP_MOD = 15,         /* Controller/switch message */
    OFPT131_PORT_MOD = 16,          /* Controller/switch message */
    OFPT131_TABLE_MOD = 17,         /* Controller/switch message */

    /* Multipart messages. */
    OFPT131_MULTIPART_REQUEST = 18, /* Controller/switch message */
    OFPT131_MULTIPART_REPLY = 19,   /* Controller/switch message */

    /* Barrier messages. */
    OFPT131_BARRIER_REQUEST = 20,   /* Controller/switch message */
    OFPT131_BARRIER_REPLY = 21,     /* Controller/switch message */

    /* Queue Configuration messages. */
    OFPT131_QUEUE_GET_CONFIG_REQUEST = 22,  /* Controller/switch message */
    OFPT131_QUEUE_GET_CONFIG_REPLY = 23,    /* Controller/switch message */

    /* Controller role change request messages. */
    OFPT131_ROLE_REQUEST = 24,      /* Controller/switch message */
    OFPT131_ROLE_REPLY = 25,        /* Controller/switch message */

    /* Asynchronous message configuration. */
    OFPT131_GET_ASYNC_REQUEST = 26, /* Controller/switch message */
    OFPT131_GET_ASYNC_REPLY = 27,   /* Controller/switch message */
    OFPT131_SET_ASYNC = 28,         /* Controller/switch message */

    /* Meters and rate limiters configuration messages. */
    OFPT131_METER_MOD = 29,         /* Controller/switch message */
};



enum ofp131_multipart_types {
    /* Description of this OpenFlow switch.
     * The request body is empty.
     * The reply body is struct ofp_desc. */
    OFPMP_DESC = 0,
    /* Individual flow statistics.
     * The request body is struct ofp_flow_stats_request.
     * The reply body is an array of struct ofp_flow_stats. */
    OFPMP_FLOW = 1,
    /* Aggregate flow statistics.
     * The request body is struct ofp_aggregate_stats_request.
     * The reply body is struct ofp_aggregate_stats_reply. */
    OFPMP_AGGREGATE = 2,
    /* Flow table statistics.
     * The request body is empty.
     * The reply body is an array of struct ofp_table_stats. */
    OFPMP_TABLE = 3,
    /* Port statistics.
     * The request body is struct ofp_port_stats_request.
     * The reply body is an array of struct ofp_port_stats. */
    OFPMP_PORT_STATS = 4,
    /* Queue statistics for a port
     * The request body is struct ofp_queue_stats_request.
     * The reply body is an array of struct ofp_queue_stats */
    OFPMP_QUEUE = 5,
    /* Group counter statistics.
     * The request body is struct ofp_group_stats_request.
     * The reply is an array of struct ofp_group_stats. */
    OFPMP_GROUP = 6,
    /* Group description.
     * The request body is empty.
     * The reply body is an array of struct ofp_group_desc_stats. */
    OFPMP_GROUP_DESC = 7,
    /* Group features.
     * The request body is empty.
     * The reply body is struct ofp_group_features. */
    OFPMP_GROUP_FEATURES = 8,
    /* Meter statistics.
     * The request body is struct ofp_meter_multipart_requests.
     * The reply body is an array of struct ofp_meter_stats. */
    OFPMP_METER = 9,
    /* Meter configuration.
     * The request body is struct ofp_meter_multipart_requests.
     * The reply body is an array of struct ofp_meter_config. */
    OFPMP_METER_CONFIG = 10,
    /* Meter features.
       73 © 2012; The Open Networking Foundation
       OpenFlow Switch Specication Version 1.3.1
     * The request body is empty.
     * The reply body is struct ofp_meter_features. */
    OFPMP_METER_FEATURES = 11,
    /* Table features.
     * The request body is either empty or contains an array of
     * struct ofp_table_features containing the controller's
     * desired view of the switch. If the switch is unable to
     * set the specified view an error is returned.
     * The reply body is an array of struct ofp_table_features. */
    OFPMP_TABLE_FEATURES = 12,
    /* Port description.
     * The request body is empty.
     * The reply body is an array of struct ofp_port. */
    OFPMP_PORT_DESC = 13,
    /* Experimenter extension.
     * The request and reply bodies begin with
     * struct ofp_experimenter_multipart_header.
     * The request and reply bodies are otherwise experimenter-defined. */
    OFPMP_EXPERIMENTER = 0xffff
};


struct ofp131_switch_features {
    struct ofp_header header;
    uint64_t datapath_id; /* Datapath unique ID. The lower 48-bits are for
                             a MAC address, while the upper 16-bits are
                             implementer-defined. */
    uint32_t n_buffers; /* Max packets buffered at once. */
    uint8_t n_tables; /* Number of tables supported by datapath. */
    uint8_t auxiliary_id; /* Identify auxiliary connections */
    uint8_t pad[2]; /* Align to 64-bits. */
    /* Features. */
    uint32_t capabilities; /* Bitmap of support "ofp_capabilities". */
    uint32_t reserved;
};

struct ofp131_multipart_request {
    struct ofp_header header;
    uint16_t type; /* One of the OFPMP_* constants. */
    uint16_t flags; /* OFPMPF_REQ_* flags. */
    uint8_t pad[4];
    uint8_t body[0]; /* Body of the request. */
};

enum ofp131_multipart_request_flags {
    OFPMPF_REQ_MORE = 1 << 0 /* More requests to follow. */
};

struct ofp131_multipart_reply {
    struct ofp_header header;
    uint16_t type; /* One of the OFPMP_* constants. */
    uint16_t flags; /* OFPMPF_REPLY_* flags. */
    uint8_t pad[4];
    uint8_t body[0]; /* Body of the reply. */
};

enum ofp131_multipart_reply_flags {
    OFPMPF_REPLY_MORE = 1 << 0 /* More replies to follow. */
};

#endif
