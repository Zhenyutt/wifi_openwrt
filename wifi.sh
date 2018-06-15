#!/bin/sh


export WIRELESS_RESET_INTERVAL=10


get_wifi_all_ifname()
{
	local wifi_iface_num=$(cmscfg -n wifi_iface_num -gg)
	local ifnames=""
	for iface_idx in $(seq 0 $(($wifi_iface_num-1))); do
		ifname=$(cmscfg -n "wifi_iface${iface_idx}_ifname" -ggg)
		if [ -z $ifname ]; then
			continue
		fi
		ifnames="$ifnames $ifname"
	done
	echo "$ifnames"
}


wifi_disconnect_reset_all()
{
	local reset_interval=${1:-$WIRELESS_RESET_INTERVAL}
	local ifnames=$(get_wifi_all_ifname)
	## disconnect all sta
	for disconnect_idx in $(seq 1 3); do
		for ifname in $ifnames; do
			iwpriv $ifname set DisConnectAllSta=1 >/dev/null 2>&1
		done
	done

	## shutdown all iface
	for ifname in $ifnames; do
		iwpriv $ifname set RadioOn=0 >/dev/null 2>&1
	done

	## reset interval 10s to ensure sta renew ip
	## case1: windows7, broadcom 802.11n, total need 7~12s
	## case2: linux mint17.1, intel N6235, total need >10s
	sleep $reset_interval;

	## reset all iface
	for ifname in $ifnames; do
		iwpriv $ifname set RadioOn=1 >/dev/null 2>&1
	done

}

WIFI_TMP_CONF=/tmp/wifi.conf
WIFI_CONF=/mnt/jffs2/conf/user/wifi.conf
wifi_get_changed()
{
	local old_conf="${1:-$WIFI_TMP_CONF}"
	local new_conf="${2:-$WIFI_CONF}"

	[ -f "$old_conf" ] || touch "$old_conf"
	diff "$old_conf" "$new_conf" \
		| sed -e '1,2 d'                                       `: # strip header result` \
		| sed -E -n -e '/^\+[^#]/ s/\+(wifi[^=]+)=.*/\1/p'     `: # filter wifi_* key name` \
		| grep -E -o "wifi|wifi_radio[0-9]+|wifi_iface[0-9]+"  `: # generate sets: wifi wifi_radioX wifi_ifaceX` \
		| sort | uniq                                          `: # remove duplicate`
}


wifi_update_changed()
{
	cp -f "$WIFI_TMP_CONF" "$WIFI_TMP_CONF".bak
	cp -f "$WIFI_CONF" "$WIFI_TMP_CONF"
}


wifi_flush_changed()
{
	rm -f "$WIFI_TMP_CONF" "$WIFI_TMP_CONF".bak
}


wifi_ui_mfg_dump()
{
	local select_band="${1}"
	local select_type="${2}"

	local wifi_iface_num=$(cmscfg -n wifi_iface_num -gg)
	for iface_idx in $(seq 0 $(($wifi_iface_num-1))); do
		local radio_idx=$(cmscfg -n "wifi_iface${iface_idx}_radio_index" -ggg)
		local radio_band=$(cmscfg -n "wifi_radio${radio_idx}_band" -ggg)
		if [ "$select_band" != "$radio_band" ]; then
			continue
		fi

		if [ "$select_type" = "ssid" ]; then
			local ssid="$(cmscfg -n "wifi_iface${iface_idx}_ssid" -ggg)"
			echo $ssid
		elif [ "$select_type" = "mac" ]; then
			local mac=$(cmscfg -n "wifi_iface${iface_idx}_macaddr" -ggg)
			echo $mac
		elif [ "$select_type" = "password" ]; then
			local password="$(cmscfg -n "wifi_iface${iface_idx}_key" -ggg)"
			echo $password
		fi
	done
}


wifi_usage()
{
cat<<EOF
Usage: (disconnect_reset_all)
	## disconnect all sta and reset all interface by reset interval times(s).
	$0 disconnect_reset_all [reset_interval: default=$WIRELESS_RESET_INTERVAL]
	Example:
		$0 disconnect_reset_all $WIRELESS_RESET_INTERVAL
Usage:
	$0 get_changed       # get changed wifi config
	$0 update_changed    # update changed wifi config
	$0 flush_changed     # flush changed wifi config

Usage: (ui_mfg_dump)
	## mfg tool to dump some wifi setting for ui only
	## band="2.4GHz"|"5GHz", type="ssid"|"mac"|"password"
	$0 ui_mfg_dump [band] [type]
	Example:
		$0 ui_mfg_dump 2.4GHz ssid

Usage:
	## should be used in button event only
	$0 wps_pbc                  # trigger iface0 wps

Usage:
	$0 wan_changed              # restart radius auth daemon


EOF
}


wifi_wps_pbc()
{
	ifname=$(cmscfg -n wifi_iface0_ifname -ggg)
	if [ -z "$ifname" ]; then
		echo "Error: no valid wireless interface to do WPS PBC" >&2
		exit 1
	fi

	wifi_wps "$ifname" stop
	wifi_wps "$ifname" pbc &
}

wifi_wan_changed()
{
	local wifi_iface_num=$(cmscfg -n wifi_iface_num -gg)
	local encryption=""
	local ifnames=""
	local own_ip_addr=$(cmscfg -n wan_ip_1 -gg)

	for iface_idx in $(seq 0 $(($wifi_iface_num-1))); do
		encryption=$(cmscfg -n "wifi_iface${iface_idx}_encryption" -ggg)
		ifname=$(cmscfg -n "wifi_iface${iface_idx}_ifname" -ggg)
		if [ -n "$(echo $encryption | grep wpa)" ]; then
			iwpriv $ifname set own_ip_addr=$own_ip_addr
		fi
	done

	local wifi_radio_num=$(cmscfg -n wifi_radio_num -gg)
	local authenticator=""
	for radio_idx in $(seq 0 $(($wifi_radio_num-1))); do
		## restart authenticator daemon
		authenticator_daemon=$(cmscfg -n "wifi_radio${iface_idx}_authenticator_daemon" -ggg)
		killall -SIGUSR1 $authenticator_daemon
	done
}


main()
{
	subcmd=$1
	shift
	case $subcmd in
		disconnect_reset_all)
			wifi_disconnect_reset_all "$@"
			;;
		#### wifi config change tool ####
		get_changed)
			wifi_get_changed "$@"
			;;
		update_changed)
			wifi_update_changed "$@"
			;;
		flush_changed)
			wifi_flush_changed "$@"
			;;
		#### ui tool ####
		ui_mfg_dump)
			wifi_ui_mfg_dump "$@"
			;;
		wps_pbc)
			wifi_wps_pbc "$@"
			;;
		eeprom_fixup)
			wifi_eeprom_fixup "$@"
			;;
		#### wan change tool ####
		wan_changed)
			wifi_wan_changed "$@"
			;;
		*)
			wifi_usage
			;;
	esac
}

main "$@"
