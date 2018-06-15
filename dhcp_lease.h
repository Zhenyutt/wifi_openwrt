#ifndef _DHCP_LEASE_H
#define _DHCP_LEASE_H

#define DEFAULT_LEASE_FILE "/var/dnsmasq.leases"


struct lease_info {
	unsigned long expires;
	char macstr[MAX_MACSTR_SIZE];
	char ipstr[MAX_IPSTR_SIZE];
	char hostname[256];
	char clientid[764];
};

struct lease_info *query_lease_info_mac(char *lease_file, char *macstr);


#endif //_DHCP_LEASE_H


