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

/* Instruction header that is common to all instructions. The length includes
* the header and any padding used to make the instruction 64-bit aligned.
* NB: The length of an instruction *must* always be a multiple of eight. */
struct ofp131_instruction {
    uint16_t type; /* Instruction type */
    uint16_t len; /* Length of this struct in bytes. */
};
OFP_ASSERT(sizeof(struct ofp131_instruction) == 4);

enum ofp131_instruction_type {
    OFPIT_GOTO_TABLE = 1, /* Setup the next table in the lookup
    pipeline */
    OFPIT_WRITE_METADATA = 2, /* Setup the metadata field for use later in
    pipeline */
    OFPIT_WRITE_ACTIONS = 3, /* Write the action(s) onto the datapath action
    set */
    OFPIT_APPLY_ACTIONS = 4, /* Applies the action(s) immediately */
    OFPIT_CLEAR_ACTIONS = 5, /* Clears all actions from the datapath
    action set */
    OFPIT_METER = 6, /* Apply meter (rate limiter) */
    OFPIT_EXPERIMENTER = 0xFFFF /* Experimenter instruction */
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



enum ofp131_table_config {
    OFPTC_DEPRECATED_MASK = 3, /* Deprecated bits */
};



/* Table Feature property types.
* Low order bit cleared indicates a property for a regular Flow Entry.
* Low order bit set indicates a property for the Table-Miss Flow Entry.
*/
enum ofp131_table_feature_prop_type {
    OFPTFPT_INSTRUCTIONS = 0, /* Instructions property. */
    OFPTFPT_INSTRUCTIONS_MISS = 1, /* Instructions for table-miss. */
    OFPTFPT_NEXT_TABLES = 2, /* Next Table property. */
    OFPTFPT_NEXT_TABLES_MISS = 3, /* Next Table for table-miss. */
    OFPTFPT_WRITE_ACTIONS = 4, /* Write Actions property. */
    OFPTFPT_WRITE_ACTIONS_MISS = 5, /* Write Actions for table-miss. */
    OFPTFPT_APPLY_ACTIONS = 6, /* Apply Actions property. */
    OFPTFPT_APPLY_ACTIONS_MISS = 7, /* Apply Actions for table-miss. */
    OFPTFPT_MATCH = 8, /* Match property. */
    OFPTFPT_WILDCARDS = 10, /* Wildcards property. */
    OFPTFPT_WRITE_SETFIELD = 12, /* Write Set-Field property. */
    OFPTFPT_WRITE_SETFIELD_MISS = 13, /* Write Set-Field for table-miss. */
    OFPTFPT_APPLY_SETFIELD = 14, /* Apply Set-Field property. */
    OFPTFPT_APPLY_SETFIELD_MISS = 15, /* Apply Set-Field for table-miss. */
    OFPTFPT_EXPERIMENTER = 0xFFFE, /* Experimenter property. */
    OFPTFPT_EXPERIMENTER_MISS = 0xFFFF, /* Experimenter for table-miss. */
};

/* Common header for all Table Feature Properties */
struct ofp131_table_feature_prop_header {
    uint16_t type; /* One of OFPTFPT_*. */
    uint16_t length; /* Length in bytes of this property. */
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_header) == 4);

/* Instructions property */
struct ofp131_table_feature_prop_instructions {
    uint16_t type; /* One of OFPTFPT_INSTRUCTIONS,
                      OFPTFPT_INSTRUCTIONS_MISS. */
    uint16_t length; /* Length in bytes of this property. */
    /* Followed by:
    * - Exactly (length - 4) bytes containing the instruction ids, then
    * - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
    * bytes of all-zero bytes */
    struct ofp131_instruction instruction_ids[0]; /* List of instructions */
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_instructions) == 4);

/* Next Tables property */
struct ofp131_table_feature_prop_next_tables {
    uint16_t type; /* One of OFPTFPT_NEXT_TABLES,
                      OFPTFPT_NEXT_TABLES_MISS. */
    uint16_t length; /* Length in bytes of this property. */
    /* Followed by:
    * - Exactly (length - 4) bytes containing the table_ids, then
    * - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
    * bytes of all-zero bytes */
    uint8_t next_table_ids[0];
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_next_tables) == 4);

