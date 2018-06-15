#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> //close()

/***********************************
 *  wireless header
 * *********************************/
#include <net/if.h>
#define __user
#include <wireless.h> //struct iwreq
#undef __user


#include "common.h"
#include "cms.h"
#include "cms_helper.h"
#include "config.h"
#include "ralink_wps.h"


int get_socket(char *ifname)
{
	int sock = 0;
	struct iwreq wrq;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
		return -1;

	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name) - 1);

	if(ioctl(sock, SIOCGIFHWADDR, &wrq) < 0) {
		close(sock);
		return -1;
	} else {
		return sock;
	}

	return -1;
}

int valid_pin_code(char *pin_str)
{
	if(pin_str == NULL) {
		return -1;
	}

	long int pin_code = 0;

	if(strlen(pin_str) == 4) {
		//format example: "1234"
		//no need to checksum
		pin_code = strtol_default(pin_str, NULL, 10, -1);
		if(pin_code < 0) {
			return -1;
		} else {
			return 0; //ok
		}
	} else if(strlen(pin_str) == 9) {
		//format example: 1234-5678
		if(pin_str[4] == '-') {
			memmove(pin_str + 4, pin_str + 5, 5);
		}
	} else if(strlen(pin_str) == 8) {
		;
	} else {
		return -1;
	}

	//valid checksum
	pin_code = strtol_default(pin_str, NULL, 10, -1);
	if(pin_code < 0) {
		return -1;
	}

	int check_sum = 0;
	check_sum += 3 * ((pin_code / 10000000) % 10);
	check_sum += 1 * ((pin_code / 1000000) % 10);
	check_sum += 3 * ((pin_code / 100000) % 10);
	check_sum += 1 * ((pin_code / 10000) % 10);
	check_sum += 3 * ((pin_code / 1000) % 10);
	check_sum += 1 * ((pin_code / 100) % 10);
	check_sum += 3 * ((pin_code / 10) % 10);
	check_sum += 1 * ((pin_code / 1) % 10);
	check_sum = check_sum % 10;
	if(check_sum == 0) {
		return 0;
	}

	return -1; //fail
}

int query_wps_status(char *ifname, int sock)
{
	struct iwreq wrq;
	int wps_status = 0;

	(void) memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name) - 1);
	wrq.u.data.pointer = (void *) &wps_status;
	wrq.u.data.length = sizeof(wps_status);
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;
	if(ioctl(sock, RT_PRIV_IOCTL, &wrq) < 0) {
		fprintf(stderr, "%s(%s): unable to query wps status\n", __FUNCTION__, ifname);
		return -1;
	}

	//printf("wps status=%d\n",wps_status);
	return wps_status;
}


int get_wps_profile(char *ifname, int sock)
{
	struct iwreq wrq;
	WSC_CONFIGURED_VALUE Profile;

	//ioctl to fetch wps profile.
	memset(&wrq, 0x00, sizeof(wrq));
	strncpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name) - 1);
	wrq.u.data.pointer = (void *) &Profile;
	wrq.u.data.length = sizeof(Profile);
	if(ioctl(sock, RTPRIV_IOCTL_WSC_PROFILE, &wrq) < 0) {
		fprintf(stderr, "%s(%s): unable to get station information\n", __FUNCTION__, ifname);
		return -1;
	}

	//debug
	printf("WPS configure=%d (1 un-configured; 2 configured)\n", Profile.WscConfigured);
	printf("WPS SSID=%s\n", Profile.WscSsid);
	printf("WPS AuthMode=0x%02x (0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x20: wpa2-psk)\n", Profile.WscAuthMode);
	printf("WPS EncrypType=0x%02x (0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes)\n", Profile.WscEncrypType);
	printf("WPS DefaultKeyIdx=%d\n", Profile.WscEncrypType);
	printf("WPS WPAKey=%s \n", Profile.WscWPAKey);

	return 0;
}


int ralink_wps_routine(struct wifi_iface *iface)
{
	int ret = WPS_SUCCESS;
	char *ifname = iface->ifname;

	int sock = get_socket(ifname);
	if(sock < 0) {
		printf("%s is NOT a valid wifi device.\n", ifname);
		return WPS_FAIL;
	}

	int wps_status = 0;
	int timeout = 120;

	wps_set_status(iface, "waiting");
	SET_WPS_LED(FEQ_4HZ);
	while(1) {
		//timeout
		if(timeout == 0) {
			wps_set_status(iface, "timeout");
			printf("timeout!!\n");
			ret = WPS_TIMEOUT;
			break;
		}

		//query wps status
		wps_status = query_wps_status(ifname, sock);
		printf("status=%d (time=%d)\n", wps_status, timeout);

		//if done
		if(wps_status == STATUS_WSC_CONFIGURED) {
			wps_set_status(iface, "success");
			//write wps profile back
			get_wps_profile(ifname, sock);
			ret = WPS_SUCCESS;
			break;
		}
		timeout--;
		sleep(1);
	}
	SET_WPS_LED(FEQ_2HZ);
	close(sock);

	return ret;
}

