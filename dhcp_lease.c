#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dhcp_lease.h"

struct lease_info *query_lease_info_mac(char *lease_file, char *macstr)
{
	static struct lease_info lease;
	memset(&lease, 0x00, sizeof(lease));

	FILE *fp = fopen(lease_file, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error: fail to open leasefile: %s\n", lease_file);
		return &lease;
	}

	int found = 0;
	do {
		//parse lease info
		int num = fscanf(fp, "%lu %255s %16s %255s %764s", &(lease.expires), lease.macstr, lease.ipstr, lease.hostname, lease.clientid);
		if(num != 5) {
			break;
		}

		//treat '*' as empty string
		if(strcmp(lease.hostname, "*") == 0) {
			lease.hostname[0] = 0;
		}
		if(strcmp(lease.clientid, "*") == 0) {
			lease.clientid[0] = 0;
		}

		//compare macstr with lease info
		if(strcasecmp(lease.macstr, macstr) == 0) {
			found = 1;
			break;
		}
	} while(1);

	fclose(fp);

	if(found == 0) {
		memset(&lease, 0x00, sizeof(lease));
	}

	return &lease;
}
