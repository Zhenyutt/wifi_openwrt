#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "ralink.h"
#include "config.h"
#include "cms_helper.h"
#include "common.h"


#define CHANNEL_SUPPORTED_AUTO			0
#define CHANNEL_SUPPORTED_2_4MHz	    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
#define CHANNEL_SUPPORTED_5MHz_REGION_1	36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140 //5GHz (Region 1)
#define CHANNEL_SUPPORTED_5MHz_REGION_15	149, 153, 157, 161, 165, 169, 173 //5GHz (Regin 15)


static unsigned int channel_supported_2_4MHz[] = {
	CHANNEL_SUPPORTED_AUTO,
	CHANNEL_SUPPORTED_2_4MHz,
};


static unsigned int channel_supported_5MHz[] = {
	CHANNEL_SUPPORTED_AUTO,
	CHANNEL_SUPPORTED_5MHz_REGION_1,
	CHANNEL_SUPPORTED_5MHz_REGION_15,
};


static unsigned int channel_supported_2_4MHz_5MHz[] = {
	CHANNEL_SUPPORTED_AUTO,
	CHANNEL_SUPPORTED_2_4MHz,
	CHANNEL_SUPPORTED_5MHz_REGION_1,
	CHANNEL_SUPPORTED_5MHz_REGION_15,
};


struct band_channel_supported {
	char *band;
	unsigned int *channel_supported;
	unsigned int channel_supported_size;
};


static struct band_channel_supported band_channel_supported_map[] = {
	{"2.4GHz", channel_supported_2_4MHz, ARRAY_SIZE(channel_supported_2_4MHz) },
	{"5GHz", channel_supported_5MHz, ARRAY_SIZE(channel_supported_5MHz) },
	{"2.4GHz-5GHz-mixed", channel_supported_2_4MHz_5MHz, ARRAY_SIZE(channel_supported_2_4MHz_5MHz) },
};


static struct ralink_encryption ralink_encryption_map[] = {
	//open
	{ "none", "OPEN", "NONE", 0},
	//wep
	{ "wep-open", "OPEN", "WEP", 0},
	{ "wep-shared", "SHARED", "WEP", 0},
	//psk
	{ "psk+tkip", "WPAPSK", "TKIP", 0},
	{ "psk+ccmp", "WPAPSK", "AES", 0},
	{ "psk+tkip+ccmp", "WPAPSK", "TKIPAES", 0},
	//psk2
	{ "psk2+tkip", "WPA2PSK", "TKIP", 0},
	{ "psk2+ccmp", "WPA2PSK", "AES", 0},
	{ "psk2+tkip+ccmp", "WPA2PSK", "TKIPAES", 0},
	//psk-mixed
	{ "psk-mixed+tkip", "WPAPSKWPA2PSK", "TKIP", 0},
	{ "psk-mixed+ccmp", "WPAPSKWPA2PSK", "AES", 0},
	{ "psk-mixed+tkip+ccmp", "WPAPSKWPA2PSK", "TKIPAES", 0},
	//psk-mixed for WPA"1"PSK
	{ "psk-mixed+tkip", "WPA1PSKWPA2PSK", "TKIP", 0},
	{ "psk-mixed+ccmp", "WPA1PSKWPA2PSK", "AES", 0},
	{ "psk-mixed+tkip+ccmp", "WPA1PSKWPA2PSK", "TKIPAES", 0},


	//non-psk
	//psk
	{ "wpa+tkip", "WPA", "TKIP", 0},
	{ "wpa+ccmp", "WPA", "AES", 0},
	{ "wpa+tkip+ccmp", "WPA", "TKIPAES", 0},
	//wpa2
	{ "wpa2+tkip", "WPA2", "TKIP", 0},
	{ "wpa2+ccmp", "WPA2", "AES", 0},
	{ "wpa2+tkip+ccmp", "WPA2", "TKIPAES", 0},
	//wpa-mixed
	{ "wpa-mixed+tkip", "WPA1WPA2", "TKIP", 0},
	{ "wpa-mixed+ccmp", "WPA1WPA2", "AES", 0},
	{ "wpa-mixed+tkip+ccmp", "WPA1WPA2", "TKIPAES", 0},

};


