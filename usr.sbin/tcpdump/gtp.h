/*	$OpenBSD: gtp.h,v 1.1 2009/11/04 09:43:11 jsing Exp $ */
/*
 * Copyright (c) 2009 Joel Sing <jsing@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __GTP_H_DEFINED
#define __GTP_H_DEFINED

#define GTP_VERSION_0			0
#define GTP_VERSION_1			1
#define GTP_VERSION_2			2

#define GTP_VERSION_MASK		0x07 << 5
#define GTP_VERSION_SHIFT		5

#define GTP_V0_PROTO			0
#define GTP_V0_PRIME_PROTO		1
#define GTP_V1_CTRL_PROTO		2
#define GTP_V1_USER_PROTO		3
#define GTP_V1_PRIME_PROTO		4

#define GTPV0_PORT			3386

#define GTPV1_C_PORT			2123
#define GTPV1_U_PORT			2152
#define GTPV1_PRIME_PORT		3386

#define GTPV0_HDR_PROTO_TYPE		1 << 4
#define GTPV0_HDR_SNN			1 << 0

#define GTPV0_IE_TYPE_MASK		1 << 7
#define GTPV0_IE_VALUE_MASK		~GTPV1_IE_TYPE_MASK

/*
 * GTPv0 Message Types.
 */
#define GTPV0_ECHO_REQUEST			1
#define GTPV0_ECHO_RESPONSE			2
#define GTPV0_VERSION_NOT_SUPPORTED		3
#define GTPV0_NODE_ALIVE_REQUEST		4
#define GTPV0_NODE_ALIVE_RESPONSE		5
#define GTPV0_REDIRECTION_REQUEST		6
#define GTPV0_REDIRECTION_RESPONSE		7
#define GTPV0_CREATE_PDP_CONTEXT_REQUEST	16
#define GTPV0_CREATE_PDP_CONTEXT_RESPONSE	17
#define GTPV0_UPDATE_PDP_CONTEXT_REQUEST	18
#define GTPV0_UPDATE_PDP_CONTEXT_RESPONSE	19
#define GTPV0_DELETE_PDP_CONTEXT_REQUEST	20
#define GTPV0_DELETE_PDP_CONTEXT_RESPONSE	21
#define GTPV0_CREATE_AA_PDP_CONTEXT_REQUEST	22
#define GTPV0_CREATE_AA_PDP_CONTEXT_RESPONSE	23
#define GTPV0_DELETE_AA_PDP_CONTEXT_REQUEST	24
#define GTPV0_DELETE_AA_PDP_CONTEXT_RESPONSE	25
#define GTPV0_ERROR_INDICATION			26
#define GTPV0_PDU_NOTIFICATION_REQUEST		27
#define GTPV0_PDU_NOTIFICATION_RESPONSE		28
#define GTPV0_PDU_NOTIFICATION_REJECT_REQUEST	29
#define GTPV0_PDU_NOTIFICATION_REJECT_RESPONSE	30
#define GTPV0_SEND_ROUTEING_INFO_REQUEST	32
#define GTPV0_SEND_ROUTEING_INFO_RESPONSE	33
#define GTPV0_FAILURE_REPORT_REQUEST		34
#define GTPV0_FAILURE_REPORT_RESPONSE		35
#define GTPV0_MS_GPRS_PRESENT_REQUEST		36
#define GTPV0_MS_GPRS_PRESENT_RESPONSE		37
#define GTPV0_IDENTIFICATION_REQUEST		48
#define GTPV0_IDENTIFICATION_RESPONSE		49
#define GTPV0_SGSN_CONTEXT_REQUEST		50
#define GTPV0_SGSN_CONTEXT_RESPONSE		51
#define GTPV0_SGSN_CONTEXT_ACKNOWLEDGE		52
#define GTPV0_DATA_RECORD_TRANSFER_REQUEST	240
#define GTPV0_DATA_RECORD_TRANSFER_RESPONSE	241
#define GTPV0_T_PDU				255

/*
 * GTPv0 Information Elements.
 */