/* Actions property */
struct ofp131_table_feature_prop_actions {
    uint16_t type; /* One of OFPTFPT_WRITE_ACTIONS,
                      OFPTFPT_WRITE_ACTIONS_MISS,
                      OFPTFPT_APPLY_ACTIONS,
                      OFPTFPT_APPLY_ACTIONS_MISS. */
    uint16_t length; /* Length in bytes of this property. */
    /* Followed by:
    * - Exactly (length - 4) bytes containing the action_ids, then
    * - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
    * bytes of all-zero bytes */
    struct ofp_action_header action_ids[0]; /* List of actions */
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_actions) == 4);

/* Match, Wildcard or Set-Field property */
struct ofp131_table_feature_prop_oxm {
    uint16_t type; /* One of OFPTFPT_MATCH,
                    OFPTFPT_WILDCARDS,
                    OFPTFPT_WRITE_SETFIELD,
                    OFPTFPT_WRITE_SETFIELD_MISS,
                    OFPTFPT_APPLY_SETFIELD,
                    OFPTFPT_APPLY_SETFIELD_MISS. */
    uint16_t length; /* Length in bytes of this property. */
    /* Followed by:
    * - Exactly (length - 4) bytes containing the oxm_ids, then
    * - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
    * bytes of all-zero bytes */
    uint32_t oxm_ids[0]; /* Array of OXM headers */
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_oxm) == 4);

/* Experimenter table feature property */
struct ofp131_table_feature_prop_experimenter {
    uint16_t type; /* One of OFPTFPT_EXPERIMENTER,
                      OFPTFPT_EXPERIMENTER_MISS. */
    uint16_t length; /* Length in bytes of this property. */
    uint32_t experimenter; /* Experimenter ID which takes the same
                              form as in struct
                              ofp_experimenter_header. */
    uint32_t exp_type; /* Experimenter defined. */
    /* Followed by:
    * - Exactly (length - 12) bytes containing the experimenter data, then
    * - Exactly (length + 7)/8*8 - (length) (between 0 and 7)
    * bytes of all-zero bytes */
    uint32_t experimenter_data[0];
};
OFP_ASSERT(sizeof(struct ofp131_table_feature_prop_experimenter) == 12);

/* Body for ofp_multipart_request of type OFPMP_TABLE_FEATURES./
* Body of reply to OFPMP_TABLE_FEATURES request. */
#define OFP_MAX_TABLE_NAME_LEN 32
struct ofp131_table_features {
    uint16_t length; /* Length is padded to 64 bits. */
    uint8_t table_id; /* Identifier of table. Lower numbered tables
    are consulted first. */
    uint8_t pad[5]; /* Align to 64-bits. */
    char name[OFP_MAX_TABLE_NAME_LEN];
    uint64_t metadata_match; /* Bits of metadata table can match. */
    uint64_t metadata_write; /* Bits of metadata table can write. */
    uint32_t config; /* Bitmap of OFPTC_* values */
    uint32_t max_entries; /* Max number of entries supported. */
    /* Table Feature Property list */
    struct ofp131_table_feature_prop_header properties[0];
};
OFP_ASSERT(sizeof(struct ofp131_table_features) == 64);




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
    uint16_t type;    /* One of the OFPMP_* constants. */
    uint16_t flags;   /* OFPMPF_REPLY_* flags. */
    uint8_t pad[4];
    uint8_t body[0];  /* Body of the reply. */
};

enum ofp131_multipart_reply_flags {
    OFPMPF_REPLY_MORE = 1 << 0 /* More replies to follow. */
};


/* Controller roles. */
enum ofp131_controller_role {
    OPFCR_ROLE_NOCHANGE = 0,    /* Don't change current role. */
    OPFCR_ROLE_EQUAL    = 1,    /* Default role, full access. */
    OPFCR_ROLE_MASTER   = 2,    /* Full access, at most one master. */
    OPFCR_ROLE_SLAVE    = 3,    /* Read-only access. */
};

/* Role request and reply message. */
struct ofp131_role_request {
    struct ofp_header header; /* Type OFPT_ROLE_REQUEST/OFPT_ROLE_REPLY. */
    uint32_t role;            /* One of NX_ROLE_*. */
    uint8_t pad[4];           /* Align to 64 bits. */
    uint64_t generation_id;   /* Master Election Generation Id */
};
OFP_ASSERT(sizeof(struct ofp131_role_request)==24);
#endif