static struct ralink_wireless_mode ralink_wireless_mode_map[] = {
	{ "B/G/N",	"11b/g/n", 						WirelessMode_11BGN_Mixed,		BasicRate_1 | BasicRate_2 | BasicRate_5_5 | BasicRate_11 },	//default value
	{ "B",		"11b", 	 						WirelessMode_11B_Only,			BasicRate_1 | BasicRate_2},
	{ "G",		"11g"/*non driver spec*/, 		WirelessMode_11G_Only,			BasicRate_1 | BasicRate_2 | BasicRate_5_5 | BasicRate_11 | BasicRate_6 | BasicRate_12 | BasicRate_24 },
	{ "N", 		"11n"/*non driver spec*/, 		WirelessMode_11N_Only_in_24G,	BasicRate_1 | BasicRate_2 | BasicRate_5_5 | BasicRate_11 },
	{ "B/G", 	"11b/g", 						WirelessMode_11BG_Mixed,		BasicRate_1 | BasicRate_2 | BasicRate_5_5 | BasicRate_11 },
	{ "G/N", 	"11g/n"/*non driver spec*/, 	WirelessMode_11GN_Mixed,		BasicRate_1 | BasicRate_2 | BasicRate_5_5 | BasicRate_11 },
	{ "A", 		"11a", 							WirelessMode_11A_Only, 			BasicRate_6 | BasicRate_12 | BasicRate_24 },
	{ "AN", 	"11an", 						WirelessMode_11N_Only_in_5G,	BasicRate_6 | BasicRate_12 | BasicRate_24 },
	{ "A/AN",	"11a/n"/*non driver spec*/, 	WirelessMode_11AN_Mixed,		BasicRate_6 | BasicRate_12 | BasicRate_24 },
	{ "AN/AC",	"11an/ac"/*non driver spec*/,	WirelessMode_11AN_AC_Mixed_5G,	BasicRate_6 | BasicRate_12 | BasicRate_24 },
	{ "A/AN/AC", "11a/an/ac"/*non driver spec*/, WirelessMode_11A_AN_AC_Mixed_5G, BasicRate_6 | BasicRate_12 | BasicRate_24 },
};



static struct ralink_htmode ralink_htmode_map[] = {
	{ "HT20", 	HT_BW_20, 		HT_EXTCHA_Below		, VHT_BW_Disable },
	{ "HT40",	HT_BW_20_40,	HT_EXTCHA_BY_CHANNEL, VHT_BW_Disable },
	{ "HT40-", 	HT_BW_20_40,	HT_EXTCHA_Below		, VHT_BW_Disable },
	{ "HT40+", 	HT_BW_20_40, 	HT_EXTCHA_Above		, VHT_BW_Disable },
	{ "HT80", 	HT_BW_20_40, 	HT_EXTCHA_Below		, VHT_BW_Enable },
};


static struct ralink_macfilter ralink_macfilter_map[] = {
	{ "disabled", AccessPolicy_Disable },
	{ "allow", AccessPolicy_Allow_all_entries_in_ACL_table },
	{ "deny", AccessPolicy_Reject_all_entries_in_ACL_table },
};


static struct ralink_txpower_dBm ralink_txpower_dBm_map[] = {
	{MAX_RALINK_TX_POWER_DBM + TxPower_91_100_dBm_diff, 100, TxPower_91_100_dBm_diff},
	{MAX_RALINK_TX_POWER_DBM + TxPower_61_90_dBm_diff,   75, TxPower_61_90_dBm_diff},
	{MAX_RALINK_TX_POWER_DBM + TxPower_31_60_dBm_diff,   50, TxPower_31_60_dBm_diff},
	{MAX_RALINK_TX_POWER_DBM + TxPower_16_30_dBm_diff,   25, TxPower_16_30_dBm_diff},
	{MAX_RALINK_TX_POWER_DBM + TxPower_10_15_dBm_diff,   12, TxPower_10_15_dBm_diff},
};


struct ralink_txpower_dBm* query_ralink_txpower_dBm(int txpower_dBm)
{
	struct ralink_txpower_dBm *p = NULL;
	array_for_each_entry(p, ralink_txpower_dBm_map) {
		if(txpower_dBm == p->txpower_dBm) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match txpower_dBm(%d), use defult(%d)\n", __FUNCTION__, txpower_dBm, ralink_txpower_dBm_map->txpower_dBm);
	return ralink_txpower_dBm_map;
}


struct ralink_wireless_mode* query_ralink_wireless_mode(char *wireless_mode)
{
	if(wireless_mode == NULL) {
		return ralink_wireless_mode_map;
	}

