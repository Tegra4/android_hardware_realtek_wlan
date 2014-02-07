/*
 * Copyright [2011] The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
*/

#define LOG_TAG "wlan_prov"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/misc.h>
#include <cutils/android_reboot.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef BUILD_WITH_CHAABI_SUPPORT
#include "umip_access.h"
#elif BUILD_WITH_TOKEN_SUPPORT
#include "tee_token_if.h"
#define WLAN_TOKEN_DG_ID		11	/* Group ID */
#define WLAN_TOKEN_SG_ID		10	/* Subgroup ID */
#define WLAN_TOKEN_ITEM_ID		1	/* Item ID */
#endif

#define MAC_ADDRESS_LEN 6
const unsigned char NullMacAddr[MAC_ADDRESS_LEN] = { 0, 0, 0, 0, 0, 0 };

#define BUFF_SIZE 40
const char CAL_file_name[] = "/config/wifi/mac.txt";

static int get_prov_mac_address(unsigned char** macAddr);
static int cal_update_mac_file(unsigned char *MacAddr);
static int cal_check_mac_file_exist();
static void cal_randomize_mac(unsigned char *MacAddr);
static int cal_parse_buff(char *buff, unsigned char *macAddr);

int main(int argc, char **argv)
{
	FILE *calBinFile = NULL;
	unsigned char CalMacAddr[MAC_ADDRESS_LEN];
	unsigned char *ProvMacAddr = NULL;
	int mac_line;
	int res = 0;

	/* Check parameters */
	if (argc != 1) {
		/* No param expected */
		LOGE("No param expected");
		return -1;
	}

	/* Read MAC address from Chaabi */
	if (get_prov_mac_address(&ProvMacAddr) < 0) {
		/* read error*/
		LOGI("Provisioned MAC not found, request to randomize one");
		ProvMacAddr = (unsigned char *) malloc(MAC_ADDRESS_LEN);
		if (ProvMacAddr == NULL) {
			res = -1;
			goto fatal;
		}
		memcpy(ProvMacAddr, NullMacAddr, MAC_ADDRESS_LEN);
	} else
		LOGI("Provisioned Mac address:  %02x:%02x:%02x:%02x:%02x:%02x",
					ProvMacAddr[0], ProvMacAddr[1], ProvMacAddr[2],
					ProvMacAddr[3], ProvMacAddr[4], ProvMacAddr[5]);

	/* Verified if mac randomize is really necessary */
	if (ProvMacAddr && (memcmp(ProvMacAddr, NullMacAddr, MAC_ADDRESS_LEN) == 0)) {
		/* Provisioned MAC address is null due to engineering mode or read error */
		if (cal_check_mac_file_exist() > 0){
			LOGI("mac address exist, Nothing to do..");
			goto end;
		}

		LOGI("Mac address doesn't exist, randomize one.");
		cal_randomize_mac(ProvMacAddr);
	}

	if (ProvMacAddr && (cal_update_mac_file(ProvMacAddr) < 0))
		goto fatal;

end:
	LOGI("End ...");
	sync();

	if(ProvMacAddr)
	    free(ProvMacAddr);

	return res;
fatal:
	sync();
	LOGE("Fatal MAC file not found, using default MAC address.");
	return res;
}

static int cal_check_mac_file_exist(){
	FILE *fp;
	char buff[BUFF_SIZE];
	int ret;
	unsigned char macAddr[MAC_ADDRESS_LEN];

	LOGI("%s Enter", __FUNCTION__);
	fp = fopen(CAL_file_name, "r");

	if (fp == NULL){
		LOGE("Error on opening mac file");
		return -1;
	}

	if( fgets(buff, BUFF_SIZE, fp) == NULL ) {
		fclose(fp);
		LOGE("error on fget");
		return -1;
	}
	fclose(fp);

	return cal_parse_buff(buff, macAddr);
}

