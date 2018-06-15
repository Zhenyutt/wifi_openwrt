#ifndef _RALINK_DRIVER_H
#define _RALINK_DRIVER_H

#define IFNAMSIZ	16
#define MAC_ADDR_LENGTH		6
#define MAX_NUMBER_OF_MAC_MT7603	75	// if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#define MAX_NUMBER_OF_MAC_MT7612	116	// if MAX_MBSSID_NUM is 8, this value can't be larger than 211

#define RTPRIV_IOCTL_GET_MAC_TABLE			(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)	// modified by Red@Ralink, 2009/09/30


enum {
	MODE_CCK = 0,
	MODE_OFDM = 1,
	MODE_HTMIX = 2,
	MODE_HTGREENFIELD = 3,
	MODE_VHT = 4,
};

enum {
	BAND_WIDTH_20 = 0,
	BAND_WIDTH_40 = 1,
	BAND_WIDTH_80 = 2,
	BAND_WIDTH_BOTH = 3,
	BAND_WIDTH_10 = 4,
};


typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned int UINT32;
typedef unsigned long ULONG;
typedef signed char CHAR;
typedef signed short SHORT;
typedef signed int INT;
typedef signed long LONG;



typedef struct _ra_handle {
	int sock;
	char ifname[IFNAMSIZ];
} ra_handle;


// MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!!
typedef union _HTTRANSMIT_SETTING {
	struct {
		USHORT MCS: 6;	/* MCS */
		USHORT ldpc: 1;
		USHORT BW: 2;	/* channel bandwidth 20MHz/40/80 MHz */
		USHORT ShortGI: 1;
		USHORT STBC: 1;	/* only support in HT/VHT mode with MCS0~7 */
		USHORT eTxBF: 1;
		USHORT iTxBF: 1;
		USHORT MODE: 3;	/* Use definition MODE_xxx. */
	} field;
	USHORT word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;


typedef struct _RT_802_11_MAC_ENTRY_MT7603 {
	UCHAR ApIdx;
	UCHAR Addr[MAC_ADDR_LENGTH];
	UCHAR Aid;
	UCHAR Psm;					// 0:PWR_ACTIVE, 1:PWR_SAVE
	UCHAR MimoPs;				// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
	UINT32 ConnectedTime;
	HTTRANSMIT_SETTING TxRate;	// MACHTTRANSMIT_SETTING is not matching

	UINT32 LastRxRate;
	SHORT StreamSnr[3];			// BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed
	SHORT SoundingRespSnr[3];	// SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed
	//SHORT TxPER;  			// TX PER over the last second. Percent
	//SHORT reserved;

} RT_802_11_MAC_ENTRY_MT7603 , *PRT_802_11_MAC_ENTRY_MT7603 ;

typedef struct _RT_802_11_MAC_TABLE_MT7603  {
	ULONG Num;
	RT_802_11_MAC_ENTRY_MT7603  Entry[MAX_NUMBER_OF_MAC_MT7603 ];
} RT_802_11_MAC_TABLE_MT7603 , *PRT_802_11_MAC_TABLE_MT7603 ;


typedef struct _RT_802_11_MAC_ENTRY_MT7612 {
	UCHAR ApIdx;
	UCHAR Addr[MAC_ADDR_LENGTH];
	UCHAR Aid;
	UCHAR Psm;					// 0:PWR_ACTIVE, 1:PWR_SAVE
	UCHAR MimoPs;				// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
	UINT32 ConnectedTime;
	HTTRANSMIT_SETTING TxRate;

	UINT32 LastRxRate;
	int StreamSnr[3];			// BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed
	int SoundingRespSnr[3];	// SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed
	//SHORT TxPER;  			// TX PER over the last second. Percent
	//SHORT reserved;

} RT_802_11_MAC_ENTRY_MT7612 , *PRT_802_11_MAC_ENTRY_MT7612 ;

typedef struct _RT_802_11_MAC_TABLE_MT7612  {
	ULONG Num;
	RT_802_11_MAC_ENTRY_MT7612  Entry[MAX_NUMBER_OF_MAC_MT7612 ];
} RT_802_11_MAC_TABLE_MT7612 , *PRT_802_11_MAC_TABLE_MT7612 ;


//function
char* get_phy_mode_mt7603(int Mode);
char* get_phy_mode_mt7612(int Mode);
char* get_BW(int BW);
int get_rate(HTTRANSMIT_SETTING HTSetting);
int get_best_rssi(int rssi0, int rssi1, int rssi2);

#endif
