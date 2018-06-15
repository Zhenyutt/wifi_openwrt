#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <net/if.h>
#define __user
#include <wireless.h> //struct iwreq
#undef __user

#include <json-c/json.h>

#include "common.h"
#include "config.h"
#include "cms.h"
#include "cms_helper.h"
#include "ralink.h"
#include "ralink_driver.h"
#include "dhcp_lease.h"


int sta_list_query(char *ifname, json_object *sta_list)
{
	//init sock
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		fprintf(stderr, "Error: fail to create netlink socket\n");
		return -1;
	}

	int ret = 0;
	struct iwreq wrq = {};	// union
	RT_802_11_MAC_TABLE_MT7603	table_MT7603 = {0};
	RT_802_11_MAC_TABLE_MT7612	table_MT7612 = {0};

	int ap_idx = 0;
	if(memcmp(ifname, "rai", strlen("rai")) == 0) {
		ap_idx = strtoul(ifname + strlen("rai"), NULL, 10);
	} else if(memcmp(ifname, "ra", strlen("ra")) == 0) {
		ap_idx = strtoul(ifname + strlen("ra"), NULL, 10);
	}

	do {
		snprintf(wrq.ifr_ifrn.ifrn_name, sizeof(wrq.ifr_ifrn.ifrn_name), "%s", ifname);
		if(strstr(ifname, "rai") == NULL) {
			wrq.u.data.pointer = (void *)&table_MT7603;
			wrq.u.data.length = sizeof(table_MT7603);
		} else {
			wrq.u.data.pointer = (void *)&table_MT7612;
			wrq.u.data.length = sizeof(table_MT7612);
		}
		wrq.u.data.flags = 0;
		ret = ioctl(sock, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq);
	} while(0);

	close(sock); //free socket


	if(ret < 0) {
		fprintf(stderr, "Error: fail to ioctl() to query sta list\n");
		return -1;
	}



	int i = 0;
	if(strstr(ifname, "rai") == NULL) {
		for(i = 0; i < table_MT7603.Num; i++) {
			if(table_MT7603.Entry[i].ApIdx != ap_idx) {
				continue;
			}
			char macstr[MAX_MACSTR_SIZE] = {0};
			snprintf(macstr, sizeof(macstr), "%02x:%02x:%02x:%02x:%02x:%02x",
			         table_MT7603.Entry[i].Addr[0], table_MT7603.Entry[i].Addr[1], table_MT7603.Entry[i].Addr[2],
			         table_MT7603.Entry[i].Addr[3], table_MT7603.Entry[i].Addr[4], table_MT7603.Entry[i].Addr[5]);

			json_object *sta_info = json_object_new_object();

			json_object_array_add(sta_list, sta_info);
			json_object_object_add(sta_info, "mac", json_object_new_string(strtoupper(macstr, strlen(macstr))));
			json_object_object_add(sta_info, "ip", json_object_new_string(""));	// check ip from "query_lease_info_mac"
			json_object_object_add(sta_info, "hostname", json_object_new_string(""));	// check hostname from "query_lease_info_mac"
			json_object_object_add(sta_info, "mode", json_object_new_string(get_phy_mode_mt7603(table_MT7603.Entry[i].TxRate.field.MODE)));
			json_object_object_add(sta_info, "rssi_dbm", json_object_new_int(get_best_rssi(table_MT7603.Entry[i].AvgRssi0, table_MT7603.Entry[i].AvgRssi1, table_MT7603.Entry[i].AvgRssi2)));
			json_object_object_add(sta_info, "speed_mbps", json_object_new_int(get_rate(table_MT7603.Entry[i].TxRate)));

#if 0
			// debug
			printf("RT_802_11_MAC_TABLE ApIdx=%d\n", table_MT7603.Entry[i].ApIdx);
			printf("RT_802_11_MAC_TABLE Addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			       table_MT7603.Entry[i].Addr[0], table_MT7603.Entry[i].Addr[1], table_MT7603.Entry[i].Addr[2],
			       table_MT7603.Entry[i].Addr[3], table_MT7603.Entry[i].Addr[4], table_MT7603.Entry[i].Addr[5]);
			printf("RT_802_11_MAC_TABLE Aid=%d\n", table_MT7603.Entry[i].Aid);
			printf("RT_802_11_MAC_TABLE Psm=%d\n", table_MT7603.Entry[i].Psm);
			printf("RT_802_11_MAC_TABLE AvgRssi0=%d\n", table_MT7603.Entry[i].AvgRssi0);
			printf("RT_802_11_MAC_TABLE AvgRssi1=%d\n", table_MT7603.Entry[i].AvgRssi1);
			printf("RT_802_11_MAC_TABLE AvgRssi2=%d\n", table_MT7603.Entry[i].AvgRssi2);
			printf("RT_802_11_MAC_TABLE MODE=%s\n", get_phy_mode_mt7603(table_MT7603.Entry[i].TxRate.field.MODE));
			printf("RT_802_11_MAC_TABLE BW=%s\n", get_BW(table_MT7603.Entry[i].TxRate.field.BW));
			printf("RT_802_11_MAC_TABLE MCS=%d\n", table_MT7603.Entry[i].TxRate.field.MCS);
			printf("RT_802_11_MAC_TABLE ShortGI=%d\n", table_MT7603.Entry[i].TxRate.field.ShortGI);
			printf("RT_802_11_MAC_TABLE Speed=%d\n", get_rate(table_MT7603.Entry[i].TxRate));
			printf("RT_802_11_MAC_TABLE ConnectedTime=%d\n", table_MT7603.Entry[i].ConnectedTime);
#endif
		}
	} else {
		for(i = 0; i < table_MT7612.Num; i++) {
			if(table_MT7612.Entry[i].ApIdx != ap_idx) {
				continue;
			}
			char macstr[MAX_MACSTR_SIZE] = {0};
			snprintf(macstr, sizeof(macstr), "%02x:%02x:%02x:%02x:%02x:%02x",
			         table_MT7612.Entry[i].Addr[0], table_MT7612.Entry[i].Addr[1], table_MT7612.Entry[i].Addr[2],
			         table_MT7612.Entry[i].Addr[3], table_MT7612.Entry[i].Addr[4], table_MT7612.Entry[i].Addr[5]);

			json_object *sta_info = json_object_new_object();

			json_object_array_add(sta_list, sta_info);
			json_object_object_add(sta_info, "mac", json_object_new_string(strtoupper(macstr, strlen(macstr))));
			json_object_object_add(sta_info, "ip", json_object_new_string(""));	// check ip from "query_lease_info_mac"
			json_object_object_add(sta_info, "hostname", json_object_new_string(""));	// check hostname from "query_lease_info_mac"
			json_object_object_add(sta_info, "mode", json_object_new_string(get_phy_mode_mt7612(table_MT7612.Entry[i].TxRate.field.MODE)));
			json_object_object_add(sta_info, "rssi_dbm", json_object_new_int(get_best_rssi(table_MT7612.Entry[i].AvgRssi0, table_MT7612.Entry[i].AvgRssi1, table_MT7612.Entry[i].AvgRssi2)));
			json_object_object_add(sta_info, "speed_mbps", json_object_new_int(get_rate(table_MT7612.Entry[i].TxRate)));

#if 0
			// debug
			printf("RT_802_11_MAC_TABLE ApIdx=%d\n", table_MT7612.Entry[i].ApIdx);
			printf("RT_802_11_MAC_TABLE Addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			       table_MT7612.Entry[i].Addr[0], table_MT7612.Entry[i].Addr[1], table_MT7612.Entry[i].Addr[2],
			       table_MT7612.Entry[i].Addr[3], table_MT7612.Entry[i].Addr[4], table_MT7612.Entry[i].Addr[5]);
			printf("RT_802_11_MAC_TABLE Aid=%d\n", table_MT7612.Entry[i].Aid);
			printf("RT_802_11_MAC_TABLE Psm=%d\n", table_MT7612.Entry[i].Psm);
			printf("RT_802_11_MAC_TABLE AvgRssi0=%d\n", table_MT7612.Entry[i].AvgRssi0);
			printf("RT_802_11_MAC_TABLE AvgRssi1=%d\n", table_MT7612.Entry[i].AvgRssi1);
			printf("RT_802_11_MAC_TABLE AvgRssi2=%d\n", table_MT7612.Entry[i].AvgRssi2);
			printf("RT_802_11_MAC_TABLE MODE=%s\n", get_phy_mode_mt7612(table_MT7612.Entry[i].TxRate.field.MODE));
			printf("RT_802_11_MAC_TABLE BW=%s\n", get_BW(table_MT7612.Entry[i].TxRate.field.BW));
			printf("RT_802_11_MAC_TABLE MCS=%d\n", table_MT7612.Entry[i].TxRate.field.MCS);
			printf("RT_802_11_MAC_TABLE ShortGI=%d\n", table_MT7612.Entry[i].TxRate.field.ShortGI);
			printf("RT_802_11_MAC_TABLE Speed=%d\n", get_rate(table_MT7612.Entry[i].TxRate));
			printf("RT_802_11_MAC_TABLE ConnectedTime=%d\n", table_MT7612.Entry[i].ConnectedTime);
#endif
		}
	}
	return 0;
}


