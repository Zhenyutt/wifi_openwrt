#ifndef _RALINK_WPS_H
#define _RALINK_WPS_H

#define _MKSTR(str)	#str
#define MKSTR(str)	_MKSTR(str)

#define printf(s, args...)          	fprintf(stderr,"wifi_wps: "s, ## args)
#define SET_WPS_LED(st)					CMSSetValue("wifi_led", st, SetValueUser)
#define FEQ_2HZ							"2"
#define FEQ_4HZ							"4"

enum {
	WPS_MODE_STOP = 0,
	WPS_MODE_PIN = 1,
	WPS_MODE_PBC = 2,
};

enum {
	WPS_FAIL = -1,
	WPS_SUCCESS,
	WPS_TIMEOUT,
};

#define LINUX
//#include "rtmp_type.h"
//#include "wsc.h"
//#include "oid.h"

/*
 * wps profile structure
 *
 */
typedef 	unsigned short	USHORT;
typedef 	unsigned char	UCHAR;

#define GNU_PACKED  __attribute__ ((packed))

typedef struct GNU_PACKED _WSC_CONFIGURED_VALUE {
	USHORT WscConfigured; // 1 un-configured; 2 configured
	UCHAR	WscSsid[32 + 1];
	USHORT WscAuthMode;	// mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x20: wpa2-psk
	USHORT	WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
	UCHAR	DefaultKeyIdx;
	UCHAR	WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;

/*
 * private IOCTL for ralink driver
 *
 */
#ifdef LINUX
#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE                              0x8BE0
#define SIOCIWFIRSTPRIV								SIOCDEVPRIVATE
#endif
#endif
#endif // LINUX //


/*
 * OID for ralink driver
 *
 */
#define RT_OID_WSC_QUERY_STATUS						0x0751
#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_WSC_PROFILE                    (SIOCIWFIRSTPRIV + 0x12)


/*
 * Wsc status code
 *
 */

#define	STATUS_WSC_NOTUSED						0
#define	STATUS_WSC_IDLE							1
#define STATUS_WSC_FAIL        			        2		// WSC Process Fail
#define	STATUS_WSC_LINK_UP						3		// Start WSC Process
#define	STATUS_WSC_EAPOL_START_RECEIVED			4		// Received EAPOL-Start
#define	STATUS_WSC_EAP_REQ_ID_SENT				5		// Sending EAP-Req(ID)
#define	STATUS_WSC_EAP_RSP_ID_RECEIVED			6		// Receive EAP-Rsp(ID)
#define	STATUS_WSC_EAP_RSP_WRONG_SMI			7		// Receive EAP-Req with wrong WSC SMI Vendor Id
#define	STATUS_WSC_EAP_RSP_WRONG_VENDOR_TYPE	8		// Receive EAPReq with wrong WSC Vendor Type
#define	STATUS_WSC_EAP_REQ_WSC_START			9		// Sending EAP-Req(WSC_START)
#define	STATUS_WSC_EAP_M1_SENT					10		// Send M1
#define	STATUS_WSC_EAP_M1_RECEIVED				11		// Received M1
#define	STATUS_WSC_EAP_M2_SENT					12		// Send M2
#define	STATUS_WSC_EAP_M2_RECEIVED				13		// Received M2
#define	STATUS_WSC_EAP_M2D_RECEIVED				14		// Received M2D
#define	STATUS_WSC_EAP_M3_SENT					15		// Send M3
#define	STATUS_WSC_EAP_M3_RECEIVED				16		// Received M3
#define	STATUS_WSC_EAP_M4_SENT					17		// Send M4
#define	STATUS_WSC_EAP_M4_RECEIVED				18		// Received M4
#define	STATUS_WSC_EAP_M5_SENT					19		// Send M5
#define	STATUS_WSC_EAP_M5_RECEIVED				20		// Received M5
#define	STATUS_WSC_EAP_M6_SENT					21		// Send M6
#define	STATUS_WSC_EAP_M6_RECEIVED				22		// Received M6
#define	STATUS_WSC_EAP_M7_SENT					23		// Send M7
#define	STATUS_WSC_EAP_M7_RECEIVED				24		// Received M7
#define	STATUS_WSC_EAP_M8_SENT					25		// Send M8
#define	STATUS_WSC_EAP_M8_RECEIVED				26		// Received M8
#define	STATUS_WSC_EAP_RAP_RSP_ACK				27		// Processing EAP Response (ACK)
#define	STATUS_WSC_EAP_RAP_REQ_DONE_SENT		28		// Processing EAP Request (Done)
#define	STATUS_WSC_EAP_RAP_RSP_DONE_SENT		29		// Processing EAP Response (Done)
#define STATUS_WSC_EAP_FAIL_SENT                30      // Sending EAP-Fail
#define STATUS_WSC_ERROR_HASH_FAIL              31      // WSC_ERROR_HASH_FAIL
#define STATUS_WSC_ERROR_HMAC_FAIL              32      // WSC_ERROR_HMAC_FAIL
#define STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL      33      // WSC_ERROR_DEV_PWD_AUTH_FAIL
#define STATUS_WSC_CONFIGURED					34
#define	STATUS_WSC_SCAN_AP						35		// Scanning AP
#define	STATUS_WSC_EAPOL_START_SENT				36
#define STATUS_WSC_EAP_RSP_DONE_SENT			37
#define STATUS_WSC_WAIT_PIN_CODE                38
#define STATUS_WSC_START_ASSOC					39








int ralink_wps(int argc, char *argv[]);


#endif // _RALINK_WPS_H
