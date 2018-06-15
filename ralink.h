#ifndef _RALINK_CONFIG_H
#define _RALINK_CONFIG_H

#include "config.h"
/********************************
 * ralink
 ********************************/
#define MT7603_CONF_PATH "/tmp/etc/Wireless/RT2860AP/RT2860AP.dat"
#define MT7612_CONF_PATH "/tmp/etc/Wireless/RT2860AP/RT2860AP_11ac.dat"
#define MAX_RALINK_INTERFACE_NUM_PER_CONFIG	(4)
#define MAX_RALINK_TX_POWER_DBM		(12)	//this depend on calibration value


enum {
	AutoChannelSelect_Disable = 0,
	AutoChannelSelect_Old_Select_Algorithm = 1,
	AutoChannelSelect_New_Select_Algorithm = 2,
};


enum {
	KeyXType_Hex = 0,
	KeyXType_Ascii = 1,
};


enum {
	WirelessMode_11BG_Mixed = 0,
	WirelessMode_11B_Only = 1,
	WirelessMode_11A_Only = 2,
	WirelessMode_11ABG_Mixed = 3,
	WirelessMode_11G_Only = 4,
	WirelessMode_11ABGN_Mixed = 5,
	WirelessMode_11N_Only_in_24G = 6,
	WirelessMode_11GN_Mixed = 7,
	WirelessMode_11AN_Mixed = 8,
	WirelessMode_11BGN_Mixed = 9,
	WirelessMode_11AGN_Mixed = 10,
	WirelessMode_11N_Only_in_5G = 11,
	WirelessMode_11A_AN_AC_Mixed_5G = 14, //AC chip only
	WirelessMode_11AN_AC_Mixed_5G = 15, //AC chip only
};


enum {
	BasicRate_1 = 1 << 0,
	BasicRate_2 = 1 << 1,
	BasicRate_5_5 = 1 << 2,
	BasicRate_11 = 1 << 3,
	BasicRate_6 = 1 << 4,
	BasicRate_9 = 1 << 5,
	BasicRate_12 = 1 << 6,
	BasicRate_18 = 1 << 7,
	BasicRate_24 = 1 << 8,
	BasicRate_36 = 1 << 9,
	BasicRate_48 = 1 << 10,
	BasicRate_54 = 1 << 11,
};


enum {
	TxPower_91_100_dBm_diff = 0,	//91 ~ 100%
	TxPower_61_90_dBm_diff = -1,	//61 ~ 90%
	TxPower_31_60_dBm_diff = -3,	//31 ~ 60%
	TxPower_16_30_dBm_diff = -6,	//16 ~ 30%
	TxPower_10_15_dBm_diff = -9,	//10 ~ 15%
};


enum {
	HT_BW_20 = 0,
	HT_BW_20_40 = 1,
};


enum {
	HT_EXTCHA_Below = 0,
	HT_EXTCHA_Above = 1,
	HT_EXTCHA_BY_CHANNEL = 99,	//gemtek define
};


enum {
	VHT_BW_Disable = 0,
	VHT_BW_Enable = 1,
};


enum {
	AccessPolicy_Disable = 0,
	AccessPolicy_Allow_all_entries_in_ACL_table = 1, //white list
	AccessPolicy_Reject_all_entries_in_ACL_table = 2, //block list


};


enum {
	WscConfMode_Disable = 0x00,
	WscConfMode_Enrollee = 0x01,
	WscConfMode_Proxy = 0x02,
	WscConfMode_Registrar = 0x04,
};


enum {
	WscConfStatus_unconfigured = 1,
	WscConfStatus_configured = 2,
};


struct ralink_txpower_dBm {
	int txpower_dBm;
	int TxPower; //percentage
	int dBm_diff;
};


struct ralink_encryption {
	char *encryption; //iface_encryption
	char *AuthMode;
	char *EncrypType;
	int IEEE8021X;
};


struct ralink_wireless_mode {
	char *wireless_mode;	//iface_wireless_mode
	char *ap_scanned_mode;
	int WirelessMode;
	int BasicRate;
};


struct ralink_macfilter {
	char *macfilter;
	int  AccessPolicy;
};



struct ralink_htmode {
	char *htmode;
	int HT_BW;
	int HT_EXTCHA;
	int VHT_BW;
};


struct ralink_txpower_dBm* query_ralink_txpower_dBm(int txpower_dBm);
struct ralink_wireless_mode* query_ralink_wireless_mode(char *wireless_mode);
struct ralink_wireless_mode* query_ralink_ap_scanned_mode(char *ap_scanned_mode);
struct ralink_encryption* query_ralink_encryption(char *encryption);
struct ralink_encryption* query_ralink_AuthMode_EncrypType(char *AuthMode, char *EncrypType);
int query_ralink_KeyXType(char *key_wep);
int query_ralink_macfilter(char *macfilter);
int query_ralink_DefaultKeyID(char *encryption, unsigned int key_wep_index);
struct ralink_htmode *query_ralink_htmode(char *htmode);
const char *query_ralink_AutoChannelSkipList_with_band(char *AutoChannelSkipList, unsigned int AutoChannelSkipList_Size,
        unsigned int channel_option_num, unsigned int *channel_option, char *band);

int generate_ralink_conf(struct wifi_setting *setting, struct wifi_radio *radio);


int ralink_restart(int argc, char* argv[]);
int ralink_stop(int argc, char* argv[]);

#endif //_RALINK_CONFIG_H