#define GTPV0_TV_CAUSE				1
#define   GTPV0_TV_CAUSE_LENGTH			2
#define GTPV0_TV_IMSI				2
#define   GTPV0_TV_IMSI_LENGTH			9
#define GTPV0_TV_RAI				3
#define   GTPV0_TV_RAI_LENGTH			7
#define GTPV0_TV_TLLI				4
#define   GTPV0_TV_TLLI_LENGTH			5
#define GTPV0_TV_PTMSI				5
#define   GTPV0_TV_PTMSI_LENGTH			5
#define GTPV0_TV_QOS				6
#define   GTPV0_TV_QOS_LENGTH			4
#define GTPV0_TV_REORDER			8
#define   GTPV0_TV_REORDER_LENGTH		2
#define GTPV0_TV_AUTH_TRIPLET			9
#define   GTPV0_TV_AUTH_TRIPLET_LENGTH		29
#define GTPV0_TV_MAP_CAUSE			11
#define   GTPV0_TV_MAP_CAUSE_LENGTH		2
#define GTPV0_TV_PTMSI_SIGNATURE		12
#define   GTPV0_TV_PTMSI_SIGNATURE_LENGTH	4
#define GTPV0_TV_MS_VALIDATED			13
#define   GTPV0_TV_MS_VALIDATED_LENGTH		2
#define GTPV0_TV_RECOVERY			14
#define   GTPV0_TV_RECOVERY_LENGTH		2
#define GTPV0_TV_SELECTION_MODE			15
#define   GTPV0_TV_SELECTION_MODE_LENGTH	2
#define GTPV0_TV_FLOW_LABEL_DATA_I		16
#define   GTPV0_TV_FLOW_LABEL_DATA_I_LENGTH	3
#define GTPV0_TV_FLOW_LABEL_SIGNALLING		17
#define   GTPV0_TV_FLOW_LABEL_SIGNALLING_LENGTH	3
#define GTPV0_TV_FLOW_LABEL_DATA_II		18
#define   GTPV0_TV_FLOW_LABEL_DATA_II_LENGTH	4
#define GTPV0_TV_PACKET_XFER_CMD		126
#define   GTPV0_TV_PACKET_XFER_CMD_LENGTH	2
#define GTPV0_TV_CHARGING_ID			127
#define   GTPV0_TV_CHARGING_ID_LENGTH		5

#define GTPV0_TLV_END_USER_ADDRESS		128
#define GTPV0_TLV_MM_CONTEXT			129
#define GTPV0_TLV_PDP_CONTEXT			130
#define GTPV0_TLV_ACCESS_POINT_NAME		131
#define GTPV0_TLV_PROTOCOL_CONFIG_OPTIONS	132
#define GTPV0_TLV_GSN_ADDRESS			133
#define GTPV0_TLV_MS_ISDN			134
#define GTPV0_TLV_RELEASED_PACKETS		249
#define GTPV0_TLV_CANCELLED_PACKETS		250
#define GTPV0_TLV_CHARGING_GATEWAY_ADDRESS	251
#define GTPV0_TLV_DATA_RECORD_PACKET		252
#define GTPV0_TLV_REQUESTS_RESPONDED		253
#define GTPV0_TLV_RECOMMENDED_NODE		254
#define GTPV0_TLV_PRIVATE_EXTENSION		255

/*
 * GTP Version 1.
 */

#define GTPV1_HDR_PROTO_TYPE		1 << 4
#define GTPV1_HDR_RSVD			1 << 3
#define GTPV1_HDR_EH_FLAG		1 << 2
#define GTPV1_HDR_SN_FLAG		1 << 1
#define GTPV1_HDR_NPDU_FLAG		1 << 0

/*
 * GTPv1 Extended Headers.
 */
#define GTPV1_HDR_EXT			(GTPV1_HDR_EH_FLAG | \
					 GTPV1_HDR_SN_FLAG | \
					 GTPV1_HDR_NPDU_FLAG)