	struct ralink_wireless_mode *p = NULL;
	array_for_each_entry(p, ralink_wireless_mode_map) {
		if(strcmp(wireless_mode, p->wireless_mode) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match wireless_mode(%s), use defult(%s)\n", __FUNCTION__, wireless_mode, ralink_wireless_mode_map->wireless_mode);
	return ralink_wireless_mode_map;
}


struct ralink_wireless_mode* query_ralink_ap_scanned_mode(char *ap_scanned_mode)
{
	if(ap_scanned_mode == NULL) {
		return ralink_wireless_mode_map;
	}

	struct ralink_wireless_mode *p = NULL;
	array_for_each_entry(p, ralink_wireless_mode_map) {
		if(strcmp(ap_scanned_mode, p->ap_scanned_mode) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match ap_scanned_mode(%s), use defult(%s)\n", __FUNCTION__, ap_scanned_mode, ralink_wireless_mode_map->wireless_mode);
	return ralink_wireless_mode_map;
}


struct ralink_encryption* query_ralink_encryption(char *encryption)
{
	if(encryption == NULL) {
		return ralink_encryption_map;
	}

	struct ralink_encryption *p = NULL;
	array_for_each_entry(p, ralink_encryption_map) {
		if(strcmp(encryption, p->encryption) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match encryption(%s), use defult(%s)\n", __FUNCTION__, encryption, ralink_encryption_map->encryption);
	return ralink_encryption_map;
}


struct ralink_encryption* query_ralink_AuthMode_EncrypType(char *AuthMode, char *EncrypType)
{
	if(AuthMode == NULL || EncrypType == NULL) {
		return NULL;
	}

	struct ralink_encryption *p = NULL;
	array_for_each_entry(p, ralink_encryption_map) {
		if(strcmp(AuthMode, p->AuthMode) == 0 && strcmp(EncrypType, p->EncrypType) == 0) {
			return p;
		}
	}

	return NULL;
}


int query_ralink_KeyXType(char *key_wep)
{
	if(strlen(key_wep) == 5 || strlen(key_wep) == 13) {
		return KeyXType_Ascii;
	} else if(strlen(key_wep) == 10 || strlen(key_wep) == 26) {
		return KeyXType_Hex;
	}

	fprintf(stderr, "%s(): Error: incorrect key_wep: %s\n", __FUNCTION__, key_wep);
	return KeyXType_Ascii;
}


int query_ralink_macfilter(char *macfilter)
{
	if(macfilter == NULL) {
		return ralink_macfilter_map->AccessPolicy;
	}

	struct ralink_macfilter *p = NULL;
	array_for_each_entry(p, ralink_macfilter_map) {
		if(strcmp(macfilter, p->macfilter) == 0) {
			return p->AccessPolicy;
		}
	}

	fprintf(stderr, "%s(): Error: un-match macfilter(%s), use defult(%s)\n", __FUNCTION__, macfilter, ralink_macfilter_map->macfilter);
	return ralink_macfilter_map->AccessPolicy;
}


int query_ralink_DefaultKeyID(char *encryption, unsigned int key_wep_index)
{
	if(encryption_is_wep(encryption)) {
		return key_wep_index;//1~4
	} else if(encryption_is_psk(encryption)) {
		return 2;//ralink specific setting
	}
	return 0;//ralink specific setting
}


struct ralink_htmode *query_ralink_htmode(char *htmode)
{
	if(htmode == NULL) {
		return ralink_htmode_map;
	}

	struct ralink_htmode *p = NULL;
	array_for_each_entry(p, ralink_htmode_map) {
		if(strcmp(htmode, p->htmode) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match htmode(%s), use defult(%s)\n", __FUNCTION__, htmode, ralink_htmode_map->htmode);
	return ralink_htmode_map;
}


const char *query_ralink_AutoChannelSkipList_with_band(char *AutoChannelSkipList, unsigned int AutoChannelSkipList_Size, unsigned int channel_option_num, unsigned int *channel_option, char *band)
{
	int len = 0;

	struct band_channel_supported *pstruct = NULL;

	array_for_each_entry(pstruct, band_channel_supported_map) {
		if(strcmp(pstruct->band, band) == 0) {
			break; //found
		}
	}

	if(pstruct >= band_channel_supported_map + ARRAY_SIZE(band_channel_supported_map)) {
		//not found, reset pstruct to HEAD
		pstruct = band_channel_supported_map;
	}

	unsigned int *p = 0;
	array_for_each_entry_size(p, pstruct->channel_supported, pstruct->channel_supported_size) {
		int match = 0;
		int option_idx = 0;
		for(option_idx = 0; option_idx < channel_option_num; option_idx++) {
			if(*p == channel_option[option_idx]) {
				match = 1;
				break;	// not a skip channel
			}
		}
		if(match == 0) {
			//support channel is not in option, add it to skip list
			len += snprintf(AutoChannelSkipList + len, AutoChannelSkipList_Size - len, "%d;", *p);
			//fprintf(stderr, "skip channel=%d\n",channel_supported[supported_idx]);
		}
	}

	//prevent auto channel would use 12,13,14
	len += snprintf(AutoChannelSkipList + len, AutoChannelSkipList_Size - len, "12;13;14;");

	return AutoChannelSkipList;
}


int generate_ralink_conf(struct wifi_setting *setting, struct wifi_radio *radio)
{
	int i = 0;
	char keybuff[1024] = {0};	//use for format_string only

	unsigned int iface_num = radio->iface_num;
	struct wifi_iface **iface = radio->iface;

	FILE *fp = init_conf(radio->config_path);
	set_conf_line(fp, "Default"); //must be the first line of set_conf

	//WirelessMode, BasicRate
	struct ralink_wireless_mode *ra_wmode = query_ralink_wireless_mode(radio->wireless_mode);

	//Channel
	unsigned int Channel = radio->channel;

	//AutoChannelSelect
	unsigned int AutoChannelSelect = AutoChannelSelect_Disable;
	if(Channel == 0) {
		//Channel = 1;	//make ralink driver happy
		AutoChannelSelect = AutoChannelSelect_New_Select_Algorithm;
	}
	//AutoChannelSkipList
	char AutoChannelSkipList[MAX_CHANNEL_OPTION_NUM * 4] = {0};
	query_ralink_AutoChannelSkipList_with_band(AutoChannelSkipList, sizeof(AutoChannelSkipList), radio->channel_option_num, radio->channel_option, radio->band);

	//HT_BW, HT_EXTCHA
	struct ralink_htmode *ra_htmode = query_ralink_htmode(radio->htmode);
	if(ra_htmode->HT_EXTCHA == HT_EXTCHA_BY_CHANNEL) {
		if(1 <= Channel && Channel <= 7) {
			ra_htmode->HT_EXTCHA = HT_EXTCHA_Above;
		} else {
			ra_htmode->HT_EXTCHA = HT_EXTCHA_Below;
		}
	}

	//TxPower
	struct ralink_txpower_dBm *tx_dBm = query_ralink_txpower_dBm(radio->txpower_dBm);

	char MaxStaNum[64] = {0};
	char NoForwarding[64] = {0};
	char HideSSID[64] = {0};
	char WmmCapable[64] = {0};

	char IEEE8021X[64] = {0};
	char AuthMode[64] = {0};
	char EncrypType[64] = {0};
	char DefaultKeyID[64] = {0};
	char Key1Type[64] = {0};
	char Key2Type[64] = {0};
	char Key3Type[64] = {0};
	char Key4Type[64] = {0};

	char RADIUS_Server[64] = {0};
	char RADIUS_Port[64] = {0};
	char RADIUS_Key[132] = {0};


	/********************************************
	 * multiple config format: key=value1;value2;
	 ********************************************/
	for(i = 0; i < iface_num; i++) {
		//MaxStaNum
		snprintf(MaxStaNum + strlen(MaxStaNum), sizeof(MaxStaNum) - strlen(MaxStaNum), "%d;", iface[i]->sta_maxnum);
		//NoForwarding
		snprintf(NoForwarding + strlen(NoForwarding), sizeof(NoForwarding) - strlen(NoForwarding), "%d;", iface[i]->isolate_sta);
		//HideSSID
		snprintf(HideSSID + strlen(HideSSID), sizeof(HideSSID) - strlen(HideSSID), "%d;", iface[i]->hidden);
		//WmmCapable
		snprintf(WmmCapable + strlen(WmmCapable), sizeof(WmmCapable) - strlen(WmmCapable), "%d;", iface[i]->wmm);

		struct ralink_encryption *ra_enc = query_ralink_encryption(iface[i]->encryption);
		//IEEE8021X
		snprintf(IEEE8021X + strlen(IEEE8021X), sizeof(IEEE8021X) - strlen(IEEE8021X), "%d;", ra_enc->IEEE8021X);
		//AuthMode
		snprintf(AuthMode + strlen(AuthMode), sizeof(AuthMode) - strlen(AuthMode), "%s;", ra_enc->AuthMode);
		//EncrypType
		snprintf(EncrypType + strlen(EncrypType), sizeof(EncrypType) - strlen(EncrypType), "%s;", ra_enc->EncrypType);

		//DefaultKeyID
		int default_key_id = query_ralink_DefaultKeyID(iface[i]->encryption, iface[i]->key_wep_index);
		snprintf(DefaultKeyID + strlen(DefaultKeyID), sizeof(DefaultKeyID) - strlen(DefaultKeyID), "%d;", default_key_id);

		int keyXtype = 0;
		if(encryption_is_wep(iface[i]->encryption)) {
			keyXtype = query_ralink_KeyXType(iface[i]->key_wep);
		}
		//Key1Type
		snprintf(Key1Type + strlen(Key1Type), sizeof(Key1Type) - strlen(Key1Type), "%d;", keyXtype);
		//Key2Type
		snprintf(Key2Type + strlen(Key2Type), sizeof(Key2Type) - strlen(Key2Type), "%d;", keyXtype);
		//Key3Type
		snprintf(Key3Type + strlen(Key3Type), sizeof(Key3Type) - strlen(Key3Type), "%d;", keyXtype);
		//Key4Type
		snprintf(Key4Type + strlen(Key4Type), sizeof(Key4Type) - strlen(Key4Type), "%d;", keyXtype);

		//RadiusServer
		snprintf(RADIUS_Server + strlen(RADIUS_Server), sizeof(RADIUS_Server) - strlen(RADIUS_Server), "%s;", iface[i]->radius_server_ipaddr);
		snprintf(RADIUS_Port + strlen(RADIUS_Port), sizeof(RADIUS_Port) - strlen(RADIUS_Port), "%d;", iface[i]->radius_server_port);
		snprintf(RADIUS_Key + strlen(RADIUS_Key), sizeof(RADIUS_Key) - strlen(RADIUS_Key), "%s;", iface[i]->radius_server_key);
	}

	//WscConfMode
	unsigned int WscConfMode = cms_get_int("WscConfMode", (WscConfMode_Proxy | WscConfMode_Registrar));
	if(iface[0]->wps == 0 || iface[0]->wps_not_support == 1) {
		WscConfMode = WscConfMode_Disable;
	}

	//WscConfStatus
	unsigned int WscConfStatus = WscConfStatus_configured;
	if(iface[0]->wps_configurated == 0) {
		WscConfStatus = WscConfStatus_unconfigured;
	}

	set_conf(fp, "MacAddress", iface[0]->macaddr);
	set_conf_int(fp, "CountryRegion", cms_get_int("CountryRegion", 5));
	set_conf_int(fp, "CountryRegionABand", cms_get_int("CountryRegionABand", 7));
	set_conf(fp, "CountryCode", radio->country);
	set_conf_int(fp, "BssidNum", MAX_RALINK_INTERFACE_NUM_PER_CONFIG);
	//set_conf(fp, "SSID1", "");	//set later
	//set_conf(fp, "SSID2", "");	//set later
	//set_conf(fp, "SSID3", "");	//set later
	//set_conf(fp, "SSID4", "");	//set later
	set_conf_int(fp, "WirelessMode", ra_wmode->WirelessMode);
	set_conf(fp, "TxRate", "0");
	set_conf_int(fp, "Channel", Channel);
	set_conf_int(fp, "BasicRate", ra_wmode->BasicRate);
	set_conf_int(fp, "BeaconPeriod", radio->beacon_interval);
	set_conf_int(fp, "DtimPeriod", iface[0]->dtim_period);
	set_conf_int(fp, "TxPower", tx_dBm->TxPower);
	set_conf(fp, "DisableOLBC", "0");
	set_conf(fp, "BGProtection", "0");
	set_conf(fp, "MaxStaNum", MaxStaNum);
	set_conf(fp, "TxPreamble", "1");
	set_conf_int(fp, "RTSThreshold", radio->rts_threshold);
	set_conf_int(fp, "FragThreshold", radio->frag_threshold);
	set_conf(fp, "TxBurst", "1");
	set_conf(fp, "PktAggregate", "1");
	set_conf(fp, "TurboRate", "0");
	set_conf(fp, "WmmCapable", WmmCapable);
	set_conf(fp, "APSDCapable", "0");
	set_conf(fp, "DLSCapable", "0");
	set_conf(fp, "APAifsn", "3;7;1;1");
	set_conf(fp, "APCwmin", "4;4;3;2");
	set_conf(fp, "APCwmax", "6;10;4;3");
	set_conf(fp, "APTxop", "0;0;94;47");
	set_conf(fp, "APACM", "0;0;0;0");
	set_conf(fp, "BSSAifsn", "3;7;2;2");
	set_conf(fp, "BSSCwmin", "4;4;3;2");
	set_conf(fp, "BSSCwmax", "10;10;4;3");
	set_conf(fp, "BSSTxop", "0;0;94;47");
	set_conf(fp, "BSSACM", "0;0;0;0");
	set_conf(fp, "AckPolicy", "0;0;0;0");
	set_conf(fp, "NoForwarding", NoForwarding);
	set_conf_int(fp, "NoForwardingBTNBSSID", radio->isolate_iface);
	set_conf(fp, "HideSSID", HideSSID);
	set_conf_int(fp, "StationKeepAlive", radio->keep_alive_second);
	set_conf_int(fp, "IdleTimeout", radio->max_inactivity_second); //range 60~ , ref: MAC_TABLE_MIN_AGEOUT_TIME=60, default is MAC_TABLE_AGEOUT_TIME=300
	set_conf(fp, "ShortSlot", "1");
	set_conf_int(fp, "AutoChannelSelect", AutoChannelSelect);
	set_conf(fp, "AutoChannelSkipList", AutoChannelSkipList);
	set_conf(fp, "IEEE8021X", IEEE8021X);
	set_conf(fp, "IEEE80211H", "0");
	set_conf(fp, "CSPeriod", "10");
	set_conf(fp, "WirelessEvent", "1");
	set_conf(fp, "IdsEnable", "0");
	set_conf(fp, "AuthFloodThreshold", "32");
	set_conf(fp, "AssocReqFloodThreshold", "32");
	set_conf(fp, "ReassocReqFloodThreshold", "32");
	set_conf(fp, "ProbeReqFloodThreshold", "32");
	set_conf(fp, "DisassocFloodThreshold", "32");
	set_conf(fp, "DeauthFloodThreshold", "32");
	set_conf(fp, "EapReqFooldThreshold", "32");
	set_conf(fp, "PreAuth", "0");
	set_conf(fp, "AuthMode", AuthMode);
	set_conf(fp, "EncrypType", EncrypType);
	set_conf(fp, "RekeyInterval", "3600");
	set_conf(fp, "RekeyMethod", "TIME");
	set_conf(fp, "PMKCachePeriod", "10");
	//set_conf(fp, "WPAPSK1", "");	//set later
	//set_conf(fp, "WPAPSK2", "");	//set later
	//set_conf(fp, "WPAPSK3", "");	//set later
	//set_conf(fp, "WPAPSK4", "");	//set later
	set_conf(fp, "DefaultKeyID", DefaultKeyID);
	set_conf(fp, "Key1Type", Key1Type);
	//set_conf(fp, "Key1Str1", "");	//set later
	//set_conf(fp, "Key1Str2", "");	//set later
	//set_conf(fp, "Key1Str3", "");	//set later
	//set_conf(fp, "Key1Str4", "");	//set later
	set_conf(fp, "Key2Type", Key2Type);
	//set_conf(fp, "Key2Str1", "");	//set later
	//set_conf(fp, "Key2Str2", "");	//set later
	//set_conf(fp, "Key2Str3", "");	//set later
	//set_conf(fp, "Key2Str4", "");	//set later
	set_conf(fp, "Key3Type", Key3Type);
	//set_conf(fp, "Key3Str1", "");	//set later
	//set_conf(fp, "Key3Str2", "");	//set later
	//set_conf(fp, "Key3Str3", "");	//set later
	//set_conf(fp, "Key3Str4", "");	//set later
	set_conf(fp, "Key4Type", Key4Type);
	//set_conf(fp, "Key4Str1", "");	//set later
	//set_conf(fp, "Key4Str2", "");	//set later
	//set_conf(fp, "Key4Str3", "");	//set later
	//set_conf(fp, "Key4Str4", "");	//set later
	set_conf(fp, "HSCounter", "0");
	//set_conf(fp, "AccessPolicy0", "0");		//set later
	//set_conf(fp, "AccessControlList0", "");	//set later
	//set_conf(fp, "AccessPolicy1", "0");		//set later
	//set_conf(fp, "AccessControlList1", "");	//set later
	//set_conf(fp, "AccessPolicy2", "0");		//set later
	//set_conf(fp, "AccessControlList2", "");	//set later
	//set_conf(fp, "AccessPolicy3", "0");		//set later
	//set_conf(fp, "AccessControlList3", "");	//set later
	set_conf(fp, "WdsEnable", "0");
	set_conf(fp, "WdsEncrypType", "NONE");
	set_conf(fp, "WdsList", "");
	set_conf(fp, "WdsKey", "");
	set_conf(fp, "RADIUS_Server", RADIUS_Server);
	set_conf(fp, "RADIUS_Port", RADIUS_Port);
	set_conf(fp, "RADIUS_Key", RADIUS_Key);
	set_conf(fp, "own_ip_addr", cms_get_str("wan_ip_1", "")); //use wan ip directly
	set_conf(fp, "NasId1", "");
	set_conf(fp, "NasId2", "");
	set_conf(fp, "NasId3", "");
	set_conf(fp, "NasId4", "");
	set_conf(fp, "EAPifname", "br0");
	set_conf(fp, "PreAuthifname", "br0");
	set_conf(fp, "HT_HTC", "1");
	set_conf(fp, "HT_RDG", "1");
	set_conf_int(fp, "HT_EXTCHA", ra_htmode->HT_EXTCHA);
	set_conf(fp, "HT_LinkAdapt", "0");
	set_conf(fp, "HT_OpMode", "0");
	set_conf(fp, "HT_MpduDensity", "5");
	set_conf_int(fp, "HT_BW", ra_htmode->HT_BW);
	set_conf(fp, "HT_AutoBA", "1");
	set_conf(fp, "HT_BADecline", "0");
	set_conf(fp, "HT_AMSDU", "0");
	set_conf(fp, "HT_BAWinSize", "64");
	set_conf(fp, "HT_GI", "1");
	set_conf(fp, "HT_LDPC", "1");
	set_conf(fp, "HT_STBC", "1");
	set_conf(fp, "HT_MCS", "33");
	set_conf(fp, "HT_PROTECT", "1");
	set_conf(fp, "HT_TxStream", "2");
	set_conf(fp, "HT_RxStream", "2");
	set_conf_int(fp, "HT_DisallowTKIP", radio->disallow_tkip_ht_rates);	//Must enable(1) to pass 802.11n test plan
	set_conf_int(fp, "HT_BSSCoexistence", radio->coexistence_20_40_mhz); //Must enable(1) to pass 802.11n test plan
	set_conf_int(fp, "VHT_BW", ra_htmode->VHT_BW);
	set_conf(fp, "VHT_SGI", "1");
	set_conf(fp, "VHT_STBC", "1");
	set_conf(fp, "VHT_BW_SIGNAL", "0");
	set_conf(fp, "VHT_DisallowNonVHT", "0");
	set_conf(fp, "VHT_LDPC", "1");
	set_conf_int(fp, "EDCCA_AP_STA_TH", cms_get_int("EDCCA_AP_STA_TH", 1));
	set_conf_int(fp, "EDCCA_AP_AP_TH", cms_get_int("EDCCA_AP_AP_TH", 1));
	set_conf_int(fp, "EDCCA_AP_RSSI_TH", cms_get_int("EDCCA_AP_RSSI_TH", -80));
	set_conf_int(fp, "EDCCA_ED_TH", cms_get_int("EDCCA_ED_TH", 90));
	set_conf_int(fp, "EDCCA_FALSE_CCA_TH", cms_get_int("EDCCA_FALSE_CCA_TH", 180));
	set_conf_int(fp, "EDCCA_BLOCK_CHECK_TH", cms_get_int("EDCCA_BLOCK_CHECK_TH", 2));
	set_conf_int(fp, "ED_MODE", cms_get_int("ED_MODE", 0)); //let country region to turn on
	set_conf_int(fp, "ED_LEARN_TH", cms_get_int("ED_LEARN_TH", 50));
	set_conf(fp, "MeshId", "MESH");
	set_conf(fp, "MeshAutoLink", "1");
	set_conf(fp, "MeshAuthMode", "OPEN");
	set_conf(fp, "MeshEncrypType", "NONE");
	set_conf(fp, "MeshWPAKEY", "");
	set_conf(fp, "MeshDefaultkey", "1");
	set_conf(fp, "MeshWEPKEY", "");
	set_conf_int(fp, "WscConfMode", WscConfMode);
	set_conf_int(fp, "WscConfStatus", WscConfStatus);
	set_conf(fp, "WscManufacturer", iface[0]->wps_manufacturer);
	set_conf(fp, "WscModelName", iface[0]->wps_model_name);
	set_conf(fp, "WscDeviceName", iface[0]->wps_device_name);
	set_conf(fp, "WscModelNumber", iface[0]->wps_model_number);
	set_conf(fp, "WscSerialNumber", iface[0]->wps_serial_number);
	set_conf(fp, "RadioOn", "1");
	set_conf(fp, "PMFMFPC", "0");
	set_conf(fp, "PMFMFPR", "0");
	set_conf(fp, "PMFSHA256", "0");
	set_conf(fp, "LoadCodeMethod", "0");
	set_conf(fp, "E2pAccessMode", "2");
	set_conf(fp, "SCSEnable", "1"); //Smart Carrier Sense

	/********************************************
	 * multiple config format:
	 *                         key1=value
	 *                         key2=value
	 *                         ...
	 *
	 ********************************************/
	set_conf_line(fp, "#### multi-line setting ####");
	for(i = 0; i < iface_num; i++) {
		//SSID
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "SSID%d", i + 1), iface[i]->ssid);

		//WPAPSK
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "WPAPSK%d", i + 1), iface[i]->key_psk);

		//Key1Str
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "Key1Str%d", i + 1), iface[i]->key_wep);
		//Key2Str
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "Key2Str%d", i + 1), iface[i]->key_wep);
		//Key3Str
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "Key3Str%d", i + 1), iface[i]->key_wep);
		//Key4Str
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "Key4Str%d", i + 1), iface[i]->key_wep);

		//AccessPolicy
		set_conf_int(fp, format_string(keybuff, sizeof(keybuff), "AccessPolicy%d", i), query_ralink_macfilter(iface[i]->macfilter));
		//AccessControlList
		char AccessControlList[MAX_MACSTR_SIZE * MAX_STA_NUM] = {0};
		string_replace_to_buffer(iface[i]->macfilter_list, ",", ";", 0, AccessControlList, sizeof(AccessControlList));
		set_conf(fp, format_string(keybuff, sizeof(keybuff), "AccessControlList%d", i), AccessControlList);
	}

	close_conf(fp);

	return 0;
}

