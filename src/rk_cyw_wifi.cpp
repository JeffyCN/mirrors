#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "DeviceIo/Rk_wifi.h"
#include "slog.h"
#include "utility.h"

static bool save_last_ap = false;
static int connecting_id = -1;

static RK_WIFI_RUNNING_State_e gstate = RK_WIFI_State_OFF;

static char retry_connect_cmd[128];

typedef struct {
	char ssid[SSID_BUF_LEN];
	char bssid[BSSID_BUF_LEN];
} RK_WIFI_CONNECT_INFO;

static RK_WIFI_CONNECT_INFO connect_info =
{
	"",
	"",
};

static int is_non_psk(const char* str)
{
	RK_WIFI_encode_gbk_t *nonpsk = m_nonpsk_head;
	while (nonpsk) {
		if (strcmp(nonpsk->utf8, str) == 0) {
			return 1;
		}
		nonpsk = nonpsk->next;
	}

	return 0;
}

static RK_wifi_state_callback m_cb;
static int priority = 0;
static volatile bool wifi_wrong_key = false;
static volatile bool wifi_cancel = false;
static volatile bool wifi_connect_lock = false;
static volatile bool wifi_is_exist = false;

static void format_wifiinfo(int flag, char *info);
static int get_pid(const char Name[]);
static void RK_wifi_start_monitor(void *arg);

static char* wifi_state[] = {
	"RK_WIFI_State_IDLE",
	"RK_WIFI_State_CONNECTING",
	"RK_WIFI_State_CONNECTFAILED",
	"RK_WIFI_State_CONNECTFAILED_WRONG_KEY",
	"RK_WIFI_State_CONNECTED",
	"RK_WIFI_State_DISCONNECTED",
	"RK_WIFI_State_OPEN",
	"RK_WIFI_State_OFF",
	"RK_WIFI_State_SCAN_RESULTS",
	"RK_WIFI_State_DHCP_OK",
};

static char* exec1(const char* cmd)
{
	if (NULL == cmd || 0 == strlen(cmd)) {
		pr_err("exec1 cmd is NULL!\n");
		return NULL;
	}

	pr_info("[RKWIFI] exec1: %s\n", cmd);

	FILE* fp = NULL;
	char buf[128];
	char* ret;
	static int SIZE_UNITE = 512;
	size_t size = SIZE_UNITE;

	fp = popen((const char *) cmd, "r");
	if (NULL == fp) {
		pr_err("exec1 popen fp NULL!\n");
		return NULL;
	}

	memset(buf, 0, sizeof(buf));
	ret = (char*) malloc(sizeof(char) * size);
	memset(ret, 0, sizeof(char) * size);
	while (NULL != fgets(buf, sizeof(buf)-1, fp)) {
		if (size <= (strlen(ret) + strlen(buf))) {
			size += SIZE_UNITE;
			ret = (char*) realloc(ret, sizeof(char) * size);
		}
		strcat(ret, buf);
	}

	pclose(fp);
	ret = (char*) realloc(ret, sizeof(char) * (strlen(ret) + 1));

	return ret;
}

static int exec(const char* cmd, const char* ret)
{
	char* tmp;
	tmp = exec1(cmd);

	if (NULL == tmp) {
		pr_err("exec tmp is NULL!\n");
		return -1;
	}

	char convers[strlen(tmp) + 1];

	memset(ret, 0, strlen(ret));
	strncpy(ret, tmp, strlen(tmp) + 1);
	free(tmp);

	return 0;
}