#define GTPV1_EH_NONE			0x00
#define GTPV1_EH_MBMS_SUPPORT		0x01
#define GTPV1_EH_MSI_CHANGE_RPT		0x02
#define GTPV1_EH_PDCP_PDU_NO		0xc0
#define GTPV1_EH_SUSPEND_REQUEST	0xc1
#define GTPV1_EH_SUSPEND_RESPONSE	0xc2

#define GTPV1_IE_TYPE_MASK		1 << 7
#define GTPV1_IE_VALUE_MASK		~GTPV1_IE_TYPE_MASK

#define GTPV1_CAUSE_VALUE_MASK		3 << 6
#define GTPV1_CAUSE_REQUEST		0x00
#define GTPV1_CAUSE_ACCEPTANCE		0x80
#define GTPV1_CAUSE_REJECTION		0xc0

/*
 * GTPv1 Message Types.
 */
#define GTPV1_ECHO_REQUEST			1
#define GTPV1_ECHO_RESPONSE			2
#define GTPV1_VERSION_NOT_SUPPORTED		3
#define GTPV1_NODE_ALIVE_REQUEST		4
#define GTPV1_NODE_ALIVE_RESPONSE		5
#define GTPV1_REDIRECTION_REQUEST		6
#define GTPV1_REDIRECTION_RESPONSE		7
#define GTPV1_CREATE_PDP_REQUEST		16
#define GTPV1_CREATE_PDP_RESPONSE		17
#define GTPV1_UPDATE_PDP_REQUEST		18
#define GTPV1_UPDATE_PDP_RESPONSE		19
#define GTPV1_DELETE_PDP_REQUEST		20
#define GTPV1_DELETE_PDP_RESPONSE		21
#define GTPV1_INIT_PDP_ACTIVATE_REQUEST		22
#define GTPV1_INIT_PDP_ACTIVATE_RESPONSE	23
#define GTPV1_ERROR_INDICATION			26
#define GTPV1_PDU_NOTIFICATION_REQUEST		27
#define GTPV1_PDU_NOTIFICATION_RESPONSE		28
#define GTPV1_PDU_NOTIFICATION_REJECT_REQUEST	29
#define GTPV1_PDU_NOTIFICATION_REJECT_RESPONSE	30
#define GTPV1_SUPPORT_EXT_HEADER_NOTIFICATION	31
#define GTPV1_SEND_ROUTEING_INFO_REQUEST	32
#define GTPV1_SEND_ROUTEING_INFO_RESPONSE	33
#define GTPV1_FAILURE_REPORT_REQUEST		34
#define GTPV1_FAILURE_REPORT_RESPONSE		35
#define GTPV1_NOTE_MS_GPRS_PRESENT_REQUEST	36
#define GTPV1_NOTE_MS_GPRS_PRESENT_RESPONSE	37
#define GTPV1_IDENTIFICATION_REQUEST		48
#define GTPV1_IDENTIFICATION_RESPONSE		49
#define GTPV1_SGSN_CONTEXT_REQUEST		50
#define GTPV1_SGSN_CONTEXT_RESPONSE		51
#define GTPV1_SGSN_CONTEXT_ACKNOWLEDGE		52
#define GTPV1_FORWARD_RELOCATION_REQUEST	53
#define GTPV1_FORWARD_RELOCATION_RESPONSE	54
#define GTPV1_FORWARD_RELOCATION_COMPLETE	55
#define GTPV1_RELOCATION_CANCEL_REQUEST		56
#define GTPV1_RELOCATION_CANCEL_RESPONSE	57
#define GTPV1_FORWARD_SRNS_CONTEXT		58
#define GTPV1_FORWARD_RELOCATION_COMPLETE_ACK	59
#define GTPV1_FORWARD_SRNS_CONTEXT_ACK		60
#define GTPV1_RAN_INFORMATION_RELAY		70
#define GTPV1_MBMS_NOTIFICATION_REQUEST		96
#define GTPV1_MBMS_NOTIFICATION_RESPONSE	97
#define GTPV1_MBMS_NOTIFICATION_REJECT_REQUEST	98
#define GTPV1_MBMS_NOTIFICATION_REJECT_RESPONSE	99
#define GTPV1_CREATE_MBMS_CONTEXT_REQUEST	100
#define GTPV1_CREATE_MBMS_CONTEXT_RESPONSE	101
#define GTPV1_UPDATE_MBMS_CONTEXT_REQUEST	102
#define GTPV1_UPDATE_MBMS_CONTEXT_RESPONSE	103
#define GTPV1_DELETE_MBMS_CONTEXT_REQUEST	104
#define GTPV1_DELETE_MBMS_CONTEXT_RESPONSE	105
#define GTPV1_MBMS_REGISTRATION_REQUEST		112
#define GTPV1_MBMS_REGISTRATION_RESPONSE	113
#define GTPV1_MBMS_DEREGISTRATION_REQUEST	114
#define GTPV1_MBMS_DEREGISTRATION_RESPONSE	115
#define GTPV1_MBMS_SESSION_START_REQUEST	116
#define GTPV1_MBMS_SESSION_START_RESPONSE	117
#define GTPV1_MBMS_SESSION_STOP_REQUEST		118
#define GTPV1_MBMS_SESSION_STOP_RESPONSE	119
#define GTPV1_MBMS_SESSION_UPDATE_REQUEST	120
#define GTPV1_MBMS_SESSION_UPDATE_RESPONSE	121
#define GTPV1_MS_INFO_CHANGE_REQUEST		128
#define GTPV1_MS_INFO_CHANGE_RESPONSE		129
#define GTPV1_DATA_RECORD_XFER_REQUEST		240
#define GTPV1_DATA_RECORD_XFER_RESPONSE		241
#define GTPV1_G_PDU				255

