#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "common.h"
#include "config.h"
/*
#include "ralink.h"
#include "ralink_wps.h"
#include "ralink_logger.h"
#include "ralink_ap_scan.h"
#include "ralink_sta_list.h"
*/
#include "openwrt.h"

struct cmd_func {
	char *proc_name;
	char *subcmd;
	int (*func)(int, char **);
};


struct cmd_func cmd_func_list[] = {
	{ NULL, "restart",  openwrt_restart },
	//{ NULL, "stop",  ralink_stop },
	//{ NULL, "ap_scan",  ralink_ap_scan },
	//{ "wifi_ap_scan", "ap_scan",  ralink_ap_scan },
	//{ NULL, "sta_list",  ralink_sta_list },
	//{ "wifi_sta_list", "sta_list",  ralink_sta_list },
	//{ "wifi_wps", NULL, ralink_wps },
	//{ "wifi_logger", NULL, ralink_logger },
};



int main(int argc, char* argv[])
{
	//simulate
	cms_init();
	cms_config_import("wifi.conf");
	
	char *prog_name = NULL;

	prog_name = basename(argv[0]);
	if(strcmp(prog_name, "wifi") == 0 && argc >= 2) {
		argc--;
		argv++;
		struct cmd_func *p = NULL;
		char* subcmd = basename(argv[0]);
		array_for_each_entry(p, cmd_func_list) {
			if(p->subcmd != NULL && strcmp(subcmd, p->subcmd) == 0) {
				return p->func(argc, argv);
			}
		}
	} else {
		struct cmd_func *p = NULL;
		prog_name = basename(argv[0]);
		array_for_each_entry(p, cmd_func_list) {
			if(p->proc_name != NULL && strcmp(prog_name, p->proc_name) == 0) {
				return p->func(argc, argv);
			}
		}
	}

	return -1;
}
