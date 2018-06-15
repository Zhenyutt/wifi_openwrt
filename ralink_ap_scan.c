#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <json-c/json.h>

#include "common.h"
#include "config.h"
#include "ralink_ap_scan.h"
#include "ralink.h"


struct ap_info {
	unsigned int channel;
	char	ssid[MAX_SSID_SIZE * 4];	//large it's size to save \uXXXX format or \xYY format
	char	bssid[MAX_MACSTR_SIZE];
	char	encryption[MAX_ENCRYPTION_SIZE];
	char	mode[MAX_MODE_SIZE];
	char	htmode[MAX_HTMODE_SIZE];
	char	signal[10]; //eg: 100% or -30dBm
	unsigned int wps_enable;
};


int ap_info_fill(struct ap_info *info, char *line)
{
	if(info == NULL || line == NULL) {
		return -1;
	}
	//example line:
	//strcpy(line, "1   12345678901234567890123456789012 1c:49:7b:00:00:5a   WPA1PSKWPA2PSK/TKIPAES 100      11b/g/n NONE   In YES NONE 1         13");
#define LINE_CHANNEL_SIZE	4
#define LINE_SSID_SIZE		33
	char *next = line;

	//channel
	info->channel = strtoul(strtok_r(NULL, " ", &next), NULL, 10);
	next = line + LINE_CHANNEL_SIZE;

	//ssid, but need fix length later
	snprintf(info->ssid, sizeof(info->ssid), "%s", next);
	next += LINE_SSID_SIZE;

	//bssid
	char *bssid = strtok_r(NULL, " ", &next);
	strtoupper(bssid, strlen(bssid));
	snprintf(info->bssid, sizeof(info->bssid), "%s", bssid);

	//encryption
	char *security = strtok_r(NULL, " ", &next);
	snprintf(info->encryption, sizeof(info->encryption), security);

	//signal
	snprintf(info->signal, sizeof(info->signal), "%s%%", strtok_r(NULL, " ", &next));

	//mode
	char *ap_scanned_mode = strtok_r(NULL, " ", &next);
	snprintf(info->mode, sizeof(info->mode), "%s", query_ralink_ap_scanned_mode(ap_scanned_mode)->wireless_mode);

	//ext_channel: NONE, ABOVE, BELOW
	char *ext_channel = strtok_r(NULL, " ", &next);
	if(strcmp(ext_channel, "ABOVE") == 0 || strcmp(ext_channel, "BELOW") == 0) {
		snprintf(info->htmode, sizeof(info->mode), "%s", "HT40");
	} else {
		snprintf(info->htmode, sizeof(info->mode), "%s", "HT20");
	}

	//devcie type:  "In, Ad  (infrastructure or ad-hoc)
	/*char *device_type = */strtok_r(NULL, " ", &next);

	//wps_enable
	char *wps_enable = strtok_r(NULL, " ", &next);
	if(strcmp(wps_enable, "YES") == 0) {
		info->wps_enable = 1;
	} else {
		info->wps_enable = 0;
	}
	//wps current mode
	/*char *wps_current_mode = */strtok_r(NULL, " ", &next);

	//printable
	unsigned int ssid_printable = !!strtoul(strtok_r(NULL, " ", &next), NULL, 10);

	//ssid_len
	unsigned int ssid_len = strtoul(strtok_r(NULL, " ", &next), NULL, 10);

	//more ssid config
	if(ssid_printable == 1) {
		//fix ssid length
		if(ssid_len <= 32) {
			info->ssid[ssid_len] = '\0';
		}
	} else {
		//non ascii ssid, need decode and encode
		char ssid_utf8str[MAX_SSID_SIZE * 4 + 1] = {0}; //\xXX\xYY\xZZ... (utf hex)
		snprintf(ssid_utf8str, sizeof(ssid_utf8str), "%s", strtok_r(NULL, " \n", &next));

		char ssid_utf8[MAX_SSID_SIZE + 1] = {0};
		utf8str_to_utf8(ssid_utf8str, ssid_utf8); //\xXX\xYY\xZZ... -> utf8

		char ssid_ucs2[MAX_SSID_SIZE * 6 + 1] = {0};
		utf8_to_ucs2hex(ssid_utf8, ssid_ucs2, sizeof(ssid_ucs2)); //utf8 -> \uXXXX (ucs2)
		snprintf(info->ssid, sizeof(info->ssid), "%s", ssid_ucs2);
	}

	return 0;
}


int ralink_ap_scan_usage()
{
	printf("Usage:\n");
	printf("    wifi ap_scan [interface]    #scan neighbor AP\n");
	return 0;
}


int ralink_ap_scan(int argc, char *argv[])
{
	if(argc < 2) {
		ralink_ap_scan_usage();
		return -1;
	}

	char *ifname = argv[1];
	char command[64] = {0};

	snprintf(command, sizeof(command), "iwpriv %s set SiteSurvey=1;sleep 1;iwpriv %s get_site_survey;", ifname, ifname);

	FILE *fp = popen(command, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: fail to run scan command\n");
		return -1;
	}

	char line[1024] = {0};
	fgets(line, sizeof(line), fp); //skip line: "ra0       get_site_survey:"
	fgets(line, sizeof(line), fp); //skip line: "Ch  SSID                             BSSID               Security               Siganl(%)W-Mode  ExtCH  NT WPS DPID Printable Len SSID_in_Hex"

	json_object *scan_list = json_object_new_array();
	do {
		memset(line, 0x00, sizeof(line)); //fix: tailing SSID column is variable len
		if(fgets(line, sizeof(line), fp) == NULL) {
			break;
		}
		if(strlen(line) <= 1) {
			break; //last empty line
		}

		struct ap_info info = {0};
		if(ap_info_fill(&info, line) != 0) {
			//try to parse next ap info
			continue;
		}

		json_object *ap_info = json_object_new_object();
		if(ap_info == NULL) {
			fprintf(stderr, "Error: fail to json object for ap_info\n");
			break;
		}
		json_object_array_add(scan_list, ap_info);
		//add ap_info into scan_list
		json_object_object_add(ap_info, "channel", json_object_new_int(info.channel));
		json_object_object_add(ap_info, "ssid", json_object_new_string(info.ssid));
		json_object_object_add(ap_info, "bssid", json_object_new_string(info.bssid));
		json_object_object_add(ap_info, "encryption", json_object_new_string(info.encryption));
		json_object_object_add(ap_info, "mode", json_object_new_string(info.mode));
		json_object_object_add(ap_info, "bandwidth", json_object_new_string(info.htmode));
		json_object_object_add(ap_info, "signal", json_object_new_string(info.signal));
		json_object_object_add(ap_info, "wps_enable", json_object_new_int(info.wps_enable));
	} while(1);

	//output result in json format
	printf("%s%s\n", "", json_object_to_json_string_ext(scan_list, JSON_C_TO_STRING_PLAIN));

	//free json object
	json_object_put(scan_list);

	fclose(fp);

	return 0;
}