/*
 * GTPv1 Information Elements.
 */
#define GTPV1_TV_CAUSE				1
#define   GTPV1_TV_CAUSE_LENGTH			2
#define GTPV1_TV_IMSI				2
#define   GTPV1_TV_IMSI_LENGTH			9
#define GTPV1_TV_RAI				3
#define   GTPV1_TV_RAI_LENGTH			7
#define GTPV1_TV_TLLI				4
#define   GTPV1_TV_TLLI_LENGTH			5
#define GTPV1_TV_PTMSI				5
#define   GTPV1_TV_PTMSI_LENGTH			6
#define GTPV1_TV_REORDER			8
#define   GTPV1_TV_REORDER_LENGTH		2
#define GTPV1_TV_AUTH				9
#define   GTPV1_TV_AUTH_LENGTH			29
#define GTPV1_TV_MAP_CAUSE			11
#define   GTPV1_TV_MAP_CAUSE_LENGTH		2
#define GTPV1_TV_PTMSI_SIGNATURE		12
#define   GTPV1_TV_PTMSI_SIGNATURE_LENGTH	4
#define GTPV1_TV_MS_VALIDATED			13
#define   GTPV1_TV_MS_VALIDATED_LENGTH		2
#define GTPV1_TV_RECOVERY			14
#define   GTPV1_TV_RECOVERY_LENGTH		2
#define GTPV1_TV_SELECTION_MODE			15
#define   GTPV1_TV_SELECTION_MODE_LENGTH	2
#define GTPV1_TV_TEID_DATA_I			16
#define   GTPV1_TV_TEID_DATA_I_LENGTH		5
#define GTPV1_TV_TEID_CTRL			17
#define   GTPV1_TV_TEID_CTRL_LENGTH		5
#define GTPV1_TV_TEID_DATA_II			18
#define   GTPV1_TV_TEID_DATA_II_LENGTH		6
#define GTPV1_TV_TEARDOWN			19
#define   GTPV1_TV_TEARDOWN_LENGTH		2
#define GTPV1_TV_NSAPI				20
#define   GTPV1_TV_NSAPI_LENGTH			2
#define GTPV1_TV_RANAP				21
#define   GTPV1_TV_RANAP_LENGTH			2
#define GTPV1_TV_RAB_CONTEXT			22
#define   GTPV1_TV_RAB_CONTEXT_LENGTH		10
#define GTPV1_TV_RADIO_PRIORITY_SMS		23
#define   GTPV1_TV_RADIO_PRI_SMS_LENGTH		2
#define GTPV1_TV_RADIO_PRIORITY			24
#define   GTPV1_TV_RADIO_PRI_LENGTH		2
#define GTPV1_TV_PACKET_FLOW_ID			25
#define   GTPV1_TV_PACKET_FLOW_ID_LENGTH	3
#define GTPV1_TV_CHARGING			26
#define   GTPV1_TV_CHARGING_LENGTH		3
#define GTPV1_TV_TRACE_REFERENCE		27
#define   GTPV1_TV_TRACE_REFERENCE_LENGTH	3
#define GTPV1_TV_TRACE_TYPE			28
#define   GTPV1_TV_TRACE_TYPE_LENGTH		3
#define GTPV1_TV_MSNRR				29
#define   GTPV1_TV_MSNRR_LENGTH			2
#define GTPV1_TV_PACKET_XFER_CMD		126
#define   GTPV1_TV_PACKET_XFER_CMD_LENGTH	2
#define GTPV1_TV_CHARGING_ID			127
#define   GTPV1_TV_CHARGING_ID_LENGTH		5