static int RK_wifi_search_with_ssid(const char *ssid)
{
	RK_WIFI_SAVED_INFO wsi;
	int id, len;

	RK_wifi_getSavedInfo(&wsi);
	for (int i = 0; i < wsi.count; i++) {
		if ((strlen(wsi.save_info[i].ssid) > 64) || (strlen(ssid) > 64))
			pr_err("RK_wifi_search_with_ssid ssid error!!!\n");
		pr_err("RK_wifi_search_with_ssid save_info[%d].ssid: %s, ssid: %s \n",
				i, wsi.save_info[i].ssid, ssid);
		if (strlen(wsi.save_info[i].ssid) > strlen(ssid))
			len = strlen(wsi.save_info[i].ssid);
		else
			len = strlen(ssid);
		if (strncmp(wsi.save_info[i].ssid, ssid, len) == 0) {
			return wsi.save_info[i].id;
		}
	}

	return -1;
}

int RK_wifi_running_getConnectionInfo(RK_WIFI_INFO_Connection_s* pInfo)
{
	FILE *fp = NULL;
	char line[512];
	char *value;

	if (pInfo == NULL)
		return -1;

	if (remove("/tmp/status.tmp"))
		pr_err("remove /tmp/status.tmp failed!\n");
	exec_command_system("wpa_cli -iwlan0 status > /tmp/status.tmp");

	// check wpa is running first
	memset(line, 0, sizeof(line));
	exec("cat /tmp/status.tmp", line);
	pr_info("status.tmp: %s\n", line);

	memset(line, 0, sizeof(line));
	exec("wpa_cli -iwlan0 status", line);
	pr_info("wpa_cli status: %s\n", line);

	fp = fopen("/tmp/status.tmp", "r");
	if (!fp) {
		pr_err("fopen /tmp/status.tmp failed!\n");
		return -1;
	}

	return 0;
}

static int is_wifi_enable()
{
	int ret = 0;

	return ret;
}

int RK_wifi_scan(void)
{
	int ret;
	char str[32];

	memset(str, 0, sizeof(str));
	ret = exec("wpa_cli -iwlan0 scan", str);

	if (0 != ret)
		return -1;

	if (0 != strncmp(str, "OK", 2) &&  0 != strncmp(str, "ok", 2)) {
		pr_info("scan error: %s\n", str);
		return -2;
	}

	return 0;
}

