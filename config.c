#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <string.h>

#include "cms.h"
#include "cms_helper.h"
#include "common.h"
#include "config.h"
//#include "ralink.h"
#include "openwrt.h"



int encryption_is_psk(char *encryption)
{
	if(strncmp(encryption, "psk", strlen("psk")) == 0) {
		return 1;	//yes
	} else {
		return 0;	//no
	}
}


int encryption_is_wep(char *encryption)
{
	if(strstr(encryption, "wep") != NULL) {
		return 1;	//yes
	} else {
		return 0;	//no
	}
}


int wireless_mode_is_include_N(char *wireless_mode)
{

	if(strstr(wireless_mode, "N") != NULL) {
		return 1;	//yes
	} else {
		return 0;	//no
	}
}


int wireless_mode_is_include_AC(char *wireless_mode)
{

	if(strstr(wireless_mode, "AC") != NULL) {
		return 1;	//yes
	} else {
		return 0;	//no
	}
}


/********************************
 * config file function
 ********************************/
FILE * init_conf(const char *filename)
{
	char *filename_buffer = strdup(filename);
	char *dname = dirname(filename_buffer);
	do_system("mkdir -p %s", dname);
	free(filename_buffer);

	FILE *fp = fopen(filename, "a");
	if(fp == NULL) {
		fprintf(stderr, "%s: fail to create file", __FUNCTION__);
	}
	return fp;
}

int set_conf_line(FILE *fp, const char *line)
{
	if(fp == NULL) {
		return -1;
	}

	if(get_debug_level() > 0) {
		fprintf(stderr, "%s():%d: %s\n", __FUNCTION__, __LINE__, line);
	}

	int len = fprintf(fp, "%s\n", line);
	if(len <= 0) {
		return -1; //error
	}

	return 0;
}



int set_conf(FILE *fp, const char *key, const char *value)
{
	if(fp == NULL) {
		return -1;
	}

	if(get_debug_level() > 0) {
		fprintf(stderr, "%s():%d: %s=%s\n", __FUNCTION__, __LINE__, key, value);
	}

	int len = fprintf(fp, "%s=%s\n", key, value);
	if(len <= 0) {
		return -1; //error
	}

	return 0;
}


int set_conf_int(FILE *fp, const char *key, int value)
{
	char value_str[128] = {0};
	snprintf(value_str, sizeof(value_str), "%d", value);
	return set_conf(fp, key, value_str);
}


void close_conf(FILE *fp)
{
	fclose(fp);
}

int set_option(FILE *fp, const char *key, const char *value) {
	if(fp == NULL) {
		return -1;
	}
	
	if(value == NULL) {
		return -1;
	}
	
	if(strcmp(value, "") == 0) {
		return -1;
	} 
	
	int len = fprintf(fp, "\toption '%s' '%s'\n", key, value);
	if(len <= 0) {
		return -1; //error
	}

	return 0;
}

int set_option_int(FILE *fp, const char *key, int value) {
	char value_str[128] = {0};
	snprintf(value_str, sizeof(value_str), "%d", value);
	return set_option(fp, key, value_str);
}

/***************************************
 * WPS API
 ***************************************/
//wps
int wps_set_status(struct wifi_iface *iface, char *status)
{
	cms_set_str("wifi_iface%d_wps_status", status, iface->index);
	return 0;
}


int wifi_get_setting_changed(struct wifi_setting *setting)
{
	FILE *stream = popen("wifi.sh get_changed 2>/dev/null", "re");
	if(stream == NULL) {
		fprintf(stderr, "fail to get changed wifi config\n");
		return -1;
	}

	char line[1024] = {0};
	while(fgets(line, sizeof(line), stream) != NULL) {
		//strip tailing newline
		char *nl = strchr(line, '\n');
		if(nl) {
			*nl = '\0';
		}

		if(strcmp(line, "wifi") == 0) {
			setting->changed = 1;
			//fprintf(stderr, "setting->changed = %d\n", setting->changed);
		} else if(strncmp(line, "wifi_iface", strlen("wifi_iface")) == 0) {
			unsigned int idx = strtoul_default(line + strlen("wifi_iface"), NULL, 10, 0);
			if(idx < setting->iface_num) {
				setting->iface[idx].changed = 1;
				//fprintf(stderr, "iface[%d].changed = %d\n", idx, setting->iface[idx].changed);
			}
		} else if(strncmp(line, "wifi_radio", strlen("wifi_radio")) == 0) {
			unsigned int idx = strtoul_default(line + strlen("wifi_radio"), NULL, 10, 0);
			if(idx < setting->radio_num) {
				setting->radio[idx].changed = 1;
				//fprintf(stderr, "radio[%d].changed = %d\n", idx, setting->radio[idx].changed);
			}
		}
	} //end while get line
	pclose(stream);

	//update change file
	do_system("wifi.sh update_changed");

	return 0;
}


