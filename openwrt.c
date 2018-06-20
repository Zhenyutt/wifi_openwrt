#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "openwrt.h"
#include "config.h"
#include "cms_helper.h"
#include "common.h"



static struct openwrt_wireless_mode openwrt_wireless_mode_map[] = {
	{ "B/G/N",		"11g",	HT_BW},	//default value
	{ "B/G", 		"11g", 	None_HT},
	{ "B",			"11b",	None_HT},
	{ "A/AN/AC",	"11a",	VHT_BW},
	{ "A/AN",		"11a", 	HT_BW},
	{ "A", 			"11a", 	None_HT},
};

struct openwrt_htmode openwrt_htmode_map[] = {
	{None_HT,	"",			"NONE"},
	{HT_BW, 	"HT20",		"HT20"},
	{HT_BW,		"HT40",		"HT40"},
	{VHT_BW,	"HT20",		"VHT20"},
	{VHT_BW,	"HT40",		"VHT40"},
	{VHT_BW,	"HT80",		"VHT80"},
	{VHT_BW,	"HT160",	"VHT160"},
};

struct openwrt_encryption openwrt_encryption_map[] = {
	//open
	{ "none", "none"},
	//wep
	{ "wep-open", "wep+open"},
	{ "wep-shared",	"wep+shared"},
	//psk
	{ "psk+tkip", "psk+tkip"},
	{ "psk+ccmp", "psk+ccmp"},
	{ "psk+tkip+ccmp", "psk+tkip+ccmp"},
	//psk2
	{ "psk2+tkip", "psk2+tkip"},
	{ "psk2+ccmp", "psk2+ccmp"},
	{ "psk2+tkip+ccmp", "psk2+tkip+ccmp"},
	//psk-mixed
	{ "psk-mixed+tkip",	"psk-mixed+tkip"},
	{ "psk-mixed+ccmp", "psk-mixed+ccmp"},
	{ "psk-mixed+tkip+ccmp", "psk-mixed+tkip+ccmp"},
	
	//non-psk
	//{ "wpa2+ccmp",			"wpa2+ccmp"},
	//{ "wpa-mixed+tkip+ccmp","wpa-mixed+tkip+ccmp"},
};

struct openwrt_wireless_mode* query_openwrt_wireless_mode(char *wireless_mode) {
	if(wireless_mode == NULL) {
		return openwrt_wireless_mode_map;
	}