int restart_ralink_iface(struct wifi_setting *setting, struct wifi_radio *radio)
{
	int i = 0;

	unsigned int iface_num = radio->iface_num;
	struct wifi_iface **iface = radio->iface;


	// change iface, must be restart all iface of radio
	for(i = 0; i < iface_num; i++) {
		if(iface[i]->changed == 1) {
			iface[i]->radio->changed = 1;
		}
	}

	//stop first
	for(i = 0; i < iface_num; i++) {
		if(setting->changed == 0 && iface[i]->changed == 0 && iface[i]->radio->changed == 0) {
			continue; //nothing changed, skip to restart this interface
		}

		do_system("ifconfig %s down", iface[i]->ifname);
	}

	//then start
	for(i = 0; i < iface_num; i++) {
		if(setting->changed == 0 && iface[i]->changed == 0 && iface[i]->radio->changed == 0) {
			continue; //nothing changed, skip to restart this interface
		}

		if(setting->supported == 1 && setting->enable == 1 && iface[i]->radio->enable == 1 && iface[i]->enable == 1) {
			do_system("ifconfig %s up", iface[i]->ifname);

			if(strlen(iface[i]->bridge) > 0) {
				do_system("brctl addif %s %s >/dev/null 2>&1", iface[i]->bridge, iface[i]->ifname);
			}

			cms_set_str("wifi_iface%d_status", "ready", iface[i]->index);
		} else {
			cms_set_str("wifi_iface%d_status", "stop", iface[i]->index);
		}
	}

	// turn on/off radius auth daemon
	if(radio->authenticator_enable == 1) {
		do_system("pkill %s 2>/dev/null", radio->authenticator_daemon);
		do_system("%s &", radio->authenticator_daemon);
	} else {
		do_system("pkill %s 2>/dev/null", radio->authenticator_daemon);
	}

	// restart firewall.sh
	do_system("/bin/firewall.sh > /dev/null 2>&1 &");

	if(radio->enable == 1) {
		cms_set_str("wifi_radio%d_status", "ready", radio->index);
	} else {
		cms_set_str("wifi_radio%d_status", "stop", radio->index);
	}

	return 0;
}