char* RK_wifi_scan_r_sec(const unsigned int cols)
{
	char line[256], utf[256];
	char item[384];
	char col[128];
	char *scan_r, *p_strtok;
	size_t size = 0, index = 0;
	static size_t UNIT_SIZE = 512;
	FILE *fp = NULL;
	int is_utf8;
	int is_nonpsk;

	if (!(cols & 0x1F)) {
		scan_r = (char*) malloc(3 * sizeof(char));
		memset(scan_r, 0, 3);
		strcpy(scan_r, "[]");
		pr_err("_scan_r: %s\n");
		return scan_r;
	}

	remove("/tmp/scan_r.tmp");
	exec_command_system("wpa_cli -iwlan0 scan_r > /tmp/scan_r.tmp");

	fp = fopen("/tmp/scan_r.tmp", "r");
	if (!fp) {
		pr_err("open /tmp/scan_r.tmp fail!!!\n");
		return NULL;
	}

	memset(line, 0, sizeof(line));
	fgets(line, sizeof(line), fp);

	size += UNIT_SIZE;
	scan_r = (char*) malloc(size * sizeof(char));
	memset(scan_r, 0, size);
	strcat(scan_r, "[");

	m_gbk_head = encode_gbk_reset(m_gbk_head);
	m_nonpsk_head = encode_gbk_reset(m_nonpsk_head);
	while (fgets(line, sizeof(line) - 1, fp)) {
		index = 0;
		is_nonpsk = 0;
		p_strtok = strtok(line, "\t");
		memset(item, 0, sizeof(item));
		strcat(item, "{");
		while (p_strtok) {
			if (p_strtok[strlen(p_strtok) - 1] == '\n')
				p_strtok[strlen(p_strtok) - 1] = '\0';
			if ((cols & (1 << index)) || 3 == index) {
				memset(col, 0, sizeof(col));
				if (0 == index) {
					snprintf(col, sizeof(col), "\"bssid\":\"%s\",", p_strtok);
				} else if (1 == index) {
					snprintf(col, sizeof(col), "\"frequency\":%d,", atoi(p_strtok));
				} else if (2 == index) {
					snprintf(col, sizeof(col), "\"rssi\":%d,", atoi(p_strtok));
				} else if (3 == index) {
					if (cols & (1 << index)) {
						snprintf(col, sizeof(col), "\"flags\":\"%s\",", p_strtok);
					}
					if (!strstr(p_strtok, "WPA") && !strstr(p_strtok, "WEP")) {
						is_nonpsk = 1;
					}
				} else if (4 == index) {
					char utf8[strlen(p_strtok) + 1];
					memset(utf8, 0, sizeof(utf8));

					if (strlen(p_strtok) > 0) {
						char dst[strlen(p_strtok) + 1];
						memset(dst, 0, sizeof(dst));
						spec_char_convers(p_strtok, dst);

						// Strings that will send should retain escape characters
						// Strings whether GBK or UTF8 that need save local should remove escape characters
						// The ssid can't contains escape character while do connect
						is_utf8 = RK_encode_is_utf8(dst, strlen(dst));
						char utf8_noescape[sizeof(utf8)];
						char dst_noescape[sizeof(utf8)];
						memset(utf8_noescape, 0, sizeof(utf8_noescape));
						memset(dst_noescape, 0, sizeof(dst_noescape));
						if (!is_utf8) {
							RK_encode_gbk_to_utf8(dst, strlen(dst), utf8);
							remove_escape_character(dst, dst_noescape);
							remove_escape_character(utf8, utf8_noescape);
							m_gbk_head = encode_gbk_insert(m_gbk_head, dst_noescape, utf8_noescape);

							// if convert gbk to utf8 failed, ignore it
							if (!RK_encode_is_utf8(utf8, strlen(utf8))) {
								continue;
							}
						} else {
							strncpy(utf8, dst, strlen(dst));
							remove_escape_character(dst, dst_noescape);
							remove_escape_character(utf8, utf8_noescape);
						}

						// Decide whether encrypted or not
						if (is_nonpsk) {
							m_nonpsk_head = encode_gbk_insert(m_nonpsk_head, dst_noescape, utf8_noescape);
						}
					}
					snprintf(col, sizeof(col), "\"ssid\":\"%s\",", utf8);
				}
				strcat(item, col);
			}
			p_strtok = strtok(NULL, "\t");
			index++;
		}
		if (item[strlen(item) - 1] == ',') {
			item[strlen(item) - 1] = '\0';
		}
		strcat(item, "},");

		if (size <= (strlen(scan_r) + strlen(item)) + 3) {
			size += UNIT_SIZE;
			scan_r = (char*) realloc(scan_r, sizeof(char) * size);
		}
		strcat(scan_r, item);
	}
	if (scan_r[strlen(scan_r) - 1] == ',') {
		scan_r[strlen(scan_r) - 1] = '\0';
	}
	strcat(scan_r, "]");
	fclose(fp);
	pr_err("__scan_r: %s\n");
	return scan_r;
}

static int set_hide_network(const int id)
{
	int ret;
	char str[8];
	char cmd[128];

	memset(str, 0, sizeof(str));
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d scan_ssid %d", id, 1);
	ret = exec(cmd, str);

	if (0 != ret || 0 == strlen(str) || 0 != strncmp(str, "OK", 2))
		return -1;

	return 0;
}


#define WIFI_CONNECT_RETRY 50
static bool check_wifi_isconnected(void)
{

	return isWifiConnected;
}