	struct openwrt_wireless_mode *p = NULL;
	array_for_each_entry(p, openwrt_wireless_mode_map) {
		if(strcmp(wireless_mode, p->wireless_mode) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match wireless_mode(%s), use defult(%s)\n", __FUNCTION__, wireless_mode, openwrt_wireless_mode_map->wireless_mode);
	return openwrt_wireless_mode_map;
}

struct openwrt_htmode* query_openwrt_htmode(char *htmode, int ht) {
	if(htmode == NULL) {
		return openwrt_htmode_map;
	}
	
	struct openwrt_htmode *p = NULL;
	array_for_each_entry(p, openwrt_htmode_map) {
		if(strcmp(htmode, p->htmode) == 0 && ht == p->ht) {
			return p;
		}
	}
	
	fprintf(stderr, "%s(): Error: un-match htmode(%s), use defult(%s)\n", __FUNCTION__, htmode, openwrt_htmode_map->htmode);
	return openwrt_htmode_map;
}



struct openwrt_encryption* query_openwrt_encryption(char *encryption) {
	if(encryption == NULL) {
		return openwrt_encryption_map;
	}
	
	struct openwrt_encryption *p = NULL;
	array_for_each_entry(p, openwrt_encryption_map) {
		if(strcmp(encryption, p->encryption) == 0) {
			return p;
		}
	}

	fprintf(stderr, "%s(): Error: un-match encryption(%s), use defult(%s)\n", __FUNCTION__, encryption, openwrt_encryption_map->encryption);
	return openwrt_encryption_map;
}


char *openwrt_channel(char *strbuff, size_t buff_size, int channel) {
	if(channel == 0) {
		snprintf(strbuff, buff_size, "auto");
	}
	else {
		snprintf(strbuff, buff_size, "%d", channel);
	}
	return strbuff;
}

char *openwrt_channels(char *strbuff, size_t buff_size, int *channel_option) {
	
	return strbuff;
}


char *openwrt_key(char *strbuff, size_t buff_size, char *encryption, int key_wep_index, char *key_psk) {
	if(encryption_is_wep(encryption)) {
		snprintf(strbuff, buff_size, "%d", key_wep_index);
	}
	else if(encryption_is_psk(encryption)) {
		snprintf(strbuff, buff_size, "%s", key_psk);
	}
	else {
		strbuff[0] = '\0';
	}
	return strbuff;
}

int generate_openwrt_conf(struct wifi_setting *setting, struct wifi_radio *radio) {
	int i;
	char strbuff[STRBUFF_SIZE] = {0};
	
	struct wifi_iface **iface = radio->iface;
	
	//FILE *fp = stdout;
	FILE *fp = init_conf(radio->config_path);
	if(fp == NULL) {
		return -1;
	}
	
	
	
	
	struct openwrt_wireless_mode *op_wmode = query_openwrt_wireless_mode(radio->wireless_mode);
	struct openwrt_htmode *op_htmode = query_openwrt_htmode(radio->htmode, op_wmode->ht);
	
	
	
	
	set_conf_line(fp, format_string(strbuff, sizeof(strbuff), "config 'wifi-device' 'radio%d'", radio->index));
	set_option(fp, "type", "mac80211");
	//set_option(fp, "phy", "");
	//set_option(fp, "macaddr", "");
	//set_option(fp, "ifname", "");
	set_option_int(fp, "disabled", radio->enable ? 0 : 1);
	set_option(fp, "channel", openwrt_channel(strbuff, sizeof(strbuff), radio->channel));
	set_option_channels(fp, "channels", radio->channel_option, radio->channel_option_num);
	set_option(fp, "hwmode", op_wmode->hwmode);
	set_option(fp, "htmode", op_htmode->map_htmode);
	//set_option_int(fp, "chanbw", "");
	//set_option(fp, "ht_capab", "");
	set_option_int(fp, "txpower", radio->txpower_dBm);
	//set_option_int(fp, "diversity", "");
	//set_option_int(fp, "rxantenna", "");
	//set_option_int(fp, "txantenna", "");
	set_option(fp, "country", radio->country);
	set_option_int(fp, "country_ie", 1);
	//set_option_int(fp, "distance", "");
	set_option_int(fp, "beacon_int", radio->beacon_interval);
	//set_list(fp, "basic_rate", "", "");
	//set_list(fp, "supported_rates", "", "");
	//set_option(fp, "require_mode", "");
	//set_option_int(fp, "log_level", "");
	//set_option_int(fp, "legacy_rates", "");
	//set_option_int(fp, "noscan", "");
	//set_option_int(fp, "htcoex", "");
	set_option_int(fp, "frag", radio->frag_threshold);
	set_option_int(fp, "rts", radio->rts_threshold);
	//set_option_int(fp, "antenna_gain", "");
	
	//Inactivity Timeout Options
	//set_option_int(fp, "disassoc_low_ack", );
	//set_option_int(fp, "max_inactivity", );
	//set_option_int(fp, "skip_inactivity_poll", );
	//set_option_int(fp, "max_listen_interval", );
	
	set_conf_line(fp, "");
		
	
	
	for(i = 0; i < radio->iface_num; i++) {
		struct openwrt_encryption *op_enc = query_openwrt_encryption(iface[i]->encryption);
		
		
		set_conf_line(fp, "config 'wifi-iface'");
		set_option(fp, "device", format_string(strbuff, sizeof(strbuff), "radio%d", radio->index));
		set_option(fp, "network", "lan"); //bridge
		set_option(fp, "mode", iface[i]->mode);
		set_option_int(fp, "disabled", iface[i]->enable ? 0 : 1);
		set_option(fp, "ssid", iface[i]->ssid);
		//set_option(fp, "bssid", "");
		//set_option(fp, "mesh_id", "");
		set_option_int(fp, "hidden", iface[i]->hidden);
		set_option_int(fp, "isolate", iface[i]->isolate_sta);
		//set_option_int(fp, "doth", "");
		set_option_int(fp, "wmm", iface[i]->wmm);
		set_option(fp, "encryption", op_enc->map_encryption);
		set_option(fp, "key", openwrt_key(strbuff, sizeof(strbuff), iface[i]->encryption, iface[i]->key_wep_index, iface[i]->key_psk));
		
		//set wep key
		sprintf(strbuff, "key%d", iface[i]->key_wep_index);
		set_option(fp, strbuff, iface[i]->key_wep);
		
		set_option(fp, "macfilter", iface[i]->macfilter);
		
		char AccessControlList[MAX_MACSTR_SIZE * MAX_STA_NUM] = {0};
		string_replace_to_buffer(iface[i]->macfilter_list, ",", " ", 0, AccessControlList, sizeof(AccessControlList));
		set_option(fp, "maclist", AccessControlList);
		
		//set_option(fp, "iapp_interface", "");
		//set_option_int(fp, "rsn_preauth", "");
		//set_option_int(fp, "ieee80211w", "");
		//set_option_int(fp, "ieee80211w_max_timeout", "");
		//set_option_int(fp, "ieee80211w_retry_timeout", "");
		set_option_int(fp, "maxassoc", iface[i]->sta_maxnum);
		set_option(fp, "macaddr", iface[i]->macaddr);
		set_option_int(fp, "dtim_period", iface[i]->dtim_period);
		set_option_int(fp, "short_preamble", iface[i]->short_preamble);
		//set_option_int(fp, "max_listen_int", "");
		//set_option_int(fp, "mcast_rate", "");
		//set_option_int(fp, "wds", "");
		//set_option_int(fp, "start_disabled", "");
		//set_option_int(fp, "powersave", "");
		
		
		//wps
		//set_conf_line(fp, "\tlist 'wps_config' 'push_button'");
		//set_list(fp, "wps_config", "");
		set_option(fp, "wps_device_name", iface[i]->wps_device_name);
		//set_option(fp, "wps_device_type", "");
		//set_option_int(fp, "wps_label", "");
		set_option(fp, "wps_manufacturer", iface[i]->wps_manufacturer);
		//set_option_int(fp, "wps_pushbutton", "");
		//set_option(fp, "wps_pin", "");
		set_conf_line(fp, "");
	}
	
	close_conf(fp);
	return 0;
}


int restart_openwrt_iface(struct wifi_setting *setting, struct wifi_radio *radio)
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

int openwrt_restart(int argc, char* argv[])
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
	
	//create file and clear
	FILE *fp = fopen(setting.radio->config_path, "w");
	if(fp == NULL) {
		fprintf(stderr, "%s: fail to create file", __FUNCTION__);
	}
	
	array_for_each_entry_size(radio, setting.radio, setting.radio_num) {
		//load module
		if(setting.supported == 1 && setting.enable == 1 && radio->enable == 1) {
			do_system("insmod /lib/modules/$(uname -r)/%s >/dev/null 2>&1", radio->driver_path);
		}
		
		generate_openwrt_conf(&setting, radio);
		restart_openwrt_iface(&setting, radio);
	}
	


	if(setting.enable == 1) {
		cms_set_str("wifi_status", "ready");
	} else {
		cms_set_str("wifi_status", "stop");
	}
	return 0;
}

int openwrt_stop(int argc, char* argv[])
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