/* create the mac file with the mac address MacAddr */
static int cal_create_new_mac_file(unsigned char *MacAddr){
	char buff[BUFF_SIZE];
	int ret, fd;

	LOGI("%s Enter", __FUNCTION__);

	if (MacAddr == NULL)
		return -1;

	ret = sprintf(buff, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n", MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
	if (ret != (MAC_ADDRESS_LEN*2)+1 ){
		LOGE("error on writing into the buffer");
		return -1;
	}

	fd = open(CAL_file_name, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW , 0664);
	if (fd < 0) {
		LOGE("error on open the file %s", CAL_file_name);
		return -1;
	}

	if (write(fd, buff, strlen(buff)) < 0) {
		LOGE("error on writing on file %s", CAL_file_name);
		close(fd);
		return -1;
	}

	if (fchmod(fd, 0664) < 0) {
		LOGE("error on changing permission on file %s", CAL_file_name);
		close(fd);
		return -1;
	}

	close(fd);
	return 1;
}

/* get the provisioned mac address from secured partition */
/* depending of technology used */
static int get_prov_mac_address(unsigned char** macAddr) {
	int ret = 0;
#ifdef BUILD_WITH_CHAABI_SUPPORT
	/* Read MAC address from Chaabi */
	ret = get_customer_data(ACD_WLAN_MAC_ADDR_FIELD_INDEX,
									(void ** const) macAddr);
	if (ret != MAC_ADDRESS_LEN) {
		LOGI("get_customer_data error: 0x%x", ret);
		if (ret >0) {
			free(*macAddr);
			*macAddr = NULL;
		}
		return -1;
	}
#elif BUILD_WITH_TOKEN_SUPPORT
	*macAddr = malloc(sizeof(unsigned char) * MAC_ADDRESS_LEN);
	ret = tee_token_item_read(WLAN_TOKEN_DG_ID, WLAN_TOKEN_SG_ID, WLAN_TOKEN_ITEM_ID,
								0, *macAddr, MAC_ADDRESS_LEN, 0);
	if (ret != 0) {
		LOGI("tee_token_item_read error: 0x%x", ret);
		free(*macAddr);
		*macAddr = NULL;
		return -1;
	}
#else
	ret = -1;
#endif
	return ret;
}

/* update the mac file with the mac address MacAddr */
static int cal_update_mac_file(unsigned char *MacAddr)
{
	FILE *fp;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	char buff[BUFF_SIZE];
	int ret= 0, i;

	LOGI("%s Enter", __FUNCTION__);
	fp = fopen(CAL_file_name, "r");

	if (fp == NULL){
		LOGI("mac file does not exist, create a new one");
		return cal_create_new_mac_file(MacAddr);
	}

	if ( fgets(buff, BUFF_SIZE, fp) == NULL){
		fclose(fp);
		LOGE("error on fget");
		return cal_create_new_mac_file(MacAddr);
	}

	fclose(fp);

	if (cal_parse_buff(buff, macAddr)) {
		if (memcmp(MacAddr, macAddr, MAC_ADDRESS_LEN) != 0) {
			LOGI("mac from file is is not aligned, mac has to be updated");
			return cal_create_new_mac_file(MacAddr);
		}
		else {
			LOGI("mac from file is aligned, nothing to do");
			return 1;
		}
	}

	LOGI("error on parsing mac, mac has to be updated");

	return cal_create_new_mac_file(MacAddr);
}

/*generate new MAC address randomly */
static void cal_randomize_mac(unsigned char *MacAddr){

	struct timeval tv;

	LOGI("%s Enter", __FUNCTION__);

	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	MacAddr[0] = 0x00;
	MacAddr[1] = 0x09;
	MacAddr[2] = 0x4C;
	MacAddr[3] = (unsigned char) (rand() & 0xff);
	MacAddr[4] = (unsigned char) (rand() & 0xff);
	MacAddr[5] = (unsigned char) (rand() & 0xff);
}

/* Parse and covert the buff array into the macAddr array*/
static int cal_parse_buff(char *buff, unsigned char *macAddr){

	if( sscanf(buff, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", &macAddr[0], &macAddr[1], &macAddr[2], &macAddr[3], &macAddr[4], &macAddr[5]) == 6){
			LOGI("parsed mac %02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
			return 1;
	}

	LOGI("Problems on parsing the mac address");
	return -1;
}
