#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>


#include "common.h"
#include "cms.h"
#include "cms_helper.h"
#include "config.h"



#define STA_PREFIX	"STA("
int logger_message_parser(struct wifi_setting *setting, char *msg)
{
	// example msg:
	//     00:11:22.333333   ra0      Custom driver event:(RT2860) STA(xx:xx:xx:xx:xx:xx) message body
	if(strstr(msg, "Custom driver event:") == NULL) {
		return -1; //not ralink message
	}

	//move pointer to iface
	msg = msg + 18;
	char *ifname = strtok_r(NULL, " ", &msg);
	if(ifname == NULL || msg == NULL) {
		return -1;
	}

	//get ifname and iface
	struct wifi_iface *iface = query_iface_by_ifname(setting, ifname);
	if(iface == NULL) {
		return -1; //no valid inface
	}

	//get mac string from "STA(xx:xx:xx:xx:xx:xx)"
	char mac_str[18] = {0};
	char *p = strstr(msg, STA_PREFIX);
	if(p) {
		strncpy(mac_str, p + strlen(STA_PREFIX), sizeof(mac_str) - 1);
	}

	static char assoc_mac_str[18] = {0};
	static time_t assoc_time = 0;
	static char fail_mac_str[18] = {0};
	static time_t fail_time = 0;

	if(strcmp(mac_str, "00:00:00:00:00:00") == 0) {
		if(strstr(msg, " connects with our wireless client") != NULL) {
			//ap start
			syslog(LOG_INFO, "Successfully start AP %s\n", iface->radio->band);
		} else if(strstr(msg, " disconnects with our wireless client") != NULL) {
			//ap stop
			syslog(LOG_INFO, "Successfully stop AP %s\n", iface->radio->band);
		}
	} else {
		if(strstr(msg, "had associated successfully") != NULL) {
			//join
			time_t now = time(NULL);
			if(strcasecmp(mac_str, assoc_mac_str) == 0 && now <= assoc_time + 3) {
				; //skip to log duplicate message
			} else {
				syslog(LOG_INFO, "Successfully associate with client %s\n", mac_str);
			}
			snprintf(assoc_mac_str, sizeof(assoc_mac_str), "%s", mac_str);
			assoc_time = now;
		} else if(strstr(msg, "had disassociated") != NULL
		          || strstr(msg, "had deauthenticated") != NULL
		          || strstr(msg, "had been aged-out and disassociated") != NULL) {
			//leave
			memset(assoc_mac_str, 0x00, sizeof(assoc_mac_str));
			memset(fail_mac_str, 0x00, sizeof(fail_mac_str));
			syslog(LOG_INFO, "Successfully disassociate with client %s\n", mac_str);
		} else if(strstr(msg, "occurred MIC different in Key Handshaking") != NULL
		          || strstr(msg, "occurred ICV error in RX") != NULL) {
			//unauth or wrong password
			time_t now = time(NULL);
			if(strcasecmp(mac_str, fail_mac_str) == 0 && now <= fail_time + 3) {
				; //skip to log duplicate message
			} else {
				syslog(LOG_WARNING, "Failed to connect unauthorized client %s\n", mac_str);
			}
			snprintf(fail_mac_str, sizeof(fail_mac_str), "%s", mac_str);
			fail_time = now;
		} else if(strstr(msg, "set key done in WPA/WPAPSK") != NULL
		          || strstr(msg, "set key done in WPA2/WPA2PSK") != NULL) {
			//authed
			syslog(LOG_INFO, "Successfully connect authorized client %s\n", mac_str);
		} else if(strstr(msg, "channel switch to") != NULL) {
			//auto channel result
			msg = strstr(msg, "channel switch to") + strlen("channel switch to");
			unsigned int selected_channel = strtoul_default(msg, NULL, 10, 0);
			if(selected_channel > 0) {
				cms_set_uint("wifi_radio%d_selected_channel", selected_channel, iface->radio->index);
			}
		}
	}
	return 0;
}


int ralink_logger(int argc, char *argv[])
{

	FILE *stream = popen("iwevent 2>&1", "re");
	if(stream == NULL) {
		fprintf(stderr, "fail to start iwevent to be monitor\n");
		return -1;
	}

	struct wifi_setting setting = {0};
	wifi_get_setting(&setting);
	openlog("Wi-Fi", LOG_PID, 0);

	char line[1024] = {0};
	while(fgets(line, sizeof(line), stream) != NULL) {
		//strip tailing newline
		char *nl = strchr(line, '\n');
		if(nl) {
			*nl = '\0';
		}

		//fprintf(stderr, "[%s]\n", line);
		logger_message_parser(&setting, line);
	}
	pclose(stream);

	return 0;
}
