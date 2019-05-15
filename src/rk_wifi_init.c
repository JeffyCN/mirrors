#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <dirent.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#define WIFI_CHIP_TYPE_PATH	"/sys/class/rkwifi/chip"
#define WIFI_POWER_INF		"/sys/class/rkwifi/power"
#define WIFI_DRIVER_INF		"/sys/class/rkwifi/driver"
#define WIFI_PRELOAD_INF	"/sys/class/rkwifi/preload"

#define UNKKOWN_DRIVER_MODULE_ARG ""

#define BT_TTY_DEV "/dev/ttyS4"

int check_wifi_chip_type(void);
int check_wifi_chip_type_string(char *type);
int rk_wifi_power_ctrl(int on);
int rk_wifi_load_driver(int enable);
int check_wireless_ready(void);
int get_kernel_version(void);

static int identify_sucess = -1;
static char recoginze_wifi_chip[64];
static const char USB_DIR[] = "/sys/bus/usb/devices";
static const char SDIO_DIR[]= "/sys/bus/sdio/devices";
static const char PCIE_DIR[]= "/sys/bus/pci/devices";

static const char PREFIX_SDIO[] = "SDIO_ID=";
static const char PREFIX_PCIE[] = "PCI_ID=";
static const char PREFIX_USB[] = "PRODUCT=";

static int invalid_wifi_device_id = -1;
static char wifi_type[64] = {0};
static char *bt_firmware_patch = {0};
static int wifi_dirver_is_loaded = -1;
static char bt_tty_dev[64] = { 0 };

typedef struct _wifi_devices
{
  char wifi_name[64];
  char wifi_vid_pid[64];
} wifi_device;

typedef struct _wifi_ko_file_name
{
	char wifi_name[64];
	char wifi_module_path[128];
	char wifi_module_arg[128];
	char bt_firmware_path[128];

} wifi_ko_file_name;

static wifi_device supported_wifi_devices[] = {
	{"RTL8188EU",	"0bda:8179"},
	{"RTL8188EU",	"0bda:0179"},
	{"RTL8723BU",	"0bda:b720"},
	{"RTL8723BS",	"024c:b723"},
	{"RTL8822BS",	"024c:b822"},
	{"RTL8723CS",	"024c:b703"},
	{"RTL8723DS",	"024c:d723"},
	{"RTL8188FU",	"0bda:f179"},
	{"RTL8822BU",	"0bda:b82c"},
	{"RTL8189ES",	"024c:8179"},
	{"RTL8189FS",	"024c:f179"},
	{"RTL8192DU",	"0bda:8194"},
	{"RTL8812AU",	"0bda:8812"},
	{"AP6354",		"02d0:4354"},
	{"AP6330",		"02d0:4330"},
	{"AP6356S",		"02d0:4356"},
	{"AP6398S",		"02d0:4359"},
	{"AP6335",		"02d0:4335"},
	{"AP6255",      "02d0:a9bf"},
	{"AP6212A",     "02d0:a9a6"},
	{"RTL8822BE",	"10ec:b822"},
};

// TODO: use new ANDROID_SOCKET mechanism, once support for multiple
// sockets is in
#define RTL8188EU_DRIVER_MODULE_PATH "/system/lib/modules/8188eu.ko"
#define RTL8723BU_DRIVER_MODULE_PATH "/system/lib/modules/8723bu.ko"
#define RTL8723BS_DRIVER_MODULE_PATH "/system/lib/modules/8723bs.ko"
#define RTL8723BS_VQ0_DRIVER_MODULE_PATH "/system/lib/modules/8723bs-vq0.ko"
#define RTL8723CS_DRIVER_MODULE_PATH "/system/lib/modules/8723cs.ko"
#define RTL8723DS_DRIVER_MODULE_PATH "/system/lib/modules/8723ds.ko"
#define RTL8188FU_DRIVER_MODULE_PATH "/system/lib/modules/8188fu.ko"
#define RTL8822BU_DRIVER_MODULE_PATH "/system/lib/modules/8822bu.ko"
#define RTL8822BS_DRIVER_MODULE_PATH "/system/lib/modules/8822bs.ko"
#define RTL8189ES_DRIVER_MODULE_PATH "/system/lib/modules/8189es.ko"
#define RTL8189FS_DRIVER_MODULE_PATH "/system/lib/modules/8189fs.ko"
#define RTL8192DU_DRIVER_MODULE_PATH "/system/lib/modules/8192du.ko"
#define RTL8812AU_DRIVER_MODULE_PATH "/system/lib/modules/8812au.ko"
#define RTL8822BE_DRIVER_MODULE_PATH "/system/lib/modules/8822be.ko"
#define BCM_DRIVER_MODULE_PATH       "/system/lib/modules/bcmdhd.ko"
#define DRIVER_MODULE_PATH_UNKNOW    ""