#define GTPV1_TLV_END_USER_ADDRESS		128
#define GTPV1_TLV_MM_CONTEXT			129
#define GTPV1_TLV_PDP_CONTEXT			130
#define GTPV1_TLV_ACCESS_POINT_NAME		131
#define GTPV1_TLV_PROTOCOL_CONFIG_OPTIONS	132
#define GTPV1_TLV_GSN_ADDRESS			133
#define GTPV1_TLV_MSISDN			134
#define GTPV1_TLV_QOS_PROFILE			135
#define GTPV1_TLV_AUTHENTICATION		136
#define GTPV1_TLV_TRAFFIC_FLOW			137
#define GTPV1_TLV_TARGET_IDENTIFICATION		138
#define GTPV1_TLV_UTRAN_CONTAINER		139
#define GTPV1_TLV_RAB_SETUP_INFORMATION		140
#define GTPV1_TLV_EXT_HEADER_TYPE_LIST		141
#define GTPV1_TLV_TRIGGER_ID			142
#define GTPV1_TLV_OMC_IDENTITY			143
#define GTPV1_TLV_RAN_CONTAINER			144
#define GTPV1_TLV_PDP_CONTEXT_PRIORITIZATION	145
#define GTPV1_TLV_ADDITIONAL_RAB_SETUP_INFO	146
#define GTPV1_TLV_SGSN_NUMBER			147
#define GTPV1_TLV_COMMON_FLAGS			148
#define GTPV1_TLV_APN_RESTRICTION		149
#define GTPV1_TLV_RADIO_PRIORITY_LCS		150
#define GTPV1_TLV_RAT_TYPE			151
#define GTPV1_TLV_USER_LOCATION_INFO		152
#define GTPV1_TLV_MS_TIME_ZONE			153
#define GTPV1_TLV_IMEI_SV			154
#define GTPV1_TLV_CAMEL_CHARGING_CONTAINER	155
#define GTPV1_TLV_MBMS_UE_CONTEXT		156
#define GTPV1_TLV_TMGI				157
#define GTPV1_TLV_RIM_ROUTING_ADDRESS		158
#define GTPV1_TLV_MBMS_PROTOCOL_CONFIG_OPTIONS	159
#define GTPV1_TLV_MBMS_SERVICE_AREA		160
#define GTPV1_TLV_SOURCE_RNC_PDCP_CONTEXT_INFO	161
#define GTPV1_TLV_ADDITIONAL_TRACE_INFO		162
#define GTPV1_TLV_HOP_COUNTER			163
#define GTPV1_TLV_SELECTED_PLMN_ID		164
#define GTPV1_TLV_MBMS_SESSION_IDENTIFIER	165
#define GTPV1_TLV_MBMS_2G_3G_INDICATOR		166
#define GTPV1_TLV_ENHANCED_NSAPI		167
#define GTPV1_TLV_MBMS_SESSION_DURATION		168
#define GTPV1_TLV_ADDITIONAL_MBMS_TRACE_INFO	169
#define GTPV1_TLV_MBMS_SESSION_REPITITION_NO	170
#define GTPV1_TLV_MBMS_TIME_TO_DATA_TRANSFER	171
#define GTPV1_TLV_PS_HANDOVER_REQUEST_CONTEXT	172
#define GTPV1_TLV_BSS_CONTAINER			173
#define GTPV1_TLV_CELL_IDENTIFICATION		174
#define GTPV1_TLV_PDU_NUMBERS			175
#define GTPV1_TLV_BSSGP_CAUSE			176
#define GTPV1_TLV_REQUIRED_MBMS_BEARER_CAP	177
#define GTPV1_TLV_RIM_ROUTING_ADDRESS_DISC	178
#define GTPV1_TLV_LIST_OF_SETUP_PFCS		179
#define GTPV1_TLV_PS_HANDOVER_XID_PARAMETERS	180
#define GTPV1_TLV_MS_INFO_CHANGE_REPORTING	181
#define GTPV1_TLV_DIRECT_TUNNEL_FLAGS		182
#define GTPV1_TLV_CORRELATION_ID		183
#define GTPV1_TLV_BEARER_CONTROL_MODE		184
#define GTPV1_TLV_MBMS_FLOW_IDENTIFIER		185