static void check_ping_test()
{
	char line[1024];
	int ping_retry = 10;
	int dns_retry;

	while (ping_retry--) {
		dns_retry = 5;
		memset(line, 0, sizeof(line));
		pr_info("check dns\n");
		while (dns_retry--) {
			exec("cat /etc/resolv.conf", line);
			if (strstr(line, "nameserver"))
				break;
			sleep(1);
		}
		pr_info("dns ok: %s\n", line);
		pr_info("to ping\n");
		sync();
		sleep(1);
		memset(line, 0, sizeof(line));
		exec("ping www.baidu.com -c 1", line);
		//RK_shell_exec("ping 8.8.8.8 -c 1", line, sizeof(line));
		//usleep(100000);
		pr_info("line: %s\n", line);
		//if (strstr(line, "PING www.baidu.com") && strstr(line, "bytes from")) {
		//if (strstr(line, "PING 8.8.8.8") && strstr(line, "bytes from")) {
		if (strstr(line, "bytes from")) {
			pr_info("ping ok\n");
			break;
		}
		usleep(200000);
	}
}

static void* wifi_connect_state_check(void *arg)
{
	return NULL;
}

int RK_wifi_connect(const char* ssid, const char* psk)
{
	int ret = 0;

	return ret;
}

int RK_wifi_forget_with_bssid(const char *bssid)
{

	return 0;
}

int RK_wifi_cancel(void)
{

}

int RK_wifi_connect_with_ssid(const char *ssid)
{
	return -1;
}

int RK_wifi_disconnect_network(void)
{
	exec_command_system("wpa_cli -iwlan0 disconnect");
	return 0;
}

int RK_wifi_reset(void)
{

}

int RK_wifi_recovery(void)
{

}

int RK_wifi_get_mac(char *wifi_mac)
{
	int sock_mac;
	struct ifreq ifr_mac;
	char mac_addr[18] = {0};

	if(!wifi_mac) {
		pr_err("%s: wifi_mac is null\n", __func__);
		return -1;
	}

	sock_mac = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_mac == -1) {
		pr_info("create mac socket failed.");
		return -1;
	}

	memset(&ifr_mac, 0, sizeof(ifr_mac));
	strncpy(ifr_mac.ifr_name, "wlan0", sizeof(ifr_mac.ifr_name) - 1);

	if ((ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0) {
		pr_info("Mac socket ioctl failed.");
		return -1;
	}

	sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);

	close(sock_mac);
	strncpy(wifi_mac, mac_addr, 18);
	pr_info("the wifi mac : %s\r\n", wifi_mac);

	return 0;
}

int RK_wifi_has_config()
{

}

int RK_wifi_ping(char *address)
{

}

#define EVENT_BUF_SIZE 1024
#define PROPERTY_VALUE_MAX 32
#define PROPERTY_KEY_MAX 32
#include <poll.h>
#include <wpa_ctrl.h>
static void wifi_close_sockets();
static const char WPA_EVENT_IGNORE[]    = "CTRL-EVENT-IGNORE ";
static const char IFNAME[]              = "IFNAME=";
static const char IFACE_DIR[]           = "/var/run/wpa_supplicant";
#define WIFI_CHIP_TYPE_PATH				"/sys/class/rkwifi/chip"
#define WIFI_DRIVER_INF         		"/sys/class/rkwifi/driver"
#define IFNAMELEN                       (sizeof(IFNAME) - 1)
static struct wpa_ctrl *ctrl_conn;
static struct wpa_ctrl *monitor_conn;
#define DBG_NETWORK 1

static int exit_sockets[2];
static char primary_iface[PROPERTY_VALUE_MAX] = "wlan0";

#define HOSTAPD "hostapd"
#define WPA_SUPPLICANT "wpa_supplicant"
#define DNSMASQ "dnsmasq"
#define UDHCPC "udhcpc"

static int get_pid(const char Name[]) {
    int len;
    char name[32] = {0};
    len = strlen(Name);
    strncpy(name,Name,len);
    name[31] ='\0';
    char cmdresult[256] = {0};
    char cmd[64] = {0};
    FILE *pFile = NULL;
    int  pid = 0;

    sprintf(cmd, "pidof %s", name);
    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
        pclose(pFile);
    }
    return pid;
}