#define RTL8822BS_DRIVER_MODULE_NAME "8822bs"
#define RTL8723CS_DRIVER_MODULE_NAME "8723cs"
#define RTL8723DS_DRIVER_MODULE_NAME "8723ds"
#define RTL8188EU_DRIVER_MODULE_NAME "8188eu"
#define RTL8723BU_DRIVER_MODULE_NAME "8723bu"
#define RTL8723BS_DRIVER_MODULE_NAME "8723bs"
#define RTL8723BS_VQ0_DRIVER_MODULE_NAME "8723bs_vq0"
#define RTL8188FU_DRIVER_MODULE_NAME "8188fu"
#define RTL8189ES_DRIVER_MODULE_NAME "8189es"
#define RTL8189FS_DRIVER_MODULE_NAME "8189fs"
#define RTL8192DU_DRIVER_MODULE_NAME "8192du"
#define RTL8812AU_DRIVER_MODULE_NAME "8812au"
#define BCM_DRIVER_MODULE_NAME       "bcmdhd"

#define AP6330_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/rk903.hcd"
#define AP6212_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/bcm43438a0.hcd"
#define AP6212A_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/bcm43438a1.hcd"
#define AP6335_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/bcm4339a0.hcd"
#define AP6255_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/BCM4345C0.hcd"
#define AP6354_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/bcm4354a1.hcd"
#define AP6356_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/BCM4356A2.hcd"
#define AP6398s_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/BCM4359C0.hcd"
#define AP6236_BT_FIRMWARE_MODULE_PATH "/system/etc/firmware/BCM4343B0.hcd"