int do_wps_stop_last_session(struct wifi_setting *setting, struct wifi_iface *wps_iface)
{
	do_system("(pgrep -f wifi_wps | grep -v %d | xargs kill)>/dev/null 2>&1", getpid());//kill all but itself
	do_system("wifi.sh wps_led stop");//SET_WPS_LED(FEQ_2HZ);

	//set other wps status as stop
	struct wifi_iface *iface = NULL;
	array_for_each_entry_size(iface, setting->iface, setting->iface_num) {
		if(iface != wps_iface) {
			wps_set_status(iface, "stop");
		}
	}

	return 0;
}

int do_wps_stop(char *ifname)
{
	do_system("iwpriv %s set WscConfMode=0", ifname);
	do_system("iwpriv %s set WscConfMode=6", ifname);
	return 0;
}

int do_wps_pbc(char *ifname)
{
	do_system("iwpriv %s set WscConfMode=6", ifname);
	do_system("iwpriv %s set WscConfStatus=2", ifname);
	do_system("iwpriv %s set WscMode=2", ifname);
	do_system("iwpriv %s set WscGetConf=1", ifname);
	return 0;
}

int do_wps_pin(char *ifname, char *pin_str)
{
	if(valid_pin_code(pin_str) != 0) {
		printf("invalid pin code\n");
		return -1;
	}
	do_system("iwpriv %s set WscConfMode=6", ifname);
	do_system("iwpriv %s set WscConfStatus=2", ifname);
	do_system("iwpriv %s set WscMode=1", ifname);
	do_system("iwpriv %s set WscPinCode='%s'", ifname, pin_str);
	do_system("iwpriv %s set WscGetConf=1", ifname);

	return 0;
}

int ralink_wps_usage(void)
{
	printf("Usage:\n");
	printf("    wifi_wps [interface] stop            # stop\n");
	printf("    wifi_wps [interface] pbc             # start wbc\n");
	printf("    wifi_wps [interface] pin [pin_code]  # start pin\n");
	return 0;
}

int ralink_wps(int argc, char *argv[])
{
	if(argc < 3) {
		ralink_wps_usage();
		return -1;
	}

	char *ifname = argv[1];
	char *subcmd = argv[2];
	int ret = -1;

	struct wifi_setting setting = {0};
	wifi_get_setting(&setting);
	struct wifi_iface *iface = query_iface_by_ifname(&setting, ifname);
	if(iface == NULL) {
		fprintf(stderr, "Invalid interface to do WPS: %s\n", ifname);
		return -1;
	}

	set_debug_level(1);


	openlog("Wi-Fi", LOG_PID, 0);
	if(iface->wps == 0) {
		wps_set_status(iface, "disabled");
		fprintf(stderr, "Error: wps is disabled\n");
	} else if(iface->wps_not_support == 1) {
		wps_set_status(iface, "encryption_not_support");
		fprintf(stderr, "Error: encryption not support\n");
	} else {
		//wps == 1 && wps_not_support == 0
		do_wps_stop_last_session(&setting, iface);
		wps_set_status(iface, "deactivate");
		if(strcmp(subcmd, "stop") == 0) {
			syslog(LOG_INFO, "Successfully stop WPS");
			do_wps_stop(ifname);
			wps_set_status(iface, "stop");
			SET_WPS_LED(FEQ_2HZ);
			return 0;
		} else if(strcmp(subcmd, "pbc") == 0) {
			syslog(LOG_INFO, "Successfully start WPS PBC");
			ret = do_wps_pbc(ifname);
		} else if(strcmp(subcmd, "pin") == 0) {
			if(argc == 4) {
				syslog(LOG_INFO, "Successfully start WPS PIN");
				char* pin_str = argv[3];
				ret = do_wps_pin(ifname, pin_str);
			} else {
				fprintf(stderr, "Error: missing pin_code\n");
				return -1;
			}
		}

		if(ret == 0) {
			ret = ralink_wps_routine(iface);
			if(ret == WPS_SUCCESS) {
				syslog(LOG_INFO, "WPS done");
			} else if(ret == WPS_FAIL) {
				syslog(LOG_INFO, "WPS fail");
			} else if(ret == WPS_TIMEOUT) {
				syslog(LOG_INFO, "WPS timeout");
			} else {
				fprintf(stderr, "unkown wps status\n");;
			}
		}
	}


	return  -1;
}