static void wifi_close_sockets() {
	if (ctrl_conn != NULL) {
		wpa_ctrl_close(ctrl_conn);
		ctrl_conn = NULL;
	}

	if (monitor_conn != NULL) {
		wpa_ctrl_close(monitor_conn);
		monitor_conn = NULL;
	}

	if (exit_sockets[0] >= 0) {
		close(exit_sockets[0]);
		exit_sockets[0] = -1;
	}

	if (exit_sockets[1] >= 0) {
		close(exit_sockets[1]);
		exit_sockets[1] = -1;
	}
}

static int str_starts_with(char * str, char * search_str)
{
	if ((str == NULL) || (search_str == NULL))
		return 0;
	return (strstr(str, search_str) == str);
}

static void get_wifi_info_by_event(char *event, RK_WIFI_RUNNING_State_e state, RK_WIFI_INFO_Connection_s *info)
{
	int len = 0;
	char buf[10];
	char *start_tag = NULL, *end_tag = NULL, *id_tag = NULL, *reason_tag = NULL;

	if(event == NULL)
		return;

	memset(info, 0, sizeof(RK_WIFI_INFO_Connection_s));
	RK_wifi_running_getConnectionInfo(info);

	switch(state) {
	case RK_WIFI_State_DISCONNECTED:
		start_tag = strstr(event, "bssid=");
		if(start_tag)
			strncpy(info->bssid, start_tag + strlen("bssid="), 17);

		//strncpy(info->ssid, connect_info.ssid, SSID_BUF_LEN);
		reason_tag =  strstr(event, "reason=");
		if(reason_tag) {
			memset(buf, 0, sizeof(buf));
			strncpy(buf, reason_tag + strlen("reason="), 2);
			info->reason = atoi(buf);
		}

		break;

	case RK_WIFI_State_CONNECTFAILED_WRONG_KEY:
		start_tag = strstr(event, "ssid=\"");
		if(start_tag) {
			end_tag = strstr(event, "\" auth_failures");
			if(!end_tag) {
				pr_err("%s: don't find end tag\n", __func__);
				break;
			}
			len = strlen(start_tag) - strlen(end_tag) - strlen("ssid=\"");
			char value[128] = {0};
			char sname[128];
			char sname1[128];
			char utf8[128];
			memset(sname, 0, sizeof(sname));
			memset(sname1, 0, sizeof(sname));
			memset(utf8, 0, sizeof(utf8));
			strncpy(value, start_tag + strlen("ssid=\""), len);
			spec_char_convers(value, sname);
			remove_escape_character(sname, sname1);
			get_encode_gbk_utf8(m_gbk_head, sname1, utf8);
			pr_info("convers str: %s, sname: %s, ori: %s\n", value, sname1, utf8);
			strncpy(info->ssid,  utf8, strlen(utf8));
		}

		id_tag =  strstr(event, "id=");
		if(id_tag) {
			len = strlen(id_tag) - strlen("id=") - strlen(start_tag) - 1;
			memset(buf, 0, sizeof(buf));
			strncpy(buf, id_tag + strlen("id="), len);
			info->id = atoi(buf);
		}

		strncpy(info->bssid, connect_info.bssid, BSSID_BUF_LEN);
		break;
	}
}

static void get_valid_connect_info(RK_WIFI_INFO_Connection_s *info)
{
	int count = 10;

	while(count--) {
		RK_wifi_running_getConnectionInfo(info);
		if(!info->id && !strlen(info->ssid)
				&& !strlen(info->bssid) && !info->freq
				&& !strlen(info->mode) && !strlen(info->ip_address)
				&& !strlen(info->mac_address) && !strlen(info->wpa_state)) {
			pr_info("wait to get valid connect info\n");
			usleep(100000);
		} else {
			break;
		}
	}
}