wifi_ko_file_name module_list[] =
{
	{"RTL8723BU", RTL8723BU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8188EU", RTL8188EU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8192DU", RTL8192DU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8822BU", RTL8822BU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8822BS", RTL8822BS_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8188FU", RTL8188FU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8189ES", RTL8189ES_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8723BS", RTL8723BS_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8723CS", RTL8723CS_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8723DS", RTL8723DS_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, "RTL8723DS"},
	{"RTL8812AU", RTL8812AU_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8189FS", RTL8189FS_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"RTL8822BE", RTL8822BE_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"AP6335",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6335_BT_FIRMWARE_MODULE_PATH},
	{"AP6330",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6330_BT_FIRMWARE_MODULE_PATH},
	{"AP6354",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6354_BT_FIRMWARE_MODULE_PATH},
	{"AP6356S",         BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6356_BT_FIRMWARE_MODULE_PATH},
	{"AP6255",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6255_BT_FIRMWARE_MODULE_PATH},
	{"AP6212",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6212_BT_FIRMWARE_MODULE_PATH},
	{"AP6212A",         BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6212A_BT_FIRMWARE_MODULE_PATH},
	{"AP6356",          BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6356_BT_FIRMWARE_MODULE_PATH},
	{"AP6398S",         BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, AP6398s_BT_FIRMWARE_MODULE_PATH},
	{"APXXX",           BCM_DRIVER_MODULE_PATH, UNKKOWN_DRIVER_MODULE_ARG, ""},
	{"UNKNOW",       DRIVER_MODULE_PATH_UNKNOW, UNKKOWN_DRIVER_MODULE_ARG, ""}

};

static const char RECOGNIZE_WIFI_CHIP[] = "/data/wifi_chip";

int save_wifi_chip_type(char *type)
{
	int ret, found;
	int fd;
	char buf[64];

	ret = access(RECOGNIZE_WIFI_CHIP, R_OK|W_OK);

	if ((ret == 0) || (errno == EACCES)) {
		if ((ret != 0) && (chmod(RECOGNIZE_WIFI_CHIP, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
			printf("Cannot set RW to \"%s\": %s \n", RECOGNIZE_WIFI_CHIP, strerror(errno));
			return -1;
		}
		printf("%s is exit \n", RECOGNIZE_WIFI_CHIP);
		return 0;
	}

	fd = open(RECOGNIZE_WIFI_CHIP, O_CREAT|O_RDWR, 0664);
	if (fd < 0) {
		printf("Cannot create \"%s\": %s", RECOGNIZE_WIFI_CHIP, strerror(errno));
		return -1;
	}

	printf("%s is not exit,save wifi chip \n", RECOGNIZE_WIFI_CHIP);
	strcpy(buf, type);
	printf("recognized wifi chip = %s, save to %s \n", buf, RECOGNIZE_WIFI_CHIP);

	if (write(fd, buf, strlen(buf) + 1) != (strlen(buf) + 1)) {
		printf("Error writing \"%s\": %s \n", RECOGNIZE_WIFI_CHIP, strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);

	if (chmod(RECOGNIZE_WIFI_CHIP, 0664) < 0) {
		printf("Error changing permissions of %s to 0664: %s \n",RECOGNIZE_WIFI_CHIP, strerror(errno));
		unlink(RECOGNIZE_WIFI_CHIP);
		return -1;
	}

	return 1;
}

int get_wifi_device_id(const char *bus_dir, const char *prefix)
{
	int idnum;
	int i = 0;
	int ret = invalid_wifi_device_id;
	DIR *dir;
	struct dirent *next;
	FILE *fp = NULL;
	idnum = sizeof(supported_wifi_devices) / sizeof(supported_wifi_devices[0]);
	dir = opendir(bus_dir);
	if (!dir) {
		printf("open dir failed: %s \n", strerror(errno));
		return invalid_wifi_device_id;
	}

	while ((next = readdir(dir)) != NULL) {
		char line[256];
		char uevent_file[256] = {0};
		sprintf(uevent_file, "%s/%s/uevent", bus_dir, next->d_name);
		printf("uevent path:%s \n", uevent_file);
		fp = fopen(uevent_file, "r");
		if (NULL == fp) {
			continue;
		}

		while (fgets(line, sizeof(line), fp)) {
			char *pos = NULL;
			int product_vid = 0;
			int product_did = 0;
			int producd_bcddev = 0;
			char temp[10] = {0};
			pos = strstr(line, prefix);
			printf("line: %s, prefix: %s.\n", line, prefix);
			if (pos != NULL) {
				if (strncmp(bus_dir, USB_DIR, sizeof(USB_DIR)) == 0)
					sscanf(pos + 8, "%x/%x/%x", &product_vid, &product_did, &producd_bcddev);
				else if (strncmp(bus_dir, SDIO_DIR, sizeof(SDIO_DIR)) == 0)
					sscanf(pos + 8, "%x:%x", &product_vid, &product_did);
				else if (strncmp(bus_dir, PCIE_DIR, sizeof(PCIE_DIR)) == 0)
					sscanf(pos + 7, "%x:%x", &product_vid, &product_did);
				else
					return invalid_wifi_device_id;

				sprintf(temp, "%04x:%04x", product_vid, product_did);
				printf("pid:vid : %s \n", temp);
				for (i = 0; i < idnum; i++) {
					if (0 == strncmp(temp, supported_wifi_devices[i].wifi_vid_pid, 9)) {
						printf("found device pid:vid : %s \n", temp);
						strcpy(recoginze_wifi_chip, supported_wifi_devices[i].wifi_name);
						identify_sucess = 1 ;
						ret = 0;
						fclose(fp);
						goto ready;
					}
				}
			}
		}
		fclose(fp);
	}

	ret = invalid_wifi_device_id;
ready:
	closedir(dir);
	printf("wifi detectd return ret:%d \n", ret);
	return ret;
}

int check_wifi_chip_type_string(char *type)
{
	if (identify_sucess == -1) {
		if (get_wifi_device_id(SDIO_DIR, PREFIX_SDIO) == 0)
			printf("SDIO WIFI identify sucess \n");
		else if (get_wifi_device_id(USB_DIR, PREFIX_USB) == 0)
			printf("USB WIFI identify sucess \n");
		else if (get_wifi_device_id(PCIE_DIR, PREFIX_PCIE) == 0)
			printf("PCIE WIFI identify sucess \n");
		else {
			printf("maybe there is no usb wifi or sdio or pcie wifi, set default wifi module Brocom APXXX \n");
			strcpy(recoginze_wifi_chip, "APXXX");
			identify_sucess = 1 ;
		}
	}

	strcpy(type, recoginze_wifi_chip);
	printf("%s: %s \n", __func__, type);
	return 0;
}

/* 0 - not ready; 1 - ready. */
int check_wireless_ready(void)
{
	char line[1024], *ptr = NULL;
	FILE *fp = NULL;

	fp = fopen("/proc/net/dev", "r");
	if (fp == NULL) {
		printf("Couldn't open /proc/net/dev \n");
		return 0;
	}

	while(fgets(line, 1024, fp)) {
		if ((strstr(line, "wlan0:") != NULL) || (strstr(line, "p2p0:") != NULL)) {
			printf("Wifi driver is ready for now... \n");
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);

	printf("Wifi driver is not ready.\n");
	return 0;
}

static const char BT_TEST_FILE[] = "/userdata/bt_pcba_test";
static int create_bt_test_file_for_brcm(void)
{
	FILE* fp;
	char cmdline[512] = {0};

	fp = fopen(BT_TEST_FILE, "wt+");

	if (fp != 0) {
		fputs("echo 0 > /sys/class/rfkill/rfkill0/state\n", fp);
		fputs("sleep 1\n", fp);
		fputs("echo 1 > /sys/class/rfkill/rfkill0/state\n", fp);
		fputs("sleep 1\n", fp);
		sprintf(cmdline, "brcm_patchram_plus1 --bd_addr_rand --enable_hci --no2bytes --use_baudrate_for_download  --tosleep  200000 --baudrate 1500000 --patchram  %s %s &\n", bt_firmware_patch, bt_tty_dev);
		fputs(cmdline, fp);
		fputs("sleep 1\n", fp);
		fclose(fp);
		system("chmod 777 /userdata/bt_pcba_test");
		system("mount --bind /userdata/bt_pcba_test /usr/bin/bt_pcba_test");
		return 0;
	}
	return -1;
}

static int create_bt_test_file_for_rtl(void)
{
	FILE* fp;
	char cmdline[512] = {0};

	fp = fopen(BT_TEST_FILE, "wt+");

	if (fp != 0) {
		fputs("echo 0 > /sys/class/rfkill/rfkill0/state\n", fp);
		fputs("sleep 1\n", fp);
		fputs("echo 1 > /sys/class/rfkill/rfkill0/state\n", fp);
		fputs("sleep 1\n", fp);

		fputs("insmod /usr/lib/modules/hci_uart.ko\n", fp);
		fputs("sleep 1\n", fp);

		sprintf(cmdline, "rtk_hciattach -n -s 115200 %s rtk_h5 &\n", bt_tty_dev);
		fputs(cmdline, fp);
		fputs("sleep 1\n", fp);

		fclose(fp);
		system("chmod 777 /userdata/bt_pcba_test");
		system("mount --bind /userdata/bt_pcba_test /usr/bin/bt_pcba_test");
		return 0;
	}

	return -1;
}

int wifibt_load_driver(void)
{
	char* wifi_ko_path = NULL ;
	char* wifi_ko_arg =NULL;
	int i = 0;
	int count = 100; /* wait at most 20 seconds for completion */
	char temp[256] = {0};

	if (wifi_dirver_is_loaded == 1) {
		return 0;
	}

	if (check_wireless_ready()) {
		wifi_dirver_is_loaded = 1;
	}

	printf("%s \n", __func__);

	if (wifi_type[0] == 0) {
		check_wifi_chip_type_string(wifi_type);
		save_wifi_chip_type(wifi_type);
	}

	for (i = 0; i < (int)(sizeof(module_list) / sizeof(module_list[0])); i++) {
		if (!strcmp(wifi_type , module_list[i].wifi_name)) {
			wifi_ko_path = module_list[i].wifi_module_path;
			wifi_ko_arg = module_list[i].wifi_module_arg;
			bt_firmware_patch = module_list[i].bt_firmware_path;
			printf("%s matched ko file path  %s \n", __func__, wifi_ko_path);
			break;
		}
	}

	if (wifi_ko_path == NULL) {
		printf("%s falied to find wifi driver for type=%s \n", __func__, wifi_type);
		return -1;
	}

	sprintf(temp, "insmod %s %s", wifi_ko_path, wifi_ko_arg);
	printf("%s %s\n", __func__, temp);
	if (system(temp)) {
		printf("%s insmod %s failed \n", __func__, wifi_ko_path);
		return -1;
	}

	while (count-- > 0) {
		if (check_wireless_ready())
			break;
		usleep(200000);
	}

	//bt init
	if (strstr(bt_firmware_patch , "system")) {
		create_bt_test_file_for_brcm();
		memset(temp, 0, 256);
		system("echo 0 > /sys/class/rfkill/rfkill0/state");
		usleep(2);
		system("echo 1 > /sys/class/rfkill/rfkill0/state");
		usleep(2);

		sprintf(temp, "brcm_patchram_plus1 --bd_addr_rand --enable_hci --no2bytes --use_baudrate_for_download  --tosleep  200000 --baudrate 1500000 --patchram  %s %s &", bt_firmware_patch, bt_tty_dev);
		printf("%s %s\n", __func__, temp);
		if (system(temp)) {
			printf("bt_init: %s failed \n", temp);
			return -1;
		}
	} else if (strstr(bt_firmware_patch , "RTL")) {
		create_bt_test_file_for_rtl();
		system("echo 0 > /sys/class/rfkill/rfkill0/state");
		usleep(5000);
		system("echo 1 > /sys/class/rfkill/rfkill0/state\n");
		usleep(5000);

		system("insmod /usr/lib/modules/hci_uart.ko\n");
		usleep(5000);

		memset(temp, 0, 256);
		sprintf(temp, "rtk_hciattach -n -s 115200 %s rtk_h5 &", bt_tty_dev);
		system(temp);
		usleep(5000);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	printf("Rockchip Linux WifiBt init \n");

	strncpy(bt_tty_dev, argv[1], 10);
	printf("BT TTY: %s \n", bt_tty_dev);

	wifibt_load_driver();

	return 0;
}