int sta_list_print(char *option)
{
	if(strcmp(option, "all") == 0) {
		int i;
		int iface_num = cms_get_int("wifi_iface_num", 0);
		char *ifname = NULL;
		json_object *sta_list_all = json_object_new_object();

		for(i = 0; i < iface_num; i++) {
			if(cms_get_int("wifi_iface%d_enable", 0, i) == 1) {
				ifname = cms_get_str("wifi_iface%d_ifname", "", i);
				json_object *sta_list = json_object_new_array();
				sta_list_query(ifname, sta_list);
				json_object_object_add(sta_list_all, ifname, sta_list);
			}
		}
		printf("%s%s\n", "", json_object_to_json_string_ext(sta_list_all, JSON_C_TO_STRING_PLAIN));
		// free json object
		json_object_put(sta_list_all);
	} else {
		json_object *sta_list = json_object_new_array();
		sta_list_query(option, sta_list);
		printf("%s%s\n", "", json_object_to_json_string_ext(sta_list, JSON_C_TO_STRING_PLAIN));
		// free json object
		json_object_put(sta_list);
	}

	return 0;
}


int ralink_sta_list_usage()
{
	printf("Usage:\n");
	printf("    wifi sta_list [interface]    #list connected STA\n");
	printf("    wifi sta_list all  			 #list all connected STA\n");
	return 0;
}


int ralink_sta_list(int argc, char *argv[])
{
	if(argc < 2) {
		ralink_sta_list_usage();
		return -1;
	}

	char *ifname = argv[1];

	sta_list_print(ifname);

	return 0;

}
