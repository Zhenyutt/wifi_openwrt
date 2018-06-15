#ifndef _OPENWRT_CONFIG_H
#define _OPENWRT_CONFIG_H

#include "config.h"

#define STRBUFF_SIZE 1024

enum {
	None_HT = 0,
	HT_BW = 1,
	VHT_BW = 2
};

struct openwrt_wireless_mode {
	char *wireless_mode;
	char *hwmode;
	int ht;
};

struct openwrt_htmode {
	int ht;
	char *htmode;
	char *map_htmode;
	
};

struct openwrt_encryption {
	char *encryption;
	char *map_encryption;
};

struct openwrt_wireless_mode* query_openwrt_wireless_mode(char *wireless_mode);
struct openwrt_htmode* query_openwrt_htmode(char *htmode, int ht);
struct openwrt_encryption* query_openwrt_encryption(char *encryption);

char *openwrt_channel(char *strbuff, size_t buff_size, int channel);
char *openwrt_key(char *strbuff, size_t buff_size, char *encryption, int key_wep_index, char *key_psk);

int generate_openwrt_conf(struct wifi_setting *setting, struct wifi_radio *radio);
int restart_openwrt_iface(struct wifi_setting *setting, struct wifi_radio *radio);
int openwrt_restart(int argc, char* argv[]);
int openwrt_stop(int argc, char* argv[]);

#endif //_OPENWRT_CONFIG_H