static int dispatch_event(char* event)
{
	RK_WIFI_INFO_Connection_s info;

	if (strstr(event, "CTRL-EVENT-BSS") || strstr(event, "CTRL-EVENT-TERMINATING"))
		return 0;

	pr_info("%s: %s\n", __func__, event);

	if (str_starts_with(event, (char *)WPA_EVENT_DISCONNECTED)) {
		pr_info("%s: wifi is disconnect\n", __FUNCTION__);
		exec_command_system("ip addr flush dev wlan0");
		get_wifi_info_by_event(event, RK_WIFI_State_DISCONNECTED, &info);
		wifi_state_send(RK_WIFI_State_DISCONNECTED, &info);
		exec_command_system("wpa_cli -i wlan0 reconnect");
	} else if (str_starts_with(event, (char *)WPA_EVENT_CONNECTED)) {
		pr_info("%s: wifi is connected\n", __func__);
		get_valid_connect_info(&info);
		wifi_state_send(RK_WIFI_State_CONNECTED, &info);
	} else if (str_starts_with(event, (char *)WPA_EVENT_SCAN_RESULTS)) {
		pr_info("%s: wifi event results\n", __func__);
		exec_command_system("echo 1 > /tmp/scan_r");
		wifi_state_send(RK_WIFI_State_SCAN_RESULTS, NULL);
	} else if (strstr(event, "reason=WRONG_KEY")) {
		wifi_wrong_key = true;
		pr_info("%s: wifi reason=WRONG_KEY \n", __func__);
		get_wifi_info_by_event(event, RK_WIFI_State_CONNECTFAILED_WRONG_KEY, &info);
		wifi_state_send(RK_WIFI_State_CONNECTFAILED_WRONG_KEY, &info);
	} else if (str_starts_with(event, (char *)WPA_EVENT_TERMINATING)) {
		pr_info("%s: wifi is WPA_EVENT_TERMINATING!\n", __func__);
		wifi_close_sockets();
		return -1;
	}

	return 0;
}

static int check_wpa_supplicant_state() {
	int count = 5;
	int wpa_supplicant_pid = 0;
	wpa_supplicant_pid = get_pid(WPA_SUPPLICANT);
	//pr_info("%s: wpa_supplicant_pid = %d\n",__FUNCTION__,wpa_supplicant_pid);
	if(wpa_supplicant_pid > 0) {
		return 1;
	}
	return 0;
}

static int wifi_ctrl_recv(char *reply, size_t *reply_len)
{
	int res;
	int ctrlfd = wpa_ctrl_get_fd(monitor_conn);
	struct pollfd rfds[2];

	memset(rfds, 0, 2 * sizeof(struct pollfd));
	rfds[0].fd = ctrlfd;
	rfds[0].events |= POLLIN;
	rfds[1].fd = exit_sockets[1];
	rfds[1].events |= POLLIN;
	do {
		res = TEMP_FAILURE_RETRY(poll(rfds, 2, 30000));
		if (res < 0) {
			pr_info("Error poll = %d\n", res);
			return res;
		} else if (res == 0) {
			/* timed out, check if supplicant is activeor not .. */
			res = check_wpa_supplicant_state();
			if (res < 0)
				return -2;
		}
	} while (res == 0);

	if (rfds[0].revents & POLLIN) {
		return wpa_ctrl_recv(monitor_conn, reply, reply_len);
	}
	return -2;
}