#define GTPV1_TLV_RELEASED_PACKETS		249
#define GTPV1_TLV_CANCELLED_PACKETS		250
#define GTPV1_TLV_CHARGING_GATEWAY_ADDRESS	251
#define GTPV1_TLV_DATA_RECORD_PACKET		252
#define GTPV1_TLV_REQUESTS_RESPONDED		253
#define GTPV1_TLV_ADDRESS_OF_RECOMMENDED_NODE	254

#define GTPV1_TLV_PRIVATE_EXTENSION		255

struct gtp_v0_hdr {
	u_int8_t flags;
	u_int8_t msgtype;
	u_int16_t length;
	u_int16_t seqno;
	u_int16_t flow;
	u_int8_t npduno;
	u_int8_t spare1;
	u_int8_t spare2;
	u_int8_t spare3;
	u_int64_t tid;
} __packed;

struct gtp_v0_prime_hdr {
	u_int8_t flags;
	u_int8_t msgtype;
	u_int16_t length;
	u_int16_t seqno;
} __packed;

struct gtp_v1_hdr {
	u_int8_t flags;
	u_int8_t msgtype;
	u_int16_t length;
	u_int32_t teid;
} __packed;

struct gtp_v1_hdr_ext {
	struct gtp_v1_hdr gh;
	u_int16_t seqno;
	u_int8_t npduno;
	u_int8_t nexthdr;
} __packed;

struct gtp_v1_prime_hdr {
	u_int8_t flags;
	u_int8_t msgtype;
	u_int16_t length;
	u_int16_t seqno;
} __packed;

static struct tok gtp_type[] = {
        { 0,    "GTPv0" },
        { 1,    "GTPv0'" },
        { 2,    "GTPv1-C" },
        { 3,    "GTPv1-U" },
        { 4,    "GTPv1'" }
};

static const char *gtp_rat_type[] = {
        NULL, "UTRAN", "GERAN", "WLAN", "GAN", "HSPA Evolution"
};

static const char *gtp_packet_xfer_cmd[] = {
	NULL, "Send Data Record", "Send Duplicated Record",
	"Cancel Data Record", "Release Data Record"
};

static const char *mbms_2g3g_indicator[] = {
	"2G Only", "3G Only", "2G and 3G"
};

static const char *ms_info_change_rpt[] = {
	"Stop Reporting", "Start Reporting CGI/SAI", "Start Reporting RAI"
};

#endif