int ralink_restart(int argc, char* argv[])
{
	cms_set_str("wifi_status", "busy");

	struct wifi_setting setting = {0};
	struct wifi_radio *radio = NULL;
	struct wifi_iface *iface = NULL;

	wifi_get_setting(&setting);
	wifi_get_setting_changed(&setting);
	//set all radio in busy status
	array_for_each_entry_size(radio, setting.radio, setting.radio_num) {
		cms_set_str("wifi_radio%d_status", "busy", radio->index);
	}
	//set all iface in busy status
	array_for_each_entry_size(iface, setting.iface, setting.iface_num) {
		cms_set_str("wifi_iface%d_status", "busy", iface->index);
	}

	if(setting.supported == 1 && setting.enable == 1) {
		//start wifi logger only
		do_system("pgrep wifi_logger >/dev/null 2>&1 || (wifi_logger &)"); //WARNING: wifi_logger must start before wifi interface up
	} else {
		do_system("killall wifi_logger");
	}

	array_for_each_entry_size(radio, setting.radio, setting.radio_num) {
		//load module
		if(setting.supported == 1 && setting.enable == 1 && radio->enable == 1) {
			do_system("insmod /lib/modules/$(uname -r)/%s >/dev/null 2>&1", radio->driver_path);
		}

		generate_ralink_conf(&setting, radio);
		restart_ralink_iface(&setting, radio);
	}

	if(setting.enable == 1) {
		cms_set_str("wifi_status", "ready");
	} else {
		cms_set_str("wifi_status", "stop");
	}
	return 0;
}


int ralink_stop(int argc, char* argv[])
{
	cms_set_str("wifi_status", "busy");

	struct wifi_setting setting = {0};
	wifi_get_setting(&setting);
	struct wifi_radio *radio = NULL;
	struct wifi_iface *iface = NULL;

	array_for_each_entry_size(iface, setting.iface, setting.iface_num) {
		do_system("ifconfig %s down >/dev/null 2>&1", iface->ifname);
		cms_set_str("wifi_iface%d_status", "stop", iface->index);
	}

	array_for_each_entry_size(radio, setting.radio, setting.radio_num) {
		do_system("rmmod %s >/dev/null 2>&1", basename(radio->driver_path));
		do_system("pkill %s 2>/dev/null", radio->authenticator_daemon);
		cms_set_str("wifi_radio%d_status", "stop", radio->index);
	}

	do_system("pkill wifi_logger");
	do_system("wifi.sh flush_changed");

	cms_set_str("wifi_status", "stop");
	return 0;
}