static int wifi_wait_on_socket(char *buf, size_t buflen)
{
	size_t nread = buflen - 1;
	int result;
	char *match, *match2;

	if (monitor_conn == NULL) {
		return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
			primary_iface, WPA_EVENT_TERMINATING);
	}

	result = wifi_ctrl_recv(buf, &nread);

	/* Terminate reception on exit socket */
	if (result == -2) {
		return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
			primary_iface, WPA_EVENT_TERMINATING);
	}

	if (result < 0) {
		//pr_info("wifi_ctrl_recv failed: %s\n", strerror(errno));
		//return snprintf(buf, buflen, "IFNAME=%s %s - recv error",
		//	primary_iface, WPA_EVENT_TERMINATING);
	}

	buf[nread] = '\0';

	/* Check for EOF on the socket */
	if (result == 0 && nread == 0) {
		/* Fabricate an event to pass up */
		pr_info("Received EOF on supplicant socket\n");
		return snprintf(buf, buflen, "IFNAME=%s %s - signal 0 received",
			primary_iface, WPA_EVENT_TERMINATING);
	}

	if (strncmp(buf, IFNAME, IFNAMELEN) == 0) {
		match = strchr(buf, ' ');
		if (match != NULL) {
			if (match[1] == '<') {
				match2 = strchr(match + 2, '>');
					if (match2 != NULL) {
						nread -= (match2 - match);
						memmove(match + 1, match2 + 1, nread - (match - buf) + 1);
					}
			}
		} else {
			return snprintf(buf, buflen, "%s", WPA_EVENT_IGNORE);
		}
	} else if (buf[0] == '<') {
		match = strchr(buf, '>');
		if (match != NULL) {
			nread -= (match + 1 - buf);
			memmove(buf, match + 1, nread + 1);
			if (0)
				pr_info("supplicant generated event without interface - %s\n", buf);
		}
	} else {
		if (0)
			pr_info("supplicant generated event without interface and without message level - %s\n", buf);
	}

	return nread;
}

static int wifi_connect_on_socket_path(const char *path)
{
	char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

	if(!check_wpa_supplicant_state()) {
		pr_info("%s: wpa_supplicant is not ready\n",__FUNCTION__);
		return -1;
	}

	ctrl_conn = wpa_ctrl_open(path);
	if (ctrl_conn == NULL) {
		pr_info("Unable to open connection to supplicant on \"%s\": %s\n",
		path, strerror(errno));
		return -1;
	}
	monitor_conn = wpa_ctrl_open(path);
	if (monitor_conn == NULL) {
		wpa_ctrl_close(ctrl_conn);
		ctrl_conn = NULL;
		return -1;
	}
	if (wpa_ctrl_attach(monitor_conn) != 0) {
		wpa_ctrl_close(monitor_conn);
		wpa_ctrl_close(ctrl_conn);
		ctrl_conn = monitor_conn = NULL;
		return -1;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1) {
		wpa_ctrl_close(monitor_conn);
		wpa_ctrl_close(ctrl_conn);
		ctrl_conn = monitor_conn = NULL;
		return -1;
	}
	return 0;
}

/* Establishes the control and monitor socket connections on the interface */
static int wifi_connect_to_supplicant()
{
	static char path[1024];
	int count = 10;

	pr_info("%s \n", __FUNCTION__);
	while(count-- > 0) {
		if (access(IFACE_DIR, F_OK) == 0)
			break;
		sleep(1);
	}

	snprintf(path, sizeof(path), "%s/%s", IFACE_DIR, primary_iface);

	return wifi_connect_on_socket_path(path);
}

static void RK_wifi_start_monitor(void *arg)
{
	char eventStr[EVENT_BUF_SIZE];
	int ret;

	prctl(PR_SET_NAME,"RK_wifi_start_monitor");

	if ((ret = wifi_connect_to_supplicant()) != 0) {
		pr_info("%s, connect to supplicant fail.\n", __FUNCTION__);
		return;
	}

	for (;;) {
		memset(eventStr, 0, EVENT_BUF_SIZE);
		if (!wifi_wait_on_socket(eventStr, EVENT_BUF_SIZE))
			continue;

		if (dispatch_event(eventStr)) {
			pr_info("disconnecting from the supplicant, no more events\n");
			break;
		}
	}
}

static void execute(const char cmdline[], char recv_buff[], int len)
{
	//pr_info("[AIRKISS] execute: %s\n", cmdline);

	FILE *stream = NULL;
	char *tmp_buff = recv_buff;

	if ((stream = popen(cmdline, "r")) == NULL) {
		pr_info("[AIRKISS] execute: %s failed\n", cmdline);
		return;
	}

	if (recv_buff == NULL) {
		pclose(stream);
		return;
	}

	memset(recv_buff, 0, len);
	while (fgets(tmp_buff, len, stream)) {
		tmp_buff += strlen(tmp_buff);
		len -= strlen(tmp_buff);
		if (len <= 1) {
			pr_info("[AIRKISS] overflow, please enlarge recv_buff\n");
			break;
		}
	}
	pclose(stream);
}

