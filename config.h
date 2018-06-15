#ifndef _CONFIG_H
#define _CONFIG_H


#define DEBUG_FLAG		(0x1UL)

#define DEFAULT_TXPOWER_DBM	12
#define DEFAULT_RTS_THRESHOLD	2347
#define DEFAULT_FRAG_THRESHOLD	2346
#define DEFAULT_MAX_INACTIVITY_SECOND	60
#define DEFAULT_KEEP_ALIVE_SECOND	30

#define MAX_CHIP_SIZE	16
#define MAX_PATH_SIZE	256
#define MAX_IFACE_NUM_PER_RADIO	4

#define MAX_MODE_SIZE	16
#define MAX_STATUS_SIZE	16
#define MAX_CHANNEL_OPTION_NUM	32
#define MAX_BAND_SIZE	16
#define MAX_WIRELESS_MODE_SIZE	16
#define MAX_HTMODE_SIZE		8
#define MAX_COUNTRY_SIZE	3		//ISO 3166-1 alpha-2
#define MAX_TXPOWER_ADJECT_SIZE	16
#define MAX_TXPOWER_DBM_OPTION_NUM 8

#define MAX_IFNAME_SIZE		8
#define MAX_MACSTR_SIZE		19
#define MAX_IPSTR_SIZE		16
#define _MAX_SSID_SIZE		32
#define MAX_SSID_SIZE		(_MAX_SSID_SIZE+1)
#define MAX_ENCRYPTION_SIZE	32
#define MAX_KEY_SIZE		(64+1)
#define	MAX_MACFILTER_SIZE	(16)
#define	MAX_MACFILTER_LIST_SIZE (MAX_MACSTR_SIZE*MAX_ACL_NUM)
#define MAX_STA_NUM			64
#define MAX_ACL_NUM			64
#define MAX_WPS_METHOD		32
#define MAX_WPS_DEVICE_NAME_SIZE	(32+1)
#define MAX_WPS_MANUFACTURER_SIZE	(64+1)
#define MAX_WPS_MODEL_NAME_SIZE		(32+1)
#define MAX_WPS_MODEL_NUMBER_SIZE	(32+1)
#define MAX_WPS_SERIAL_NUMBER_SIZE	(32+1)

#define MAX_AUTH_NAME_SIZE	16


struct wifi_radio {
	unsigned int 		iface_num;
	struct wifi_iface*	iface[MAX_IFACE_NUM_PER_RADIO];
	unsigned int	changed; //flag of changed

	unsigned int	index;
	char			chip[MAX_CHIP_SIZE];
	char			driver_path[MAX_PATH_SIZE];
	char			config_path[MAX_PATH_SIZE];
	unsigned int 	enable;
	char			status[MAX_STATUS_SIZE];
	unsigned int	auto_disable;
	unsigned int	beacon_interval;
	unsigned int	channel;
	unsigned int	channel_option[MAX_CHANNEL_OPTION_NUM];
	unsigned int	channel_option_num;
	unsigned int	selected_channel;
	char			band[MAX_BAND_SIZE];
	char			wireless_mode[MAX_WIRELESS_MODE_SIZE];
	char			htmode[MAX_HTMODE_SIZE];
	char			country[MAX_COUNTRY_SIZE];
	char			txpower_ajdust[MAX_TXPOWER_ADJECT_SIZE];
	unsigned int	txpower_dBm;
	unsigned int	txpower_dBm_option[MAX_TXPOWER_DBM_OPTION_NUM];


	unsigned int	rts_threshold;
	unsigned int	frag_threshold;
	unsigned int	max_inactivity_second;
	unsigned int	keep_alive_second;
	unsigned int	isolate_iface;
	unsigned int	sta_maxnum;
	unsigned int	sta_num;
	char			sta_list[MAX_MACSTR_SIZE * MAX_STA_NUM];
	unsigned int	authenticator_enable;
	char			authenticator_daemon[MAX_AUTH_NAME_SIZE];
	//802.11n test plan
	unsigned int	coexistence_20_40_mhz;
	unsigned int	disallow_tkip_ht_rates;

};


struct wifi_iface {
	//radio
	struct wifi_radio *radio;
	unsigned int	changed; //flag of change

	unsigned int 	index;
	unsigned int 	enable;
	char		 	mode[MAX_MODE_SIZE];
	char		 	status[MAX_STATUS_SIZE];
	char		 	ifname[MAX_IFNAME_SIZE];
	char		 	bridge[MAX_IFNAME_SIZE];
	char		 	macaddr[MAX_MACSTR_SIZE];
	char		 	ssid[MAX_SSID_SIZE];
	unsigned int 	dtim_period;
	unsigned int 	short_preamble;
	unsigned int 	hidden;
	unsigned int 	isolate_sta;
	char		 	encryption[MAX_ENCRYPTION_SIZE];
	char		 	key[MAX_KEY_SIZE];
	unsigned int 	key_encrypted;
	char		 	key_wep[MAX_KEY_SIZE];
	unsigned int 	key_wep_index;
	char		 	key_psk[MAX_KEY_SIZE];
	char		 	macfilter[MAX_MACFILTER_SIZE];
	char		 	macfilter_list[MAX_MACFILTER_LIST_SIZE];
	unsigned int 	sta_maxnum;
	unsigned int 	sta_num;
	char		 	sta_list[MAX_MACSTR_SIZE * MAX_STA_NUM];
	unsigned int 	wmm;
	unsigned int 	wps;
	unsigned int	wps_not_support; //depend on "wps_support_encryption" and "encryption" and "hidden"
	char		 	wps_method[MAX_WPS_METHOD];
	char			wps_support_encryption[MAX_ENCRYPTION_SIZE * 8];
	unsigned int 	wps_configurated;
	char		 	wps_device_name[MAX_WPS_DEVICE_NAME_SIZE];
	char		 	wps_manufacturer[MAX_WPS_MANUFACTURER_SIZE];
	char		 	wps_model_name[MAX_WPS_MODEL_NAME_SIZE];
	char		 	wps_model_number[MAX_WPS_MODEL_NUMBER_SIZE];
	char		 	wps_serial_number[MAX_WPS_SERIAL_NUMBER_SIZE];
	char			radius_server_ipaddr[MAX_IPSTR_SIZE];
	unsigned int	radius_server_port;
	char			radius_server_key[MAX_KEY_SIZE];
};


struct wifi_setting {
	//global enable for all wifi function
	unsigned int	supported;	//profile_wifi
	unsigned int 	enable;
	unsigned int	changed;	//flag of change

	//radio
	unsigned int 		radio_num;
	struct wifi_radio 	*radio;
	//iface
	unsigned int 		iface_num;
	struct wifi_iface 	*iface;

	int 				debug_level;
};

int encryption_is_psk(char *encryption);
int encryption_is_wep(char *encryption);
int wireless_mode_is_include_N(char *wireless_mode);
int wireless_mode_is_include_AC(char *wireless_mode);

FILE *init_conf(const char *filename);
int set_conf_line(FILE *fp, const char *line);
int set_conf(FILE *fp, const char *key, const char *value);
int set_conf_int(FILE *fp, const char *key, int value);
void close_conf(FILE *fp);

//wps
int wps_set_status(struct wifi_iface *iface, char *status);

int wifi_get_setting(struct wifi_setting *setting);
int wifi_get_setting_changed(struct wifi_setting *setting);
struct wifi_iface* query_iface_by_ifname(struct wifi_setting *setting, const char *ifname);

#endif //_CONFIG_H
