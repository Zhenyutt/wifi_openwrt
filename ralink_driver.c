#include "ralink_driver.h"


char* get_phy_mode_mt7603(int Mode)
{
	switch(Mode) {
	case MODE_CCK:
		return "802.11b";

	case MODE_OFDM:
		return "802.11g";

	case MODE_HTMIX:
		return "802.11n";

	case MODE_HTGREENFIELD:
		return "GREEN";

	default:
		return "N/A";
	}
}


char* get_phy_mode_mt7612(int Mode)
{
	switch(Mode) {
	case MODE_CCK:

	case MODE_OFDM:
		return "802.11a";

	case MODE_HTMIX:
		return "802.11an";

	case MODE_HTGREENFIELD:
		return "GREEN";

	case MODE_VHT:
		return "802.11ac";

	default:
		return "N/A";
	}
}

char* get_BW(int BW)
{
	switch(BW) {
	case BAND_WIDTH_10:
		return "10M";

	case BAND_WIDTH_20:
		return "20M";

	case BAND_WIDTH_40:
		return "40M";

	case BAND_WIDTH_80:
		return "80M";

	default:
		return "N/A";
	}
}

int get_rate(HTTRANSMIT_SETTING HTSetting)
{
	int MCSMappingRateTable[] = {
		2, 4, 11, 22, /* CCK*/
		12, 18, 24, 36, 48, 72, 96, 108, /* OFDM*/
		13, 26, 39,  52, 78, 104, 117, 130, 26, 52, 78, 104, 156, 208, 234, 260,        /* 20MHz, 800ns GI, MCS: 0 ~ 15*/
		39, 78, 117, 156, 234, 312, 351, 390,                                           /* 20MHz, 800ns GI, MCS: 16 ~ 23*/
		27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,     /* 40MHz, 800ns GI, MCS: 0 ~ 15*/
		81, 162, 243, 324, 486, 648, 729, 810,                                          /* 40MHz, 800ns GI, MCS: 16 ~ 23*/
		14, 29, 43,  57, 87, 115, 130, 144, 29, 59, 87, 115, 173, 230, 260, 288,        /* 20MHz, 400ns GI, MCS: 0 ~ 15*/
		43, 87, 130, 173, 260, 317, 390, 433,                                           /* 20MHz, 400ns GI, MCS: 16 ~ 23*/
		30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,     /* 40MHz, 400ns GI, MCS: 0 ~ 15*/
		90, 180, 270, 360, 540, 720, 810, 900,                                          /* 40MHz, 400ns GI, MCS: 16 ~ 23*/
		13, 26, 39, 52, 78, 104, 117, 130, 156,                                         /* 11ac: 20Mhz, 800ns GI, NSS 0, MCS: 0~8 */
		26, 52, 78, 104, 156, 208, 234, 260, 312,                                       /* 11ac: 20Mhz, 800ns GI, NSS 1, MCS: 0~8 */
		27, 54, 81, 108, 162, 216, 243, 270, 324, 360,                                  /*11ac: 40Mhz, 800ns GI, NSS 0, MCS: 0~9 */
		54, 108, 162, 216, 324, 432, 486, 540, 648, 720,                                /*11ac: 40Mhz, 800ns GI, NSS 1, MCS: 0~9 */
		59, 117, 176, 234, 351, 468, 527, 585, 702, 780,                                /*11ac: 80Mhz, 800ns GI, NSS 0, MCS: 0~9 */
		117, 234, 351, 468, 702, 936, 1053, 1179, 1404, 1560,                           /*11ac: 80Mhz, 800ns GI, NSS 1, MCS: 0~9 */
		14, 29, 43,  57, 87, 115, 130, 144, 173,                                        /* 11ac: 20Mhz, 400ns GI, NSS 0, MCS: 0~8 */
		29, 57, 87,  115, 173, 231, 261, 289, 347,                                      /* 11ac: 20Mhz, 400ns GI, NSS 1, MCS: 0~8 */
		30, 60, 90, 120, 180, 240, 270, 300, 360, 400,                                  /*11ac: 40Mhz, 400ns GI, NSS 0, MCS: 0~9 */
		60, 120, 180, 240, 360, 480, 540, 600, 720, 800,                                /*11ac: 40Mhz, 400ns GI, NSS 1, MCS: 0~9 */
		65, 130, 195, 260, 390, 520, 585, 650, 780, 867,                                /*11ac: 80Mhz, 400ns GI, NSS 0, MCS: 0~9 */
		130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1733                           /*11ac: 80Mhz, 400ns GI, NSS 1, MCS: 0~9 */
	};
	int rate_index = 0;
	int rate_count = sizeof(MCSMappingRateTable) / sizeof(int);


	if(HTSetting.field.MODE >= MODE_VHT) {
		if(HTSetting.field.BW == BAND_WIDTH_20) {
			rate_index = 108 + (HTSetting.field.ShortGI * 58) + ((HTSetting.field.MCS >> 4) * 9) + (HTSetting.field.MCS & 0xf);
		} else if(HTSetting.field.BW == BAND_WIDTH_40) {
			rate_index = 126 + (HTSetting.field.ShortGI * 58) + ((HTSetting.field.MCS >> 4) * 10) + (HTSetting.field.MCS & 0xf);
		} else if(HTSetting.field.BW == BAND_WIDTH_80) {
			rate_index = 146 + (HTSetting.field.ShortGI * 58) + ((HTSetting.field.MCS >> 4) * 10) + (HTSetting.field.MCS & 0xf);
		}
	} else if(HTSetting.field.MODE >= MODE_HTMIX) {
		rate_index = 12 + (HTSetting.field.BW * 24) + (HTSetting.field.ShortGI * 48) + (HTSetting.field.MCS);
	} else if(HTSetting.field.MODE == MODE_OFDM) {
		rate_index = HTSetting.field.MCS + 4;
	} else if(HTSetting.field.MODE == MODE_CCK) {
		rate_index = HTSetting.field.MCS;
	}

	if(rate_index < 0) {
		rate_index = 0;
	} else if(rate_index >= rate_count) {
		rate_index = rate_count - 1;
	}

	return (MCSMappingRateTable[rate_index] * 5) / 10;
}

int get_best_rssi(int rssi0, int rssi1, int rssi2)
{
	int rssi = -999;

	if(rssi0 != 0) {
		rssi = rssi0;
	}

	if((rssi1 != 0) && (rssi1 > rssi)) {
		rssi = rssi1;
	}

	if((rssi2 != 0) && (rssi2 > rssi)) {
		rssi = rssi2;
	}
	return rssi;
}