static int start_airkiss()
{
	int ret = -1, cnt = 3;

	while(cnt--) {
		exec_command_system("rm /tmp/airkiss.conf");
		exec_command_system("killall rk_airkiss");
		usleep(500000);
		exec_command_system("rk_airkiss &");
		usleep(500000);

		if(get_pid("rk_airkiss") > 0) {
			pr_info("[AIRKISS] start rk_airkiss success, cnt: %d\n", cnt);
			ret = 0;
			break;
		}
	}

	return ret;
}

static bool check_airkiss_conf()
{
	int wait_cnt = 60;
	bool file_exist = false;

	while (wait_cnt--) {
		pr_info("[AIRKISS] check airkiss conf, wait_cnt: %d\n", wait_cnt);
		if (access("/tmp/airkiss.conf", F_OK) == 0) {
			pr_info("[AIRKISS] geted airkiss data\n");
			file_exist = true;
			break;
		}
		sleep(1);
	}

	return file_exist;
}

static void get_airkiss_ssid_password(char *ssid, char *password)
{
	char *cp;
	char ret_buf[1024];

	/*
	 * cat /tmp/airkiss.conf
	 * rk_ssid=<unknown ssid>
	 * rk_password=cccccc
	 */
	execute("cat /tmp/airkiss.conf | grep rk_ssid", ret_buf, 1024);
	pr_info("[AIRKISS] ssid ret_buf: %s\n", ret_buf);
	if (cp = strstr(ret_buf, "=")) {
		strcpy(ssid, cp + 1);
		ssid[strlen(ssid) - 1] = '\0';
	}

	execute("cat /tmp/airkiss.conf | grep rk_password", ret_buf, 1024);
	pr_info("[AIRKISS] password ret_buf: %s\n", ret_buf);
	if (cp = strstr(ret_buf, "=")) {
		strcpy(password, cp + 1);
		password[strlen(password) - 1] = '\0';
	}

	pr_info("[AIRKISS] SSID: %s[%d], PSK: %s[%d]\n", ssid, strlen(ssid), password, strlen(password));
}

static int RK_wifi_rtl_airkiss_start(char *ssid, char *password)
{
	int reset_cnt = 1;
	bool file_exist = false;

	pr_info("=== %s ===\n", __func__);

retry:
	if(start_airkiss() < 0) {
		pr_info("[AIRKISS] start rk_airkiss failed\n");
		return -1;
	}

	file_exist = check_airkiss_conf();

	if ((!file_exist) && (--reset_cnt)) {
		pr_info("[AIRKISS] geted airkiss data failed, reset_cnt: %d\n", reset_cnt);
		goto retry;
	}

	if (!file_exist) {
		pr_info("[AIRKISS] don't get airkiss data\n");
		exec_command_system("killall rk_airkiss");
		return -1;
	}

	get_airkiss_ssid_password(ssid, password);
	return 0;
}

static void RK_wifi_rtl_airkiss_stop()
{
	exec_command_system("rm /tmp/airkiss.conf");
	exec_command_system("killall rk_airkiss");
}

int RK_wifi_airkiss_start(char *ssid, char *password)
{
#ifdef REALTEK
	return RK_wifi_rtl_airkiss_start(ssid, password);
#else
	pr_info("Currently only supports realtek airkiss config\n");
	return -1;
#endif
}

void RK_wifi_airkiss_stop()
{
#ifdef REALTEK
	return RK_wifi_rtl_airkiss_stop();
#else
	pr_info("Currently only supports realtek airkiss config\n");
#endif
}