int wifi_get_setting(struct wifi_setting *setting)
{
	set_debug_level(cms_get_uint("wifi_debug_level", 0));

	//free old one
	if(setting->radio_num > 0) {
		free(setting->radio);
		setting->radio_num = 0;
		setting->radio = NULL;
	}
	if(setting->iface_num > 0) {
		free(setting->iface);
		setting->iface_num = 0;
		setting->iface = NULL;
	}

	//global setting
	setting->supported = cms_get_uint("profile_wifi", 0);
	setting->enable = cms_get_uint("wifi_enable", 0);
	setting->radio_num = cms_get_uint("wifi_radio_num", 0);
	setting->radio = (struct wifi_radio *)calloc(1, sizeof(struct wifi_radio) * setting->radio_num);
	setting->iface_num = cms_get_uint("wifi_iface_num", 0);
	setting->iface = (struct wifi_iface *)calloc(1, sizeof(struct wifi_iface) * setting->iface_num);
	setting->debug_level = cms_get_uint("wifi_debug_level", 0);

	unsigned int i = 0;

	//for each radio
	for(i = 0; i < setting->radio_num; i++) {
		struct wifi_radio *radio = &(setting->radio[i]);
		radio->index = cms_get_uint("wifi_radio%d_index", i, i);
		radio->enable = cms_get_uint("wifi_radio%d_enable", 0, i);
		radio->channel = cms_get_uint("wifi_radio%d_channel", 0, i);
		radio->channel_option_num = cms_get_uint_array("wifi_radio%d_channel_option", 0, radio->channel_option, MAX_CHANNEL_OPTION_NUM, ",", i);

		radio->beacon_interval = cms_get_uint("wifi_radio%d_beacon_interval", 100, i);
		radio->txpower_dBm = cms_get_uint("wifi_radio%d_txpower_dBm", DEFAULT_TXPOWER_DBM, i);

		radio->rts_threshold = cms_get_uint("wifi_radio%d_rts_threshold", DEFAULT_RTS_THRESHOLD, i);
		radio->frag_threshold = cms_get_uint("wifi_radio%d_frag_threshold", DEFAULT_FRAG_THRESHOLD, i);
		radio->max_inactivity_second = cms_get_uint("wifi_radio%d_max_inactivity_second", DEFAULT_MAX_INACTIVITY_SECOND, i);
		radio->keep_alive_second = cms_get_uint("wifi_radio%d_keep_alive_second", DEFAULT_KEEP_ALIVE_SECOND, i);
		radio->coexistence_20_40_mhz = cms_get_uint("wifi_radio%d_coexistence_20_40_mhz", 1, i);
		radio->disallow_tkip_ht_rates = cms_get_uint("wifi_radio%d_disallow_tkip_ht_rates", 1, i);

		snprintf(radio->chip, sizeof(radio->chip), "%s", cms_get_str("wifi_radio%d_chip", "unknown", i));
		snprintf(radio->driver_path, sizeof(radio->driver_path), "%s", cms_get_str("wifi_radio%d_driver_path", "", i));
		snprintf(radio->config_path, sizeof(radio->config_path), "%s", cms_get_str("wifi_radio%d_config_path", "", i));
		snprintf(radio->country, sizeof(radio->country), "%s", cms_get_str("wifi_radio%d_country", "US", i));
		snprintf(radio->wireless_mode, sizeof(radio->wireless_mode), "%s", cms_get_str("wifi_radio%d_wireless_mode", "unknown", i));
		snprintf(radio->htmode, sizeof(radio->htmode), "%s", cms_get_str("wifi_radio%d_htmode", "unknown", i));
		snprintf(radio->band, sizeof(radio->band), "%s", cms_get_str("wifi_radio%d_band", "", i));

		//Authenticator
		snprintf(radio->authenticator_daemon, sizeof(radio->authenticator_daemon), "%s", cms_get_str("wifi_radio%d_authenticator_daemon", "", i));
	}

	//for each iface
	for(i = 0; i < setting->iface_num; i++) {
		struct wifi_iface *iface = &(setting->iface[i]);
		//radio in iface and iface in radio
		int radio_index = cms_get_uint("wifi_iface%d_radio_index", 0, i);
		iface->radio = &setting->radio[radio_index];
		iface->radio->iface[iface->radio->iface_num++] = iface;

		iface->index = cms_get_uint("wifi_iface%d_index", i, i);
		iface->dtim_period = cms_get_uint("wifi_iface%d_dtim_period", 1, i);
		iface->enable = cms_get_uint("wifi_iface%d_enable", 0, i);
		iface->sta_maxnum = cms_get_uint("wifi_iface%d_sta_maxnum", MAX_STA_NUM, i);
		iface->isolate_sta = cms_get_uint("wifi_iface%d_isolate_sta", 0, i);
		iface->hidden = cms_get_uint("wifi_iface%d_hidden", 0, i);
		iface->wmm = cms_get_uint("wifi_iface%d_wmm", 0, i);

		snprintf(iface->ifname, sizeof(iface->ifname), "%s", cms_get_str("wifi_iface%d_ifname", "", i));
		snprintf(iface->bridge, sizeof(iface->bridge), "%s", cms_get_str("wifi_iface%d_bridge", "", i));
		snprintf(iface->macaddr, sizeof(iface->macaddr), "%s", cms_get_str("wifi_iface%d_macaddr", "", i));

		snprintf(iface->ssid, sizeof(iface->ssid), "%s", cms_get_str("wifi_iface%d_ssid", "missing_ssid", i));
		snprintf(iface->encryption, sizeof(iface->encryption), "%s", cms_get_str("wifi_iface%d_encryption", "none", i));

		iface->key_encrypted = cms_get_uint("wifi_iface%d_key_encrypted", 0, i);
		iface->key_wep_index = cms_get_uint("wifi_iface%d_key_wep_index", 1, i);
		if(iface->key_encrypted == 1) {
			char plain_text[128] = {0};
			char *enc_text = NULL;

			enc_text = cms_get_str("wifi_iface%d_key_wep", "", i);
			gmtk_decrypt_helper(enc_text, plain_text, sizeof(plain_text));
			snprintf(iface->key_wep, sizeof(iface->key_wep), "%s", plain_text);

			enc_text = cms_get_str("wifi_iface%d_key_psk", "", i);
			gmtk_decrypt_helper(enc_text, plain_text, sizeof(plain_text));
			snprintf(iface->key_psk, sizeof(iface->key_psk), "%s", plain_text);
		} else {
			//plain key
			snprintf(iface->key_wep, sizeof(iface->key_wep), "%s", cms_get_str("wifi_iface%d_key_wep", "", i));
			snprintf(iface->key_psk, sizeof(iface->key_psk), "%s", cms_get_str("wifi_iface%d_key_psk", "", i));
		}
		// set wifi_ifaceX_key
		if(strstr(iface->encryption, "wep") != NULL) {
			cms_set_str("wifi_iface%d_key", iface->key_wep, i);
		} else if(strstr(iface->encryption, "psk") != NULL) {
			cms_set_str("wifi_iface%d_key", iface->key_psk, i);
		} else {
			cms_set_str("wifi_iface%d_key", "", i);
		}
		// set authenticator_enable
		if(strstr(iface->encryption, "wpa") != NULL) {
			iface->radio->authenticator_enable = 1;
		}

		snprintf(iface->macfilter, sizeof(iface->macfilter), "%s", cms_get_str("wifi_iface%d_macfilter", "disabled", i));
		snprintf(iface->macfilter_list, sizeof(iface->macfilter_list), "%s", cms_get_str("wifi_iface%d_macfilter_list", "", i));

		//wps
		iface->wps = cms_get_uint("wifi_iface%d_wps", 1, i);
		snprintf(iface->wps_method, sizeof(iface->wps_method), "%s", cms_get_str("wifi_iface%d_wps_method", "pbc,pin", i));
		snprintf(iface->wps_support_encryption, sizeof(iface->wps_support_encryption), "%s", cms_get_str("wifi_iface%d_wps_support_encryption", "none,psk2+ccmp,psk-mixed+tkip+ccmp", i));
		iface->wps_configurated = cms_get_uint("wifi_iface%d_wps_configurated", 1, i);
		snprintf(iface->wps_device_name, sizeof(iface->wps_device_name), "%s", cms_get_str("wifi_iface%d_wps_device_name", "", i));
		snprintf(iface->wps_manufacturer, sizeof(iface->wps_manufacturer), "%s", cms_get_str("wifi_iface%d_wps_manufacturer", "", i));
		snprintf(iface->wps_model_name, sizeof(iface->wps_model_name), "%s", cms_get_str("wifi_iface%d_wps_model_name", "", i));
		snprintf(iface->wps_model_number, sizeof(iface->wps_model_number), "%s", cms_get_str("wifi_iface%d_wps_model_number", "", i));
		snprintf(iface->wps_serial_number, sizeof(iface->wps_serial_number), "%s", cms_get_str("wifi_iface%d_wps_serial_number", "", i));

		if(strstr(iface->wps_support_encryption, iface->encryption) == NULL) {
			iface->wps_not_support = 1; //current encryption do not support wps
		}

		if(iface->hidden == 1) {
			iface->wps_not_support = 1; //hide ssid do not support wps
		}

		//Radius Server
		snprintf(iface->radius_server_ipaddr, sizeof(iface->radius_server_ipaddr), "%s", cms_get_str("wifi_iface%d_radius_server_ipaddr", "", i));
		iface->radius_server_port = cms_get_uint("wifi_iface%d_radius_server_port", 1812, i);
		snprintf(iface->radius_server_key, sizeof(iface->radius_server_key), "%s", cms_get_str("wifi_iface%d_radius_server_key", "", i));
	}

	return 0;
}


struct wifi_iface* query_iface_by_ifname(struct wifi_setting *setting, const char *ifname)
{
	struct wifi_iface *iface = NULL;

	array_for_each_entry_size(iface, setting->iface, setting->iface_num) {
		if(strcmp(iface->ifname, ifname) == 0) {
			return iface;
		}
	}
	return NULL;
}
